#include "DictionaryLoader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace io {

std::vector<core::TermInfo> DictionaryLoader::load(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("DictionaryLoader: cannot open " + path);

    std::vector<core::TermInfo> dict;
    dict.reserve(1000);

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string word;
        int freq = 10;
        ss >> word >> freq;
        if (word.empty()) continue;

    
        std::transform(word.begin(), word.end(), word.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        dict.push_back({std::move(word), freq});
    }
    return dict;
}

void DictionaryLoader::save(const std::string&                 path,
                             const std::vector<core::TermInfo>& dict) {
    std::ofstream f(path);
    if (!f.is_open())
        throw std::runtime_error("DictionaryLoader: cannot write " + path);

    for (const auto& ti : dict)
        if (!ti.term.empty())           
            f << ti.term << ' ' << ti.global_frequency << '\n';
}

}