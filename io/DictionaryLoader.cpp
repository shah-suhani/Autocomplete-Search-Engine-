#include "DictionaryLoader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
using namespace std;

namespace io {

vector<core::TermInfo> DictionaryLoader::load(const string& path) {
    ifstream file(path);
    if (!file.is_open())
        throw runtime_error("DictionaryLoader: cannot open " + path);

    vector<core::TermInfo> dict;
    dict.reserve(1000);

    string line;
    while (getline(file, line)) {
        // Skip blank lines and comment lines.
        if (line.empty() || line[0] == '#')
            continue;

        istringstream ss(line);
        string word;
        int freq = 10;
        ss >> word >> freq;

        if (word.empty())
            continue;

        transform(word.begin(), word.end(), word.begin(),
                       [](unsigned char c) { return tolower(c); });

        dict.push_back({move(word), freq});
    }
    return dict;
}

void DictionaryLoader::save(const string& path,
                             const vector<core::TermInfo>& dict) {
    ofstream file(path);
    if (!file.is_open())
        throw runtime_error("DictionaryLoader: cannot write " + path);

    for (const auto& entry : dict) {
        if (!entry.term.empty())
            file << entry.term << ' ' << entry.globalFrequency << '\n';
    }
}

}
