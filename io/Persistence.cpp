#include "Persistence.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace io {

std::string Persistence::profilePath(const std::string& userId) {
    return "data/" + userId + ".profile";
}

void Persistence::save_profile(const ranking::UserProfile& profile) {
    std::filesystem::createDirectories("data");
    std::ofstream f(profilePath(profile.userId()));
    if (!f.isOpen()) return;

    f << "STATS\n";
    for (const auto& [id, stats] : profile.allStats())
        f << id << ' ' << stats.selectCount << ' '
          << static_cast<long long>(stats.lastUsed) << '\n';

    f << "HIDDEN\n";
    for (int id : profile.hiddenSet())
        f << id << '\n';
}

void Persistence::load_profile(ranking::UserProfile& profile) {
    std::ifstream f(profilePath(profile.userId()));
    if (!f.isOpen()) return;

    std::unordered_map<int, ranking::UserStats> stats;
    std::unordered_set<int> hidden;

    std::string line;
    enum class Section { NONE, STATS, HIDDEN } sec = Section::NONE;

    while (std::getline(f, line)) {
        if (line == "STATS")  { sec = Section::STATS;  continue; }
        if (line == "HIDDEN") { sec = Section::HIDDEN; continue; }
        if (line.empty())     continue;

        std::istringstream ss(line);
        if (sec == Section::STATS) {
            int id, count; long long ts;
            ss >> id >> count >> ts;
            stats[id] = {count, static_cast<std::time_t>(ts)};
        } else if (sec == Section::HIDDEN) {
            int id; ss >> id;
            hidden.insert(id);
        }
    }
    profile.loadStats(std::move(stats), std::move(hidden));
}

} 
