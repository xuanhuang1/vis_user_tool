cmake_minimum_required(VERSION 3.5)
project(vistool_gl_examples)

find_package(OpenGL 3 REQUIRED)
find_package(glfw3 REQUIRED)

file (GLOB SOURCE_FILES
	"../src/isosurface/MarchingCube.cpp"
	"../imgui/imgui.cpp"
	"../imgui/imgui_draw.cpp"
	"../imgui/imgui_demo.cpp"
	"../imgui/imgui_widgets.cpp"
	"../imgui/examples/imgui_impl_opengl3.cpp"
	"../imgui/examples/imgui_impl_glfw.cpp"
  )

file (GLOB PYTHON_FILES "funcs.cpp" "")

include_directories(
  	../external/glfw-3.3.2/include/
	../external/glm-0.9.7.1/
	../external/glew-2.1.0/include/)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

find_package(pybind11 REQUIRED)
pybind11_add_module(vistool_gl_py 
	${SOURCE_FILES}
	${PYTHON_FILES}
)

			
target_link_libraries(vistool_gl_py PUBLIC
			glfw
			GLEW
			${OPENGL_LIBRARIES})





    

