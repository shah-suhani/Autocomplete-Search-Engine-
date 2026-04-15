#include "UserProfile.hpp"
#include <ctime>
using namespace std;

namespace ranking {

UserProfile::UserProfile(string userId)
    : userId_(move(userId)) {}

const string& UserProfile::userId() const {
    return userId_;
}

void UserProfile::recordSelection(int termId) {
    auto& s = stats_[termId];
    ++s.selectCount;
    s.lastUsed = time(nullptr);
}

UserStats UserProfile::getStats(int termId) const {
    auto it = stats_.find(termId);
    if (it == stats_.end())
        return UserStats{};
    return it->second;
}

void UserProfile::hide(int termId)   { hidden_.insert(termId); }
void UserProfile::unhide(int termId) { hidden_.erase(termId);  }

bool UserProfile::isHidden(int termId) const {
    return hidden_.count(termId) > 0;
}

const unordered_map<int, UserStats>& UserProfile::allStats() const {
    return stats_;
}

const unordered_set<int>& UserProfile::hiddenSet() const {
    return hidden_;
}

void UserProfile::loadStats(unordered_map<int, UserStats> stats,
                             unordered_set<int> hidden) {
    stats_ = move(stats);
    hidden_ = move(hidden);
}

}
