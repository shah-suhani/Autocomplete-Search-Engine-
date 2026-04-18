#pragma once
#include <unordered_map>
#include <vector>
#include <string>

namespace expansion {


class AbbreviationEngine {
public:
    void add(const std::string& abbrev, const std::string& expansion);
    void load_from_file(const std::string& path);
    void save_to_file (const std::string& path) const;

    bool               has   (const std::string& abbrev) const;
    const std::string& expand(const std::string& abbrev) const; // returns "" on miss

private:
    std::unordered_map<std::string, std::string> table_;
    static const std::string empty_;
};

} 