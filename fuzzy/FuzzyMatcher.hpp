#pragma once
#include "../core/TrieNode.hpp"
#include "../ranking/UserProfile.hpp"
#include "BKTree.hpp"
#include <vector>
#include <string>

namespace fuzzy {

struct FuzzyResult {
    int   term_id;
    int   edit_distance;
    float score;
};

class FuzzyMatcher {
public:
    FuzzyMatcher(const std::vector<core::TermInfo>& dict,
                 const ranking::UserProfile&        profile,
                 int                                max_dist = 2);

    // Build BK-tree from dictionary — call once at startup
    void build();

    std::vector<FuzzyResult> match(const std::string& query,
                                   int result_limit = 5) const;

    // Rebuild after dictionary changes
    void rebuild();

private:
    const std::vector<core::TermInfo>& dict_;
    const ranking::UserProfile&        profile_;
    int                                max_dist_;
    BKTree                             bk_tree_;
};

} 
