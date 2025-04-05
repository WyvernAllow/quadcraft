cmake_minimum_required(VERSION 3.5)
project(imgui)

add_library(${PROJECT_NAME}
	imgui/imconfig.h
	imgui/imgui.cpp
	imgui/imgui.h
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_impl_glfw.cpp
	imgui/imgui_impl_glfw.h
	imgui/imgui_impl_opengl3.cpp
	imgui/imgui_impl_opengl3.h
	imgui/imgui_impl_opengl3_loader.h
	imgui/imgui_internal.h
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
	imgui/imstb_rectpack.h
	imgui/imstb_textedit.h
	imgui/imstb_truetype.h
)

target_include_directories(${PROJECT_NAME} PUBLIC
	imgui/
)