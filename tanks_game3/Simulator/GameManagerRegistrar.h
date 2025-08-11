
#pragma once
#include <vector>
#include <memory>
#include <functional>
#include "../common/AbstractGameManager.h"

class GameManagerRegistrar {
public:

    GameManagerRegistrar() {}

    static GameManagerRegistrar& getGameManagerRegistrar();

    void addGameManagerFactory(std::function<std::unique_ptr<AbstractGameManager>(bool)>&& factory) {
        managers.emplace_back(factory);
    }

    auto begin() const {
        return managers.begin();
    }
    auto end() const {
        return managers.end();
    }
    auto getAt(int index) const {
        return managers[index];
    }
    std::size_t count() const { return managers.size(); }
    void clear() { managers.clear(); }

    static GameManagerRegistrar registrar;

private:
    std::vector<std::function<std::unique_ptr<AbstractGameManager>(bool)>> managers;
};



