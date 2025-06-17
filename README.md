# Advanced Topics Ex2 - Game Build Instructions

## Projected submitted by

    Yael Sagi 206480972 

## Overview
A deterministic tank battle game.

## Viewing the README

To view this README properly in the terminal, you can use the `less` command:
```bash

less README.md
```
To exit 'less'
```bash

q
```

## Requirements

C++20 or higher (equivalent to a recent version of the g++ compiler).
Linux or Windows with a Bash-compatible terminal (for running make).

No CMake is required – the project uses a simple Makefile.

## Description
A tank battle simulation game written in C++.

Each tank moves, shoots, and interacts with walls, mines, and shells based on an input board file.  
The game proceeds in turns until one tank wins or a tie occurs.

This project is text-based only (no graphics, no visuals).

## Approved Implementation Choices

1. I implemented GetBattleInfo so it would cancel backward movements.
2. After waiting for two steps to execute the MoveBackward command, on the 3rd step, it is executed.
All other commands are ignored during this waiting period, and the backward movement proceeds as planned.

## Input File Format

The input file is divided into two main sections: **Game Settings** and **Board Description**.

### 1. **Game Settings (First 5 Lines)**

The first 5 lines of the input file define the game settings. These lines must appear in this exact order:

    <MapName>
    MaxSteps = <NUM>
    NumShells = <NUM>
    Rows = <NUM>
    Cols = <NUM>

Where:
- `MapName` - A free text naming or describing the map
- `MaxSteps` — The maximum number of steps the game will run for.
- `NumShells` — Number of shells each tank starts with.
- `Rows` — Number of rows in the game board.
- `Cols` — Number of columns in the game board.

### 2. **Board Layout (Lines 6 and onward)**

Starting from line 6, each line corresponds to a row of the game board. The characters in each line describe the initial state of the game board, from top to bottom (i.e., line 6 is row 0, line 7 is row 1, etc.).

The i'th character in the j'th row represents the object at position (i, j) on the board.

#### Allowed Map Characters

- `#`: Wall  
- `@`: Mine  
- `1`–`2`: Tank belonging to Player 1 and Player 2  
- Space (` `): Empty cell  
- Any other character: Treated as Empty cell  

## Output Files

The game generates three output files:

1. **output_<input_file>.txt**  
2. **input_errors.txt**  
3. **detailed_output_<input_file>.txt**

### 1. output_<input_file>.txt
> The output file will be named `output_<input_file>.txt`, where `<input_file>` is the name of the input file used (without extension).
The output is a plain text file that logs the game round-by-round. Each line corresponds to a full round and lists the action taken by each tank in the order they were "born".

#### Tank Order
Tanks are listed in the order they appear on the board, scanned row by row from top to bottom, and left to right within each row. This order remains fixed throughout the game.

#### Per-Round Format
Each line contains the action performed by every tank during that round, separated by commas. Actions are named exactly as in the `ActionRequest` enum.

- If an action cannot be executed, it is marked with `(ignored)`.
- If a tank is destroyed during the round, its action in that round will include `(killed)`.
- A dead tank’s entry will be the word `killed`.

If an action is both ignored and the tank is killed in the same round, it will be shown as: SomeAction (ignored) (killed)


#### Final Line
The last line in the file indicates the game result in one of the following formats:

- `Player <X> won with <Y> tanks still alive`
- `Tie, both players have zero tanks`
- `Tie, reached max steps = <MAX_STEPS>, player 1 has <X> tanks, player 2 has <Y> tanks`
- `Tie, both players have zero shells for <zero_shells_steps> steps`


### 2. input_errors.txt

This file logs errors found when parsing the input file. These include:

- Missing configuration lines.
- Unrecognized characters in the map.
- Invalid board dimensions.

### 3. detailed_output_<input_file>.txt

This file logs a detailed, step-by-step trace of the game's execution. For each step, it records:
The action requested by each tank.
Whether the action was executed successfully or ignored (e.g., due to cooldown or invalid move).
All object movements, including tanks and shells, with their new positions.
All collisions that occurred during the step (e.g., shell hitting a tank, tank hitting a mine).
The results of those collisions, such as damage dealt, destruction of tanks or walls, and any resulting status (like a tank being destroyed).

## Compilation Instructions
Open a terminal and navigate to the project directory:

cd /path/to/advanced_topics_ex1

Then compile the project using the provided Makefile:

make

This will create an executable file called tanks_game.

## Running Instructions

After compiling the project with make, you can run the game as follows:

On Linux:
    ./tanks_game <Map_File_Name>

On Windows:
    tanks_game.exe <Map_File_Name>

Replace <Map_File_Name> with your input file name.
For example:
    ./tanks_game simple_board.txt

This will start the game using the map described in the input file.
If you forget to provide a file, the program will display a usage hint.

## Cleaning Build Files

To remove all compiled object files and the executable, run:

make clean

## Important Notes

The compiler flags used are: -std=c++20 -Wall -Wextra -Werror -pedantic.

This project does not use any graphical libraries (e.g., SFML).

Only standard C++ and the Makefile are required \u2014 no external dependencies.

