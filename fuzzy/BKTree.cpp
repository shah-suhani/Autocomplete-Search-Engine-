#include "BKTree.hpp"
#include "Levenshtein.hpp"
#include <algorithm>

namespace fuzzy {

void BKTree::build(const std::vector<std::string>& terms) {
    owned_terms_ = terms;
    terms_       = &owned_terms_;
    root_        = nullptr;

    for (int i = 0; i < static_cast<int>(terms.size()); ++i) {
        if (!terms[i].empty())
            insert(i, terms[i]);
    }
}


void BKTree::insert(int term_id, const std::string& term) {
    if (!root_) {
        root_ = std::make_unique<BKNode>();
        root_->term_id = term_id;
        return;
    }

    BKNode* cur = root_.get();
    while (true) {
        int dist = Levenshtein::distance(term, (*terms_)[cur->term_id], 10);
        if (dist == 0) return; // duplicate

        auto it = cur->children.find(dist);
        if (it == cur->children.end()) {
            auto node      = std::make_unique<BKNode>();
            node->term_id  = term_id;
            cur->children[dist] = std::move(node);
            return;
        }
        cur = it->second.get();
    }
}

std::vector<BKResult> BKTree::search(const std::string& query,
                                      int max_dist) const {
    std::vector<BKResult> results;
    if (!root_) return results;
    search_recursive(root_.get(), query, max_dist, results);
    std::sort(results.begin(), results.end(),
              [](const BKResult& a, const BKResult& b) {
                  return a.edit_distance < b.edit_distance;
              });
    return results;
}

void BKTree::search_recursive(const BKNode*          node,
                               const std::string&     query,
                               int                    max_dist,
                               std::vector<BKResult>& results) const {
    int d = Levenshtein::distance(query, (*terms_)[node->term_id], max_dist + 1);

    if (d <= max_dist)
        results.push_back({node->term_id, d});

    int lo = d - max_dist;
    int hi = d + max_dist;
    for (const auto& [edge_dist, child] : node->children) {
        if (edge_dist >= lo && edge_dist <= hi)
            search_recursive(child.get(), query, max_dist, results);
    }
}

} 