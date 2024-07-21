This project is for CS 5635 / CS 6635 scientific visualization

The goal is to make an interactive tool for 3D frog dissection, inspire by the LBNL DSD Whole Frog Project
https://froggy.lbl.gov/

Dependencis:
GLFW
GLEW
GLM (header only)

Those can be found under /external.

The src/ directory contains the source code. The marching cube isosurface implementation is under src/isosurface, and the direct rendering will be done through shaders.

The CMakeList.txt has been tested on linux only.


Usage:
after complie run the program, a window will show up with two rendering mode.
Press S for surface, V fpr volume (implemeneted as red triangles/blue box now)