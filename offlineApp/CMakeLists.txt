cmake_minimum_required(VERSION 3.5)
project(vistool_offline)

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

find_package(OpenGL 3 REQUIRED)
find_package(ospray 3.0.0 REQUIRED)


add_executable(vistool_osp_offline
    main.cpp
    ../loader.cpp
    ../util.cpp)

			
target_link_libraries(vistool_osp_offline
			${OPENGL_LIBRARIES}
            ospray::ospray)





    

