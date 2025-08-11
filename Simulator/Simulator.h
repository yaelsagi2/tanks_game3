#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "common/SatelliteView.h"

namespace Simulator_206480972_206899163 {
/**
 * @brief Simulator class for running
**/
class Simulator {
public:
    Simulator(int argc, char* argv[]);
    void run(); 
private:
    static inline int modp(int x, int m) { return (x % m + m) % m; }


    // void parseArguments(int argc, char* argv[]);
    // void loadSharedObjects();
    // void unloadSharedObjects();
    

    void runComparativeMode(const ParsedArgs& args);
    void runCompetitionMode(const ParsedArgs& args);
    std::vector<std::string> getAllMapNames(const std::string& folder_path);
    // std::string getTimestamp(); 

};
} // namespace Simulator_206480972_206899163
