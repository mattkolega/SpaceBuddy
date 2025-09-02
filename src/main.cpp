#include "application.h"

#include <common/logger.h>

int main() {
    try {
        Application app;
        app.init();

        app.run();
    } catch (std::exception const &e) {
        Logger::err("{}", e.what());
        return EXIT_FAILURE;
    }

    return 0;
}