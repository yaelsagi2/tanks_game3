#include "GameManager.h"
#include "MyPlayerFactory.h"
#include "MyTankAlgorithmFactory.h"


int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <input_file_path>\n";
            return 0;
        }
        const std::string filename = argv[1];
        GameManager game(MyPlayerFactory{}, MyTankAlgorithmFactory{});
        bool flag = game.readBoard(filename);
        if (!flag) {
            std::cerr << "Failed to read the game board. Exiting.\n";
            return 0;
        }
        game.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 0;
    } catch (...) {
        std::cerr << "An unknown error occurred.\n";
        return 0;
    }
}

