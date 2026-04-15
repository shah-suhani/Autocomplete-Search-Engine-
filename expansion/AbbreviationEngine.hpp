#pragma once
#include <unordered_map>
#include <string>
using namespace std;

namespace expansion {

class AbbreviationEngine {
public:
    void add(const string& abbrev, const string& expansion);

    void loadFromFile(const string& path);

    void saveToFile(const string& path) const;

    bool has(const string& abbrev) const;

    const string& expand(const string& abbrev) const;

private:
    unordered_map<string, string> table_;

    static const string kEmpty_;
};

} 
