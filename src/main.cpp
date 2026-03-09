#include "GameEngine.h"

// ============================================================
//  main()  –  entry point
//
//  All setup, the SFML window, and the game loop live inside
//  GameEngine. main() only instantiates it and calls run().
// ============================================================
int main() {
    GameEngine game;
    game.run();
    return 0;
}
