#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <vector>
#include <memory>
#include <unordered_map>
#include "Point.h"
#include "GameObject.h"
#include "Tank.h"
#include "Wall.h"
#include "Mine.h"
#include "Shell.h"
#include "Direction.h"
#include <vector>

class GameBoard {
private:
    int rows;  ///< Number of rows on the board
    int cols;  ///< Number of columns on the board
    int maxSteps; ///< Maximum number of steps in the game
    std::vector<std::unique_ptr<GameObject>> objects; ///< All game objects owned by the board
    std::unordered_map<Point, GameObject*> object_at; ///< Maps positions to game objects
    std::vector<Tank*> player1_tanks; ///< Pointers to player 1's tanks
    std::vector<Tank*> player2_tanks; ///< Pointers to player 2's tanks

public:
    /**
     * @brief Constructs a GameBoard with the given parameters.
     * @param rows Number of rows.
     * @param cols Number of columns.
     * @param maxSteps Maximum number of steps in the game.
     * @param all_objects All game objects (ownership transferred).
     * @param map Mapping from board positions to game objects.
     * @param p1 Vector of pointers to player 1's tanks.
     * @param p2 Vector of pointers to player 2's tanks.
     */
    GameBoard(int rows, int cols, int maxSteps,
          std::vector<std::unique_ptr<GameObject>>&& all_objects, std::unordered_map<Point, GameObject*>&& map,
          std::vector<Tank*>& p1, std::vector<Tank*>& p2);

    // Rule of 5
    GameBoard(const GameBoard&) = delete; // Copy constructor deleted
    GameBoard& operator=(const GameBoard&) = delete; // Copy assignment operator deleted
    GameBoard(GameBoard&&) noexcept = default; // Move constructor
    GameBoard& operator=(GameBoard&&) noexcept = default; // Move assignment operator
    ~GameBoard() = default; // Destructor

    // Getters

    /**
     * @brief Returns the number of columns on the board.
     */
    int getCols() const;

    /**
     * @brief Returns the number of rows on the board.
     */
    int getRows() const;

    /**
     * @brief Returns the maximum number of steps in the game.
     */
    int getMaxSteps() const;

    /**
     * @brief Returns the GameObject at the given position, or nullptr if none exists.
     * @param p The position to query.
     */
    GameObject* getObjectAt(Point p) const;

    /**
     * @brief Returns a reference to the vector of tanks for the specified player.
     * @param player_id The player ID (1 or 2).
     */
    std::vector<Tank*>& getPlayerTanks(int player_id);

    /**
     * @brief Checks if there is a wall at the given position.
     * @param pos The position to check.
     */
    bool isWallAt(const Point& pos) const;

    /**
     * @brief Removes the object at the given position from the board.
     * @param p The position to remove the object from.
     */
    void removeObjectAt(Point p);

    /**
     * @brief Prints the current state of the game board to stdout.
     */
    void printBoardState() const;

    /**
     * @brief Returns a vector of pointers to all Shell objects on the board.
     */
    std::vector<Shell*> getShells() const;

    /**
     * @brief Returns a vector of pointers to all Wall objects on the board.
     */
    std::vector<Wall*> getWalls() const;

    /**
     * @brief Returns a vector of pointers to all Mine objects on the board.
     */
    std::vector<Mine*> getMines() const;

    /**
     * @brief Returns a vector of pointers to all tanks belonging to the specified player.
     * @param player_index The player index (1 or 2).
     */
    std::vector<Tank*> getTanksPlayer(int player_index) const;

    /**
     * @brief Returns a vector of pointers to all tanks on the board.
     */
    std::vector <Tank*> getAllTanks() const;

    /**
     * @brief Returns the player ID (1 or 2) for the given tank pointer, or 0 if not found.
     * @param tank Pointer to the tank.
     */
    int getTankPlayerId(const Tank* tank) const;

    /**
     * @brief Checks if the given GameObject pointer is currently on the board.
     * @param obj Pointer to the GameObject.
     */
    bool isObjectOnBoard(const GameObject* obj) const;

    /**
     * @brief Checks if there is any object at the given position.
     * @param pos The position to check.
     */
    bool isObjectOnBoard(const Point& pos) const;

    /**
     * @brief Adds a new Shell to the board.
     * @param shell The Shell object to add (moved).
     */
    void addShell(Shell&& shell);

    /**
     * @brief Removes the specified Wall from the board.
     * @param wall Pointer to the Wall to remove.
     */
    void removeWall(Wall* wall);

    /**
     * @brief Removes the specified Shell from the board.
     * @param shell Pointer to the Shell to remove.
     */
    void removeShell(Shell* shell);

    /**
     * @brief Removes the specified Tank from the board.
     * @param tank Pointer to the Tank to remove.
     */
    void removeTank(Tank* tank);

    /**
     * @brief Removes the specified Mine from the board.
     * @param mine Pointer to the Mine to remove.
     */
    void removeMine(Mine* mine);

    /**
     * @brief Updates the object_at map for all objects on the board.
     *        Rebuilds the mapping from scratch based on current object positions.
     */
    void updateAllObjectsMap();

};


#endif // GAMEBOARD_H
