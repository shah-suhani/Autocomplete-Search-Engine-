#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <ctime>
using namespace std;

namespace ranking {

struct UserStats {
    int selectCount = 0;
    time_t lastUsed = 0;
};

class UserProfile {
public:
    explicit UserProfile(string userId);

    const string& userId() const;

    void recordSelection(int termId);

    UserStats getStats(int termId) const;

    void hide(int termId);
    void unhide(int termId);
    bool isHidden(int termId) const;

    const unordered_map<int, UserStats>& allStats()  const;
    const unordered_set<int>& hiddenSet() const;

    void loadStats(unordered_map<int, UserStats> stats,
                   unordered_set<int> hidden);

private:
    string userId_;
    unordered_map<int, UserStats> stats_;
    unordered_set<int> hidden_;
};

}
