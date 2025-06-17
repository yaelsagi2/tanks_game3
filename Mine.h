#ifndef MINE_H
#define MINE_H

#include "Point.h"
#include "GameObject.h"

/**
 * @class Mine
 * @brief Class representing a mine in the game.
 *
 * A Mine has a position on the board and provides access to its properties.
 */
class Mine : public GameObject {
private:
    Point position; ///< The position of the mine on the board.

public:
    /**
     * @brief Constructs a Mine object at the specified coordinates.
     * @param x The x-coordinate of the mine.
     * @param y The y-coordinate of the mine.
     */
    Mine(int x, int y);

    /**
     * @brief Gets the position of the mine.
     * @return The position as a Point.
     */
    Point getPosition() const override;

    /**
     * @brief Gets the character representation of the mine.
     * @return The character representing the mine.
     */
    char toChar() const override;
};

#endif // MINE_H
