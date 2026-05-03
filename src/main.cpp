#include "app.h"

#include "common/log.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        log::fatal("Incorrect number of args. Expecting rom path");
        return EXIT_FAILURE;
    }

    auto app = App::create();
    if (!app) return EXIT_FAILURE;

    if (!app->run(argv[1])) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}