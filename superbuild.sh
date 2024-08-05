#!/bin/bash

mkdir dependency_dir
cd dependency_dir/
git clone git@github.com:RenderKit/ospray.git
cd ospray
git checkout v2.12.0
mkdir build
cd build
# may need
#sudo apt-get install cmake-curses-gui
#sudo apt-get install libtbb-dev
cmake ../scripts/superbuild
cmake --build .

dp_path=$(pwd)
echo $dp_path
cd ../../../
mkdir build
cd build
echo cmake -Dospray_DIR=$dp_path ..
cmake -Dospray_DIR=$dp_path/install/ospray/lib/cmake/ospray-2.12.0 -Drkcommon_DIR=$dp_path/install/rkcommon/lib/cmake/rkcommon-1.11.0 -DBUILD_RENDERING_APPS_PY=ON ..
make
