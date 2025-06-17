// #include "PlayerFactory.h"
#include "Player1.h"
#include "Player2.h"
#include <stdexcept> // For std::invalid_argument
#include <memory>
#include <iostream>

// Forward declaration or include the header for MyPlayerFactory if not already included
#include "MyPlayerFactory.h"

/**
 * @brief Default constructor for MyPlayerFactory.
 */
MyPlayerFactory::MyPlayerFactory() {}

/**
 * @brief Creates a new Player instance for the specified player and game parameters.
 * 
 * Depending on the player_index, creates either a Player1 or Player2 object.
 * Throws std::invalid_argument if the player_index is not 1 or 2.
 * 
 * @param player_index The index of the player (1 or 2).
 * @param x Number of rows in the board.
 * @param y Number of columns in the board.
 * @param max_steps Maximum allowed steps.
 * @param num_shells Number of shells available.
 * @return A unique pointer to the created Player.
 * @throws std::invalid_argument if player_index is not 1 or 2.
 */
std::unique_ptr<Player> MyPlayerFactory::create(int player_index, size_t x, size_t y,
                                                size_t max_steps, size_t num_shells) const {
    // If player_index is 1, create and return a Player1 instance
    if (player_index == 1) {
        return std::make_unique<Player1>(player_index, x, y, max_steps, num_shells);
    // If player_index is 2, create and return a Player2 instance
    } else if (player_index == 2) {
        return std::make_unique<Player2>(player_index, x, y, max_steps, num_shells);
    // If player_index is not 1 or 2, throw an exception
    } else {
        throw std::invalid_argument("Unsupported player index"); 
    }
}
