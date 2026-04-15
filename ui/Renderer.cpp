#include "Renderer.hpp"
#include <cstdio>

namespace ui {

static void put(const char* s) { fputs(s, stdout); }
static void nl()               { fputs("\r\n", stdout); }

void Renderer::clear_screen() { put("\033[2J\033[H"); }

void Renderer::draw_header() {
    put("\033[1;36m╔══════════════════════════════════════╗\033[0m"); nl();
    put("\033[1;36m║     Autocomplete Search Engine       ║\033[0m"); nl();
    put("\033[1;36m╚══════════════════════════════════════╝\033[0m"); nl();
    nl();
}

const char* Renderer::mode_label(SearchMode m) {
    switch (m) {
        case SearchMode::EXACT:        return "exact";
        case SearchMode::FUZZY:        return "fuzzy";
        case SearchMode::ABBREVIATION: return "abbrev";
    }
    return "?";
}

void Renderer::draw_search(const RenderState& s) {
  
    put("\033[1mSearch:\033[0m ");
    put(s.query.c_str());
    put("\033[1;33m_\033[0m");
    nl(); nl();

    if (s.suggestions.empty()) {
        if (s.query.empty())
            put("  \033[2mStart typing to search...\033[0m");
        else
            put("  \033[2mNo suggestions found.\033[0m");
        nl();
    } else {
        put("\033[1mSuggestions:\033[0m"); nl();
        for (int i = 0; i < static_cast<int>(s.suggestions.size()); ++i) {
            bool sel = (i == s.selected_index);
            if (sel) put("\033[1;32m  > ");
            else     put("      ");
            fprintf(stdout, "%d. %s", i + 1, s.suggestions[i].c_str());
            if (sel) put("\033[0m");
            nl();
        }
    }
    nl();

    if (!s.status_msg.empty()) {
        put("\033[1;33m  "); put(s.status_msg.c_str()); put("\033[0m"); nl();
        nl();
    }

    put("\033[2m");
    fprintf(stdout, "  [mode: %s]  [cache: %s]  [%.2f ms]",
            mode_label(s.search_mode),
            s.cache_hit ? "\033[32mHIT\033[2m" : "\033[33mMISS\033[2m",
            s.elapsed_ms);
    put("\033[0m"); nl(); nl();

    put("\033[2m  1-5=select  TAB=fill  ↑↓=navigate  ENTER=confirm  M=menu  ^D=quit\033[0m");
    nl();
}

void Renderer::draw_menu(const RenderState& s) {
    put("\033[1;36m╔══════════════════════════════════════╗\033[0m"); nl();
    put("\033[1;36m║          Word Manager                ║\033[0m"); nl();
    put("\033[1;36m╚══════════════════════════════════════╝\033[0m"); nl();
    nl();

    put("  \033[1mA\033[0m  →  Add new word to dictionary");    nl();
    put("  \033[1mD\033[0m  →  Delete highlighted suggestion"); nl();
    put("  \033[1mB\033[0m  →  Add abbreviation");              nl();
    put("  \033[1mH\033[0m  →  Hide word (just for you)");      nl();
    put("  \033[1mESC\033[0m → Back to search");                nl();
    nl();

    if (!s.status_msg.empty()) {
        put("\033[1;33m  "); put(s.status_msg.c_str()); put("\033[0m"); nl();
    }
}

void Renderer::draw_input(const RenderState& s) {
    put("\033[1;36m╔══════════════════════════════════════╗\033[0m"); nl();
    put("\033[1;36m║          Word Manager                ║\033[0m"); nl();
    put("\033[1;36m╚══════════════════════════════════════╝\033[0m"); nl();
    nl();

    put("\033[1m  "); put(s.input_prompt.c_str()); put("\033[0m ");
    put(s.input_buffer.c_str());
    put("\033[1;33m_\033[0m"); nl();
    nl();
    put("\033[2m  ENTER=confirm  ESC=cancel\033[0m"); nl();
    nl();

    if (!s.status_msg.empty()) {
        put("\033[1;33m  "); put(s.status_msg.c_str()); put("\033[0m"); nl();
    }
}

void Renderer::draw(const RenderState& state) {
    clear_screen();
    draw_header();

    switch (state.app_mode) {
        case AppMode::SEARCH: draw_search(state); break;
        case AppMode::MENU: draw_menu  (state); break;
        case AppMode::INPUT: draw_input (state); break;
    }

    fflush(stdout);
}

}
