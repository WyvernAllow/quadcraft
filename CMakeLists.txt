cmake_minimum_required(VERSION 3.5)
project(quadcraft VERSION 0.1.0 LANGUAGES C)

add_executable(${PROJECT_NAME}
    src/block.h
    src/block.c
    src/camera.c
    src/camera.h
    src/chunk.c
    src/chunk.h
    src/lmath.c
    src/lmath.h
    src/main.c
    src/utils.c
    src/utils.h
)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)
target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -std=c99 -pedantic -Wall -Wextra)
endif()

add_subdirectory(deps/glfw)
add_subdirectory(deps/glad)
add_subdirectory(deps/stb_image)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad
    stb_image
)

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
)

add_dependencies(${PROJECT_NAME} copy_assets)