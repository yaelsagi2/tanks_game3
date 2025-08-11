# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -pedantic

# Common source files
COMMON_SRCS := \
    Direction.cpp \
    GameBoard.cpp \
    GameBoardSatelliteView.cpp \
    GameManager.cpp \
    HybridTankAlgorithm.cpp \
    Logger.cpp \
    Mine.cpp \
    Point.cpp \
    Player.cpp \
    SimpleBattleInfo.cpp \
    Shell.cpp \
    Tank.cpp \
    Wall.cpp \

COMMON_OBJS := $(COMMON_SRCS:.cpp=.o)

SIM_SRCS := ./Simulator/Simulator.cpp
GM_SRCS  := ./GameManager/game_manager.cpp
ALG_SRCS := ./Algorithm/algorithm.cpp

SIM_BIN := simulator
GM_BIN  := game-manager_206480972_206899163
ALG_BIN := algorithm_206480972_206899163

all: sim gm algo

.PHONY: all clean

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I./common -c $< -o $@

# Targets
sim: $(COMMON_OBJS) Simulator/Simulator.o
	$(CXX) $(CXXFLAGS) $(COMMON_OBJS) Simulator/Simulator.o -o $(SIM_BIN)

gm: $(COMMON_OBJS) GameManager/game_manager.o
	$(CXX) $(CXXFLAGS) $(COMMON_OBJS) GameManager/game_manager.o -o $(GM_BIN)

algo: $(COMMON_OBJS) Algorithm/algorithm.o
	$(CXX) $(CXXFLAGS) $(COMMON_OBJS) Algorithm/algorithm.o -o $(ALG_BIN)


clean:
	rm $(ALG_BIN) $(GM_BIN) $(SIM_BIN)

