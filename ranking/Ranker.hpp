#pragma once
#include "../core/TrieNode.hpp"
#include "UserProfile.hpp"
#include <vector>
#include <functional>
#include <unordered_map>
using namespace std;

namespace ranking {

struct RankWeights {
    float globalFreq = 0.5f;
    float userFreq   = 0.3f;
    float recency    = 0.2f;
};

class Ranker {
public:\
    Ranker(const vector<core::TermInfo>& dict,
           const UserProfile& profile,
           RankWeights weights = {});

    float score(int termId) const;

    function<float(int)> scorer() const;

    void rankInplace(vector<int>& termIds) const;

    void invalidate(int termId);

private:
    const vector<core::TermInfo>& dict_;
    const UserProfile& profile_;
    RankWeights weights_;

    mutable unordered_map<int, float> scoreCache_;

    float computeScore(int termId) const;
    float normGlobal(int freq) const;
    float normUser(int userFreq) const;
    float recencyScore(time_t lastUsed) const;
};

}
