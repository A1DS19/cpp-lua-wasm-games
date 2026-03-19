
#include "sol/state.hpp"
#include "sol/types.hpp"

#include <SDL2/SDL.h>
#include <SDL_events.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <cstdlib>
#include <sol/sol.hpp>
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif
#include <iostream>

SDL_Window* p_window{nullptr};
SDL_Renderer* p_renderer{nullptr};
sol::state lua;
int box_x{50}, box_y{50};

bool init_sdl() {
    std::cout << "initializing sdl\n";
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "sdl initialization failed: " << SDL_GetError() << "\n";
        return EXIT_FAILURE;
    }

    p_window = SDL_CreateWindow("sdl and emscripten test", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);

    if (p_window == nullptr) {
        std::cerr << "failed to create sdl window: " << SDL_GetError() << "\n";
    }

    p_renderer = SDL_CreateRenderer(p_window, -1, SDL_RENDERER_ACCELERATED);
    if (p_renderer == nullptr) {
        std::cerr << "failed to create sdl renderer: " << SDL_GetError() << "\n";
    }

    std::cout << "sdl initialization success\n";

    return true;
}

void cleanup() {
    SDL_DestroyRenderer(p_renderer);
    SDL_DestroyWindow(p_window);
    SDL_Quit();
}

void game_loop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
        }
    }

    lua["move_box"]();

    SDL_SetRenderDrawColor(p_renderer, 0, 0, 0, 255);
    SDL_RenderClear(p_renderer);

    SDL_SetRenderDrawColor(p_renderer, 255, 0, 0, 255);
    SDL_Rect box{box_x, box_y, 50, 50};
    SDL_RenderFillRect(p_renderer, &box);
    SDL_RenderPresent(p_renderer);
}

void register_lua_functions() {
    lua.open_libraries(sol::lib::base);
    lua["say_hello_cpp"] = [] { std::cout << "hello from lua\n"; };
    lua["move_box_cpp"] = [](int x, int y) {
        box_x = x;
        box_y = y;
    };
    lua["get_x_cpp"] = [] { return box_x; };
    lua["get_y_cpp"] = [] { return box_y; };
}

auto main() -> int {
    std::cout << "starting game...\n";
    if (!init_sdl()) {
        return EXIT_FAILURE;
    }

    register_lua_functions();
    lua.safe_script_file("assets/scripts/main.lua"); // load script once (defines functions)

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(game_loop, 0, 1);
#else
    while (true) {
        game_loop();
    }
#endif
    cleanup();

    return EXIT_SUCCESS;
}
