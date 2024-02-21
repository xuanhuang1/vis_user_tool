cmake_minimum_required(VERSION 3.5)
project(vistool_examples)

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

find_package(OpenGL 3 REQUIRED)
find_package(glfw3 REQUIRED)


add_executable(vistool_ospray_vol
    ospVol.cpp
    ArcballCamera.cpp
    TransferFunctionWidget.cpp
    GLFWOSPWindow.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_glfw_gl3.cpp
    ../loader.cpp
    ../util.cpp)

target_include_directories(vistool_ospray_vol
			PUBLIC /home/xuanhuang/packages/ospray_all/ospray/ospray/include 
			PUBLIC /home/xuanhuang/packages/ospray_all/ospray/build/install/rkcommon/include )
			
target_link_libraries(vistool_ospray_vol 
			glfw
			${OPENGL_LIBRARIES}
			/home/xuanhuang/packages/ospray_all/ospray/build/ospray/build/libospray.so)





    

