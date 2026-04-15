#include "AbbreviationEngine.hpp"
#include <fstream>
#include <string>
using namespace std;

namespace expansion {

const string AbbreviationEngine::kEmpty_;

static void trimInplace(string& s) {
    const char* ws = " \t\r";
    size_t first = s.find_first_not_of(ws);
    if (first == string::npos) {
        s.clear();
        return;
    }
    size_t last = s.find_last_not_of(ws);
    s = s.substr(first, last - first + 1);
}

void AbbreviationEngine::add(const string& abbrev,
                              const string& expansion) {
    table_[abbrev] = expansion;
}

void AbbreviationEngine::loadFromFile(const string& path) {
    ifstream file(path);
    if (!file.is_open())
        return;

    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        auto sep = line.find('|');
        if (sep == string::npos)
            continue;

        string abbrev    = line.substr(0, sep);
        string expansion = line.substr(sep + 1);

        trimInplace(abbrev);
        trimInplace(expansion);

        if (!abbrev.empty() && !expansion.empty())
            table_[abbrev] = expansion;
    }
}

void AbbreviationEngine::saveToFile(const string& path) const {
    ofstream file(path);
    for (const auto& [abbrev, expansion] : table_)
        file << abbrev << " | " << expansion << '\n';
}

bool AbbreviationEngine::has(const string& abbrev) const {
    return table_.count(abbrev) > 0;
}

const string& AbbreviationEngine::expand(const string& abbrev) const {
    auto it = table_.find(abbrev);
    if (it == table_.end())
        return kEmpty_;
    return it->second;
}

}
