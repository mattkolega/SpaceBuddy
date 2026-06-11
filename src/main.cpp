#include "app.h"

#include "common/log.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        log::fatal("Incorrect number of args. Expecting rom path");
        return EXIT_FAILURE;
    }

    if (!SDL_SetAppMetadata("SpaceBuddy", "0.1.0", "com.mattkolega.spacebuddy")) {
        log::fatal("Failed to set metadata. SDL_Error: {}", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log::fatal("Failed to initialise SDL. SDL_Error: {}", SDL_GetError());
        return EXIT_FAILURE;
    }

    auto app = App::create();
    if (!app) return EXIT_FAILURE;

    if (!app->run(argv[1])) return EXIT_FAILURE;

    SDL_Quit();

    return EXIT_SUCCESS;
}
