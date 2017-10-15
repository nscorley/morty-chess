# Morty Chess
A chess engine inspired by the wise teachings of Rick &amp; Morty.

## Installation Instructions
How to install the engine for use with a chess GUI.

1. Clone the repository onto your local machine
2. Extract the zip file and locate the executable ("morty-chess")
3. Connect the executable to your preferred chess GUI. We suggest [Scid vs. Mac](http://scidvspc.sourceforge.net/) for OSX and [Arena](http://www.playwitharena.com/) for linux and windows.

## Contribution Instructions
MortyChess was written in C++ and built using CMake. Follow these instructions to set up the development environment on your machine.

1. Clone the repository onto your local machine
2. Install CMake version >= 3.9
3. Install the current version of Clang for linting
4. Install Boost version >= 1.61, save it in system library folder
5. `cd` into the project main directory
6. make the `build` directory: `mkdir build`
7. Run `cmake .`
8. `cd` into the `build` directory, then run `make`
9. An executable called `app` should have appeared. Run with `./app`

For a list of current development goals, please see the project panel. All contributors welcome.


## Authors

* **Nathaniel Corley** - *Full-stack website design, engine programming* - [nscorley](https://github.com/nscorley)
* **Stiven Deleur** - *engine programming* - [deleurapps](https://github.com/DeleurApps)

## License
This project is licensed under the MIT License.
