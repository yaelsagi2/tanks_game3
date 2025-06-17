# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic

# Output binary
TARGET = tanks_game

# Source files
SRCS = \
    Direction.cpp \
    GameBoard.cpp \
	GameBoardSatelliteView.cpp \
    GameManager.cpp \
	HybridTankAlgorithm.cpp \
	Logger.cpp \
    Main.cpp \
    Mine.cpp \
    Point.cpp \
	Player1.cpp \
    Player2.cpp \
	MyPlayerFactory.cpp \
	SimpleBattleInfo.cpp \
    Shell.cpp \
    Tank.cpp \
	MyTankAlgorithmFactory.cpp \
	Utils.cpp \
	Wall.cpp \

# Object files
OBJS = $(SRCS:.cpp=.o)

# Include directories
INCLUDES = -I. -Icommon

# Linker libraries
LIBS = 

# Build rule
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

# Compile rule
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

# Run rule
run: $(TARGET)
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=your_input_file.txt"; \
	else \
		./$(TARGET) $(FILE); \
	fi
