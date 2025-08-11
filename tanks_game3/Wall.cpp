#include "Wall.h"

// Constructs a Wall at the given (x, y) position, initializing hit count to 0.
Wall::Wall(int x, int y) : position(x, y), cnt_hits(0) {}

// Returns the position of the wall.
Point Wall::getPosition() const {
    return position;
}

// Returns the character representation of the wall.
char Wall::toChar() const { 
    return '#';
}

// Returns the number of times the wall has been hit.
int Wall::getHitCount() const {
    return cnt_hits;
}

// Increments the wall's hit count by one.
void Wall::incrementHitCount() {
    cnt_hits++;
}


