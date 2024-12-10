#include <spdlog/spdlog.h>

#include <cstdlib>
#include <stdexcept>

#include "application.hpp"

int main() {
    try {
        application app;
        app.run();
    } catch (std::exception& e) {
        spdlog::critical("{}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}