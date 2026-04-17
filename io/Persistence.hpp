#pragma once
#include "../ranking/UserProfile.hpp"
#include <string>

namespace io {

class Persistence {
public:
    static std::string profilePath(const std::string& userId);
    static void save_profile(const ranking::UserProfile& profile);
    static void load_profile(ranking::UserProfile& profile);
};

}
