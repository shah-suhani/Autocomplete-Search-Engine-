#include "FuzzyMatcher.hpp"
#include <algorithm>

namespace fuzzy {

FuzzyMatcher::FuzzyMatcher(const std::vector<core::TermInfo>& dict,
                             const ranking::UserProfile&        profile,
                             int                                max_dist)
    : dict_(dict), profile_(profile), max_dist_(max_dist) {}

void FuzzyMatcher::build() {
    std::vector<std::string> terms;
    terms.reserve(dict_.size());
    for (const auto& ti : dict_) terms.push_back(ti.term);
    bk_tree_.build(terms);
}

void FuzzyMatcher::rebuild() { build(); }
std::vector<FuzzyResult> FuzzyMatcher::match(const std::string& query,
                                              int result_limit) const {
    auto bk_results = bk_tree_.search(query, max_dist_);

    std::vector<FuzzyResult> results;
    results.reserve(bk_results.size());

    for (const auto& r : bk_results) {
        if (profile_.is_hidden(r.term_id)) continue;
        if (dict_[r.term_id].term.empty()) continue;

        float score = static_cast<float>(dict_[r.term_id].global_frequency)
                      / (1.0f + static_cast<float>(r.edit_distance) * 2.0f);

        auto us = profile_.get_stats(r.term_id);
        if (us.select_count > 0)
            score += static_cast<float>(us.select_count) * 5.0f;

        results.push_back({r.term_id, r.edit_distance, score});
    }

    std::sort(results.begin(), results.end(),
              [](const FuzzyResult& a, const FuzzyResult& b) {
                  if (a.edit_distance != b.edit_distance)
                      return a.edit_distance < b.edit_distance;
                  return a.score > b.score;
              });

    if (static_cast<int>(results.size()) > result_limit)
        results.resize(result_limit);

    return results;
}

} 
