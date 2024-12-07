cmake_minimum_required (VERSION 3.5)

if (POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(quadcraft VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME}
	src/main.cpp
	src/extensions.cpp
	src/device.hpp
	src/device.cpp
	src/swapchain.hpp
	src/swapchain.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

add_subdirectory(deps/spdlog)
add_subdirectory(deps/glfw)
add_subdirectory(deps/glm)

find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
	spdlog
	glfw
	glm
	Vulkan::Vulkan
)

set(SHADER_SOURCES
	res/shaders/main.vert
	res/shaders/main.frag
)

foreach(SHADER_PATH ${SHADER_SOURCES})
	set(INPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_PATH})
	set(OUTPUT_PATH ${CMAKE_BINARY_DIR}/${SHADER_PATH}.spv)

	add_custom_command(
		OUTPUT ${OUTPUT_PATH}
		COMMAND Vulkan::glslc ${INPUT_PATH} -o ${OUTPUT_PATH}
		DEPENDS ${SHADER_PATH}
		COMMENT "Compiling ${SHADER_PATH}"
	)
	list(APPEND SPIRV_FILES ${OUTPUT_PATH})
endforeach()

add_custom_target(shaders_target ALL DEPENDS ${SPIRV_FILES})
add_dependencies(${PROJECT_NAME} shaders_target)