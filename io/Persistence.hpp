#pragma once
#include "../ranking/UserProfile.hpp"
#include <string>

namespace io {

class Persistence {
public:
    static std::string profilePath(const std::string& userId);
    static void saveProfile(const ranking::UserProfile& profile);
    static void loadProfile(ranking::UserProfile& profile);
};

}
