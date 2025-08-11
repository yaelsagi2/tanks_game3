#include <vector>
/**
 * @struct MapData
 * @brief Holds the parsed data for a single map file.
 */
struct MapData {
    int maxSteps;                            ///< Maximum number of steps allowed in the game
    int numShells;                           ///< Number of shells available
    int rows;                                ///< Number of rows in the map
    int cols;                               ///< Number of columns in the map
    std::vector<std::vector<char>> grid;    ///< 2D vector representing the map layout

    MapData(int maxSteps, int numShells, int rows, int cols, std::vector<std::vector<char>>&& grid)
        : maxSteps(maxSteps), numShells(numShells), rows(rows), cols(cols), grid(std::move(grid)) {}
};