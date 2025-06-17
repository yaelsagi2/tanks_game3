#include "TankAlgorithm.h"
#include "SatelliteView.h"
#include "SimpleBattleInfo.h"
#include "BattleInfo.h"
#include "Player2.h"
#include <iostream>

// Constructor: Initializes Player2 with game parameters
Player2::Player2(int player_index, size_t rows, size_t cols, size_t max_steps, size_t num_shells)
    : Player(player_index, rows, cols, max_steps, num_shells),
      player_index(player_index),
      rows(rows),
      cols(cols),
      max_steps(max_steps),
      num_shells(num_shells) {}

// Get the number of columns in the board
size_t Player2::getCols() const {
    return cols;
}

// Get the number of rows in the board
size_t Player2::getRows() const {
    return rows;
}

// Get the number of shells available
size_t Player2::getAmmo() const {
    return num_shells;
}

// Get the maximum number of steps allowed
size_t Player2::getMaxSteps() const {
    return max_steps;
}

// Update the tank algorithm with the latest battle info from the satellite view
void Player2::updateTankWithBattleInfo(TankAlgorithm &tank, SatelliteView &satelliteView) {
    // Create a SimpleBattleInfo object from the satellite view and update the tank
    SimpleBattleInfo battleInfo = SimpleBattleInfo(satelliteView,rows,cols, num_shells, 2);
    tank.updateBattleInfo(battleInfo);
}

// Get the player index (should be 2)
int Player2::getID() const {
    return 2;
}

