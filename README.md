## Pacman Reinforcement Learning Pipeline

Reinforcement Learning Pipeline to train a Python based agent to learn how to play a PacMan style game built in a C++ environment.

## Project Status

This project is currently in testing. The communication between the Python model and the C++ environment via JSON IO-pipelines works. The basic training architecture (replay buffer and DQN) works and has been successfully tested on a minimal version of the game.
Currently working on upscaling the training to more complex versions of the game by refining feature engineering and hyperparameter tuning. 



![Demo](demo.gif)


## System Architecture

The project implements a hybrid C++/Python architecture to benefit from a high performance game-engine and powerful ML tools.

* **Game Environment (C++):** Built from scratch using **SFML** for rendering and physics. Handles the game loop,state management and custom reward function.
* **IPC Bridge:** OS-specific `EventLogger` implementations handling inter-process communitcation via **IO Pipes** (Windows Pipes / Unix Fork & Exec) to stream game state data.
* **ML Worker (Python):** Consumes JSON game snapshots via `stdin`, separating the AI logic from the game-engine.
* **Feature Engineering:** `feature_engineer.py` extracts custom features from the raw game states (e.g., relative enemy positions, wall distances).
* **RL Agent:** `rl_agent.py` implements a **Deep Q-Learning (DQN)** agent with Experience Replay and Target Network for stable training.
