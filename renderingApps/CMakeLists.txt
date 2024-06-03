cmake_minimum_required(VERSION 3.5)
project(vistool_examples)

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

find_package(OpenGL 3 REQUIRED)
find_package(glfw3 REQUIRED)


add_executable(interactiveRender
    interactiveRender.cpp
    ArcballCamera.cpp
    clipping_plane.cpp
    TransferFunctionWidget.cpp
    KeyframeWidget.cpp
    GLFWOSPWindow.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_glfw_gl3.cpp
    ../loader.cpp
    ../util.cpp
    mesh/rectMesh.cpp)

target_include_directories(interactiveRender
			PUBLIC /home/xuanhuang/packages/ospray_all/ospray/ospray/include 
			PUBLIC /home/xuanhuang/packages/ospray_all/ospray/build/install/rkcommon/include )
			
target_link_libraries(interactiveRender 
			glfw
			${OPENGL_LIBRARIES}
			/home/xuanhuang/packages/ospray_all/ospray/build/ospray/build/libospray.so)



add_executable(offlineRender
    offlineRender.cpp
    ArcballCamera.cpp
    clipping_plane.cpp
    TransferFunctionWidget.cpp
    KeyframeWidget.cpp
    GLFWOSPWindow.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_glfw_gl3.cpp
    ../loader.cpp
    ../util.cpp
    mesh/rectMesh.cpp)

target_include_directories(offlineRender
			PUBLIC /home/xuanhuang/packages/ospray_all/ospray/ospray/include 
			PUBLIC /home/xuanhuang/packages/ospray_all/ospray/build/install/rkcommon/include )
			
target_link_libraries(offlineRender 
			glfw
			${OPENGL_LIBRARIES}
			/home/xuanhuang/packages/ospray_all/ospray/build/ospray/build/libospray.so)





    

