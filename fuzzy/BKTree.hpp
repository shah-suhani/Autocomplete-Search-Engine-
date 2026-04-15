#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
using namespace std;

namespace fuzzy {


struct BKNode {
    int term_id;  
    unordered_map<int, unique_ptr<BKNode>> children; 
};

struct BKResult {
    int term_id;
    int edit_distance;
};

class BKTree {
public:
    BKTree() = default;

    void build(const vector<string>& terms);

    vector<BKResult> search(const string& query, int max_dist) const;

    bool empty() const { return root_ == nullptr; }

private:
    unique_ptr<BKNode> root_;

    void insert(int term_id, const string& term);

    void search_recursive(const BKNode*          node,
                          const string&     query,
                          int                    max_dist,
                          vector<BKResult>& results) const;

    const vector<string>* terms_ = nullptr;
    vector<string>        owned_terms_;
};

} 
