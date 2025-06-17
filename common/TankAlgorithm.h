#pragma once
#include "common/ActionRequest.h"
#include "common/BattleInfo.h"

class TankAlgorithm {
public:
	virtual ~TankAlgorithm() {}
    virtual ActionRequest getAction() = 0;
    virtual void updateBattleInfo(BattleInfo& info) = 0;
};
