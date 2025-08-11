#pragma once

#include <functional>
#include <string>
#include <vector>

#include "../common/SatelliteView.h"
#include "../common/GameResult.h"

class GameResultWriter {

public:

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
    void writeComparativeResults(
        const std::string& game_managers_folder,
        const std::string& game_map_path,
        const std::string& algo1_so,
        const std::string& algo2_so,
        const std::vector<std::pair<std::string, GameResult>>& results,
        const std::function<std::vector<std::string>(const SatelliteView&)>& renderGameState
    );

private:


    std::string reasonToString(GameResult::Reason r);
    std::string resultMessage(const GameResult& gr);
    std::string joinLines(const std::vector<std::string>& lines);
    std::string justFilename(const std::string& path);
};

