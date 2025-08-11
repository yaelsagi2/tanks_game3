//
// Created by geva on 11/08/2025.
//

#include "GameResultWriter.h"
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <filesystem>

// Assumes your GameResult & SatelliteView as given by you:
// struct GameResult { int winner; enum Reason { ALL_TANKS_DEAD, MAX_STEPS, ZERO_SHELLS }; Reason reason;
//                     std::vector<size_t> remaining_tanks; std::unique_ptr<SatelliteView> gameState; size_t rounds; };

std::string GameResultWriter::reasonToString(GameResult::Reason r) {
    switch (r) {
        case GameResult::ALL_TANKS_DEAD: return "ALL_TANKS_DEAD";
        case GameResult::MAX_STEPS:      return "MAX_STEPS";
        case GameResult::ZERO_SHELLS:    return "ZERO_SHELLS";
    }
    return "UNKNOWN";
}

 std::string GameResultWriter::resultMessage(const GameResult& gr) {
    if (gr.winner == 0) {
        return std::string("Tie: ") + reasonToString(gr.reason);
    }
    return std::string("Player ") + std::to_string(gr.winner) + " won: " + reasonToString(gr.reason);
}

 std::string GameResultWriter::joinLines(const std::vector<std::string>& lines) {
    std::ostringstream oss;
    for (size_t i = 0; i < lines.size(); ++i) {
        oss << lines[i];
        if (i + 1 < lines.size()) oss << "\n";
    }
    return oss.str();
}

 std::string GameResultWriter::justFilename(const std::string& path) {
    try {
        return std::filesystem::path(path).filename().string();
    } catch (...) {
        // Fallback if filesystem throws for some odd reason
        auto pos = path.find_last_of("/\\");
        return (pos == std::string::npos) ? path : path.substr(pos + 1);
    }
}

/**
 * Writes the comparative results file (or prints to screen if file creation fails).
 *
 * @param game_managers_folder folder where the output file should be created
 * @param game_map_path path to the single game map file (we will print only its filename)
 * @param algo1_so path to algorithm1 .so (we will print only its filename)
 * @param algo2_so path to algorithm2 .so (we will print only its filename)
 * @param results vector of (game_manager_name, GameResult)
 * @param renderGameState function that renders a SatelliteView into lines (final screen state)
 */
void GameResultWriter::writeComparativeResults(
    const std::string& game_managers_folder,
    const std::string& game_map_path,
    const std::string& algo1_so,
    const std::string& algo2_so,
    const std::vector<std::pair<std::string, GameResult>>& results,
    const std::function<std::vector<std::string>(const SatelliteView&)>& renderGameState
) {
    // 1) Pre-render maps and build signatures to group identical outcomes.
    struct Bucket {
        std::vector<std::string> names;     // game manager names in this bucket
        GameResult representative;          // for message/rounds/map printing
        std::vector<std::string> finalMap;  // cached final map lines
    };

    std::unordered_map<std::string, Bucket> buckets; // signature -> bucket
    buckets.reserve(results.size() * 2 + 1);

    for (auto& [gmName, gr] : results) {
        // Render final map (empty if no state)
        std::vector<std::string> mapLines;
        if (gr.gameState) {
            mapLines = renderGameState(*gr.gameState);
        }

        // Build a signature that uniquely identifies the "exact same final result"
        // We include: winner, reason, rounds, and the full final map text.
        std::ostringstream sig;
        sig << "W:" << gr.winner
            << "|R:" << static_cast<int>(gr.reason)
            << "|T:" << gr.rounds
            << "|M:\n" << joinLines(mapLines);
        const std::string key = sig.str();

        auto& bucket = buckets[key];
        if (bucket.names.empty()) {
            bucket.representative = gr;      // first one becomes representative
            bucket.finalMap = mapLines;      // cache map for printing
        }
        bucket.names.push_back(gmName);
    }

    // 2) Move buckets to a vector and sort by descending group size, then by first name for determinism.
    std::vector<Bucket> groups;
    groups.reserve(buckets.size());
    for (auto& kv : buckets) {
        auto& b = kv.second;
        std::sort(b.names.begin(), b.names.end()); // alphabetical within group
        groups.push_back(std::move(b));
    }
    std::sort(groups.begin(), groups.end(), [](const Bucket& a, const Bucket& b) {
        if (a.names.size() != b.names.size()) return a.names.size() > b.names.size();
        return a.names.front() < b.names.front();
    });

    // 3) Build the full output content in memory first.
    std::ostringstream out;

    // Lines 1-4 (headers)
    out << "game_map="  << justFilename(game_map_path) << "\n";
    out << "algorithm1=" << justFilename(algo1_so) << "\n";
    out << "algorithm2=" << justFilename(algo2_so) << "\n";
    out << "\n";

    // For each group: lines 5-8 + blank line between groups
    for (size_t gi = 0; gi < groups.size(); ++gi) {
        const auto& g = groups[gi];
        // 5th line: comma-separated biggest list of <game_manager> names with identical result
        for (size_t i = 0; i < g.names.size(); ++i) {
            if (i) out << ",";
            out << g.names[i];
        }
        out << "\n";

        // 6th line: game result message (as in assignment 2)
        out << resultMessage(g.representative) << "\n";

        // 7th line: round number (just a number)
        out << g.representative.rounds << "\n";

        // 8th line on: full map of the final state
        if (!g.finalMap.empty()) {
            for (const auto& line : g.finalMap) out << line << "\n";
        }

        // If there are more groups, add an empty line between them
        if (gi + 1 < groups.size()) out << "\n";
    }

    const std::string content = out.str();

    // 4) Build filename with a monotonically increasing time-based number.
    // Use nanoseconds since epoch to reduce collision risk.
    const auto now = std::chrono::time_point_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now()
    );
    const auto tnum = now.time_since_epoch().count();

    std::string filename = "comparative_results_" + std::to_string(tnum) + ".txt";
    std::string fullpath;
    try {
        std::filesystem::path dir(game_managers_folder);
        fullpath = (dir / filename).string();
    } catch (...) {
        // If folder path is weird, just try to write in place
        fullpath = filename;
    }

    // 5) Try writing to file; on failure, print error and dump content to screen.
    std::ofstream ofs(fullpath, std::ios::out | std::ios::trunc);
    if (!ofs) {
        std::cerr << "Error: could not create output file at '" << fullpath
                  << "'. Printing results to screen instead.\n";
        std::cout << content;
        return;
    }
    ofs << content;
    ofs.close();
}