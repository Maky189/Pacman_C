# Pacman Game

This is a simple implementation of the classic Pacman game using C and OpenGL. The game features basic gameplay mechanics, including Pacman moving through a maze, collecting points, and avoiding ghosts.

## Features

- Pacman can move in four directions: up, down, left, and right.
- Ghosts move around the maze and try to catch Pacman.
- Pacman can collect points and power-ups.
- The game includes a start screen and a game over screen.

## Requirements

- GCC (GNU Compiler Collection)
- OpenGL
- GLFW
- SOIL (Simple OpenGL Image Library)

## Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/yourusername/Pacman_C.git
    cd Pacman_C
    ```

2. Build the project using the provided Makefile:
    ```sh
    make
    ```

## Usage

1. Run the game:
    ```sh
    make run
    ```

2. Use the arrow keys to move Pacman:
    - Right arrow: Move right
    - Down arrow: Move down
    - Left arrow: Move left
    - Up arrow: Move up

3. Press `P` to start the game from the start screen.

4. Press `ESC` to exit the game.

## Project Structure

- `main.c`: Contains the main game loop and initialization code.
- `Pacman.c`: Contains the implementation of Pacman, ghosts, and the game scenario.
- `Pacman.h`: Header file with function declarations and data structures.
- `Makefile`: Build script to compile the project.


## Acknowledgements

- The game uses the Simple OpenGL Image Library (SOIL) for loading textures.
- The game is inspired by the classic Pacman game developed by Namco.
