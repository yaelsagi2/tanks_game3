#include <iostream>
#include "Shell.h"
#include "Direction.h"
#include "Point.h"

// Constructor: Create a shell with position, direction, and owner id
Shell::Shell(Point position, Direction dir, int id) : direction(dir), id(id), new_shell(true) {
    this->position = Point(position.getX(), position.getY());
}

// Constructor: Create a shell with position only (direction and id defaulted)
Shell::Shell(Point position) : direction(Direction::None), id(0), new_shell(false) {
    this->position = Point(position.getX(), position.getY());
}

// Get the current position of the shell
Point Shell::getPosition() const {
    return position;
}

// Get the direction of the shell
Direction Shell::getDirection() const {
    return direction;
}

// Get the ID of the tank that fired the shell
int Shell::getId() const {
    return id;
}

// Return if the shell is newly created on the board
bool Shell::getNewShell() const {
    // return if a shell is new in the game board - "shooted now"
    return new_shell;
}

// Get the character representation of the shell
char Shell::toChar() const {
    // This function returns the character representation of the shell
    return '*';  
}

// Mark the shell as no longer new (after first move)
void Shell::setNewShell() {
    // change status of a shell - for future moving on game board
    new_shell = false;
}

// Set the position of the shell
void Shell::setPosition(const Point& newPosition) {
    this->position = newPosition;
}

// Move the shell two points in its direction, wrapping around the board if necessary
void Shell::move(int boardWidth, int boardHeight) {
    // Move the shell 2 points in the direction it is facing
    std::pair<int, int> offset = directionOffset(direction);
    int dx = offset.first;
    int dy = offset.second;

    int newX = (position.getX() + 2 * dx + boardHeight) % boardHeight;
    int newY = (position.getY() + 2 * dy + boardWidth) % boardWidth;

    Point position = Point(newX, newY);
    this->setPosition(position);
}

