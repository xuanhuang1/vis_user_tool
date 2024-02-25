cmake_minimum_required(VERSION 3.5)
project(vistool_examples)

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

find_package(OpenGL 3 REQUIRED)
find_package(glfw3 REQUIRED)

file (GLOB SOURCE_FILES
  "../ArcballCamera.cpp"
  "../TransferFunctionWidget.cpp"
  "../KeyframeWidget.cpp"
  "../GLFWOSPWindow.cpp"
  "../../mesh/rectMesh.cpp"
  "../imgui/*.cpp" "../../loader.cpp" "../../util.cpp")

file (GLOB PYTHON_FILES "funcs.cpp" "")

include_directories(
  PUBLIC /home/xuanhuang/packages/ospray_all/ospray/ospray/include 
  PUBLIC /home/xuanhuang/packages/ospray_all/ospray/build/install/rkcommon/include )

find_package(pybind11 REQUIRED)
pybind11_add_module(vistool_py 
	${SOURCE_FILES}
	${PYTHON_FILES}
)

			
target_link_libraries(vistool_py PUBLIC
			glfw
			${OPENGL_LIBRARIES}
			/home/xuanhuang/packages/ospray_all/ospray/build/ospray/build/libospray.so)





    

