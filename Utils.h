#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include "Point.h"
#include <fstream>
#include <memory>
#include "Tank.h"
#include "Wall.h"  
#include "Mine.h"
#include "Shell.h"
#include "GameBoard.h"
#include "Direction.h"
// This header file contains utility functions for reading and parsing map data files in a game

/**
 * @struct MapData
 * @brief Structure to hold parsed map data from the input file.
 *
 * Contains map name, board dimensions, number of steps, number of shells,
 * positions of walls, mines, and tanks for both players, and a validity flag.
 */
struct MapData {
    std::string name;
    int maxSteps = 0;
    int numShells = 0;
    int rows = 0;
    int cols = 0;
    std::vector<Point> wallPosition;
    std::vector<Point> minesPosition;
    std::vector<Point> player1_tank_position;
    std::vector<Point> player2_tank_position;
    bool isValid = true; // Flag to indicate if the map data is valid
};

/**
 * @brief Removes leading and trailing whitespace from a string.
 * @param str The string to trim.
 * @return The trimmed string.
 */
std::string trim(const std::string& str);

/**
 * @brief Parses a key-value pair from a line.
 * @param line The line to parse.
 * @param key The expected key.
 * @param errors Vector to store error messages.
 * @param num_line The line number in the file.
 * @param isValid Reference to a flag indicating validity.
 * @return The parsed integer value, or -1 on error.
 */
int parseKeyValue(const std::string& line, std::string key, std::vector<std::string>& errors, int num_line, bool& isValid);

/**
 * @brief Reads and parses the game board from a file.
 * @param filename The path to the input file.
 * @param errors Vector to store error messages.
 * @return A unique pointer to the created GameBoard, or nullptr on error.
 */
std::unique_ptr<GameBoard> readFile(const std::string& filename, std::vector<std::string>& errors);

/**
 * @brief Reads the header line of the map file.
 * @param input_file The input file stream.
 * @param maxSteps Output for the maximum number of steps.
 * @param numShells Output for the number of shells.
 * @param rows Output for the number of rows.
 * @param cols Output for the number of columns.
 * @param errors Vector to store error messages.
 * @param isValid Reference to a flag indicating validity.
 * @return True if headers are valid, false otherwise.
 */
bool readHeadersLine(std::ifstream& input_file, int& maxSteps, int& numShells, int& rows, int& cols, std::vector<std::string>& errors, bool& isValid);

/**
 * @brief Reads the map body and populates game objects and tank vectors.
 * @param input_file The input file stream.
 * @param rows Number of rows in the map.
 * @param cols Number of columns in the map.
 * @param objects Vector to store created game objects.
 * @param object_at Map from Point to GameObject pointer.
 * @param player1_tanks Vector to store player 1 tanks.
 * @param player2_tanks Vector to store player 2 tanks.
 * @param errors Vector to store error messages.
 * @param numShells Number of shells for tanks.
 */
void readMapBody(std::ifstream& input_file, int rows, int cols,
    std::vector<std::unique_ptr<GameObject>>& objects,
    std::unordered_map<Point, GameObject*>& object_at,
    std::vector<Tank*>& player1_tanks,
    std::vector<Tank*>& player2_tanks,
    std::vector<std::string>& errors, int& numShells);

/**
 * @brief Normalizes a line from the input file to the expected length.
 * @param input_file The input file stream.
 * @param expected_len The expected length of the line.
 * @param line_num The line number in the file.
 * @param errors Vector to store error messages.
 * @return The normalized line.
 */
std::string normalizeLine(std::ifstream& input_file, int expected_len, int line_num, std::vector<std::string>& errors);

/**
 * @brief Handles a single cell in the map and creates the corresponding game object.
 * @param cell The character representing the object.
 * @param row The row index.
 * @param col The column index.
 * @param numShells Number of shells for tanks.
 * @param objects Vector to store created game objects.
 * @param object_at Map from Point to GameObject pointer.
 * @param player1_tanks Vector to store player 1 tanks.
 * @param player2_tanks Vector to store player 2 tanks.
 * @param errors Vector to store error messages.
 */
void handleCell(char cell, int row, int col, int numShells,
                std::vector<std::unique_ptr<GameObject>>& objects,
                std::unordered_map<Point, GameObject*>& object_at,
                std::vector<Tank*>& player1_tanks,
                std::vector<Tank*>& player2_tanks, std::vector<std::string>& errors);

#endif // UTILS_H
