#!/usr/bin/env python
import sys, os

# add path to python lib
sys.path.append("/home/xuanhuang/projects/vis_interface/vis_user_tool/build/AIExample")

import numpy
import vistool_ai_py

# print helper
vistool_ai_py.frog_helper();

# set rendering attributes
rndr = vistool_ai_py.RendererHandler()
rndr.rotateUp(1.0) # in radian
rndr.rotateLeft(0.5)
rndr.setIsoValue(180)
rndr.setIsoValueVolumeRange(200)
rndr.setRenderMode("volume")

# launch renderer
frog_renderer_dir = "/home/xuanhuang/projects/vis_interface/vis_user_tool/AIExample" 
vistool_ai_py.run_frog(frog_renderer_dir, rndr)
