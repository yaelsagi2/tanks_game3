#include <iostream>
#include "GameBoard.h"
#include "Tank.h"
#include "Wall.h"
#include "Mine.h"
#include "Shell.h"
#include "Point.h"
#include "GameObject.h"
#include "Direction.h"
#include <algorithm>
#include <queue>
#include <cmath>
#include <vector>

// Constructor
GameBoard::GameBoard(int rows, int cols, int maxSteps,
                     std::vector<std::unique_ptr<GameObject>>&& all_objects,
                     std::unordered_map<Point, GameObject*>&& map,
                     std::vector<Tank*>& p1,
                     std::vector<Tank*>& p2)
    : rows(rows), cols(cols), maxSteps(maxSteps),
      objects(std::move(all_objects)),
      object_at(std::move(map)),
      player1_tanks(p1),
      player2_tanks(p2) {}

// Getters
int GameBoard::getCols() const {
     return cols;
}

int GameBoard::getRows() const {
     return rows;
}

int GameBoard::getMaxSteps() const {
     return maxSteps;
}

GameObject* GameBoard::getObjectAt(Point p) const {
    // This function returns the GameObject at a specific position on the board
    // If no object exists at that position, it returns nullptr.
    auto it = object_at.find(p);
    return (it != object_at.end()) ? it->second : nullptr;
}

bool GameBoard::isObjectOnBoard(const GameObject* object) const {
    // This function checks if a GameObject is on the board by checking if its position exists in the object_at map
    if (!object) return false;
    for (auto& ptr: objects){
        if (ptr.get() == object) {
            return true;
        }
    }
    return false;
}

bool GameBoard::isObjectOnBoard(const Point& point) const {
    // This function checks if an object exists at a specific position on the board 
    auto it = object_at.find(point);
    if (it == object_at.end()) {
        return false;
    }
    const GameObject* obj = it->second;
    for (const auto& ptr : objects) {
        if (ptr.get() == obj) {
            return true;
        }
    }

    return false; 
}

void GameBoard::removeObjectAt(Point p) {
    // This function removes the GameObject at a specific position on the board
    // If no object exists at that position, it does nothing.
    object_at.erase(p);
}

std::vector<Tank*>& GameBoard::getPlayerTanks(int player_id) {
    // This function returns the tanks for a specific player
    return (player_id == 1) ? player1_tanks : player2_tanks;
}

void GameBoard::printBoardState() const {
    // This function prints the current state of the game board
    std::vector<std::vector<char>> board(rows, std::vector<char>(cols, ' '));

    for (const auto& [pos, obj] : object_at) {
        if(!this->isObjectOnBoard(obj)) {
            board[pos.getX()][pos.getY()] = ' '; // If the object is not on the board, leave it empty
        }
        else {
            board[pos.getX()][pos.getY()] = obj->toChar();
        }
    }

    for (int x = 0; x < rows; ++x) {
        for (int y = 0; y < cols; ++y) {
            std::cout << board[x][y];
        }
        std::cout << '\n';
    }
}

bool GameBoard::isWallAt(const Point& pos) const {
    // This function checks if there's a wall object at the given position
    if (this->isObjectOnBoard(pos) == false) {
        return false; // No object at the position
    }
    auto it = object_at.find(pos);
    if (it == object_at.end()) {
        return false;
    }

    return dynamic_cast<Wall*>(it->second) != nullptr;
}

void GameBoard::addShell(Shell&& shell) {
    // This function adds a new Shell to the game board
    auto shell_ptr = std::make_unique<Shell>(std::move(shell));
    Point pos = shell_ptr->getPosition();
    
    object_at[pos] = shell_ptr.get(); 
    objects.push_back(std::move(shell_ptr));
}

void GameBoard::removeShell(Shell* shell) {
    // This function removes a shell from the game board
    if (!this->isObjectOnBoard(shell)) return;
    if (!shell) return;
    Point pos = shell->getPosition();
    object_at.erase(pos);
    objects.erase(std::remove_if(objects.begin(), objects.end(),
            [shell](const std::unique_ptr<GameObject>& obj) {
                return obj.get() == shell; }),objects.end());
}



void GameBoard::removeWall(Wall* wall) {
    // This function removes a wall from the game board
    if (!this->isObjectOnBoard(wall)) return;
    if (wall == nullptr) return;
    object_at.erase(wall->getPosition());

    auto it = std::remove_if(objects.begin(), objects.end(),
        [wall](const std::unique_ptr<GameObject>& obj) {
            return obj.get() == wall;
        });

    if (it != objects.end()) {
        objects.erase(it, objects.end());
    }
}

void GameBoard::removeTank(Tank* tank) {
    // This function removes a tank from the game board
    if (!this->isObjectOnBoard(tank)) return;
    object_at.erase(tank->getPosition());
    // Now remove from objects (this deletes the Tank)
    auto it = std::remove_if(objects.begin(), objects.end(),
        [tank](const std::unique_ptr<GameObject>& obj) {
            return obj.get() == tank;
        });
    if (it != objects.end()) {
        objects.erase(it, objects.end());
    }
}

void GameBoard::removeMine(Mine* mine) {
    // This function removes a mine from the board
    if (!this->isObjectOnBoard(mine)) return;
    if(mine == nullptr) return; 
    object_at.erase(mine->getPosition());

    auto it = std::remove_if(objects.begin(), objects.end(),
        [mine](const std::unique_ptr<GameObject>& obj) {
            return obj.get() == mine;
        });

    if (it != objects.end()) {
        objects.erase(it, objects.end());
    }
}

std::vector<Wall*> GameBoard::getWalls() const {
    // This function returns a vector of pointers to all Wall objects on the board
    std::vector<Wall*> result;
    for (const auto& obj : objects) {
        if (auto wall = dynamic_cast<Wall*>(obj.get())) {
            result.push_back(wall);
        }
    }
    return result;
}
std::vector<Mine*> GameBoard::getMines() const {
    // This function returns a vector of pointers to all Mine objects on the board
    std::vector<Mine*> result;
    for (const auto& obj : objects) {
        if (auto mine = dynamic_cast<Mine*>(obj.get())) {
            result.push_back(mine);
    }
}
    return result;
}

std::vector<Shell*> GameBoard::getShells() const {
    // This function returns a vector of pointers to all Shell objects on the board
    std::vector<Shell*> result;
     for (const auto& obj : objects) {
        if (auto shell = dynamic_cast<Shell*>(obj.get())) {
            result.push_back(shell);
        }
    }
    return result;
}

std::vector<Tank*> GameBoard::getTanksPlayer(int player_index) const {
    // This function returns a vector of pointers to all tanks belonging to the specified player
    std::vector<Tank*> result;
    for (const auto& obj : objects) {
        if (!this->isObjectOnBoard(obj.get())) {continue;}
        if (auto tank = dynamic_cast<Tank*>(obj.get())) {
            // Skip dead tanks
            if (!this->isObjectOnBoard(tank)) {continue;}
                if (tank->getPlayerIndex() == player_index) {
                    result.push_back(tank);
            }
        }
    }
    return result;
}

std::vector<Tank*> GameBoard::getAllTanks() const { 
    // This function returns a vector of pointers to all tanks on the board
    std::vector<Tank*> all_tanks;
    for (const auto& obj : objects) {
        if (!this->isObjectOnBoard(obj.get())) {continue;}
        if (auto tank = dynamic_cast<Tank*>(obj.get())) {
            if (this->isObjectOnBoard(tank)) {
                all_tanks.push_back(tank);
            }
        }
    }
    return all_tanks;
}

int GameBoard::getTankPlayerId(const Tank* tank) const {
    // This function returns the player ID (1 or 2) for the given tank pointer, or 0 if not found
    for (const auto& t : player1_tanks) {
        if (t == tank) return 1;
    }
    for (const auto& t : player2_tanks) {
        if (t == tank) return 2;
    }
    return 0;
}

void GameBoard::updateAllObjectsMap() {
    // This function updates the object_at map to reflect the current state of all objects on the board
    object_at.clear();
    for (const auto& obj_ptr : objects) {
        if (!this->isObjectOnBoard(obj_ptr.get())) {
            continue;
        }
        if (obj_ptr) {
            object_at[obj_ptr->getPosition()] = obj_ptr.get();
        }
    }
}


