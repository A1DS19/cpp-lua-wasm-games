#include "engine/engine.hpp"
#include "engine/src/engine.hpp"
#include "game.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
    auto& eng = engine::Engine::get_instance();

    try {
        eng.set_application(new Game);

        if (eng.init()) {
            eng.run();
        }
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        eng.destroy();
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "fatal: unknown exception\n";
        eng.destroy();
        return EXIT_FAILURE;
    }

    eng.destroy();
    return EXIT_SUCCESS;
}
