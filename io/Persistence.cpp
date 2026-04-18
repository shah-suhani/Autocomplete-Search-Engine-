#include "Persistence.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace io {

std::string Persistence::profile_path(const std::string& user_id) {
    return "data/" + user_id + ".profile";
}


void Persistence::save_profile(const ranking::UserProfile& profile) {
    std::filesystem::create_directories("data");
    std::ofstream f(profile_path(profile.user_id()));
    if (!f.is_open()) return;

    f << "STATS\n";
    for (const auto& [id, stats] : profile.all_stats())
        f << id << ' ' << stats.select_count << ' '
          << static_cast<long long>(stats.last_used) << '\n';

    f << "HIDDEN\n";
    for (int id : profile.hidden_set())
        f << id << '\n';
}

void Persistence::load_profile(ranking::UserProfile& profile) {
    std::ifstream f(profile_path(profile.user_id()));
    if (!f.is_open()) return;

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
    profile.load_stats(std::move(stats), std::move(hidden));
}

} 