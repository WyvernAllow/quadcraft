cmake_minimum_required(VERSION 3.5.0)
project(quadcraft VERSION 0.1.0 LANGUAGES C)

add_executable(${PROJECT_NAME}
    src/main.c
    src/lmath.c
    src/lmath.h
    src/camera.c
    src/camera.h
    src/chunk_mesh.h
    src/chunk_mesh.c
    src/block_type.h
    src/block_type.c
    src/chunk.h
    src/chunk.c
    src/utils.h
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -std=c99 -pedantic -Wall -Wextra)
endif()

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
add_subdirectory(deps/SDL)
add_subdirectory(deps/FastNoiseLite)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3-static
    FastNoiseLite
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

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
)

add_dependencies(${PROJECT_NAME} copy_assets)