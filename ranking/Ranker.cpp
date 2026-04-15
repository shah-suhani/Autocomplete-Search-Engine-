#include "Ranker.hpp"
#include <algorithm>
#include <cmath>
#include <ctime>
using namespace std;

namespace ranking {

Ranker::Ranker(const vector<core::TermInfo>& dict,
               const UserProfile& profile,
               RankWeights weights)
    : dict_(dict), profile_(profile), weights_(weights) {}


float Ranker::normGlobal(int freq) const {
    return min(static_cast<float>(freq) / 100.0f, 1.0f);
}

float Ranker::normUser(int userFreq) const {
    return static_cast<float>(userFreq) /
           (static_cast<float>(userFreq) + 5.0f);
}

float Ranker::recencyScore(time_t lastUsed) const {
    if (lastUsed == 0)
        return 0.0f;

    double hoursElapsed = difftime(time(nullptr), lastUsed) / 3600.0;
    return static_cast<float>(exp(-0.0096 * hoursElapsed));
}

float Ranker::computeScore(int termId) const {
    if (termId < 0 || termId >= static_cast<int>(dict_.size()))
        return 0.0f;
    if (dict_[termId].term.empty())
        return 0.0f;

    float g = normGlobal(dict_[termId].globalFrequency);

    UserStats us = profile_.getStats(termId);
    float u = normUser(us.selectCount);
    float r = recencyScore(us.lastUsed);

    return weights_.globalFreq * g +
           weights_.userFreq   * u +
           weights_.recency    * r;
}

float Ranker::score(int termId) const {.
    auto it = scoreCache_.find(termId);
    if (it != scoreCache_.end())
        return it->second;

    float s = computeScore(termId);
    scoreCache_[termId] = s;
    return s;
}

void Ranker::invalidate(int termId) {
    scoreCache_.erase(termId);
}

function<float(int)> Ranker::scorer() const {
    return [this](int id) { return this->score(id); };
}

void Ranker::rankInplace(vector<int>& termIds) const {
    sort(termIds.begin(), termIds.end(),
              [this](int a, int b) { return score(a) > score(b); });
}

}
