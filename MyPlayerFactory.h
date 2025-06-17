#pragma once
#include <cstddef> // for size_t
#include <memory>  // for std::unique_ptr
#include <vector>  // for std::vector
#include "common/PlayerFactory.h"

/**
 * @class MyPlayerFactory
 * @brief Factory class for creating Player instances.
 *
 * Implements the PlayerFactory interface and provides a method to create
 * a new Player (such as Player1 or Player2) for a given player index and game parameters.
 */
class MyPlayerFactory : public PlayerFactory {
public:
    /**
     * @brief Default constructor.
     */
    MyPlayerFactory();

    /**
     * @brief Creates a new Player instance for the specified player and game parameters.
     * @param player_index The index of the player (1 or 2).
     * @param x Number of rows in the board.
     * @param y Number of columns in the board.
     * @param max_steps Maximum allowed steps.
     * @param num_shells Number of shells available.
     * @return A unique pointer to the created Player.
     */
    std::unique_ptr<Player> create(int player_index, size_t x, size_t y,
                                   size_t max_steps, size_t num_shells) const override;
};
