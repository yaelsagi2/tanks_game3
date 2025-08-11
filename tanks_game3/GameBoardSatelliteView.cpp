#include "GameBoardSatelliteView.h"
#include "GameBoard.h"
#include <cstddef>
#include <iostream> // Include iostream for debug output

// Constructor: initializes the satellite view with a reference to the board and the player's tank
GameBoardSatelliteView::GameBoardSatelliteView(GameBoard* board, Tank* selfTank)
    : board(board), selfTank(selfTank) {}


GameBoardSatelliteView::GameBoardSatelliteView(const MapData* map) : map_view(map->grid){
}

// Returns a character representing the object at (x, y)
char GameBoardSatelliteView::getObjectAt(size_t x, size_t y) const { 
    int cols = board->getCols();
    int rows = board->getRows();
    if (x >= static_cast<size_t>(rows) || y >= static_cast<size_t>(cols)) { // Out of bounds check
        return '&';
    } 
    if (board->isObjectOnBoard(Point(x, y)) == false) { // No object at this position
        return ' ';
    }
    Point p(static_cast<int>(x), static_cast<int>(y));
    GameObject* obj = board->getObjectAt(p);
    
    if (!obj || obj == nullptr) { // If no object found, return space
        return ' '; 
    }
    if (obj == selfTank) { // If the object is the player's own tank
        return '%';
    }
    if (dynamic_cast<const Wall*>(obj)) {  // If the object is a wall
        return '#';
    }
    if (dynamic_cast<const Shell*>(obj)) { // If the object is a shell
        return '*';
    }
    if (dynamic_cast<const Mine*>(obj)) { // If the object is a mine
        return '@';
    }
    if (const Tank* tank = dynamic_cast<const Tank*>(obj)) { // If the object is a tank (not self)

        return (tank->getPlayerIndex() == 1) ? '1' : '2';
    }
    // Default: empty space
    return ' ';
}


void GameBoardSatelliteView::printView() const {
    // Prints the satellite view of the board - used for debugging
    int cols = board->getCols();
    int rows = board->getRows();
    for (int x = 0; x < rows; ++x) {
        for (int y = 0; y < cols; ++y) {
            // Only print if there is an object at this position
            if (board->isObjectOnBoard(Point(x, y))) {
            }
            if (!board->isObjectOnBoard(Point(x, y))) {
                std::cout << ' '; // Print space if no object
            }
        }
        std::cout << std::endl;
    }
}


