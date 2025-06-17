#pragma once
#include "common/Player.h"

/**
 * @class Player1
 * @brief Represents player 1 in the game, managing player-specific state and actions.
 *
 * Stores player index, board dimensions, max steps, and shell count.
 * Provides methods to update the tank with battle info and access player/game parameters.
 */
class Player1 : public Player {
protected:
    int player_index;   ///< Player index (should be 1)
    size_t rows;        ///< Number of rows in the game board
    size_t cols;        ///< Number of columns in the game board
    size_t max_steps;   ///< Maximum number of steps allowed
    size_t num_shells;  ///< Number of shells available
    
public:
    /**
     * @brief Constructs a Player1 object with game parameters.
     * @param player_index The player index (should be 1).
     * @param rows Number of rows in the board.
     * @param cols Number of columns in the board.
     * @param max_steps Maximum allowed steps.
     * @param num_shells Number of shells available.
     */
    Player1(int player_index, size_t rows, size_t cols, size_t max_steps, size_t num_shells);

    /**
     * @brief Updates the tank algorithm with the latest battle info.
     * @param tank The tank algorithm to update.
     * @param satellite_view The current satellite view of the board.
     */
    void updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) override;

    /**
     * @brief Gets the number of columns in the board.
     * @return Number of columns.
     */
    size_t getCols() const;

    /**
     * @brief Gets the number of rows in the board.
     * @return Number of rows.
     */
    size_t getRows() const;

    /**
     * @brief Gets the number of shells available.
     * @return Number of shells.
     */
    size_t getAmmo() const;

    /**
     * @brief Gets the maximum number of steps allowed.
     * @return Maximum steps.
     */
    size_t getMaxSteps() const;

    /**
     * @brief Gets the number of shells available.
     * @return Number of shells.
     */
    size_t getNumShells() const;

    /**
     * @brief Gets the player index (should be 1).
     * @return Player index.
     */
    int getID() const;
};

