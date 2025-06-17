#pragma once
#include <cstddef> // for size_t
#include "TankAlgorithm.h"
#include "SatelliteView.h"
class Player {
public:
	Player(int player_index, size_t cols, size_t rows, size_t max_steps, size_t num_shells) {
    (void)player_index; 
    (void)cols;         
    (void)rows;         
    (void)max_steps;    
    (void)num_shells;   
  }
	virtual ~Player() {}
  virtual void updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) = 0;
};  