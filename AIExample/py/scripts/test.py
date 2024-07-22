#!/usr/bin/env python
import sys, os
scriptdir = os.path.split(__file__)[0]
sys.path.insert(0, os.path.join(scriptdir, '..'))

import numpy
import vistool_ai_py

vistool_ai_py.frog_helper();

rndr = vistool_ai_py.RendererHandler()
rndr.rotateUp(0.5)
rndr.setIsoValueVolumeRange(200)
rndr.setRenderMode("volume")

frog_renderer_dir = "/home/xuanhuang/projects/vis_interface/vis_user_tool/AIExample" 
vistool_ai_py.run_frog(frog_renderer_dir, rndr)
