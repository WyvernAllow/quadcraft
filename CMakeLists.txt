cmake_minimum_required (VERSION 3.8)
project("quadcraft" VERSION 0.1.0 LANGUAGES C)

add_executable(${PROJECT_NAME}
	"src/main.c"
)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)