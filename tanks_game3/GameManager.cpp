
#include "GameManager.h"
#include "common/GameResult.h"
#include "GameBoard.h"
#include "common/Player.h"
#include "common/SatelliteView.h"
#include "common/TankAlgorithm.h"
#include "GameBoardSatelliteView.h"
#include <memory>
#include <vector>
#include <list>
#include <iostream>
#include <algorithm>
// TODO:think where should combine verbose and create logger corresponding to verbose
namespace GameManager_206480972_206899163 {
    GameResult GameManager::run( size_t map_width, size_t map_height, const SatelliteView& map, size_t max_steps, size_t num_shells, Player& player1, Player& player2,
        TankAlgorithmFactory player1_tank_algo_factory, TankAlgorithmFactory player2_tank_algo_factory) {
        //This function runs the game loop, processing each step until the game is over.
        std::unique_ptr <GameBoard> board = std::make_unique<GameBoard>(map_width, map_height, map, num_shells, max_steps); // converting SatelliteView to GameBoard
        resetGameState();
        players.push_back(&player1);
        players.push_back(&player2);
        const std::vector<Tank*>& p1_tanks = board->getPlayerTanks(1);
        const std::vector<Tank*>& p2_tanks = board->getPlayerTanks(2);
        initAllTanksSorted(player1_tank_algo_factory, player2_tank_algo_factory);
        if (checkImmediateEnd(p1_tanks, p2_tanks)) { // Check if the game can end immediately
            return buildImmediateResult(p1_tanks, p2_tanks);
        }
        GameResult result = runGameLoop(); // Run the game loop until the game is over

        return result;
    }
    void GameManager::initAllTanksSorted(TankAlgorithmFactory player1_tank_algo_factory, TankAlgorithmFactory player2_tank_algo_factory) {
    // This function initializes all tanks in the game, assigning them IDs and creating their algorithms.
    // It combines tanks from both players, sorts them by their position from top left to bottom right and assigns IDs.
    // 1. Combine all tanks
    std::vector<Tank*> all_tanks = board->getPlayerTanks(1);
    const std::vector<Tank*> & p2_tanks = board->getPlayerTanks(2);
    all_tanks.insert(all_tanks.end(), p2_tanks.begin(), p2_tanks.end());

    // 2. Sort globally by (x,y)
    std::sort(all_tanks.begin(), all_tanks.end(), [](Tank* a, Tank* b) {
        if (a->getPosition().getX() != b->getPosition().getX())
            return a->getPosition().getX() < b->getPosition().getX();
        return a->getPosition().getY() < b->getPosition().getY();
    });

    // 3. Assign IDs per player and fill tanks vector
    int id1 = 0, id2 = 0;
    for (Tank* tank : all_tanks) {
        int player_id = board->getTankPlayerId(tank);
        int tank_id = (player_id == 1) ? id1++ : id2++;
        tank->setID(tank_id);
        auto algo = (player_id == 1 ? player1_tank_algo_factory : player2_tank_algo_factory)(player_id, tank_id);
        tanks.push_back(TankData{std::move(algo), player_id, tank, true});
    }
}

void GameManager::resetGameState() {
    // This function resets the game state, clearing all tanks, players, and board.
    players.clear();
    tanks.clear();
    current_step = 0;
    remaining_step_after_amo = -1;
    game_over = false;
}

std::vector<std::pair<TankData*, ActionRequest>> GameManager::gatherRequests()
 {
    // This function gathers action requests from all tanks and stores them in a vector.
    std::vector<std::pair<TankData*, ActionRequest>> actions;
    for (TankData& td : tanks) {
        if (!this->board->isObjectOnBoard(td.tank)) { // Skip dead tanks
            continue;
        }
        ActionRequest req = td.algorithm->getAction();
        actions.emplace_back(&td, req);
    }
    return actions;
}


std::list<std::tuple<TankData*, ActionRequest, bool>> GameManager::processRequests(const std::vector<std::pair<TankData*, ActionRequest>>& actions) {
    // This function processes the gathered action requests and executes them.
    std::list<std::tuple<TankData*,ActionRequest ,bool>> approved_actions;
    for (const auto& [td, req] : actions) {
        if (!this->board->isObjectOnBoard(td->tank)) {
            continue;
        }
        if (isActionLegal(td->tank, req)) {
            approved_actions.emplace_back(td,req,true);
        }
        else {
            approved_actions.emplace_back(td,req,false);
        }
    }
    return approved_actions;
}

bool GameManager::isActionLegal(Tank* tank, ActionRequest action) const {
    // This function checks if the action is legal for a given tank - true if legal, false otherwise.
    if (!this->board->isObjectOnBoard(tank)) {
        return false; // If tank is null or not on the board, action is illegal
    }
    int backward_steps = tank->getBackwardSteps();     // If the tank is in the middle of a backward move, only backward actions are allowed
    if (backward_steps >= 1 && backward_steps < 3 && action != ActionRequest::MoveBackward) {
        if (action == ActionRequest::MoveForward || action == ActionRequest::GetBattleInfo ) { // moving forward cancelling
            tank->setBackwardSteps(0); // Cancel backward move            
            // logger->logActionDetailed(current_step, "Tank " + std::to_string(tank->getId()) + " cancelled backward move at (" +
            // std::to_string(tank->getPosition().getX()) + ", " + std::to_string(tank->getPosition().getY()) + ").", "");
            return true;
        }
        return false; 
    }
    switch (action) { 
        case ActionRequest::MoveForward: { 
            // Check if moving forward would result in hitting a wall
            Point next = getNextPosition(tank->getPosition(), tank->getCanonDir(), 1);
            return !(board->isWallAt(next));
        }
        case ActionRequest::MoveBackward: { // If the tank has completed the backward steps, check for wall collisions
            if (backward_steps == 3) {
                Point back_pos = getNextPosition(tank->getPosition(), tank->getCanonDir(), -1);
                return !(board->isWallAt(back_pos));
            }
            return true;
        }
        case ActionRequest::Shoot:  // Check if the tank can shoot (e.g., has ammo and no cooldown)
            return tank->canShoot();
        case ActionRequest::RotateLeft45:  // Rotation is always legal
        case ActionRequest::RotateLeft90:  // Rotation is always legal
        case ActionRequest::RotateRight45: // Rotation is always legal
        case ActionRequest::RotateRight90: // Rotation is always legal
        case ActionRequest::GetBattleInfo: // Get battle info is always legal
        case ActionRequest::DoNothing: // Do nothing is always legal
            return true;
        default:
            return false;
    }
}

Point GameManager::getNextPosition(const Point& from, Direction dir, int steps) const {
    // This function calculates the next position based on the current position, direction, and number of steps
    std::pair <int, int> offset = directionOffset(dir);
    int dx = offset.first;
    int dy = offset.second;

    int new_x = (from.getX() + dx * steps + board->getRows()) % board->getRows();
    int new_y = (from.getY() + dy * steps + board->getCols()) % board->getCols();
    return Point(new_x, new_y);
}


void GameManager::executeAction(TankData* td, const ActionRequest& action) {
    // This function executes the action for the given tank and updates the game state accordingly.
    switch (action) {
        case ActionRequest::MoveForward:
            executeMoveForward(td);
            break;
        case ActionRequest::MoveBackward:
            executeMoveBackward(td);
            break;
        case ActionRequest::RotateLeft90:
            executeRotateLeft(td, 90);
            break;
        case ActionRequest::RotateRight90:
            executeRotateRight(td, 90);
            break;
        case ActionRequest::RotateLeft45:
            executeRotateLeft(td, 45);   
            break; 
        case ActionRequest::RotateRight45:
            executeRotateRight(td, 45);
            break;
        case ActionRequest::Shoot:
            executeShoot(td);
            break;
        case ActionRequest::GetBattleInfo:
            executeGetBattleInfo(td);
            break;
        case ActionRequest::DoNothing:
            break;
    }
}

void GameManager::executeMoveForward(TankData* td) {
    // This function moves the tank forward in the direction it is facing
    if (!this->board->isObjectOnBoard(td->tank))
        return; // If tank is not on the board, do nothing
    Tank* tank = td->tank;
    // int tank_id = tank->getId();
    // Point old_pos = tank->getPosition();

    // If tank was in backward movement, cancel it
    int backward_steps = tank->getBackwardSteps();
    if (backward_steps > 0) { // If tank was in the middle of a backward move
        tank->setBackwardSteps(0); // Cancel backward move
        // logger->logActionDetailed(current_step, "Tank " + std::to_string(tankId) + " cancelled backward move at (" +
        //     std::to_string(oldPos.getX()) + ", " + std::to_string(oldPos.getY()) + ").","");
        return;
    }

    // Perform move forward
    tank->moveForward(board->getCols(), board->getRows());
    // Point newPos = tank->getPosition();

    // logger->logActionDetailed(current_step, "Tank " + std::to_string(tankId) + " moved forward to (" +
    //     std::to_string(newPos.getX()) + ", " + std::to_string(newPos.getY()) + ").", "");
}
    
void GameManager::executeMoveBackward(TankData* td) {
    // This function moves the tank backward in the direction it is facing
    if (!this->board->isObjectOnBoard(td->tank))
        return; // If tank is not on the board, do nothing
    Tank* tank = td->tank;
    // int tank_id = tank->getId();
    // Point curr_pos = tank->getPosition();
    int backward_steps = tank->getBackwardSteps();
    if (backward_steps == 0) {
        tank->setBackwardSteps(1); // Start backward move
        // logger->logActionDetailed(current_step, "Tank " + std::to_string(tank_id) + " initiated backward move. Waiting 2 steps.","");
    }
    else if (backward_steps == 1 || backward_steps == 2) { // Still waiting before actual move
        tank->setBackwardSteps(backward_steps + 1); // Increment backward steps
        // logger->logActionDetailed(current_step, "Tank " + std::to_string(tankId) + " is waiting for backward move. Step " + std::to_string(backwardSteps - 1) + ".", "");
    }
    else if (backward_steps == 3) {
        // Execute actual backward move
        tank->moveBackward(board->getCols(), board->getRows());
        // Point new_pos = tank->getPosition();
        tank->setBackwardSteps(0); // Reset backward steps after moving
        // logger->logActionDetailed(current_step, "Tank " + std::to_string(tankId) + " moved backward to (" +std::to_string(newPos.getX()) + ", " + std::to_string(newPos.getY()) + ").","");
    }
    else {
        // Should never reach here, fallback
        // logger->logActionDetailed(current_step,
        // "Tank " + std::to_string(tankId) + " failed to move backward from (" +
        // std::to_string(currPos.getX()) + ", " + std::to_string(currPos.getY()) + ").",
        // "invalid backward state");
    }
}

void GameManager::executeRotateLeft(TankData* td, int angle) {
    // Rotate the tank left by the specified angle
    Tank* tank = td->tank;
    // int tankId = tank->getId();

    tank->rotateLeft(angle / 45);  // angle is 45 or 90 → convert to 1 or 2
    // logger->logActionDetailed(current_step,
    //     "Tank " + std::to_string(tankId) + " rotated left by " + std::to_string(angle / 45) + "/8.","");
}

void GameManager::executeRotateRight(TankData* td, int angle) {
    // Rotate the tank right by the specified angle
    Tank* tank = td->tank;
    // int tankId = tank->getId();
    tank->rotateRight(angle / 45);  // angle is 45 or 90 → convert to 1 or 2

    // logger->logActionDetailed(current_step,
    //     "Tank " + std::to_string(tankId) + " rotated right by " + std::to_string(angle / 45) + "/8.","");
}

void GameManager::executeShoot(TankData* td) {
    // This function executes the shoot action for the tank
    Tank* tank = td->tank;
    // int tank_id = tank->getId();
    if (!tank->canShoot()) {
        // logger->logActionDetailed(current_step,
            // "Tank " + std::to_string(tank_id) + " tried to shoot but failed.","cooldown or no ammo");
        return;
    }
    Shell shell = tank->shoot(board->getCols(), board->getRows());
    Point shell_pos = shell.getPosition();
    bool dont_add_shell_to_board = false;
    if (board->isWallAt(shell_pos)) { 
        Wall* wall = dynamic_cast<Wall*>(board->getObjectAt(shell_pos));
        if (wall) {
            if (wall->getHitCount() == 0) {
                wall->incrementHitCount();
            } else {
                board->removeWall(wall);
            }
        }
        dont_add_shell_to_board = true;
    }
    if (!dont_add_shell_to_board) {
        board->addShell(std::move(shell));
    }
}

void GameManager::executeGetBattleInfo(TankData* tank_data) {
    // Execute the GetBattleInfo action for the tank
    if (!this->board->isObjectOnBoard(tank_data->tank))
            return;
    // int id = tank_data->tank->getId();
    int backwardSteps = tank_data->tank->getBackwardSteps();

    // If tank was in backward mode, cancel it
    if (backwardSteps > 0) {
        backwardSteps = 0;
        // logger->logActionDetailed(current_step,"Tank " + std::to_string(id) + " requested battle info and cancelled backward movement.","");
    } else {
        GameBoardSatelliteView view(board.get(), tank_data->tank);
        players[tank_data->playerId-1]->updateTankWithBattleInfo(*tank_data->algorithm, view);
        // logger->logActionDetailed(current_step,"Tank " + std::to_string(id) + " requested battle info.", "");
    }

}

bool GameManager::checkImmediateEnd(const std::vector<Tank*>& p1_tanks, const std::vector<Tank*>& p2_tanks) {
    // This function checks if the game should end immediately due to one player having no tanks left.
    immediateLoseOrTie(p1_tanks, p2_tanks);
    return game_over;
}

void GameManager::immediateLoseOrTie(const std::vector<Tank*>& p1_tanks,const std::vector<Tank*>& p2_tanks) {
    // This function checks if either player has no tanks left, and sets the game_over flag accordingly.
    if (p1_tanks.empty() && p2_tanks.empty()) {
        game_over = true; // Set game over flag
    }
     else {
        if (p1_tanks.empty()) {
            game_over = true; // Set game over flag
        } else if (p2_tanks.empty()) {
            game_over = true; // Set game over flag
     }
    }   
}

GameResult GameManager::buildImmediateResult(const std::vector<Tank*>& p1_tanks, const std::vector<Tank*>& p2_tanks) const {
    // This function builds the GameResult object for an immediate end of the game.
    GameResult result;
    result.reason = GameResult::ALL_TANKS_DEAD;
    result.remaining_tanks.resize(2);
    result.remaining_tanks[0] = p1_tanks.size();
    result.remaining_tanks[1] = p2_tanks.size();
    if (p1_tanks.empty() && p2_tanks.empty()) {
        result.winner = 0; // Tie
    } else if (p1_tanks.empty()) {
        result.winner = 2; // Player 2 wins
    } else {
        result.winner = 1; // Player 1 wins
    }
    result.rounds = 0;
    result.gameState = std::make_unique<GameBoardSatelliteView>(board.get(), nullptr);
    return result;
}

GameResult GameManager::runGameLoop() {
    // This function runs the main game loop, processing player actions and updating the game state.
    while (!game_over && current_step <= board->getMaxSteps()) {
        current_step++;
        std::vector<std::pair<TankData*, ActionRequest>> requests = gatherRequests();
        auto processed = processRequests(requests);
        executeRequests(processed); 
        if (isGameOver()) {
            game_over = true; // Set game over flag if the game is over
            return writeGameResult(); // Exit if the game is over
        }
        updateShellsLocation();
        checkCollisions(); // Check for collisions after executing actions
        if(isGameOver()) {
            game_over = true; // Set game over flag if the game is over
            return writeGameResult(); // Exit if the game is over
        }
        updateGameStatus();
    }
    return GameResult{-1, GameResult::MAX_STEPS, {}, nullptr, 0}; // Return empty result if game is not over
}

void GameManager::updateGameStatus() { 
    // This function updates the game status, checking for game over conditions and updating tank states.
    // checking if both tanks don't have ammunition
    if (allTanksOutOfAmmo() && remaining_step_after_amo == -1) {
        remaining_step_after_amo = 40;
        // std::string message = "Both tanks are out of ammo. 40 steps countdown begins.";
        // logger->logLineDetailed(message);
    }
    if (remaining_step_after_amo > 0) {
        remaining_step_after_amo--;
    }
    if (remaining_step_after_amo == 0) {
        game_over = true;
        writeGameResult(); // Exit if the game is over
        // logger->logLineDetailed("Game ended in a tie: no ammo left after 40 steps.");
    }
    // updating Game State
    for (TankData& td : tanks) {
        if (!this->board->isObjectOnBoard(td.tank)) { continue; }
        td.tank->cooldownModify();
    }
}

bool GameManager::isGameOver() {
    // This function checks if the game is over
    if (isAtLeastOneTankAlive(1) && isAtLeastOneTankAlive(2)) {
        if (current_step >= board->getMaxSteps()) {
            game_over = true; // Set game over flag if max steps reached
                return true; // Game is over
            }
        else if (allTanksOutOfAmmo() && remaining_step_after_amo == 0) {
            game_over = true; // Set game over flag if all tanks are out of ammo
            return true; // Game is over
        }
        return false; // Game is not over
    }
    else {
        if (!isAtLeastOneTankAlive(1) && !isAtLeastOneTankAlive(2)) { //Tie
            game_over = true; // Set game over flag if both players have no tanks left
        }
        if (!isAtLeastOneTankAlive(1) && isAtLeastOneTankAlive(2)) {
            game_over = true; // Player 2 wins
        }
        if (isAtLeastOneTankAlive(1) && !isAtLeastOneTankAlive(2)) {
            game_over = true; // Player 1 wins
        }
        return true; // Game is over
    }
}

void GameManager::checkCollisions() {
    // this function calls all check collision options.
    checkShellWallCollisions();
    checkShellTankCollisions();
    checkShellShellCollisions();
    checkTankMineCollisions();
    checkTankTankCollision();
}

void GameManager::checkShellWallCollisions() {
    // This function checks for collisions between shells and walls and
    // removing shells and walls that are not existed anymore.
    const std::vector<Shell*>& shells = board->getShells();
    const std::vector<Wall*>& walls = board->getWalls();
    std::vector<Shell*> shells_to_remove;
    std::vector<Wall*> walls_to_remove;
    for (Shell* shell : shells) {
        Point shell_pos = shell->getPosition();
        for (Wall* wall : walls) {
            if (wall->getPosition() == shell_pos) {
                // Collision detected
                if (wall->getHitCount() == 0) {
                    // logger->logLineDetailed("Shell " + std::to_string(shell->getId()) +
                    //     " hit a wall at (" + std::to_string(shell_pos.getX()) + ", " +std::to_string(shell_pos.getY()) + ")");
                    wall->incrementHitCount();
                }
                else {
                    walls_to_remove.push_back(wall);
                    // logger->logLineDetailed("Wall at (" + std::to_string(shell_pos.getX()) + ", " +std::to_string(shell_pos.getY()) + ") destroyed.");
                }
                shells_to_remove.push_back(shell);
            }
        }
    }
    for (Shell* shell : shells_to_remove) {
        board->removeShell(shell);
    }
    for (Wall* wall : walls_to_remove) {
        board->removeWall(wall);
    }
}

void GameManager::checkShellShellCollisions() {
    // This function checks for collisions between shells and removing shells that are not existed anymore
    std::vector<Shell*> shells = board->getShells();
    std::vector<Shell*> to_remove;

    for (size_t i = 0; i < shells.size(); ++i) {
        if (!this->board->isObjectOnBoard(shells[i])) {
            continue; // Skip dead shells
        }
        for (size_t j = i + 1; j < shells.size(); ++j) {
            if (!this->board->isObjectOnBoard(shells[j])) {
                continue; // Skip dead shells
            }
            if (shells[i]->getPosition() == shells[j]->getPosition()) {
                // Collision detected
                if (std::find(to_remove.begin(), to_remove.end(), shells[i]) == to_remove.end()) {
                    to_remove.push_back(shells[i]); 
                }
                if (std::find(to_remove.begin(), to_remove.end(), shells[j]) == to_remove.end()) {
                    to_remove.push_back(shells[j]); 
                }
                // logger->logLineDetailed("Shells collided at (" + std::to_string(shells[i]->getPosition().getX()) + ", " +
                    // std::to_string(shells[i]->getPosition().getY()) + ") and both exploded.");
        }
    }
}
// Remove the shells that collided
for (Shell* s : to_remove) {
    board->removeShell(s);
    }
}

void GameManager::checkShellTankCollisions() {
    // This function checks for collisions between shells and tank and update game over if a coliision is detected
    const std::vector<Shell*>& shells = board->getShells();
    const std::vector<Tank*>& tanks = board->getAllTanks();
    std::vector<Shell*> shells_to_remove;
    std::vector<Tank*> tanks_to_remove;
    for (Shell* shell : shells) {
        Point shell_pos = shell->getPosition();
        for (Tank* tank : tanks) {
            if (!this->board->isObjectOnBoard(tank)) {continue;}
            Point tank_pos = tank->getPosition();
            if (tank && shell_pos == tank_pos) {
                tank->setAlive();
                // logger->logLineDetailed("Shell " + std::to_string(shell->getId()) + " hit Tank " + std::to_string(tank->getId()) +" at (" +
                // std::to_string(shell_pos.getX()) + ", " + std::to_string(shell_pos.getY()) + "). This Tank is player" +
                // std::to_string(board->getTankPlayerId(tank)) + " and it destroyed.");
                tanks_to_remove.push_back(tank);
                shells_to_remove.push_back(shell);
                break; // Exit the inner loop after first collision
            }
        }
    }
    for (Shell* shell : shells_to_remove) {
        board->removeShell(shell);
    }
    for (Tank* tank : tanks_to_remove) {
        board->removeTank(tank);
    }
}


void GameManager::checkTankMineCollisions() {
    // This function checks for collisions between shells and mines
    std::vector<Mine*> mines_to_remove;
    std::vector<Tank*> tanks_to_remove;
    const std::vector<Mine*>& mines = board->getMines();
    const std::vector<Tank*>& tanks = board->getAllTanks();
    for (Mine* mine : mines) {
        Point mine_pos = mine->getPosition();
        for (Tank* tank: tanks ) {
            if (!this->board->isObjectOnBoard(tank)) {continue;}
            Point tank_pos = tank->getPosition();
            if (tank && mine_pos == tank_pos) { // Collision detected: tank on mine
            tank->setAlive(); 
            // logger->logLineDetailed("Tank " + std::to_string(tank->getId()) + " of player " + std::to_string(board->getTankPlayerId(tank)) +
            // " stepped on a mine at (" + std::to_string(tank_pos.getX()) + ", " + std::to_string(tank_pos.getY()) + "). Both are destroyed.");
            
            tanks_to_remove.push_back(tank);
            mines_to_remove.push_back(mine);
            }
        }
        break;  // Exit the loop after first mine collision
    }
    // Remove the mines that were stepped on
    for (Mine* mine : mines_to_remove) {
        board->removeMine(mine);
    }
    for (Tank* tank : tanks_to_remove) {
        board->removeTank(tank);
    }
}


void GameManager::checkTankTankCollision() { 
    // Check for collisions between tanks and if a collision detected
    std::vector<Tank*> tanks1 = board->getTanksPlayer(1);
    std::vector<Tank*> tanks2 = board->getTanksPlayer(2);
    std::vector<Tank*> tanks_to_remove;
     
    for (Tank* tank1 : tanks1) {
        if (!this->board->isObjectOnBoard(tank1)) {continue;}
        Point tank1_pos = tank1->getPosition();
        for (Tank* tank2: tanks2 ) {
            if (!this->board->isObjectOnBoard(tank2)) {continue;}
            Point tank2_pos = tank2->getPosition();
            if (tank1 && tank2 && tank1_pos == tank2_pos) {
                tank1->setAlive();
                tank2->setAlive();
                // logger->logLineDetailed("Tank " + std::to_string(tank1->getId()) + " of player 1 and Tank" + std::to_string(tank2->getId()) + " of player 2 collided at (" +
                //     std::to_string(tank1_pos.getX()) + ", " +
                //     std::to_string(tank1_pos.getY()) + "). Both are destroyed.");
            tanks_to_remove.push_back(tank1);
            tanks_to_remove.push_back(tank2);
            }
        }
        break;
    }

     for (Tank* tank : tanks_to_remove) {
        board->removeTank(tank);
        
    }
}

void GameManager::gatherRequests(std::vector<std::pair<TankData*, ActionRequest>>& actions)
 {
    // This function gathers action requests from all tanks and stores them in a vector.
    for (TankData& td : tanks) {
        if (!this->board->isObjectOnBoard(td.tank)) {
            continue;
        }
        ActionRequest req = td.algorithm->getAction();
        actions.emplace_back(&td, req);
    }
}

void GameManager::executeRequests(const std::list<std::tuple<TankData*, ActionRequest, bool>>& actions) {
    // This function  the gathered action requests and updates the game state.
    for (const auto& [td, req, is_approved] : actions) {
        if (!this->board->isObjectOnBoard(td->tank)) {
            continue; // Skip dead tanks
        }
        // std::string action_name = shortActionName(req);
        std::string reason;
        if (is_approved) {
            executeAction(td, req);
            }
    }
    consolidateActions(actions);
}


void GameManager:: consolidateActions(const std::list<std::tuple<TankData*, ActionRequest, bool>>& actions) {
    //This function consolidates the actions of all tanks and updates their states.
    int i = 1, j = 1;
    std::vector<int> dead_tanks;
    for (TankData& td : tanks) {
        if (!this->board->isObjectOnBoard(td.tank)) { dead_tanks.push_back(i);}
        i++; // get dead tanks index
    }
    checkCollisions();
    i = 1;
    int size_tanks = tanks.size();
    for (const auto& [td, req, is_approved] : actions) {
        for (int dead_tank : dead_tanks) {
            if (i == dead_tank && i != size_tanks-1) {
                i++;
                // logger->logLine("killed, ",false );
            }
            else { if (i == dead_tank) {
                i++;
                // logger->logLine("killed",true); } }
        }
        // bool is_last = (j == size_tanks);
        if (is_approved && !this->board->isObjectOnBoard(td->tank)) {
            // logger->logActionSummary(shortActionName(req), false, true, is_last); 
            }
        else {
            if (!is_approved && !this->board->isObjectOnBoard(td->tank)) {
                // logger->logActionSummary(shortActionName(req), true, true, is_last); 
                }
        else {
            if (!is_approved) {
                // logger->logActionSummary(shortActionName(req), true, false, is_last); 
                }
            else { 
                // logger->logActionSummary(shortActionName(req), false, false, is_last); 
                }
        }
    }
    i++, j++;
}
    // logger->logLine("");
    // logger->logLineDetailed("");
}
    }
}

GameResult GameManager::writeGameResult() {
    // This function updates the game result based on the current game state and checks if the game is over.
    GameResult result;
    bool player1_alive = isAtLeastOneTankAlive(1);
    bool player2_alive = isAtLeastOneTankAlive(2);
    result.rounds = current_step;
    result.gameState = std::make_unique<GameBoardSatelliteView>(board.get(), nullptr);
    result.remaining_tanks.resize(2);
    result.remaining_tanks[0] = countAliveTanks(1);
    result.remaining_tanks[1] = countAliveTanks(2);
    // deciding result game
    if (player1_alive && !player2_alive) { // Player 1 wins
        result.winner = 1;
        game_over = true;
        result.reason = GameResult::ALL_TANKS_DEAD;
    } else if (!player1_alive && player2_alive) { // player 2 wins
        game_over = true;
        result.winner = 2;
        result.reason = GameResult::ALL_TANKS_DEAD;
    } else if (!player1_alive && !player2_alive) { //Tie
        result.winner = 0; 
        result.reason = GameResult::ALL_TANKS_DEAD;
        game_over = true;
    } else if (remaining_step_after_amo == 0) {
        game_over = true;
        result.winner = 0;
        result.reason = GameResult::ZERO_SHELLS;
    } else if (board->getMaxSteps() == current_step) {
        game_over = true;
        result.winner = 0;
        result.reason = GameResult::MAX_STEPS;
    } else {
        game_over = false; // Game is still ongoing
        return GameResult{-1, GameResult::MAX_STEPS, {}, nullptr, 0}; // Return empty result if game is not over
    }	
    return GameResult{-1, GameResult::MAX_STEPS, {}, nullptr, 0};
}

int GameManager::countAliveTanks(int player_index) {
    // This function counts the number of tanks that are still alive for the specified player index
    int count = 0;
    for (const TankData& t : tanks) {
        if (!this->board->isObjectOnBoard(t.tank))
            continue;
        if (t.playerId == player_index) {
            count++;
        }
    }
    return count;
}


void GameManager::updateShellsLocation() {
    // Update the location of all shells on the board
    // checking future collision in 1 point ahead andA 2 point ahead
    checkShellFutureCollisions(1); 
    checkShellFutureCollisions(2);
    moveShellTwoPoints();
}

void GameManager::checkShellFutureCollisions(int square) {
    // This function check if shells will be detected as collided in specific offset
    const std::vector<Shell*>& shells = board->getShells();
    std::vector <Shell*> shells_to_remove;
    std::vector <Tank*> tanks_to_remove;
    std::vector<Wall*> walls_to_remove;
    std::vector<Mine*> mines_to_remove;
    for (Shell* shell : shells) {
        if (this->board->isObjectOnBoard(shell) && !shell->getNewShell()) {
            Point original_position = shell->getPosition();
            std::pair <int, int> offset = directionOffset(shell->getDirection());
            // Check collisions using the new position
            int new_x = (original_position.getX() + square * offset.first + board->getCols()) % board->getCols();
            int new_y = (original_position.getY() + square * offset.second + board->getRows()) % board->getRows();
            Point new_position(new_x, new_y);
            for (TankData& tank : tanks) {
                if (tank.tank->getPosition() == new_position) {// Collision detected
                    shells_to_remove.push_back(shell);
                    tanks_to_remove.push_back(tank.tank);
                    tank.alive = false;
                    break;
                }
            }
            if(isGameOver()) {
                return; // Exit if the game is over
            }
            for (Shell* other_shell : shells) {
                if (other_shell != shell && other_shell->getPosition() == new_position) { // Collision detected
                    shells_to_remove.push_back(shell);
                    shells_to_remove.push_back(other_shell);
                    break;
                }
            }
            for (Wall* wall : board->getWalls()) { // Collision with wall
                if (wall->getPosition() == new_position) {
                    shells_to_remove.push_back(shell);
                    walls_to_remove.push_back(wall);
                }
            }
            for (Mine* mine : board->getMines()) { // Collision with mine
                if (mine->getPosition() == new_position) {
                    shells_to_remove.push_back(shell);
                    mines_to_remove.push_back(mine);
                }
            }
        }
    }
    removeCollidedItems(shells_to_remove, tanks_to_remove, walls_to_remove, mines_to_remove); // Remove items that collided
}

void GameManager::removeCollidedItems(const std::vector<Shell*>& shells_to_remove, const std::vector<Tank*>& tanks_to_remove, const std::vector<Wall*>& walls_to_remove, const std::vector<Mine*>& mines_to_remove) {
    // This function removes the shells, tanks, walls, and mines that collided
    // Remove the shells that collided
    for (Shell* shell : shells_to_remove) {
        board->removeShell(shell);
    }
    // Remove the tanks that collided
    for (Tank* tank : tanks_to_remove) {
        board->removeTank(tank);
    }
    // Remove the walls that collided
    for (Wall* wall : walls_to_remove) {
        board->removeWall(wall);
    }
    // Remove the mines that collided
    for (Mine* mine : mines_to_remove) {
        board->removeMine(mine);
    }
}

void GameManager::moveShellTwoPoints() {
    // This function moves shell  by two points ahead toward their direction
    const std::vector<Shell*>& shells = board->getShells();
    for (Shell* shell : shells) {
        if (!this->board->isObjectOnBoard(shell)) {
            continue; // Skip dead shells
        }
        if (shell != nullptr && !shell->getNewShell()) {
            // Move the shell by 2 points
            shell->move(board->getCols(), board->getRows());
            // logger->logLineDetailed("Shell " + std::to_string(shell->getId()) + " moved to (" +
            //     std::to_string(shell->getPosition().getX()) + ", " +
            //     std::to_string(shell->getPosition().getY()) + ")");
        }        

    if (shell->getNewShell()) {
        shell->setNewShell();
    }
 }
}


bool GameManager::allTanksOutOfAmmo() {
    // Check if all tanks are out of ammunition
    bool all_out_of_ammo = true;
    for (const TankData& td : tanks) {
        if (!this->board->isObjectOnBoard(td.tank))
            continue;

        if (td.tank->getAmmoCount() > 0) {
            all_out_of_ammo = false;
        }
    }
    return all_out_of_ammo;
}

bool GameManager::isAtLeastOneTankAlive(int player_index) const {
    // This function checks if at least one tank belonging to the specified player is still alive.
    // Returns true if at least one tank is alive for the given player_index, false otherwise
    bool dead = true;
    for (const TankData& t : tanks) {
        if (!this->board->isObjectOnBoard(t.tank))
            continue;
        if (t.playerId == player_index) {
            dead = false;
        }
    }
    return !dead;

}
} // namespace GameManager_206480972_206899163  
