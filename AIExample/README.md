This is an interactive tool for 3D frog dissection, inspire by the LBNL DSD Whole Frog Project
https://froggy.lbl.gov/

Dependencis:
GLFW
GLEW
GLM (header only)

The src/ directory contains the source code. The marching cube isosurface implementation is under src/isosurface, and the direct rendering will be done through shaders.

see scripts/test.py for example running script

The CMakeList.txt has been tested on linux only.

