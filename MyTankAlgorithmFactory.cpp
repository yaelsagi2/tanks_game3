#include "MyTankAlgorithmFactory.h"
#include "HybridTankAlgorithm.h"

// Create and return a unique pointer to a HybridTankAlgorithm instance
// playerId: The ID of the player (1 or 2)
// tankIndex: The index of the tank for the player
std::unique_ptr<TankAlgorithm> MyTankAlgorithmFactory::create(int playerId, int tankIndex) const {
    if (playerId == 1) { 
        return std::make_unique<HybridTankAlgorithm>(playerId, tankIndex, 5, 2, 5); 
        // Player 1 uses a more aggressive strategy with higher path recalculation interval and lower shell threat radius
    }
    else {
        return std::make_unique<HybridTankAlgorithm>(playerId, tankIndex, 3, 4, 5); 
        // Player 2 uses a more defensive strategy with lower path recalculation interval and higher shell threat radius
    }
}
