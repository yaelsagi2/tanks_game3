#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "../common/SatelliteView.h"
#include "ArgsParser.h"
#include "../common/GameResult.h"

namespace Simulator_206480972_206899163 {
/**
 * @brief Simulator class for running
**/
class Simulator {
public:
    Simulator();
    void run(); 

    void loadSharedObjects();
    void unloadSharedObjects();
    
    static void runComparativeMode(const ParsedArgs& args);
    static void runCompetitionMode(const ParsedArgs& args);
    static std::vector<std::string> getAllMapNames(const std::string& folder_path);

private:
    static int modp(int x, int m) { return (x % m + m) % m; }

    static std::vector<std::pair<int, int>> pairs_for_map(int number_of_algorithms, int number_of_maps);

    static void write_game_result_to_file(const GameResult& gameResult);
};
} // namespace Simulator_206480972_206899163
