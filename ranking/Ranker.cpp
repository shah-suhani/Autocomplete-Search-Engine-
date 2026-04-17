#include "Ranker.hpp"
#include <algorithm>
#include <cmath>
#include <ctime>

namespace ranking {

Ranker::Ranker(const std::vector<core::TermInfo>& dict,
               const UserProfile& profile,
               RankWeights weights)
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

float Ranker::compute_score(int termId) const {
    if (termId < 0 || termId >= static_cast<int>(dict_.size())) return 0.0f;
    if (dict_[termId].term.empty()) return 0.0f;

    float g = norm_global(dict_[termId].globalFrequency);
    UserStats us = profile_.get_stats(termId);
    float u = norm_user(us.select_count);
    float r = recency_score(us.last_used);

    return weights_.global_freq * g +
           weights_.user_freq   * u +
           weights_.recency     * r;
}


float Ranker::score(int termId) const {
    auto it = score_cache_.find(termId);
    if (it != score_cache_.end()) return it->second;
    float s = compute_score(termId);
    score_cache_[termId] = s;
    return s;
}

void Ranker::invalidate(int termId) {
    score_cache_.erase(termId);
}

std::function<float(int)> Ranker::scorer() const {
    return [this](int id) { return this->score(id); };
}

void Ranker::rankInplace(std::vector<int>& termIds) const {
    std::sort(termIds.begin(), termIds.end(),
              [this](int a, int b) { return score(a) > score(b); });
}

}
