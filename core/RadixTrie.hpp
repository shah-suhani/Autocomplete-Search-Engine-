#pragma once
#include "TrieNode.hpp"

#include <functional>
#include <string>
#include <vector>

namespace core {

class RadixTrie {
public:
    static constexpr int kAlphabetSize = 26;

    explicit RadixTrie(std::vector<TermInfo>& dictionary);

    void insert(int term_id);

    std::vector<int> search_prefix(const std::string& prefix, bool& cache_hit) const;

    void invalidate_cache(const std::string& term);

    void rebuild_cache(const std::string& prefix,
                       const std::function<float(int)>& scorer);

    void update_subtree_score(const std::string& term, float new_score);

    static int char_index(char c) {
        const int idx = c - 'a';
        return (idx >= 0 && idx < kAlphabetSize) ? idx : -1;
    }

private:
    std::unique_ptr<TrieNode> root_;
    std::vector<TermInfo>& dict_;

    const TrieNode* find_node(const std::string& prefix) const;
    TrieNode* find_node_mutable(const std::string& prefix);

    void collect_top_k(const TrieNode* node,
                       const std::function<float(int)>& scorer,
                       std::vector<int>& results,
                       int k) const;
};

}  // namespace core
