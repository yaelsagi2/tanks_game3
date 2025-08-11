#include "GameManager.h"
#include "GameBoard.h"
#include "Logger.h"
#include "Utils.h"
#include "Player2.h"
#include "Player1.h"
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>

bool GameManager::readBoard(const std::string& filename) {
    // This function reads the game board from a file and initializes the game state.
    // extracting the board size and max steps from the file, and tanks for each player.
    this->logger = std::make_unique<Logger>(filename);
    std::vector<std::string> errors;
    board = readFile(filename, errors);
    if (!board) {
        std::cerr << "Failed to read board from file: " << filename << std::endl;
        for (const auto& err : errors) {
            std::cerr << err << std::endl;
        }
        game_over = true;
        return false;
    }

    const std::vector<Tank*>& p1_tanks = board->getPlayerTanks(1);
    const std::vector<Tank*>& p2_tanks = board->getPlayerTanks(2);
    immediateLoseOrTie(p1_tanks, p2_tanks); 
    if(game_over)
        return true; // If game is over, no need to proceed further
    players.push_back(playerFactory->create(1, board->getRows(),board->getCols(),
                        board->getMaxSteps(), p1_tanks[0]->getAmmoCount()));
    players.push_back(playerFactory->create(2, board->getRows(), board->getCols(),
                        board->getMaxSteps(), p2_tanks[0]->getAmmoCount()));
    initAllTanksSorted();
    logInitialPositions();
    return true;

}

void GameManager::initAllTanksSorted() {
    // This function initializes all tanks in the game, assigning them IDs and creating their algorithms.
    // It combines tanks from both players, sorts them by their position from top left to bottom right and assigns IDs.
    // 1. Combine all tanks
    std::vector<Tank*> all_tanks = board->getPlayerTanks(1);
    const std::vector<Tank*>& p2_tanks = board->getPlayerTanks(2);
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
        auto algo = tankFactory->create(player_id, tank_id);
        tanks.push_back(TankData{std::move(algo), player_id, tank, true});
    }
}

void GameManager::immediateLoseOrTie(const std::vector<Tank*>& p1_tanks,const std::vector<Tank*>& p2_tanks) {
    // This function checks if a tie has occurred in the game.
    // A tie occurs when both players have no tanks left.
    if (p1_tanks.empty() && p2_tanks.empty()) {
        game_over = true; // Set game over flag
        logger->logLine("Tie, both players have zero tanks");
        logger->logLineDetailed("Tie ,Both players have no tanks left.");
    }
     else {
        if (p1_tanks.empty()) {
            game_over = true; // Set game over flag
            const int player_winner = 2;
            logger->logLine("Player " + std::to_string(player_winner) + " won with " + std::to_string(p2_tanks.size()) + " tanks still alive");
            logger->logLineDetailed("Player 1 has no tanks left, Player 2 has " + std::to_string(p2_tanks.size()) + " tanks alive.");
            logger->logFinal("Player 2 wins", false);
        } else if (p2_tanks.empty()) {
            game_over = true; // Set game over flag
            const int player_winner = 1;
            logger->logLine("Player " + std::to_string(player_winner) + " won with " + std::to_string(p1_tanks.size()) + " tanks still alive");
            logger->logLineDetailed("Player 2 has no tanks left, Player 1 has " + std::to_string(p1_tanks.size()) + " tanks alive.");
            logger->logFinal("Player 1 wins", false);
     }
    }   
}

void GameManager::logInitialPositions() {
    // This function logs the initial positions of the tanks at the start of the game.
    logger->logActionDetailed(current_step, "Game started");
    logger->logActionDetailed(current_step, "Rows:" + std::to_string(board->getRows()) +", Cols: " + std::to_string(board->getCols()) +
        ", Max Steps: " + std::to_string(board->getMaxSteps()));
    for (const TankData& t : tanks) {
        Point pos = t.tank->getPosition();
        logger->logActionDetailed(current_step, "Tank " + std::to_string(t.tank->getId()) + " of Player " + std::to_string(t.playerId) + " starts at (" + std::to_string(pos.getX()) +
            ", " + std::to_string(pos.getY()) + ")."
        );
    }
}

void GameManager::run() {
    //This function runs the game loop, processing each step until the game is over.
    if (!board) {
        std::cerr << "Game board is not initialized. Cannot run the game." << std::endl;
        return;
    }
    int maxSteps = board->getMaxSteps();
    for (int step = 0; step < maxSteps && !game_over; ++step) {
        std::vector<std::pair<TankData*, ActionRequest>> actions;
        gatherRequests(actions);
        auto approvedActions = processRequests(actions);
        executeRequests(step, approvedActions);
        board->updateAllObjectsMap();
        current_step++;
    }
    if (!game_over && current_step == board->getMaxSteps()) {
        writeGameResult();
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

void GameManager::updateGameStatus() { 
    // This function updates the game status, checking for game over conditions and updating tank states.
    if (game_over) { return; }
    else {
    // checking if both tanks don't have ammunition
    if (allTanksOutOfAmmo() && remaining_step_after_amo == -1) {
        remaining_step_after_amo = 40;
        std::string message = "Both tanks are out of ammo. 40 steps countdown begins.";
        logger->logLineDetailed(message);
    }
     if (remaining_step_after_amo > 0) {
        remaining_step_after_amo--;
        if (remaining_step_after_amo == 0) {
            game_over = true;
            logger->logLineDetailed("Game ended in a tie: no ammo left after 40 steps.");
        }
    }
    writeGameResult();
    if (game_over) {return;}
    // updating Game State
    updateShellsLocation();
    checkCollisions();
    for (TankData& td : tanks) {
        if (!this->board->isObjectOnBoard(td.tank)) { continue;}
        updateTankState(td.tank); 
    }
    writeGameResult();
    if (game_over) {return;}
    }
}

void GameManager::updateTankState(Tank* tank) {
    // This function update Tank State - by updating cooldown
    if (!this->board->isObjectOnBoard(tank))
        return;
    tank->cooldownModify();
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
    std::vector<Shell*> shellsToRemove;
    std::vector<Wall*> wallsToRemove;
    for (Shell* shell : shells) {
        Point shellPos = shell->getPosition();
        for (Wall* wall : walls) {
            if (wall->getPosition() == shellPos) {
                // Collision detected
                if (wall->getHitCount() == 0) {
                    logger->logLineDetailed("Shell " + std::to_string(shell->getId()) +
                        " hit a wall at (" + std::to_string(shellPos.getX()) + ", " +std::to_string(shellPos.getY()) + ")");
                    wall->incrementHitCount();
                }
                else { 
                    wallsToRemove.push_back(wall);
                    logger->logLineDetailed("Wall at (" + std::to_string(shellPos.getX()) + ", " +std::to_string(shellPos.getY()) + ") destroyed.");
                }    
                shellsToRemove.push_back(shell);
            }
        }
    }
    for (Shell* shell : shellsToRemove) {
        board->removeShell(shell);
    }
    for (Wall* wall : wallsToRemove) {
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
                logger->logLineDetailed("Shells collided at (" + std::to_string(shells[i]->getPosition().getX()) + ", " +
                    std::to_string(shells[i]->getPosition().getY()) + ") and both exploded.");
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
                logger->logLineDetailed("Shell " + std::to_string(shell->getId()) + " hit Tank " + std::to_string(tank->getId()) +" at (" +
                std::to_string(shell_pos.getX()) + ", " + std::to_string(shell_pos.getY()) + "). This Tank is player" +
                std::to_string(board->getTankPlayerId(tank)) + " and it destroyed.");
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
            
            logger->logLineDetailed("Tank " + std::to_string(tank->getId()) + " of player " + std::to_string(board->getTankPlayerId(tank)) +
            " stepped on a mine at (" + std::to_string(tank_pos.getX()) + ", " + std::to_string(tank_pos.getY()) + "). Both are destroyed.");
            
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
                logger->logLineDetailed("Tank " + std::to_string(tank1->getId()) + " of player 1 and Tank" + std::to_string(tank2->getId()) + " of player 2 collided at (" +
                    std::to_string(tank1_pos.getX()) + ", " +
                    std::to_string(tank1_pos.getY()) + "). Both are destroyed.");
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

void GameManager::executeRequests(int currentStep, const std::list<std::tuple<TankData*, ActionRequest, bool>>& actions) {
    // This function  the gathered action requests and updates the game state.
    for (const auto& [td, req, is_approved] : actions) {
        if (!this->board->isObjectOnBoard(td->tank)) {
            continue; // Skip dead tanks
        }
        std::string action_name = shortActionName(req);
        std::string reason;
        if (is_approved) {
            executeAction(td, req);
            logger->logActionDetailed(currentStep, "Tank " + std::to_string(td->tank->getId()) + " of player " + std::to_string(td->playerId) + " performed " + action_name, "");
            }
        else {
            reason = getIgnoreReason(td->tank, req);
            logger->logActionDetailed(currentStep, "Tank " + std::to_string(td->tank->getId()) + " of player " + std::to_string(td->playerId) + " tried " + action_name, reason);
        }
      }
    consolidateActions(actions);
    if (game_over) {
        return;
    }
    updateGameStatus();
    if (allTanksOutOfAmmo() && remaining_step_after_amo == 40) {
        game_over = true;
    }
}


void GameManager:: consolidateActions(const std::list<std::tuple<TankData*, ActionRequest, bool>>& actions) {
    //This function consolidates the actions of all tanks and updates their states.
    int i = 1, j = 1;
    std::vector<int> dead_tanks;
    for (TankData& td : tanks) {
        if (!this->board->isObjectOnBoard(td.tank)) { dead_tanks.push_back(i);}
        i++; // get dea
    }
    checkCollisions();
    i = 1;
    int size_tanks = tanks.size();
    for (const auto& [td, req, is_approved] : actions) {
        for (int dead_tank : dead_tanks) {
            if (i == dead_tank && i != size_tanks-1) {
                i++;
                logger->logLine("killed, ",false );
            }
            else { if (i == dead_tank) {
                i++;
                logger->logLine("killed",true); } }
        }
        bool is_last = (j == size_tanks);
        if (is_approved && !this->board->isObjectOnBoard(td->tank)) {
            logger->logActionSummary(shortActionName(req), false, true, is_last); }
        else {
            if (!is_approved && !this->board->isObjectOnBoard(td->tank)) {
                logger->logActionSummary(shortActionName(req), true, true, is_last); }
        else {
            if (!is_approved) {
                logger->logActionSummary(shortActionName(req), true, false, is_last); }
            else { 
                logger->logActionSummary(shortActionName(req), false, false, is_last); }
        }
    }
    i++, j++;
}
    logger->logLine("");
    logger->logLineDetailed("");
    writeGameResult();
}

Point GameManager::getNextPosition(const Point& from, Direction dir, int steps) {
    // Calculate the next position based on the current position, direction, and number of steps
    std::pair <int, int> offset = directionOffset(dir);
    int dx = offset.first;
    int dy = offset.second;

    int newX = (from.getX() + dx * steps + board->getRows()) % board->getRows();
    int newY = (from.getY() + dy * steps + board->getCols()) % board->getCols();
    return Point(newX, newY);
}

bool GameManager::isActionLegal(Tank* tank, const ActionRequest& action) {
    // Check if the action is legal for the given tank
    if (!tank || !this->board->isObjectOnBoard(tank)) {
        return false; // If tank is null or not on the board, action is illegal
    }
    int backward_steps = tank->getBackwardSteps();     // If the tank is in the middle of a backward move, only backward actions are allowed
    if (backward_steps >= 1 && action != ActionRequest::MoveBackward) {
        if (action == ActionRequest::MoveForward || action == ActionRequest::GetBattleInfo ) { // moving forward cancelling
            tank->setBackwardSteps(0); // Cancel backward move            
            logger->logActionDetailed(current_step, "Tank " + std::to_string(tank->getId()) + " cancelled backward move at (" +
            std::to_string(tank->getPosition().getX()) + ", " + std::to_string(tank->getPosition().getY()) + ").", "");
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
            if (backward_steps >= 3) {
                Point backPos = getNextPosition(tank->getPosition(), tank->getCanonDir(), -1);
                return !(board->isWallAt(backPos));
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


std::list<std::tuple<TankData*, ActionRequest, bool>> GameManager::processRequests(const std::vector<std::pair<TankData*, ActionRequest>>& actions) {
    // This function processes the gathered action requests and executes them.
    std::list<std::tuple<TankData*,ActionRequest ,bool>> approvedActions;
    for (const auto& [td, req] : actions) {
        if (!this->board->isObjectOnBoard(td->tank)) {
            continue;
        }
        if (isActionLegal(td->tank, req)) {
            approvedActions.emplace_back(td,req,true);
        }
        else {
            approvedActions.emplace_back(td,req,false);
        }
        }
    return approvedActions;
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
    // Move the tank forward in the direction it is facing
    if (!this->board->isObjectOnBoard(td->tank))
        return; // If tank is not on the board, do nothing
    Tank* tank = td->tank;
    int tankId = tank->getId();
    Point oldPos = tank->getPosition();

    // If tank was in backward movement, cancel it
    int backwardSteps = tank->getBackwardSteps();
    if (backwardSteps > 0) {
        tank->setBackwardSteps(0); // Cancel backward move
        logger->logActionDetailed(current_step, "Tank " + std::to_string(tankId) + " cancelled backward move at (" +
            std::to_string(oldPos.getX()) + ", " + std::to_string(oldPos.getY()) + ").","");
        return;
    }

    // Perform move forward
    tank->moveForward(board->getCols(), board->getRows());
    Point newPos = tank->getPosition();

    logger->logActionDetailed(current_step, "Tank " + std::to_string(tankId) + " moved forward to (" +
        std::to_string(newPos.getX()) + ", " + std::to_string(newPos.getY()) + ").", "");
}
    
void GameManager::executeMoveBackward(TankData* td) {
    // Move the tank backward in the direction it is facing
    if (!this->board->isObjectOnBoard(td->tank))
        return; // If tank is not on the board, do nothing
    Tank* tank = td->tank;
    int tankId = tank->getId();
    Point currPos = tank->getPosition();
    int backwardSteps = tank->getBackwardSteps();
    if (backwardSteps == 0) {
        tank->setBackwardSteps(1); // Start backward move
        logger->logActionDetailed(current_step, "Tank " + std::to_string(tankId) + " initiated backward move. Waiting 2 steps.","");
    }
    else if (backwardSteps == 1 || backwardSteps == 2) { // Still waiting before actual move
        tank->setBackwardSteps(backwardSteps + 1); // Increment backward steps
        logger->logActionDetailed(current_step, "Tank " + std::to_string(tankId) + " is waiting for backward move. Step " + std::to_string(backwardSteps - 1) + ".", "");
    }
    else if (backwardSteps == 3) {
        // Execute actual backward move
        tank->moveBackward(board->getCols(), board->getRows());
        Point newPos = tank->getPosition();
        tank->setBackwardSteps(0); // Reset backward steps after moving
        logger->logActionDetailed(current_step, "Tank " + std::to_string(tankId) + " moved backward to (" +std::to_string(newPos.getX()) + ", " + std::to_string(newPos.getY()) + ").","");
    }
    else {
        // Should never reach here, fallback
        logger->logActionDetailed(current_step,
            "Tank " + std::to_string(tankId) + " failed to move backward from (" +
            std::to_string(currPos.getX()) + ", " + std::to_string(currPos.getY()) + ").",
            "invalid backward state");
    }
}

void GameManager::executeRotateLeft(TankData* td, int angle) {
    // Rotate the tank left by the specified angle
    Tank* tank = td->tank;
    int tankId = tank->getId();

    tank->rotateLeft(angle / 45);  // angle is 45 or 90 → convert to 1 or 2
    logger->logActionDetailed(current_step,
        "Tank " + std::to_string(tankId) + " rotated left by " + std::to_string(angle / 45) + "/8.","");
}

void GameManager::executeRotateRight(TankData* td, int angle) {
    // Rotate the tank right by the specified angle
    Tank* tank = td->tank;
    int tankId = tank->getId();
    tank->rotateRight(angle / 45);  // angle is 45 or 90 → convert to 1 or 2

    logger->logActionDetailed(current_step,
        "Tank " + std::to_string(tankId) + " rotated right by " + std::to_string(angle / 45) + "/8.","");
}

void GameManager::executeShoot(TankData* td) {
    // Execute the shoot action for the tank
    Tank* tank = td->tank;
    int tankId = tank->getId();
    if (!tank->canShoot()) {
        logger->logActionDetailed(current_step,
            "Tank " + std::to_string(tankId) + " tried to shoot but failed.","cooldown or no ammo");
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
    int id = tank_data->tank->getId();
    int backwardSteps = tank_data->tank->getBackwardSteps();

    // If tank was in backward mode, cancel it
    if (backwardSteps > 0) {
        backwardSteps = 0;
        logger->logActionDetailed(current_step,"Tank " + std::to_string(id) + " requested battle info and cancelled backward movement.","");
    } else {
        GameBoardSatelliteView view(*board, tank_data->tank);
        players[tank_data->playerId-1]->updateTankWithBattleInfo(*tank_data->algorithm, view);
        logger->logActionDetailed(current_step,"Tank " + std::to_string(id) + " requested battle info.", "");
    }

}

bool GameManager::checkImmediateShellCollision(const Shell& shell) {
    // This function checks if the shell collides with any existing shells on the board
    Point shell_pos = shell.getPosition();
    std::vector<Shell*> existingShells = board->getShells();

    for (auto it = existingShells.begin(); it != existingShells.end(); ++it) {
        if ((*it)->getPosition() == shell_pos) {
            logger->logActionDetailed(current_step,
                "Shells collided at (" + std::to_string(shell_pos.getX()) + ", " +
                std::to_string(shell_pos.getY()) + ") and both exploded.", "");
            board->removeObjectAt(shell_pos);
            return true;
        }
    }
    return false;
}

std::string GameManager::shortActionName(ActionRequest req) const {
    // This function returns a short name for the action request
    switch (req) {
        case ActionRequest::MoveForward:    return "MoveForward";
        case ActionRequest::MoveBackward:   return "MoveBackward";
        case ActionRequest::RotateLeft90:   return "RotateLeft90";
        case ActionRequest::RotateRight90:  return "RotateRight90";
        case ActionRequest::RotateLeft45:   return "RotateLeft45";
        case ActionRequest::RotateRight45:  return "RotateRight45";
        case ActionRequest::Shoot:          return "Shoot";
        case ActionRequest::GetBattleInfo:  return "GetBattleInfo";
        case ActionRequest::DoNothing:      return "DoNothing";
        default:                            return "UnknownAction";
    }
}

std::string GameManager::getIgnoreReason(Tank* tank, const ActionRequest& action) {
    // This function returns the reason why an action is ignored for a tank
    if (!tank || !this->board->isObjectOnBoard(tank)) { return "tank is not on the board";}
    int backwardSteps = tank->getBackwardSteps();
    if (backwardSteps > 0) {
        if (action == ActionRequest::MoveBackward) { return "";}
        if (action == ActionRequest::MoveForward || action == ActionRequest::GetBattleInfo) { return ""; }
        return "not allowed due to backward movement";
    }
    switch (action) {
        case ActionRequest::MoveForward: {
            Point next = getNextPosition(tank->getPosition(), tank->getCanonDir(), 1);
            if (board->isWallAt(next)) { return "wall ahead"; }
            return "";
        }
        case ActionRequest::MoveBackward: {
            if (backwardSteps >= 3) {
                Point back = getNextPosition(tank->getPosition(), tank->getCanonDir(), -1);
                if (board->isWallAt(back)) { return "wall behind"; }
            }
            return "";
        }
        case ActionRequest::Shoot:
            if (!tank->canShoot()) {
                return "cooldown or no ammo";
            }
            return "";
        case ActionRequest::RotateLeft45:
        case ActionRequest::RotateLeft90:
        case ActionRequest::RotateRight45:
        case ActionRequest::RotateRight90:
        case ActionRequest::GetBattleInfo:
        case ActionRequest::DoNothing:
            return "";  
        default:
            return "unknown or invalid action";
    }
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

void GameManager::writeGameResult() {
    // Write the game result to a file, tie if both tanks are destroyed or win if one is destroyed
    std::string result;
    bool player1_alive = isAtLeastOneTankAlive(1);
    bool player2_alive = isAtLeastOneTankAlive(2);
    // deciding result game
    if (player1_alive && !player2_alive) {
        game_over = true;
        const int winner_id = 1;
        int player1_tanks_count = countAliveTanks(1);
        result = "Player "+ std::to_string(winner_id)+ " won with " + std::to_string(player1_tanks_count) + " tanks still alive";
    } else if (!player1_alive && player2_alive) {
        game_over = true;
        const int winner_id = 2;
        int player2_tanks_count = countAliveTanks(2);
        result ="Player "+ std::to_string(winner_id) + " won with " + std::to_string(player2_tanks_count) + " tanks still alive";
    } else if (!player1_alive && !player2_alive) {
        game_over = true;
        result = "Tie, both players have zero tanks";
    } else if (remaining_step_after_amo == 0) {
        game_over = true;
        const int extra_steps = 40;
        result = "Tie, both players have zero shells for "+ std::to_string(extra_steps)+" steps";
    } else if (board->getMaxSteps() == current_step) { 
        game_over = true;
        const int max_steps = board->getMaxSteps();
        result = "Tie, reached max steps=" + std::to_string(max_steps) + ", player 1 has " + std::to_string(countAliveTanks(1)) + " tanks, player 2 has " + std::to_string(countAliveTanks(2)) + " tanks";
    } else {
        return;
    }	
    logger->logFinal(result);
}

void GameManager::updateShellsLocation() {
    // Update the location of all shells on the board
    // checking future collision in 1 point ahead and 2 point ahead
    checkShellFutureCollisions(1); 
    checkShellFutureCollisions(2);
    moveShellTwoPoints();
}


void GameManager::checkShellFutureCollisions(int square) {
    // This function check if shells will be detected as collided in specific offset
    const std::vector<Shell*>& shells = board->getShells();
    for (Shell* shell : shells) {
        if (this->board->isObjectOnBoard(shell) && !shell->getNewShell()) {
            Point originalPosition = shell->getPosition();
            std::pair<int, int> offset = directionOffset(shell->getDirection());
            // Check collisions using the new position
            int newX = (originalPosition.getX() + square * offset.first + board->getCols()) % board->getCols();
            int newY = (originalPosition.getY() + square * offset.second + board->getRows()) % board->getRows();
            Point newPosition(newX, newY);
            checkShellTankCollisions();
            writeGameResult();
            if (game_over) {
                return;
            }
            checkShellShellCollisions();
            checkShellWallCollisions();
        }        
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
            logger->logLineDetailed("Shell " + std::to_string(shell->getId()) + " moved to (" +
                std::to_string(shell->getPosition().getX()) + ", " +
                std::to_string(shell->getPosition().getY()) + ")");
        }        

    if (shell->getNewShell()) {
        shell->setNewShell();
    }
 }
}

