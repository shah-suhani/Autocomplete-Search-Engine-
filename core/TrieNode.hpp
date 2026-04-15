#pragma once
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace core {

struct TermInfo {
    std::string term;
    int global_frequency = 0;
};

struct TrieNode;

struct Edge {
    std::string label;
    std::unique_ptr<TrieNode> child;

    Edge() = default;
    Edge(Edge&&) = default;
    Edge& operator=(Edge&&) = default;
};

struct TrieNode {
    static constexpr int K = 5;

    std::array<Edge, 26> children;
    bool is_terminal = false;
    int term_id = -1;
    float subtree_max_score = 0.0f;
    std::vector<int> top_k_cache;
    bool cache_valid = false;

    TrieNode() = default;
    ~TrieNode() = default;

    TrieNode(const TrieNode&) = delete;
    TrieNode& operator=(const TrieNode&) = delete;
};

}  // namespace core
