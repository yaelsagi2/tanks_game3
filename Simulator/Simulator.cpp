#include "Simulator.h"
#include "MapParser.h"
#include "ArgsParser.h"
#include "Loader.h"
#include "GameManagerRegistrar.h"
#include "AlgorithmRegistrar.h"

#include <map>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype> 
#include <dirent.h> // For directory operations
#include <dlfcn.h>  // For dynamic loading of shared 
#include <iostream> // For input/output operations

using namespace Simulator_206480972_206899163;

int main(int argc, char* argv[]) {
    try {
        ParsedArgs args = parseArgs(argc, argv);
        if (args.mode == ParsedArgs::Mode::Comparative) {
            runComparativeMode(args);
        } else if (args.mode == ParsedArgs::Mode::Competition) {
            runCompetitionMode(args);
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
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
    auto map_info = readMapFile(args.game_map);
    auto satellite_view = GameBoardSatelliteView(*map_info);

    for (auto& factory : game_managers_registrar) {
        auto manager = factory(args.verbose); // Create a game manager instance
        auto player1 = *algorithm1.getPlayerFactory()(player1_index, map_info.length, 
                                                      map_info.length, map_info.max_steps, map_info.num_shells);
        auto player2 = *algorithm2.getPlayerFactory()(player2_index, map_info.length, 
                                                      map_info.length, map_info.max_steps, map_info.num_shells);

        auto game_result = manager->run(map_data->cols, map_data->rows, satellite_view,
                                        map_info.max_steps, map_info.num_shells, player1, player2,
                                        algorithm1.getTankAlgorithmFactory(), algorithm2.getTankAlgorithmFactory());
        write_game_result_to_file(game_result);
    }
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
        auto map_info = readMapFile(map_names[matchup.first]);
        
        // on the given map, run all matchups of all players pairs
        for (auto [player1_index, player2_index] : matchup.second) {
            auto algorithm_player1 = play_and_algorithm_registrar.getAlgorithmRegistrar().getAt(player1_index);
            auto algorithm_player2 = play_and_algorithm_registrar.getAlgorithmRegistrar().getAt(player2_index);

            auto player1 = *algorithm_player1.getPlayerFactory()(player1_index, map_info.length,
                map_info.length, map_info.max_steps, map_info.num_shells);
            auto player2 = *algorithm_player2.getPlayerFactory()(player2_index, map_info.length,
                map_info.length, map_info.max_steps, map_info.num_shells);

            auto game_result = game_manager->run(
                map_info.length,
                map_info.height,
                *(map_info.view),
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


// void Simulator::testMultipleMaps(const std::vector<std::string>& map_files) {
//     for (const auto& file : map_files) {
//         std::vector<std::string> errors;
//         std::optional<Map> map = readMapFile(file, errors);

//         std::cout << "Testing map: " << file << "\n";

//         if (!map) {
//             std::cout << "Critical error. Map is invalid.\n";
//         } else {
//             std::cout << "Map loaded: "
//                       << map->rows << "x" << map->cols
//                       << ", MaxSteps = " << map->maxSteps
//                       << ", NumShells = " << map->numShells << "\n";
//         }

//         if (!errors.empty()) {
//             std::cout << "Warnings/Recoverable errors:\n";
//             for (const auto& e : errors) {
//                 std::cout << " - " << e << "\n";
//             }
//         }

//         std::cout << "-----------------------------\n";
//     }
// }
