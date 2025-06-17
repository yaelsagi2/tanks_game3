#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Point.h"

/**
 * @class GameObject
 * @brief Abstract base class for all objects that can exist on the game board.
 *
 * Provides a common interface for retrieving the position and character representation
 * of any game object (such as tanks, walls, mines, shells, etc.).
 */
class GameObject {
public:
    /**
     * @brief Virtual destructor for safe polymorphic deletion.
     */
    virtual ~GameObject() = default;

    /**
     * @brief Gets the position of the object on the board.
     * @return The position as a Point.
     */
    virtual Point getPosition() const = 0;

    /**
     * @brief Gets the character representation of the object.
     * @return The character representing the object.
     */
    virtual char toChar() const = 0; 
};

#endif // GAMEOBJECT_H
