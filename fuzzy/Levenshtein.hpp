#pragma once
#include <iostream>
#include <string>
using namespace std;

namespace fuzzy {

class Levenshtein {
public:
    static int distance(const string& a, const string& b, int max_dist = 2);
    static bool within(const string& a, const string& b, int max_dist = 2);
};

} 