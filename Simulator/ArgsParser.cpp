// ArgsParser.cpp

#include "ArgsParser.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cassert>
#include <stdexcept>


inline void trim_inplace(std::string& s) {
    // This function trims leading and trailing whitespace from a string in place.
    auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
}

std::pair<std::string,std::string> splitKeyValue(const std::string& arg) {
    // This function splits a key-value pair from a string argument.
    size_t pos = arg.find('=');
    if (pos == std::string::npos)
        throw std::invalid_argument("Missing '=' in argument: " + arg);
    std::string key = arg.substr(0, pos);
    std::string value = arg.substr(pos + 1);
    trim_inplace(key);
    trim_inplace(value);
    return {key, value};
}

void printUsageAndExit(const std::vector<std::string>& missing,
                              const std::vector<std::string>& unsupported) {
    // This function prints the usage instructions and exits the program.
    std::cerr << "Usage:\n"
              << "  Comparative mode:\n"
              << "    -comparative game_map=<file> game_managers_folder=<folder> algorithm1=<file> algorithm2=<file> [num_threads=<n>] [-verbose]\n"
              << "  Competition mode:\n"
              << "    -competition game_maps_folder=<folder> game_manager=<file> algorithms_folder=<folder> [num_threads=<n>] [-verbose]\n\n";

    if (!missing.empty()) {
        std::cerr << "Missing arguments:\n";
        for (auto& m : missing) std::cerr << "  - " << m << "\n";
    }
    if (!unsupported.empty()) {
        std::cerr << "Unsupported arguments:\n";
        for (auto& u : unsupported) std::cerr << "  - " << u << "\n";
    }
    exit(1);
}

ParsedArgs parseArgs(int argc, char* argv[]) {
    // Parses command line arguments and returns a ParsedArgs object.
    ParsedArgs args;
    std::vector<std::string> missing, unsupported;
    parseArgsValues(argc, argv, args, unsupported);
    validateArgs(args, missing);

    if (!missing.empty() || !unsupported.empty())
        printUsageAndExit(missing, unsupported);

    return args;
}

// Parses the values from argv into args, and fills unsupported vector
void parseArgsValues(int argc, char* argv[], ParsedArgs& args, std::vector<std::string>& unsupported) {
    if (argc < 5) {
        throw std::invalid_argument("Not enough arguments provided.");
    }
    if (argc > 8) {
        throw std::invalid_argument("Too many arguments provided.");
    }
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-verbose") {
            args.verbose = true;
        } else if (arg == "-comparative") {
            args.mode = ParsedArgs::Mode::Comparative;
        } else if (arg == "-competition") {
            args.mode = ParsedArgs::Mode::Competition;
        } else {
            try {
                auto [key, value] = splitKeyValue(arg);
                if (key == "num_threads") {
                    args.num_threads = std::stoi(value);
                    if (args.num_threads < 1)
                        unsupported.push_back(arg + " (must be >= 1)");
                } else if (key == "game_map") args.game_map = value;
                else if (key == "game_managers_folder") args.game_managers_folder = value;
                else if (key == "algorithm1") args.algorithm1 = value;
                else if (key == "algorithm2") args.algorithm2 = value;
                else if (key == "game_maps_folder") args.game_maps_folder = value;
                else if (key == "game_manager") args.game_manager_so = value;
                else if (key == "algorithms_folder") args.algorithms_folder = value;
                else unsupported.push_back(key);
            } catch (const std::exception& e) {
                unsupported.push_back(arg + " (" + e.what() + ")");
            }
        }
    }
}


void validateArgs(const ParsedArgs& args, std::vector<std::string>& missing) {
    // This function validates the parsed args and fills missing
    if (args.mode == ParsedArgs::Mode::Comparative) {
        if (args.game_map.empty()) missing.push_back("game_map");
        if (args.game_managers_folder.empty()) missing.push_back("game_managers_folder");
        if (args.algorithm1.empty()) missing.push_back("algorithm1");
        if (args.algorithm2.empty()) missing.push_back("algorithm2");
    } else if (args.mode == ParsedArgs::Mode::Competition) {
        if (args.game_maps_folder.empty()) missing.push_back("game_maps_folder");
        if (args.game_manager_so.empty()) missing.push_back("game_manager");
        if (args.algorithms_folder.empty()) missing.push_back("algorithms_folder");
    } else {
        missing.push_back("mode not specified, use (-comparative or -competition)");
    }
}

