cmake_minimum_required (VERSION 3.0)
project (Tutorials)

find_package(OpenGL REQUIRED)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

include_directories(
	../ext/
	.
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

file (GLOB SOURCE_FILES
	"src/isosurface/MarchingCube.cpp"
	"imgui/imgui.cpp"
	"imgui/imgui_draw.cpp"
	"imgui/imgui_demo.cpp"
	"imgui/imgui_widgets.cpp"
	"imgui/examples/imgui_impl_opengl3.cpp"
	"imgui/examples/imgui_impl_glfw.cpp"
  )

file (GLOB PYTHON_FILES "py/funcs.cpp" "")
set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW
)

# Main
add_executable(viewer_window 
		src/main.cpp
		${SOURCE_FILES}
		)



target_link_libraries(viewer_window
	${ALL_LIBS}
)

find_package(pybind11 REQUIRED)
pybind11_add_module(vistool_ai_py 
	${SOURCE_FILES}
	${PYTHON_FILES}
)
	
target_link_libraries(vistool_ai_py PUBLIC
			${ALL_LIBS})


