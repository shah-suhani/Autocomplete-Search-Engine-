#pragma once
#include "../ranking/UserProfile.hpp"
#include <string>
using namespace std;

namespace io {

class Persistence {
public:
    static string profilePath(const string& userId);

    static void saveProfile(const ranking::UserProfile& profile);

    static void loadProfile(ranking::UserProfile& profile);
};

} 
