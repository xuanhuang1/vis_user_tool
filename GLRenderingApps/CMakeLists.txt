cmake_minimum_required (VERSION 3.0)
project (Tutorials)

find_package(OpenGL REQUIRED)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# Compile external dependencies 
add_subdirectory (external)


include_directories(
	external/glfw-3.3.2/include/
	external/glm-0.9.7.1/
	external/glew-2.1.0/include/
	.
)


add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

# Main
add_executable(first_window 
	src/main.cpp
	src/isosurface/MarchingCube.cpp
	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_widgets.cpp
	imgui/examples/imgui_impl_opengl3.cpp
	imgui/examples/imgui_impl_glfw.cpp
	)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW
)

target_link_libraries(first_window
	${ALL_LIBS}
)
