#include "UserProfile.hpp"
#include <ctime>

namespace ranking {

UserProfile::UserProfile(std::string userId) : userId_(std::move(userId)) {}

const std::string& UserProfile::userId() const { return userId_; }

void UserProfile::record_selection(int termId) {
    auto& s = stats_[termId];
    ++s.selectCount;
    s.lastUsed = std::time(nullptr);
}

UserStats UserProfile::getStats(int termId) const {
    auto it = stats_.find(termId);
    return (it == stats_.end()) ? UserStats{} : it->second;
}

void UserProfile::hide (int termId) { hidden_.insert(termId); }
void UserProfile::unhide(int termId) { hidden_.erase(termId);  }
bool UserProfile::is_hidden(int termId) const { return hidden_.count(termId) > 0; }

const std::unordered_map<int, UserStats>& UserProfile::all_stats()  const { return stats_;  }
const std::unordered_set<int>& UserProfile::hidden_set() const { return hidden_; }

void UserProfile::load_stats(std::unordered_map<int, UserStats> stats,
                              std::unordered_set<int> hidden) {
    stats_ = std::move(stats);
    hidden_ = std::move(hidden);
}

}
