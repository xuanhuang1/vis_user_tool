cmake_minimum_required(VERSION 3.5)
project(vistool_offline)

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

find_package(OpenGL 3 REQUIRED)
find_package(ospray 3.0.0)
add_library(ospray INTERFACE)
if(ospray_FOUND)
   target_link_libraries(ospray INTERFACE ospray::ospray)
else()
    target_include_directories(ospray
        INTERFACE /home/xuanhuang/packages/ospray_all/ospray/ospray/include 
        INTERFACE /home/xuanhuang/packages/ospray_all/ospray/build/install/rkcommon/include )

    target_link_libraries(ospray
        INTERFACE /home/xuanhuang/packages/ospray_all/ospray/build/ospray/build/libospray.so)
endif()

#-------------------- umesh -------------------------
if (DEFINED OFFLINE_WITH_UMESH)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ext/umesh" ${CMAKE_CURRENT_BINARY_DIR}/umesh EXCLUDE_FROM_ALL) #kind of hacky, but it works
    add_definitions(${UMESH_DEFINITIONS})
    include_directories(submodules/umesh)
    set(UMESH_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(UMESH_TARGET umesh)
else()
    set(UMESH_TARGET "")
endif()
#----------------------------------------------------


add_executable(vistool_osp_offline
    main.cpp
    renderer.cpp
    rectMesh.cpp
    ../loader.cpp
    ../util.cpp)

			
target_link_libraries(vistool_osp_offline
			${OPENGL_LIBRARIES}
            ${UMESH_TARGET}
            ospray)





    

