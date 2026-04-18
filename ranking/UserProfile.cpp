#include "UserProfile.hpp"
#include <ctime>

namespace ranking {

UserProfile::UserProfile(std::string user_id) : user_id_(std::move(user_id)) {}

const std::string& UserProfile::user_id() const { return user_id_; }

void UserProfile::record_selection(int term_id) {
    auto& s = stats_[term_id];
    ++s.select_count;
    s.last_used = std::time(nullptr);
}

UserStats UserProfile::get_stats(int term_id) const {
    auto it = stats_.find(term_id);
    return (it == stats_.end()) ? UserStats{} : it->second;
}

void UserProfile::hide  (int term_id) { hidden_.insert(term_id); }
void UserProfile::unhide(int term_id) { hidden_.erase(term_id);  }
bool UserProfile::is_hidden(int term_id) const { return hidden_.count(term_id) > 0; }

const std::unordered_map<int, UserStats>& UserProfile::all_stats()  const { return stats_;  }
const std::unordered_set<int>&            UserProfile::hidden_set() const { return hidden_; }

void UserProfile::load_stats(std::unordered_map<int, UserStats> stats,
                              std::unordered_set<int>            hidden) {
    stats_  = std::move(stats);
    hidden_ = std::move(hidden);
}

}