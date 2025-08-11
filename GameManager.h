#pragma once
#include <memory>
#include "common/AbstractGameManager.h"
#include "common/SatelliteView.h"
#include "common/Player.h"
#include "common/TankAlgorithm.h"
#include "common/GameResult.h"
#include "GameBoard.h"
#include <list>
#include <memory>
#include <vector>
#include <string>

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
 * This class is responsible for running the game, processing player actions,
 * and determining the game result.
 */
namespace GameManager_206480972_206899163 {

class GameManager : public AbstractGameManager {  
private: 
    std::unique_ptr <GameBoard> board; // The game board
    std::unique_ptr<PlayerFactory> playerFactory;        ///< Factory for creating players
    std::unique_ptr<TankAlgorithmFactory> tankFactory;   ///< Factory for creating tank algorithms
    std::vector<TankData> tanks;                         ///< All tanks in the game
    std::vector<Player*> players;        ///< All players in the game
    int remaining_step_after_amo = -1; // Steps remaining after ammo runs out
    int current_step = 0; // Current step number
    bool game_over = false; // Whether the game is over
    bool verbose; // Whether to log detailed information
public:
    GameManager(bool verbose) : verbose(verbose) {}
    GameResult run(size_t map_width, size_t map_height, const SatelliteView& map, // <= a snapshot, NOT updated
        size_t max_steps, size_t num_shells, Player& player1, Player& player2,
        TankAlgorithmFactory player1_tank_algo_factory, TankAlgorithmFactory player2_tank_algo_factory) override;

    // Rule of 5
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    GameManager(GameManager&&) noexcept = default;
    GameManager& operator=(GameManager&&) noexcept = default;
    ~GameManager() = default;

private:
    void initAllTanksSorted(TankAlgorithmFactory player1_tank_algo_factory, TankAlgorithmFactory player2_tank_algo_factory);
    std::vector <std::pair<TankData*, ActionRequest>> gatherRequests();
    std::list<std::tuple<TankData*, ActionRequest, bool>> processRequests(const std::vector<std::pair<TankData*, ActionRequest>>& actions);
    bool isActionLegal(Tank* tank, ActionRequest action) const;
    Point getNextPosition(const Point& pos, Direction dir, int steps) const;
    void executeRequests(const std::list<std::tuple<TankData*, ActionRequest, bool>>& actions);
    void resetGameState();
    GameResult buildImmediateResult(const std::vector<Tank*>& p1_tanks, const std::vector<Tank*>& p2_tanks) const;
    bool checkImmediateEnd(const std::vector<Tank*>& p1_tanks, const std::vector<Tank*>& p2_tanks);
    void immediateLoseOrTie(const std::vector<Tank*>& p1_tanks, const std::vector<Tank*>& p2_tanks);
    GameResult runGameLoop();
    void logInitialPositions();
    void executeAction(TankData* td, const ActionRequest& action);
    void executeMoveForward(TankData* td);
    void executeMoveBackward(TankData* td);
    void executeRotateLeft(TankData* td, int angle);
    void executeRotateRight(TankData* td, int angle);
    void executeShoot(TankData* td);
    void executeGetBattleInfo(TankData* td);
    int countAliveTanks(int playerId);
    void consolidateActions(const std::list<std::tuple<TankData*, ActionRequest, bool>>& actions);
    void checkCollisions();
    void checkShellFutureCollisions(int square);
    void checkShellShellCollisions();
    void checkShellTankCollisions();
    void checkTankTankCollision();
    void checkTankWallCollisions();
    void checkShellWallCollisions();
    void checkTankMineCollisions();
    void updateShellsLocation();
    bool isAtLeastOneTankAlive(int playerId) const;
    bool isGameOver();
    std::string shortActionName(const ActionRequest& action) const;
    GameResult writeGameResult();
    bool allTanksOutOfAmmo();
    void updateGameStatus();
    void gatherRequests(std::vector<std::pair<TankData*, ActionRequest>>& actions);
    void moveShellTwoPoints();
    void removeCollidedItems(const std::vector<Shell*>& shells_to_remove,  const std::vector<Tank*>& tanks_to_remove, const std::vector<Wall*>& walls_to_remove, 
                             const std::vector<Mine*>& mines_to_remove);
    


};
}