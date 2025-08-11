#include "Simulator.h"
#include "MapParser.h"
#include "ArgsParser.h"
#include "Loader.h"
#include "GameManagerRegistrar.h"
#include "AlgorithmRegistrar.h"
#include "../GameBoardSatelliteView.h"

#include <map>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <string>
#include <set>
#include <dirent.h> // For directory operations
#include <dlfcn.h>  // For dynamic loading of shared 
#include <iostream> // For input/output operations
#include "GameResultWriter.h"

using namespace std;

using namespace Simulator_206480972_206899163;

int main(int argc, char* argv[]) {
    try {
	Simulator simulator;
        ParsedArgs args = parseArgs(argc, argv);
        if (args.mode == ParsedArgs::Mode::Comparative) {
            simulator.runComparativeMode(args);
        } else if (args.mode == ParsedArgs::Mode::Competition) {
            simulator.runCompetitionMode(args);
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

Simulator::Simulator() {
}

void Simulator::runComparativeMode(const ParsedArgs& args) {
    // Implementation for comparative mode
    // Load game managers and algorithms, run simulations, etc.
    std::vector<std::string> errors;
    auto map_data = readMapFile(args.game_map, errors);
    int player1_index = 1;
    int player2_index = 2;

    auto& play_and_algorithm_registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    auto& game_managers_registrar = GameManagerRegistrar::getGameManagerRegistrar();

    // run all game manager, and give each one the same algorithms and map
    auto algorithm1 = play_and_algorithm_registrar.getAlgorithmRegistrar().getAt(0);
    auto algorithm2 = play_and_algorithm_registrar.getAlgorithmRegistrar().getAt(1);
    auto satellite_view = GameBoardSatelliteView(&map_data);

    std::vector<GameResult> results;

    for (auto& factory : game_managers_registrar) {
        auto manager = factory(args.verbose); // Create a game manager instance
        auto player1 = *algorithm1.getPlayerFactory()(player1_index, map_data.length,
                                                      map_data.height, map_data.max_steps, map_data.num_shells);
        auto player2 = *algorithm2.getPlayerFactory()(player2_index, map_data.length,
                                                      map_data.height, map_data.max_steps, map_data.num_shells);

        auto game_result = manager->run(map_data.height, map_data.length, satellite_view,
                                        map_data.max_steps, map_data.num_shells, player1, player2,
                                        algorithm1.getTankAlgorithmFactory(), algorithm2.getTankAlgorithmFactory());

        //results.push_back(game_result);
    }
/*
    GameResultWriter::writeComparativeResults(args.game_managers_folder,
                                              args.game_map,
                                              args.algorithm1,
                                              args.algorithm2,
                                              results,
                                              ???);*/
}

void Simulator::runCompetitionMode(const ParsedArgs& args) {
    // we expect to have exactly 2 players/algorithms
    auto& play_and_algorithm_registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    // we expect to have exactly one game manager
    auto& game_managers_registrar = GameManagerRegistrar::getGameManagerRegistrar();
    std::vector<std::string> map_names = getAllMapNames(args.game_maps_folder);
    // we calculate the matchups. each matchup contains a map index as a key, and a pair of 
    // two players/algorithms as the value
    std::map <int, std::vector<std::pair<int, int>>> matchups;
    for (int k = 0; k < map_names.size(); ++k) {
        matchups[k] = pairs_for_map((int)play_and_algorithm_registrar.count(), k);
    }

    // create the game manager
    auto game_manager = game_managers_registrar.getAt(0)();

    // run matchups for each map
    for (auto& matchup : matchups) {
        // read the map from the list 
	std::vector<std::string> errors;
        auto map_info = readMapFile(map_names[matchup.first], errors);
        
        // on the given map, run all matchups of all players pairs
        for (auto [player1_index, player2_index] : matchup.second) {
            auto algorithm_player1 = play_and_algorithm_registrar.getAlgorithmRegistrar().getAt(player1_index);
            auto algorithm_player2 = play_and_algorithm_registrar.getAlgorithmRegistrar().getAt(player2_index);

            auto player1 = *algorithm_player1.getPlayerFactory()(player1_index, map_info.length,
                map_info.height, map_info.max_steps, map_info.num_shells);
            auto player2 = *algorithm_player2.getPlayerFactory()(player2_index, map_info.length,
                map_info.height, map_info.max_steps, map_info.num_shells);

            auto sattlite_view = GameBoardSatelliteView(&map_info);

            auto game_result = game_manager->run(
                map_info.length,
                map_info.height,
                sattlite_view,
                map_info.max_steps,
                map_info.num_shells,
                player1,
                player2,
                algorithm_player1.getTankAlgorithmFactory(),
                algorithm_player2.getTankAlgorithmFactory()
            );



            write_game_result_to_file(game_result);
        }
    }
}


std::vector<std::string> Simulator::getAllMapNames(const std::string& folder_path) {
    std::vector<std::string> map_names;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(folder_path)) {
            if (entry.is_regular_file()) {
                map_names.push_back(entry.path().filename().string());
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing folder '" << folder_path
            << "': " << e.what() << "\n";
    }
    return map_names;
}


std::vector<std::pair<int, int>> Simulator::pairs_for_map(int number_of_algorithms, int number_of_maps) {
    int d = 1 + (int)(number_of_maps % (number_of_algorithms - 1));
    std::set<std::pair<int, int>> uniq;
    for (int i = 0; i < number_of_algorithms; ++i) {
        int j1 = modp(i + d, number_of_algorithms);
        int j2 = modp(i - d, number_of_algorithms);
        if (i != j1) uniq.insert({ min(i, j1), max(i, j1) });
        if (i != j2) uniq.insert({ min(i, j2), max(i, j2) });
    }
    return std::vector<std::pair<int, int>>(uniq.begin(), uniq.end());
}
