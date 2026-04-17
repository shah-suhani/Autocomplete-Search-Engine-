#pragma once
#include "../core/TrieNode.hpp"
#include <string>
#include <vector>

namespace io {

class DictionaryLoader {
public:
    static std::vector<core::TermInfo> load(const std::string& path);

    static void save(const std::string& path,
                     const std::vector<core::TermInfo>& dict);
};

} 
