cmake_minimum_required(VERSION 3.8)
project("quadcraft" VERSION 0.1.0 LANGUAGES C CXX)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(deps/raylib)

add_subdirectory(deps/yaml-cpp)
add_subdirectory(deps/spdlog)

add_executable(${PROJECT_NAME}
	"src/main.cpp"

	"src/chunk.hpp"
	"src/chunk.cpp"
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${PROJECT_NAME}
	raylib
	yaml-cpp::yaml-cpp
	spdlog::spdlog
)

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
)

add_dependencies(${PROJECT_NAME} copy_assets)