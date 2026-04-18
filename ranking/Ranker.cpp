#include "Ranker.hpp"
#include <algorithm>
#include <cmath>
#include <ctime>

namespace ranking {

Ranker::Ranker(const std::vector<core::TermInfo>& dict,
               const UserProfile&                 profile,
               RankWeights                        weights)
    : dict_(dict), profile_(profile), weights_(weights) {}


float Ranker::norm_global(int freq) const {
    return std::min(static_cast<float>(freq) / 100.0f, 1.0f);
}


float Ranker::norm_user(int user_freq) const {
    return static_cast<float>(user_freq) /
           (static_cast<float>(user_freq) + 5.0f);
}

float Ranker::recency_score(std::time_t last_used) const {
    if (last_used == 0) return 0.0f;
    double hours = std::difftime(std::time(nullptr), last_used) / 3600.0;
    return static_cast<float>(std::exp(-0.0096 * hours));
}

float Ranker::compute_score(int term_id) const {
    if (term_id < 0 || term_id >= static_cast<int>(dict_.size())) return 0.0f;
    if (dict_[term_id].term.empty()) return 0.0f;

    float g = norm_global(dict_[term_id].global_frequency);
    UserStats us = profile_.get_stats(term_id);
    float u = norm_user(us.select_count);
    float r = recency_score(us.last_used);

    return weights_.global_freq * g +
           weights_.user_freq   * u +
           weights_.recency     * r;
}


float Ranker::score(int term_id) const {
    auto it = score_cache_.find(term_id);
    if (it != score_cache_.end()) return it->second;
    float s = compute_score(term_id);
    score_cache_[term_id] = s;
    return s;
}

void Ranker::invalidate(int term_id) {
    score_cache_.erase(term_id);
}

std::function<float(int)> Ranker::scorer() const {
    return [this](int id) { return this->score(id); };
}

void Ranker::rank_inplace(std::vector<int>& term_ids) const {
    std::sort(term_ids.begin(), term_ids.end(),
              [this](int a, int b) { return score(a) > score(b); });
}

}