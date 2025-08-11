#include "common/SatelliteView.h"
#include "GameBoard.h"
#include "UserCommon/MapData.h"
#include "Tank.h"
#include <vector>
#include <memory>

/**
 * @brief Provides a satellite view of the game board for a specific tank.
 *        Allows querying the board state and printing a debug view.
 */
class GameBoardSatelliteView : public SatelliteView {
public:
    /**
     * @brief Constructs a satellite view for the given board and tank.
     * @param board Reference to the game board.
     * @param selfTank Pointer to the player's own tank.
     */
    GameBoardSatelliteView(GameBoard* board, Tank* selfTank);


    GameBoardSatelliteView(const MapData* map);


    /**
     * @brief Returns a character representing the object at the specified coordinates.
     * @param x The x-coordinate.
     * @param y The y-coordinate.
     * @return Character representing the object at (x, y).
     */
    char getObjectAt(size_t x, size_t y) const override;

    /**
     * @brief Prints a debug view of the board to stdout.
     */
    void printView() const; // For debugging purposes

private:
    GameBoard* board;   ///< pointer to the game board
    Tank* selfTank;     ///< Pointer to the player's own tank

    //for initialization for GameManager
    std::vector<std::vector<char>> map_view; ///< 2D vector to hold the view of the board

};
