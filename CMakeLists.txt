cmake_minimum_required (VERSION 3.5)
project("quadcraft" VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(deps/glad)
add_subdirectory(deps/glfw)
add_subdirectory(deps/glm)
add_subdirectory(deps/spdlog)

add_executable(${PROJECT_NAME}
	"src/main.cpp"
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${PROJECT_NAME}
	glad
	glfw
	glm
	spdlog
)