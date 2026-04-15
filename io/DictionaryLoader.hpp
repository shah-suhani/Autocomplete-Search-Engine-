#pragma once
#include "../core/TrieNode.hpp"
#include <string>
#include <vector>
using namespace std;

namespace io {

class DictionaryLoader {
public:
    static vector<core::TermInfo> load(const string& path);

    static void save(const string& path,
                     const vector<core::TermInfo>& dict);
};

}
