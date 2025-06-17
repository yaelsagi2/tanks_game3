#include "TankAlgorithm.h"
#include "SatelliteView.h"
#include "SimpleBattleInfo.h"
#include "BattleInfo.h"
#include "Player1.h"
#include <iostream>

// Constructor: Initializes Player1 with game parameters
Player1::Player1(int player_index, size_t rows, size_t cols, size_t max_steps, size_t num_shells)
    : Player(player_index, rows, cols, max_steps, num_shells),
        player_index(player_index),
        rows(rows),
        cols(cols),
        max_steps(max_steps),
        num_shells(num_shells) {}

// Get the number of columns in the board
size_t Player1::getCols() const {
    // std::cout <<"num of cols" <<cols << std::endl;
    return cols;
}

// Get the number of rows in the board
size_t Player1::getRows() const {
    return rows;
}

// Get the number of shells available
size_t Player1::getAmmo() const {
    return num_shells;
}

// Get the maximum number of steps allowed
size_t Player1::getMaxSteps() const{
    return max_steps;
}

// Update the tank algorithm with the latest battle info from the satellite view
void Player1::updateTankWithBattleInfo(TankAlgorithm &tank, SatelliteView &satelliteView) {
    // Update the tank's battle information based on the satellite view
    SimpleBattleInfo battleInfo = SimpleBattleInfo(satelliteView, rows, cols, num_shells, 1);
    tank.updateBattleInfo(battleInfo);
}

// Get the player index (should be 1)
int Player1::getID() const{
    return 1;
}

