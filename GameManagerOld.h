#pragma once

#include "common/Player.h"
#include "common/PlayerFactory.h"
#include "common/TankAlgorithmFactory.h"

#include "GameBoard.h"
#include "Logger.h"
#include "MyTankAlgorithmFactory.h"
#include "TankAlgorithm.h"
#include "Tank.h"
#include "Shell.h"
#include "GameBoardSatelliteView.h"
#include "MyPlayerFactory.h"

#include <vector>
#include <string>
#include <list>
#include <tuple>
#include <memory>
#include <type_traits>

template <typename T>
concept DerivedFromPlayerFactory = std::is_base_of<PlayerFactory, std::decay_t<T>>::value;
template <typename T>
concept DerivedFromTankAlgorithmFactory = std::is_base_of<TankAlgorithmFactory, std::decay_t<T>>::value;

/**
 * @brief Structure holding data for each tank in the game.
 */
struct TankData
{
    std::unique_ptr<TankAlgorithm> algorithm; ///< The tank's AI algorithm
    int playerId;                            ///< The player ID owning this tank
    Tank *tank;                              ///< Pointer to the tank object
    bool alive = true;                       ///< Whether the tank is alive
};

/**
 * @brief Manages the overall game flow, state, and logic.
 */
class GameManager
{
private:
    std::unique_ptr<PlayerFactory> playerFactory;        ///< Factory for creating players
    std::unique_ptr<TankAlgorithmFactory> tankFactory;   ///< Factory for creating tank algorithms
    std::unique_ptr<Logger> logger;                      ///< Logger for game events
    std::unique_ptr<GameBoard> board;                    ///< The game board
    std::vector<std::unique_ptr<Player>> players;        ///< All players in the game
    std::vector<TankData> tanks;                         ///< All tanks in the game
    bool game_over = false;                              ///< Whether the game is over
    int current_step = 0;                                ///< Current step number
    int remaining_step_after_amo = -1;                   ///< Steps remaining after ammo runs out

public:
    /**
     * @brief Constructs a GameManager with the given player and tank algorithm factories.
     * @tparam PF PlayerFactory type
     * @tparam TF TankAlgorithmFactory type
     * @param pf PlayerFactory instance
     * @param tf TankAlgorithmFactory instance
     */
    template <DerivedFromPlayerFactory PF, DerivedFromTankAlgorithmFactory TF>
    GameManager(PF &&pf, TF &&tf) : playerFactory(std::make_unique<std::decay_t<PF>>(std::forward<PF>(pf))),
                                    tankFactory((std::make_unique<std::decay_t<TF>>(std::forward<TF>(tf)))),
                                    logger(nullptr),
                                    board(nullptr),
                                    game_over(false),
                                    current_step(0),
                                    remaining_step_after_amo(-1) {}

    // Rule of 5
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    GameManager(GameManager&&) noexcept = default;
    GameManager& operator=(GameManager&&) noexcept = default;
    ~GameManager() = default;

    /**
     * @brief Reads the game board from a file.
     * @param filename The file to read from.
     * @return True if successful, false otherwise.
     */
    bool readBoard(const std::string &filename);

    /**
     * @brief Runs the main game loop.
     */
    void run();

private:
    /**
     * @brief Checks for immediate loss or tie conditions.
     * @param p1_tanks Player 1's tanks.
     * @param p2_tanks Player 2's tanks.
     */
    void immediateLoseOrTie(const std::vector<Tank *> &p1_tanks, const std::vector<Tank *> &p2_tanks);

    /**
     * @brief Logs the initial positions of all tanks.
     */
    void logInitialPositions();

    /**
     * @brief Gathers action requests from all tanks.
     * @param actions Vector to store the gathered actions.
     */
    void gatherRequests(std::vector<std::pair<TankData *, ActionRequest>> &actions);

    /**
     * @brief Processes all gathered action requests.
     * @param actions The actions to process.
     * @return A list of tuples containing tank data, action requests, and a bool for validity.
     */
    std::list<std::tuple<TankData *, ActionRequest, bool>> processRequests(const std::vector<std::pair<TankData *, ActionRequest>> &actions);

    /**
     * @brief Executes all processed requests for the current step.
     * @param currentStep The current step number.
     * @param actions The actions to execute.
     */
    void executeRequests(int currentStep, const std::list<std::tuple<TankData *, ActionRequest, bool>> &actions);

    /**
     * @brief Executes a single action for a tank.
     * @param td The tank data.
     * @param action The action to execute.
     */
    void executeAction(TankData *td, const ActionRequest &action);

    /**
     * @brief Executes a move forward action for a tank.
     * @param td The tank data.
     */
    void executeMoveForward(TankData *td);

    /**
     * @brief Executes a move backward action for a tank.
     * @param td The tank data.
     */
    void executeMoveBackward(TankData *td);

    /**
     * @brief Executes a rotate left action for a tank.
     * @param td The tank data.
     * @param angle The angle to rotate.
     */
    void executeRotateLeft(TankData *td, int angle);

    /**
     * @brief Executes a rotate right action for a tank.
     * @param td The tank data.
     * @param angle The angle to rotate.
     */
    void executeRotateRight(TankData *td, int angle);

    /**
     * @brief Executes a shoot action for a tank.
     * @param td The tank data.
     */
    void executeShoot(TankData *td);

    /**
     * @brief Executes the get battle info action for a tank.
     * @param td The tank data.
     */
    void executeGetBattleInfo(TankData *td);

    /**
     * @brief Calculates the next position for a tank given a direction and number of steps.
     * @param from The starting point.
     * @param dir The direction to move.
     * @param steps Number of steps to move.
     * @return The resulting point.
     */
    Point getNextPosition(const Point &from, Direction dir, int steps);

    /**
     * @brief Checks if an action is legal for a given tank.
     * @param tank The tank.
     * @param action The action to check.
     * @return True if legal, false otherwise.
     */
    bool isActionLegal(Tank *tank, const ActionRequest &action);

    /**
     * @brief Checks for immediate shell collision.
     * @param shell The shell to check.
     * @return True if a collision occurs, false otherwise.
     */
    bool checkImmediateShellCollision(const Shell &shell);
    /**
     * @brief Returns a short name for an action request.
     * @param req The action request.
     * @return Short string name.
     */
    std::string shortActionName(ActionRequest req) const;

    /**
     * @brief Gets the reason for ignoring an action.
     * @param tank The tank.
     * @param action The action.
     * @return String describing the reason.
     */
    std::string getIgnoreReason(Tank *tank, const ActionRequest &action);


    /**
     * @brief Checks if at least one tank is alive for a player.
     * @param player_index The player index.
     * @return True if at least one tank is alive, false otherwise.
     */
    bool isAtLeastOneTankAlive(int player_index) const;

    /**
     * @brief Writes the game result to the logger or output.
     */
    void writeGameResult();

    /**
     * @brief Updates the game status (e.g., checks for win/loss/tie).
     */
    void updateGameStatus();

    /**
     * @brief Checks if all tanks are out of ammo.
     * @return True if all tanks are out of ammo, false otherwise.
     */
    bool allTanksOutOfAmmo();

    /**
     * @brief Updates the state of a tank.
     * @param tank The tank to update.
     */
    void updateTankState(Tank* tank);

    /**
     * @brief Updates the locations of all shells.
     */
    void updateShellsLocation(); 

    /**
     * @brief Checks for all types of collisions in the game.
     */
    void checkCollisions();

    /**
     * @brief Checks for collisions between shells and walls.
     */
    void checkShellWallCollisions();

    /**
     * @brief Checks for collisions between shells and tanks.
     */
    void checkShellTankCollisions();

    /**
     * @brief Checks for collisions between shells.
     */
    void checkShellShellCollisions();

    /**
     * @brief Checks for collisions between tanks and mines.
     */
    void checkTankMineCollisions();

    /**
     * @brief Checks for collisions between tanks.
     */
    void checkTankTankCollision();

    /**
     * @brief Checks for future shell collisions within a given square.
     * @param square The square to check.
     */
    void checkShellFutureCollisions(int square);

    /**
     * @brief Moves all shells two points forward.
     */
    void moveShellTwoPoints();

    /**
     * @brief Consolidates actions for the current step.
     * @param actions The actions to consolidate.
     */
    void consolidateActions(const std::list<std::tuple<TankData*, ActionRequest, bool>>& actions);

    /**
     * @brief Counts the number of alive tanks for a player.
     * @param player_index The player index.
     * @return Number of alive tanks.
     */
    int countAliveTanks(int player_index);

    /**
     * @brief Checks if a tank is in shooting range of an enemy.
     * @param tank The tank to check.
     * @return True if in range, false otherwise.
     */
    void initAllTanksSorted();
};
