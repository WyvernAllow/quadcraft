cmake_minimum_required(VERSION 3.8)
project("quadcraft" VERSION 0.1.0 LANGUAGES C)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(deps/raylib)

add_executable(${PROJECT_NAME}
	"src/main.c"
)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)

target_link_libraries(${PROJECT_NAME}
	raylib
)

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
)

add_dependencies(${PROJECT_NAME} copy_assets)