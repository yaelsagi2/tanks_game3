#pragma once

#include "common/TankAlgorithmFactory.h"
#include "common/TankAlgorithm.h"
#include "HybridTankAlgorithm.h"

/**
 * @class MyTankAlgorithmFactory
 * @brief Factory class for creating TankAlgorithm instances for tanks.
 *
 * Implements the TankAlgorithmFactory interface and provides a method to create
 * a new TankAlgorithm (such as HybridTankAlgorithm) for a given player and tank index.
 */
class MyTankAlgorithmFactory : public TankAlgorithmFactory {
public:
    /**
     * @brief Creates a new TankAlgorithm instance for the specified player and tank.
     * @param playerId The ID of the player (1 or 2).
     * @param tankIndex The index of the tank for the player.
     * @return A unique pointer to the created TankAlgorithm.
     */
    std::unique_ptr<TankAlgorithm> create(int playerId, int tankIndex) const override;
};
