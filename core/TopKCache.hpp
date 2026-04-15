#pragma once
#include <functional>
#include <vector>

namespace core {

class TopKCache {
public:
    static std::vector<int> top_k(const std::vector<int>& candidates,
                                  const std::function<float(int)>& scorer,
                                  int k);
};

}  // namespace core
