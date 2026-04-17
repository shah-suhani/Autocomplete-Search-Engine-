#include "AbbreviationEngine.hpp"
#include <fstream>
#include <sstream>

namespace expansion {

const std::string AbbreviationEngine::empty_;

void AbbreviationEngine::add(const std::string& abbrev,
                              const std::string& expansion) {
    table_[abbrev] = expansion;
}


void AbbreviationEngine::loadFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto pipe = line.find('|');
        if (pipe == std::string::npos) continue;

        std::string abbrev = line.substr(0, pipe);
        std::string expansion = line.substr(pipe + 1);

        auto trim = [](std::string& s) {
            size_t a = s.findFirstNotOf(" \t\r");
            size_t b = s.findLastNotOf(" \t\r");
            s = (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
        };
        trim(abbrev);
        trim(expansion);

        if (!abbrev.empty() && !expansion.empty())
            table_[abbrev] = expansion;
    }
}

void AbbreviationEngine::saveToFile(const std::string& path) const {
    std::ofstream f(path);
    for (const auto& [abbrev, exp] : table_)
        f << abbrev << " | " << exp << '\n';
}

bool AbbreviationEngine::has(const std::string& abbrev) const {
    return table_.count(abbrev) > 0;
}

const std::string& AbbreviationEngine::expand(const std::string& abbrev) const {
    auto it = table_.find(abbrev);
    return (it == table_.end()) ? empty_ : it->second;
}

}
