#include "Levenshtein.hpp"
#include <vector>
#include <algorithm>
using namespace std;
namespace fuzzy {

int Levenshtein::distance(const string& a,
                            const string& b,
                            int max_dist) {
    const int m = static_cast<int>(a.size());
    const int n = static_cast<int>(b.size());

    // Quick length pre-filter
    if (std::abs(m - n) > max_dist) return max_dist + 1;

    // Two rolling rows — O(n) space instead of O(m*n)
    std::vector<int> prev(n + 1), curr(n + 1);
    for (int j = 0; j <= n; ++j) prev[j] = j;

    for (int i = 1; i <= m; ++i) {
        curr[0]  = i;
        int row_min = curr[0];

        for (int j = 1; j <= n; ++j) {
            int cost = (a[i-1] == b[j-1]) ? 0 : 1;
            curr[j]  = min({prev[j] + 1,
                                  curr[j-1] + 1,
                                  prev[j-1] + cost});
            row_min  = min(row_min, curr[j]);
        }

        // Early exit: no cell in this row can lead to distance <= max_dist
        if (row_min > max_dist) return max_dist + 1;
        swap(prev, curr);
    }

    return min(prev[n], max_dist + 1);
}

bool Levenshtein::within(const string& a,
                          const string& b,
                          int max_dist) {
    return distance(a, b, max_dist) <= max_dist;
}

} // namespace fuzzy
