# ECE362_miniproj
## ECE 362 - Microprocesor System and Interfacing
## Sudoku game
Participated Winter 2018 Spark Challenge
### Group members:
* Nguyen Bao Quang
* Yuan-Cheng Chen
* Luke Drobner
* Yun-An Yang

### Background of Sudoku:
The name “Sudoku” was originated from Japanese game magazine called “パズル通信ニコリ” in 1984.
The Swiss mathematician Leonhard Euler is believed the main creator of this math puzzle game, but the game did not receive any attention it deserved. Later in the 70’s, in the U.S., a prototype of Sudoku was published by a mathematical logic game magazine, called “Number Place”. The concept is derived from “Latin Square”, which was also invented by Euler.

### Game rules:
The game is simple: In the 9x9 grid, you have to fill in the numbers from 1 to 9, so that each number appears once only in each row, column, and 3x3 sub grids.
The puzzle will be pre-filled with a number of numbers, and other grids will be left blank. The more difficult, the less number will be pre-filled. The player has to rely on the pre-filled numbers in the puzzle to logically figure out what numbers to fill into the remaining blank grids.

### Game Feature:
* Multiple sudoku problem with various difficulty.
* Sound effect and visual cue when you finished the game
* Debounced keypad input with option to rewrite or clear a previous input
* Secret cheat code that automatically solves the puzzle
### Instructions:
* Left side of LCD screen display the sudoku problem, right side display problem number/difficulty and relevant messages
* Use keypad to input your answer, * to switch problem/difficulty, 0 to clear input
* 2 potentiometer control the cursor, left size control the x axis, right control the right axis
* Potentiometer below the screen control brightness level
	
### Peripherals Used:
* GPIO: For keypad input
* ADC: Used for the potentiometer to control the cursor
* DAC + DMA: For sound effect
* Parallel communication protocol for LCD display

### Game Layout:
![game_layout](https://raw.githubusercontent.com/baoquang98/ECE362_miniproj/master/miniproject_pic.png)

