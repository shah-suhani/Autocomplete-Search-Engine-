#pragma once
#include "../ranking/UserProfile.hpp"
#include <string>

namespace io {

class Persistence {
public:
    static std::string profile_path(const std::string& user_id);
    static void save_profile(const ranking::UserProfile& profile);
    static void load_profile(ranking::UserProfile& profile);
};

} 