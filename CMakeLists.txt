cmake_minimum_required(VERSION 3.5.0)
project(quadcraft VERSION 0.1.0 LANGUAGES C)

add_executable(${PROJECT_NAME}
    src/main.c
    src/lmath.c
    src/lmath.h
    src/camera.c
    src/camera.h
)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
add_subdirectory(deps/SDL)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3-static
)

find_package(Vulkan COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

if(NOT glslc_executable)
    message(FATAL_ERROR "glslc executable not found. Please ensure Vulkan SDK is installed and glslc is available.")
else()
    message(STATUS "glslc executable found: ${glslc_executable}")
endif()

set(SHADER_SOURCES
    res/shaders/main.vert
    res/shaders/main.frag
)

foreach(SHADER_SOURCE ${SHADER_SOURCES})
    set(SHADER_OUTPUT "${CMAKE_BINARY_DIR}/${SHADER_SOURCE}.spv")

    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${glslc_executable} ${CMAKE_SOURCE_DIR}/${SHADER_SOURCE} -o ${SHADER_OUTPUT}
        DEPENDS ${SHADER_SOURCE}
        COMMENT "Compiling shader ${SHADER_SOURCE} to SPIR-V as ${SHADER_OUTPUT}"
        VERBATIM
    )

    list(APPEND SPIRV_OUTPUTS ${SHADER_OUTPUT})
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${SPIRV_OUTPUTS})
add_dependencies(${PROJECT_NAME} compile_shaders)