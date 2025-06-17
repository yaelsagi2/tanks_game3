#include "Mine.h"

// Constructor
Mine::Mine(int x, int y) : position(x, y) {}

// Getter
Point Mine::getPosition() const {
    return position;
}

char Mine::toChar() const {
    // This function returns the character representation of the mine
    return '@'; 
}

