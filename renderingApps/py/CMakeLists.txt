cmake_minimum_required(VERSION 3.5)
project(vistool_examples)

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

find_package(OpenGL 3 REQUIRED)
find_package(glfw3 REQUIRED)
find_package(ospray REQUIRED)
find_package(rkcommon REQUIRED)

get_target_property(RKCOMMON_INCLUDE_DIRS rkcommon::rkcommon
      INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(OSPRAY_INCLUDE_DIRS ospray::ospray
      INTERFACE_INCLUDE_DIRECTORIES)

file (GLOB SOURCE_FILES
  "../ArcballCamera.cpp"
  "../TransferFunctionWidget.cpp"
  "../KeyframeWidget.cpp"
  "../clipping_plane.cpp"
  "../GLFWOSPWindow.cpp"
  "../mesh/rectMesh.cpp"
  "../imgui/*.cpp" "../../loader.cpp" "../../util.cpp")

file (GLOB PYTHON_FILES "funcs.cpp" "")

message(OSPRAY_INCLUDE_DIRS=${OSPRAY_INCLUDE_DIRS})
message(RKCOMMON_INCLUDE_DIRS=${RKCOMMON_INCLUDE_DIRS})

include_directories(
  PUBLIC ${OSPRAY_INCLUDE_DIRS} 
  	${RKCOMMON_INCLUDE_DIRS})

find_package(pybind11 REQUIRED)
pybind11_add_module(vistool_py 
	${SOURCE_FILES}
	${PYTHON_FILES}
)

			
target_link_libraries(vistool_py
			glfw
			${OPENGL_LIBRARIES}
			ospray::ospray)





    

