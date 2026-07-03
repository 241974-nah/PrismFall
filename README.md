# PrismFall

A command-line interface (CLI) text-based adaptation of the classic Mastermind board game, built natively in C++ to practice core programming patterns, data validation, and state tracking.

## Technical Implementations
* **Data Structures:** Utilized fixed-size arrays/vectors to track and evaluate guess sequences against generated codes.
* **Algorithmic Logic:** Implemented precise positional checking loops to evaluate exact matches (correct color and position) versus partial matches (correct color, wrong position).
* **Robust Input Validation:** Created continuous stream checking handlers to catch malformed inputs, prevent program crashes, and enforce strict gameplay rules.

## Gameplay Overview
The game generates a hidden sequence of colors/pegs. The player has a limited number of attempts to guess the exact combination, receiving precise analytical feedback after each turn until they either solve the sequence or run out of turns.

## How to Run
1. Clone the repository:
   git clone https://github.com/241974-nah/PrismFall.git

2. Compile using any standard C++ compiler (e.g., g++):
   g++ main.cpp -o PrismFall

3. Run the executable:
   ./PrismFall
