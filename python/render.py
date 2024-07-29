import sys, os
import numpy as np
from threading import Thread

# path to the rendering app libs
sys.path.append("/home/xuanhuang/projects/vis_interface/vis_user_tool/build/renderingApps/py/")
import renderInterface

a = renderInterface.AnimationHandler()

#
# call offline render to produce video
#

#f_path = '/home/xuanhuang/projects/vis_interface/vis_user_tool/jsonSamples/example_kf_header.json'
f_path = sys.argv[1]

Thread(target = a.renderTaskOffline(f_path)).start()
