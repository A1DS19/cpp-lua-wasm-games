#include "engine/src/engine.hpp"
#include "game.hpp"

#include <cstdlib>

int main() {
    Game* game = new Game;
    engine::Engine engine;

    engine.set_application(game);

    if (engine.init()) {
        engine.run();
    }

    engine.destroy();

    return EXIT_SUCCESS;
}
