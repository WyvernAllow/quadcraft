﻿cmake_minimum_required(VERSION 3.8)

if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(quadcraft VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME}
    src/application.hpp
    src/application.cpp
    src/extensions.cpp
    src/main.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

add_subdirectory(deps/glfw)
add_subdirectory(deps/glm)
add_subdirectory(deps/spdlog)

find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glm
    spdlog
    Vulkan::Vulkan
)