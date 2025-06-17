#include "Utils.h"
#include <fstream>
#include <cctype>
#include <iostream>
#include "Wall.h"
#include "Mine.h"
#include "Tank.h"
#include "Shell.h"
#include "Logger.h"
#include "Point.h"
#include <functional>

// This file contains utility functions for reading and parsing map data files in a game

std::string trim(const std::string& str) {
    // Removes leading and trailing whitespace from a string.
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) { // String is all whitespace
        return ""; 
    }
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

int parseKeyValue (const std::string& line, std::string key , std::vector<std::string>& errors, int num_line, bool& isValid) {
    // Parses a key-value pair from a line.// Parses a key-value pair from a line.
    size_t equal_pos = line.find('=');
    if (equal_pos == std::string::npos) {
        errors.push_back("Line " + std::to_string(num_line) + ": missing '=' in line '" + line + "'");
        isValid = false; 
        return -1;
    }
    std::string k = trim(line.substr(0, equal_pos));
    std::string v = trim(line.substr(equal_pos + 1)); 
    if ( k != key ) {
        errors.push_back("Line " + std::to_string(num_line) + ": expected key '" + key + "', found '" + k + "'");
        isValid = false;
        return -1;
    } 
    try {
        return std::stoi(v); // Convert value to integer
    } catch (...) {
        errors.push_back("Line " + std::to_string(num_line) + ": invalid value '" + v + "' for key '" + key + "'");
        isValid = false;
        return -1; // Return -1 on failure
    }
}


std::unique_ptr<GameBoard> readFile(const std::string& filename, std::vector<std::string>& errors) {
    // Reads and parses the game board from a file.
    std::ifstream input_file(filename);
    bool isValid = true; // Flag to indicate if GameBoard is valid
    if (!input_file.is_open()) {
        errors.push_back("Failed to open file " + filename + " File does not exist or is not accessible.");
        isValid = false; 
        return nullptr;
    }
    int maxSteps, numShells, rows, cols;
    std::string skip_line;
    std::getline(input_file, skip_line); // Skip the first line (map name)
    if (!readHeadersLine(input_file, maxSteps, numShells, rows, cols, errors, isValid)) {
        isValid = false; 
        return nullptr; 
    }
    std::vector<std::unique_ptr<GameObject>> objects;
    std::unordered_map<Point, GameObject*> object_at;
    std::vector<Tank*> player1_tanks;
    std::vector<Tank*> player2_tanks;
    readMapBody(input_file, rows, cols, objects, object_at, player1_tanks, player2_tanks, errors, numShells);

    if (!errors.empty()) {
        std::ofstream errorFile("input_errors.txt");
        if (errorFile.is_open()) {
            for (const std::string& msg : errors) {
                errorFile << msg << std::endl;
            }
            errorFile.close();
        } else {
            std::cerr << "Failed to write to input_errors.txt" << std::endl;
        }
    }
    if (!isValid) 
        return nullptr;
    return std::make_unique<GameBoard>(rows, cols, maxSteps, std::move(objects), std::move(object_at), player1_tanks, player2_tanks);
}


bool readHeadersLine(std::ifstream& input_file,int& maxSteps, int& numShells, int& rows, int& cols, std::vector<std::string>& errors, bool& isValid) {
    // Reads the header line of the map file.
    std::string line;;
    maxSteps = parseKeyValue((std::getline(input_file, line), line), "MaxSteps", errors, 2, isValid);
    if (maxSteps <= 0) {
        errors.push_back("Invalid or missing MaxSteps value.");
        isValid = false; // Set isValid to false if MaxSteps is invalid
    }
    numShells = parseKeyValue((std::getline(input_file, line), line), "NumShells", errors, 3, isValid);
    if (numShells < 0) {
        errors.push_back("Invalid value for NumShells, must be >= 0");
        isValid = false;
    }
    rows = parseKeyValue((std::getline(input_file, line), line), "Rows", errors, 4, isValid);
    if (rows <= 0) {
        errors.push_back("Invalid or missing Rows value.");
        isValid = false; // Set isValid to false if Rows is invalid
    }
    cols = parseKeyValue((std::getline(input_file, line), line), "Cols", errors, 5, isValid);
    if (cols <= 0) {
        errors.push_back("Invalid or missing Cols value.");
        isValid = false; // Set isValid to false if Cols is invalid
    }

    return isValid;
}

void readMapBody(std::ifstream& input_file, int rows, int cols, std::vector<std::unique_ptr<GameObject>>& objects,
    std::unordered_map<Point, GameObject*>& object_at ,std::vector<Tank*>& player1_tanks,
    std::vector<Tank*>& player2_tanks,std::vector<std::string>& errors, int& numShells) {
    // Reads the map body and populates game objects and tank vectors.
    for (int row = 0; row < rows; ++row) {
        std::string line = normalizeLine(input_file, cols, row + 6, errors);
        for (int col = 0; col < cols; ++col) {
            handleCell(line[col], row, col, numShells, objects, object_at, player1_tanks, player2_tanks, errors);
        }
    }
}


std::string normalizeLine(std::ifstream& input_file, int expected_len, int line_num, std::vector<std::string>& errors) {
    // Normalizes a line from the input file to the expected length.
    std::string line;
    if (!std::getline(input_file, line)) {
        errors.push_back("Line " + std::to_string(line_num) + ": missing, padding with spaces.");
        return std::string(expected_len, ' ');
    }

    if ((int)line.length() < expected_len) {
        errors.push_back("Line " + std::to_string(line_num) + " is too short, padding with spaces.");
        line += std::string(expected_len - line.size(), ' ');
    } else if ((int)line.length() > expected_len) {
        errors.push_back("Line " + std::to_string(line_num) + " is too long, trimming.");
        line = line.substr(0, expected_len);
    }

    return line;
}


void handleCell(char cell, int row, int col, int numShells, std::vector<std::unique_ptr<GameObject>>& objects,
                std::unordered_map<Point, GameObject*>& object_at,std::vector<Tank*>& player1_tanks,
                std::vector<Tank*>& player2_tanks,std::vector<std::string>& errors) {
    // This function handles a single cell in the map and creates the corresponding game object.
    Point pos(row, col);

    std::unique_ptr<GameObject> obj;

    if (cell == '#') {
        obj = std::make_unique<Wall>(row, col);
    } else if (cell == '@') {
        obj = std::make_unique<Mine>(row, col);
    } else if (cell == '1') {
        auto tank = std::make_unique<Tank>(row, col, 1,1, numShells);
        player1_tanks.push_back(tank.get());
        obj = std::move(tank);
    } else if (cell == '2') {
        auto tank = std::make_unique<Tank>(row, col, 2,2, numShells);
        player2_tanks.push_back(tank.get());
        obj = std::move(tank);
    } else {
        if (!std::isspace(cell)) {
            errors.push_back("Line " + std::to_string(row + 6) +
                            ": illegal character '" + std::string(1, cell) + "' ignored.");
        }
    }
    
    if (obj) {
        object_at[pos] = obj.get();
        objects.push_back(std::move(obj));
    }
}
