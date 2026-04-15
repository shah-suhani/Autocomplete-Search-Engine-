#include "RadixTrie.hpp"

#include <algorithm>

namespace core {

namespace {

float term_score(const std::vector<TermInfo>& dict, int term_id) {
    return static_cast<float>(dict[term_id].global_frequency);
}

size_t common_prefix_length(const std::string& edge_label,
                            const std::string& text,
                            size_t text_pos) {
    size_t common = 0;
    const size_t max_match = std::min(edge_label.size(), text.size() - text_pos);
    while (common < max_match && edge_label[common] == text[text_pos + common]) {
        ++common;
    }
    return common;
}

void mark_terminal(TrieNode& node, int term_id, float score) {
    node.is_terminal = true;
    node.term_id = term_id;
    node.subtree_max_score = std::max(node.subtree_max_score, score);
}

void attach_child(TrieNode& parent,
                  int child_index,
                  const std::string& label,
                  std::unique_ptr<TrieNode> child) {
    parent.children[child_index].label = label;
    parent.children[child_index].child = std::move(child);
}

std::unique_ptr<TrieNode> make_terminal_node(int term_id, float score) {
    auto node = std::make_unique<TrieNode>();
    mark_terminal(*node, term_id, score);
    return node;
}

template <typename Visitor>
void visit_existing_path(TrieNode* root, const std::string& term, Visitor visitor) {
    TrieNode* current = root;
    size_t pos = 0;

    visitor(*current);

    while (pos < term.size()) {
        const int idx = RadixTrie::char_index(term[pos]);
        if (idx < 0) {
            break;
        }

        Edge& edge = current->children[idx];
        if (!edge.child) {
            break;
        }

        current = edge.child.get();
        visitor(*current);
        pos += std::min(edge.label.size(), term.size() - pos);
    }
}

void split_edge_for_insertion(Edge& edge,
                              const std::string& term,
                              size_t term_pos,
                              size_t common_chars,
                              int term_id,
                              float score) {
    auto split_node = std::make_unique<TrieNode>();

    const std::string old_suffix = edge.label.substr(common_chars);
    const int old_child_index = RadixTrie::char_index(old_suffix[0]);
    if (old_child_index >= 0) {
        attach_child(*split_node, old_child_index, old_suffix, std::move(edge.child));
        split_node->subtree_max_score =
            split_node->children[old_child_index].child->subtree_max_score;
    }

    const std::string new_suffix = term.substr(term_pos + common_chars);
    if (new_suffix.empty()) {
        mark_terminal(*split_node, term_id, score);
    } else {
        const int new_child_index = RadixTrie::char_index(new_suffix[0]);
        if (new_child_index >= 0) {
            attach_child(*split_node, new_child_index, new_suffix,
                         make_terminal_node(term_id, score));
            split_node->subtree_max_score =
                std::max(split_node->subtree_max_score, score);
        }
    }

    edge.label = edge.label.substr(0, common_chars);
    edge.child = std::move(split_node);
}

}  // namespace

RadixTrie::RadixTrie(std::vector<TermInfo>& dictionary)
    : root_(std::make_unique<TrieNode>()), dict_(dictionary) {}

void RadixTrie::insert(int term_id) {
    const std::string& term = dict_[term_id].term;
    const float score = term_score(dict_, term_id);
    TrieNode* current = root_.get();
    size_t pos = 0;

    while (pos < term.size()) {
        const int idx = char_index(term[pos]);
        if (idx < 0) {
            return;
        }

        Edge& edge = current->children[idx];

        if (!edge.child) {
            edge.label = term.substr(pos);
            edge.child = make_terminal_node(term_id, score);
            pos = term.size();
            break;
        }

        const size_t common = common_prefix_length(edge.label, term, pos);

        if (common == edge.label.size()) {
            current = edge.child.get();
            pos += common;
            continue;
        } else {
            split_edge_for_insertion(edge, term, pos, common, term_id, score);
            pos = term.size();
            break;
        }
    }

    if (pos == term.size() && current != root_.get() && !current->is_terminal) {
        current->is_terminal = true;
        current->term_id = term_id;
    }

    visit_existing_path(root_.get(), term, [score](TrieNode& node) {
        node.cache_valid = false;
        node.subtree_max_score = std::max(node.subtree_max_score, score);
    });
}

const TrieNode* RadixTrie::find_node(const std::string& prefix) const {
    const TrieNode* current = root_.get();
    size_t pos = 0;

    while (pos < prefix.size()) {
        const int idx = char_index(prefix[pos]);
        if (idx < 0) {
            return nullptr;
        }

        const Edge& edge = current->children[idx];
        if (!edge.child) {
            return nullptr;
        }

        const size_t common = common_prefix_length(edge.label, prefix, pos);

        if (common < edge.label.size()) {
            if (pos + common == prefix.size()) {
                return edge.child.get();
            }
            return nullptr;
        }

        current = edge.child.get();
        pos += common;
    }
    return current;
}

TrieNode* RadixTrie::find_node_mutable(const std::string& prefix) {
    return const_cast<TrieNode*>(find_node(prefix));
}

std::vector<int> RadixTrie::search_prefix(const std::string& prefix,
                                          bool& cache_hit) const {
    const TrieNode* node = find_node(prefix);
    if (!node) {
        cache_hit = false;
        return {};
    }

    if (node->cache_valid && !node->top_k_cache.empty()) {
        cache_hit = true;
        return node->top_k_cache;
    }

    cache_hit = false;
    auto scorer = [this](int id) -> float {
        if (id < 0 || dict_[id].term.empty()) {
            return 0.0f;
        }
        return term_score(dict_, id);
    };

    std::vector<int> results;
    results.reserve(TrieNode::K * 2);
    collect_top_k(node, scorer, results, TrieNode::K);
    return results;
}

void RadixTrie::collect_top_k(const TrieNode*                  node,
                              const std::function<float(int)>& scorer,
                              std::vector<int>& results,
                              int k) const {
    using Pair = std::pair<float, int>;

    std::vector<Pair> heap;
    heap.reserve(k + 1);

    std::vector<const TrieNode*> stack;
    stack.reserve(64);
    stack.push_back(node);

    while (!stack.empty()) {
        const TrieNode* current = stack.back();
        stack.pop_back();

        if (static_cast<int>(heap.size()) == k &&
            current->subtree_max_score <= heap.front().first) {
            continue;
        }

        if (current->is_terminal && current->term_id >= 0) {
            if (!dict_[current->term_id].term.empty()) {
                const float score = scorer(current->term_id);
                if (static_cast<int>(heap.size()) < k) {
                    heap.push_back({score, current->term_id});
                    std::push_heap(heap.begin(), heap.end(), std::greater<Pair>{});
                } else if (score > heap.front().first) {
                    std::pop_heap(heap.begin(), heap.end(), std::greater<Pair>{});
                    heap.back() = {score, current->term_id};
                    std::push_heap(heap.begin(), heap.end(), std::greater<Pair>{});
                }
            }
        }

        for (int i = 0; i < kAlphabetSize; ++i) {
            if (current->children[i].child) {
                stack.push_back(current->children[i].child.get());
            }
        }
    }

    std::sort_heap(heap.begin(), heap.end(), std::greater<Pair>{});
    results.clear();
    results.reserve(heap.size());
    for (const auto& [score, id] : heap) {
        (void)score;
        results.push_back(id);
    }
}

void RadixTrie::invalidate_cache(const std::string& term) {
    visit_existing_path(root_.get(), term, [](TrieNode& node) {
        node.cache_valid = false;
    });
}

void RadixTrie::update_subtree_score(const std::string& term, float new_score) {
    visit_existing_path(root_.get(), term, [new_score](TrieNode& node) {
        node.subtree_max_score = std::max(node.subtree_max_score, new_score);
    });
}

void RadixTrie::rebuild_cache(const std::string& prefix,
                              const std::function<float(int)>& scorer) {
    TrieNode* node = find_node_mutable(prefix);
    if (!node) {
        return;
    }

    node->top_k_cache.clear();
    collect_top_k(node, scorer, node->top_k_cache, TrieNode::K);
    node->cache_valid = true;
}

}  // namespace core
