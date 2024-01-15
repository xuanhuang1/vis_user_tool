cmake_minimum_required(VERSION 3.5)
project(vistool_examples)

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)


add_executable(vistool_ospray
    ospTutorial.cpp
    ../loader.cpp
    ../util.cpp)

target_include_directories(vistool_ospray 
			PUBLIC /home/xuanhuang/packages/ospray_all/ospray/ospray/include 
			PUBLIC /home/xuanhuang/packages/ospray_all/ospray/build/install/rkcommon/include )
			
target_link_libraries(vistool_ospray /home/xuanhuang/packages/ospray_all/ospray/build/ospray/build/libospray.so)




add_executable(vistool_opengl
    openglExample.cpp
    ../loader.cpp
    ../util.cpp
    glad/src/gl.c)
    
target_include_directories(vistool_opengl 
			PUBLIC /home/xuanhuang/projects/vis_interface/vis_user_tool/examples/glad/include)
			
target_link_libraries(vistool_opengl glfw ${OPENGL_LIBRARIES})


    

