#pragma once
#include "../core/TrieNode.hpp"
#include "UserProfile.hpp"
#include <vector>
#include <functional>
#include <unordered_map>

namespace ranking {

struct RankWeights {
    float global_freq = 0.5f;
    float user_freq   = 0.3f;
    float recency     = 0.2f;
};

class Ranker {
public:
    Ranker(const std::vector<core::TermInfo>& dict,
           const UserProfile&                 profile,
           RankWeights                        weights = {});

    float score(int term_id) const;

    std::function<float(int)> scorer() const;

    void rank_inplace(std::vector<int>& term_ids) const;

    void invalidate(int term_id);

private:
    const std::vector<core::TermInfo>& dict_;
    const UserProfile&                 profile_;
    RankWeights                        weights_;

    
    mutable std::unordered_map<int, float> score_cache_;

    float compute_score(int term_id) const;
    float norm_global  (int freq)              const;
    float norm_user    (int user_freq)         const;
    float recency_score(std::time_t last_used) const;
};

}