#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <ctime>

namespace ranking {

struct UserStats {
    int         select_count = 0;
    std::time_t last_used    = 0;
};

class UserProfile {
public:
    explicit UserProfile(std::string user_id);

    const std::string& user_id() const;

    void      record_selection(int term_id);
    UserStats get_stats(int term_id) const;

    void hide  (int term_id);
    void unhide(int term_id);
    bool is_hidden(int term_id) const;

    const std::unordered_map<int, UserStats>& all_stats()  const;
    const std::unordered_set<int>&            hidden_set() const;

    void load_stats(std::unordered_map<int, UserStats> stats,
                    std::unordered_set<int>            hidden);

private:
    std::string                        user_id_;
    std::unordered_map<int, UserStats> stats_;   
    std::unordered_set<int>            hidden_; 
};

} 