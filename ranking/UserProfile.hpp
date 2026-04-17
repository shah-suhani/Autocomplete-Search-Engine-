#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <ctime>

namespace ranking {

struct UserStats {
    int selectCount = 0;
    std::time_t lastUsed = 0;
};

class UserProfile {
public:
    explicit UserProfile(std::string userId);

    const std::string& userId() const;

    void recordSelection(int termId);
    UserStats getStats(int termId) const;

    void hide(int termId);
    void unhide(int termId);
    bool is_hidden(int termId) const;

    const std::unordered_map<int, UserStats>& allStats()  const;
    const std::unordered_set<int>& hiddenSet() const;

    void loadStats(std::unordered_map<int, UserStats> stats,
                    std::unordered_set<int> hidden);

private:
    std::string userId_;
    std::unordered_map<int, UserStats> stats_;
    std::unordered_set<int> hidden_;
};

} 
