#include "core/RadixTrie.hpp"
#include "core/TopKCache.hpp"
#include "ranking/Ranker.hpp"
#include "ranking/UserProfile.hpp"
#include "fuzzy/FuzzyMatcher.hpp"
#include "expansion/AbbreviationEngine.hpp"
#include "io/DictionaryLoader.hpp"
#include "io/Persistence.hpp"
#include "ui/TermiosController.hpp"
#include "ui/Renderer.hpp"
#include "ui/KeyHandler.hpp"

#include <iostream>
#include <chrono>
#include <algorithm>
#include <filesystem>
#include <unordered_map>

static constexpr int FUZZY_TRIGGER  = 1;   
static constexpr int MAX_RESULTS    = 5;
static constexpr int FUZZY_MAX_DIST = 2;
static constexpr int NEW_WORD_FREQ  = 15;  

static std::vector<std::string> to_display(
    const std::vector<int>&            ids,
    const std::vector<core::TermInfo>& dict,
    const ranking::UserProfile&        profile)
{
    std::vector<std::string> out;
    out.reserve(ids.size());
    for (int id : ids)
        if (!profile.is_hidden(id) && !dict[id].term.empty())
            out.push_back(dict[id].term);
    return out;
}

static void recompute_suggestions(
    const std::string&query,
    core::RadixTrie&trie,
    const expansion::AbbreviationEngine&abbrev,
    fuzzy::FuzzyMatcher&fuzzy_matcher,
    ranking::Ranker&ranker,
    const ranking::UserProfile&profile,
    const std::vector<core::TermInfo>&dict,
    const std::unordered_map<std::string, int>& term_index,
    ui::RenderState&state)
{
    auto t0 = std::chrono::high_resolution_clock::now();
    state.search_mode = ui::SearchMode::EXACT;
    state.cache_hit = false;

    // 1. Abbreviation check - O(1)
    if (abbrev.has(query)) {
        state.search_mode = ui::SearchMode::ABBREVIATION;
        state.suggestions = { abbrev.expand(query) };
        state.elapsed_ms = 0.0;
        return;
    }

    // 2. Prefix search - O(m) + O(K) 
    bool cache_hit = false;
    std::vector<int> ids = trie.search_prefix(query, cache_hit);
    state.cache_hit = cache_hit;

    ids.erase(std::remove_if(ids.begin(), ids.end(),
        [&](int id){ return profile.is_hidden(id) || dict[id].term.empty(); }),
        ids.end());

    // Rank with personalization - O(K log K), K=5 ≈ O(1)
    ranker.rank_inplace(ids);
    if (static_cast<int>(ids.size()) > MAX_RESULTS)
        ids.resize(MAX_RESULTS);

    auto exact_it = term_index.find(query);
    if (exact_it != term_index.end()) {
        int exact_id = exact_it->second;
        auto pos = std::find(ids.begin(), ids.end(), exact_id);
        if (pos != ids.end() && pos != ids.begin())
           std::rotate(ids.begin(), pos, pos + 1);
    }

    // 3. Lazy fuzzy via BK-tree - O(log N) average
    if (static_cast<int>(ids.size()) < FUZZY_TRIGGER) {
        state.search_mode = ui::SearchMode::FUZZY;
        auto fuzz = fuzzy_matcher.match(query, MAX_RESULTS);
        for (const auto& fr : fuzz) {
            if (profile.is_hidden(fr.term_id)) continue;
            if (dict[fr.term_id].term.empty()) continue;
            bool dup = std::find(ids.begin(), ids.end(), fr.term_id) != ids.end();
            if (!dup) ids.push_back(fr.term_id);
            if (static_cast<int>(ids.size()) >= MAX_RESULTS) break;
        }
    }

    if (!cache_hit)
        trie.rebuild_cache(query, ranker.scorer());

    state.suggestions = to_display(ids, dict, profile);

    if (state.suggestions.empty() && !query.empty())
        state.status_msg = "No results for \"" + query + "\". Press M → A to add it.";

    auto t1 = std::chrono::high_resolution_clock::now();
    state.elapsed_ms =
        std::chrono::duration<double, std::milli>(t1 - t0).count();
}

int main() {
    const std::string dict_path = "data/dictionary.txt";
    const std::string abbrev_path = "data/abbreviations.txt";

    std::filesystem::create_directories("data");

    std::string user_id;
    std::cout << "Enter username: ";
    std::getline(std::cin, user_id);
    if (user_id.empty()) user_id = "default";

    std::vector<core::TermInfo> dict;
    try {
        dict = io::DictionaryLoader::load(dict_path);
    } catch (const std::exception& e) {
        std::cerr << "Warning: " << e.what() << " — starting empty.\n";
    }

    // ── Reverse lookup: term string → term_id — O(1) selection lookup 
    std::unordered_map<std::string, int> term_index;
    term_index.reserve(dict.size() * 2);
    for (int i = 0; i < static_cast<int>(dict.size()); ++i)
        term_index[dict[i].term] = i;

    core::RadixTrie trie(dict);
    for (int i = 0; i < static_cast<int>(dict.size()); ++i)
        trie.insert(i);

    ranking::UserProfile profile(user_id);
    io::Persistence::load_profile(profile);

    ranking::Ranker ranker(dict, profile);

    fuzzy::FuzzyMatcher fuzzy_matcher(dict, profile, FUZZY_MAX_DIST);
    fuzzy_matcher.build();

    expansion::AbbreviationEngine abbrev;
    abbrev.load_from_file(abbrev_path);

    ui::RenderState state;
    std::string query;

    enum class InputPurpose { ADD_WORD, ADD_ABBREV_KEY, ADD_ABBREV_VAL };
    InputPurpose input_purpose = InputPurpose::ADD_WORD;
    std::string  abbrev_key_pending; 

    ui::TermiosController term_ctrl;
    ui::Renderer::draw(state);

    auto confirm_selection = [&](const std::string& word) {
        auto it = term_index.find(word);
        if (it != term_index.end()) {
            int id = it->second;
            profile.record_selection(id);
            ++dict[id].global_frequency;
            ranker.invalidate(id);
            float new_score = ranker.score(id);
            trie.update_subtree_score(word, new_score);
            trie.invalidate_cache(word);
        }
        state.status_msg  = "\xE2\x9C\x93 Selected: \"" + word + "\"";
        query = "";
        state.suggestions.clear();
        state.selected_index = 0;
        state.search_mode = ui::SearchMode::EXACT;
        state.elapsed_ms = 0.0;
    };

    while (true) {
        ui::KeyEvent ev = ui::KeyHandler::read();

     
        if (ev.action == ui::KeyAction::CTRL_D) {
            io::Persistence::save_profile(profile);
            try { io::DictionaryLoader::save(dict_path, dict); } catch (...) {}
            return 0;
        }

        if (state.app_mode == ui::AppMode::SEARCH) {
            bool query_changed = false;

            switch (ev.action) {

            case ui::KeyAction::CHAR:
                query += ev.ch;
                query_changed = true;
                state.selected_index = 0;
                state.status_msg.clear();
                break;

            case ui::KeyAction::BACKSPACE:
                if (!query.empty()) {
                    query.pop_back();
                    query_changed = true;
                    state.selected_index = 0;
                    state.status_msg.clear();
                } else {
                    state.status_msg = "Nothing to delete.";
                }
                break;

            case ui::KeyAction::ARROW_UP:
                if (!state.suggestions.empty() && state.selected_index > 0)
                    --state.selected_index;
                else if (state.suggestions.empty())
                    state.status_msg = "No suggestions to navigate.";
                break;

            case ui::KeyAction::ARROW_DOWN:
                if (!state.suggestions.empty() &&
                    state.selected_index <
                        static_cast<int>(state.suggestions.size()) - 1)
                    ++state.selected_index;
                else if (state.suggestions.empty())
                    state.status_msg = "No suggestions to navigate.";
                break;

            case ui::KeyAction::TAB:
                
                if (!state.suggestions.empty()) {
                    query = state.suggestions[state.selected_index];
                    query_changed = true;
                    state.status_msg = "Press ENTER to confirm.";
                } else {
                    state.status_msg = "No suggestion to complete.";
                }
                break;

            case ui::KeyAction::ENTER:
                if (!state.suggestions.empty()) {
                    confirm_selection(state.suggestions[state.selected_index]);
                } else if (!query.empty()) {
                    state.status_msg =
                        "No match. Press M then A to add \"" + query + "\".";
                } else {
                    state.status_msg = "Type something to search.";
                }
                query_changed = false;
                break;

            case ui::KeyAction::NUMBER: {
                int idx = ev.ch - '1';
                if (!state.suggestions.empty() &&
                    idx < static_cast<int>(state.suggestions.size())) {
                    confirm_selection(state.suggestions[idx]);
                } else if (state.suggestions.empty()) {
                    state.status_msg = "No suggestions to select.";
                } else {
                    state.status_msg = "Only " +
                        std::to_string(state.suggestions.size()) +
                        " suggestion(s) available.";
                }
                query_changed = false;
                break;
            }

            case ui::KeyAction::KEY_M:
                state.app_mode   = ui::AppMode::MENU;
                state.status_msg.clear();
                break;

            case ui::KeyAction::UNKNOWN:
                state.status_msg =
                    "Only a-z letters supported. Press M to manage words.";
                break;

            default: break;
            }

            if (query_changed && !query.empty()) {
                recompute_suggestions(query, trie, abbrev, fuzzy_matcher,
                                      ranker, profile, dict, term_index, state);
            } else if (query_changed && query.empty()) {
                state.suggestions.clear();
                state.search_mode = ui::SearchMode::EXACT;
                state.elapsed_ms = 0.0;
            }

            state.query = query;
        }

        else if (state.app_mode == ui::AppMode::MENU) {
            state.status_msg.clear();

            switch (ev.action) {

            case ui::KeyAction::KEY_A:
                input_purpose = InputPurpose::ADD_WORD;
                state.input_prompt = "Add word:";
                state.input_buffer.clear();
                state.app_mode = ui::AppMode::INPUT;
                break;

            case ui::KeyAction::KEY_D:
               
                if (state.suggestions.empty()) {
                    state.status_msg = "No suggestion selected. Search first.";
                    state.app_mode = ui::AppMode::SEARCH;
                } else {
                    const std::string& to_del =
                        state.suggestions[state.selected_index];
                    auto it = term_index.find(to_del);
                    if (it != term_index.end()) {
                        int id = it->second;
                        dict[id].term = ""; 
                        dict[id].global_frequency = 0;
                        term_index.erase(to_del);
                        try { io::DictionaryLoader::save(dict_path, dict); }
                        catch (...) {}
                    }
                    state.suggestions.erase(
                        state.suggestions.begin() + state.selected_index);
                    if (state.selected_index >=
                        static_cast<int>(state.suggestions.size()))
                        state.selected_index =
                            std::max(0,
                                static_cast<int>(state.suggestions.size()) - 1);
                    state.status_msg = "\xE2\x9C\x93 Deleted \"" + to_del + "\"";
                    state.app_mode = ui::AppMode::SEARCH;
                }
                break;

            case ui::KeyAction::KEY_B:
            
                input_purpose = InputPurpose::ADD_ABBREV_KEY;
                state.input_prompt = "Abbreviation (e.g. asap):";
                state.input_buffer.clear();
                state.app_mode = ui::AppMode::INPUT;
                break;

            case ui::KeyAction::KEY_H:
      
                if (state.suggestions.empty()) {
                    state.status_msg = "No suggestion to hide. Search first.";
                    state.app_mode = ui::AppMode::SEARCH;
                } else {
                    const std::string& to_hide =
                        state.suggestions[state.selected_index];
                    auto it = term_index.find(to_hide);
                    if (it != term_index.end())
                        profile.hide(it->second);
                    state.suggestions.erase(
                        state.suggestions.begin() + state.selected_index);
                    if (state.selected_index >=
                        static_cast<int>(state.suggestions.size()))
                        state.selected_index =
                            std::max(0,
                                static_cast<int>(state.suggestions.size()) - 1);
                    state.status_msg = "Hidden \"" + to_hide + "\" for you.";
                    state.app_mode = ui::AppMode::SEARCH;
                }
                break;

            case ui::KeyAction::ESC:
                state.app_mode = ui::AppMode::SEARCH;
                break;

            default: break;
            }
        }

 
        else if (state.app_mode == ui::AppMode::INPUT) {
            switch (ev.action) {

            case ui::KeyAction::CHAR:
                state.input_buffer += ev.ch;
                state.status_msg.clear();
                break;

            case ui::KeyAction::BACKSPACE:
                if (!state.input_buffer.empty())
                    state.input_buffer.pop_back();
                break;

            case ui::KeyAction::ENTER: {
                const std::string& val = state.input_buffer;

                if (val.empty()) {
                    state.status_msg = "Cannot be empty.";
                    break;
                }

                if (input_purpose == InputPurpose::ADD_WORD) {
                    if (term_index.count(val)) {
                        state.status_msg = "\"" + val + "\" already exists.";
                    } else {
                        int new_id = static_cast<int>(dict.size());
                        dict.push_back({val, NEW_WORD_FREQ});
                        term_index[val] = new_id;
                        trie.insert(new_id);
                        fuzzy_matcher.rebuild();
                        try { io::DictionaryLoader::save(dict_path, dict); }
                        catch (...) {}
                        state.status_msg = "\xE2\x9C\x93 Added \"" + val + "\"";
                    }
                    state.app_mode = ui::AppMode::SEARCH;

                } else if (input_purpose == InputPurpose::ADD_ABBREV_KEY) {
       
                    abbrev_key_pending = val;
                    input_purpose = InputPurpose::ADD_ABBREV_VAL;
                    state.input_prompt = "Expansion for \"" + val + "\":";
                    state.input_buffer.clear();
                    state.status_msg.clear();

                } else if (input_purpose == InputPurpose::ADD_ABBREV_VAL) {
                    abbrev.add(abbrev_key_pending, val);
                    abbrev.save_to_file(abbrev_path);
                    state.status_msg = "\xE2\x9C\x93 " + abbrev_key_pending +
                                       " \xE2\x86\x92 " + val;
                    abbrev_key_pending.clear();
                    state.app_mode = ui::AppMode::SEARCH;
                }
                break;
            }

            case ui::KeyAction::ESC:
                state.app_mode = ui::AppMode::MENU;
                state.input_buffer.clear();
                state.status_msg.clear();
                break;

            default: break;
            }
        }

        ui::Renderer::draw(state);
    }

    return 0;
}
