# PrismFall

A 2D graphical adaptation of the classic Mastermind board game, built natively in C++ using the SFML (Simple and Fast Multimedia Library) framework to practice state management, multimedia asset pipelining, and interactive user interface logic.

## Technical Implementations
* **Multimedia Integration:** Configured asset streaming to bind custom texture graphics and reactive audio cues (`.wav`) directly into real-time gameplay states.
* **Algorithmic Evaluation:** Designed evaluation matrices to instantly compute precise match scores (exact position vs. correct color) for user puzzle inputs.
* **Event-Driven UI Loop:** Implemented non-blocking event polling to handle menu navigation, grid selections, and asset updates cleanly without frame rendering lag.

## Asset Architecture
The project structure manages game loops alongside custom textures, UI fonts (`arial.ttf`), and sound banks to create a polished, responsive user experience.

## Prerequisites
To compile and run this project, you need the SFML library installed on your system.

## How to Run (Windows / GCC)
1. Clone the repository:
   git clone https://github.com/241974-nah/PrismFall.git

2. Compile linking the SFML libraries:
   g++ ConsoleApplication1.cpp -o PrismFall -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

3. Run the executable:
   ./PrismFall
