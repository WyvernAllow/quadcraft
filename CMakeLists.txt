cmake_minimum_required(VERSION 3.5.0)
project(quadcraft VERSION 0.1.0 LANGUAGES C)

add_executable(${PROJECT_NAME}
    src/main.c
)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
add_subdirectory(deps/SDL)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3-static
)