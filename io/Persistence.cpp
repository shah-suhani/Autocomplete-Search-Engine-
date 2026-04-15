#include "Persistence.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
using namespace std;

namespace io {

string Persistence::profilePath(const string& userId) {
    return "data/" + userId + ".profile";
}

void Persistence::saveProfile(const ranking::UserProfile& profile) {
    filesystem::create_directories("data");

    ofstream file(profilePath(profile.userId()));
    if (!file.is_open())
        return;

    file << "STATS\n";
    for (const auto& [id, stats] : profile.allStats()) {
        file << id << ' '
             << stats.selectCount << ' '
             << static_cast<long long>(stats.lastUsed) << '\n';
    }

    file << "HIDDEN\n";
    for (int id : profile.hiddenSet())
        file << id << '\n';
}

void Persistence::loadProfile(ranking::UserProfile& profile) {
    ifstream file(profilePath(profile.userId()));
    if (!file.is_open())
        return;

    unordered_map<int, ranking::UserStats> stats;
    unordered_set<int> hidden;

    enum class Section { NONE, STATS, HIDDEN } section = Section::NONE;

    string line;
    while (getline(file, line)) {
        if (line == "STATS")  { section = Section::STATS;  continue; }
        if (line == "HIDDEN") { section = Section::HIDDEN; continue; }
        if (line.empty())     continue;

        istringstream ss(line);

        if (section == Section::STATS) {
            int id, count;
            long long ts;
            ss >> id >> count >> ts;
            stats[id] = { count, static_cast<time_t>(ts) };
        } else if (section == Section::HIDDEN) {
            int id;
            ss >> id;
            hidden.insert(id);
        }
    }

    profile.loadStats(move(stats), move(hidden));
}

}