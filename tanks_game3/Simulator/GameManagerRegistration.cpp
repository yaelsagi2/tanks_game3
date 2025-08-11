#include "GameManagerRegistrar.h"
#include "common/GameManagerRegistration.h"

GameManagerRegistration::GameManagerRegistration(std::function<std::unique_ptr<AbstractGameManager>(bool)> factory) {
    auto& registrar = GameManagerRegistrar::getGameManagerRegistrar();
    registrar.addGameManagerFactory(std::move(factory));
}
