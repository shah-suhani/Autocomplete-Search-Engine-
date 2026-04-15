#include "TopKCache.hpp"

#include <algorithm>

namespace core {

std::vector<int> TopKCache::top_k(const std::vector<int>& candidates,
                                  const std::function<float(int)>& scorer,
                                  int k) {
    if (k <= 0 || candidates.empty()) {
        return {};
    }

    using Pair = std::pair<float, int>;

    std::vector<Pair> heap;
    heap.reserve(k + 1);

    for (int id : candidates) {
        const float score = scorer(id);

        if (static_cast<int>(heap.size()) < k) {
            heap.push_back({score, id});
            std::push_heap(heap.begin(), heap.end(), std::greater<Pair>{});
            continue;
        }

        if (score <= heap.front().first) {
            continue;
        }

        std::pop_heap(heap.begin(), heap.end(), std::greater<Pair>{});
        heap.back() = {score, id};
        std::push_heap(heap.begin(), heap.end(), std::greater<Pair>{});
    }

    std::sort_heap(heap.begin(), heap.end(), std::greater<Pair>{});

    std::vector<int> result;
    result.reserve(heap.size());
    for (const auto& [score, id] : heap) {
        (void)score;
        result.push_back(id);
    }

    return result;
}

}  // namespace core
