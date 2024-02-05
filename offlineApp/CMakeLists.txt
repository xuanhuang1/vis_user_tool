cmake_minimum_required(VERSION 3.5)
project(vistool_offline)

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

find_package(OpenGL 3 REQUIRED)


add_executable(vistool_osp_offline
    main.cpp
    ../loader.cpp
    ../util.cpp)

target_include_directories(vistool_osp_offline
			PUBLIC /home/xuanhuang/packages/ospray_all/ospray/ospray/include 
			PUBLIC /home/xuanhuang/packages/ospray_all/ospray/build/install/rkcommon/include )
			
target_link_libraries(vistool_osp_offline
			${OPENGL_LIBRARIES}
			/home/xuanhuang/packages/ospray_all/ospray/build/ospray/build/libospray.so)





    

