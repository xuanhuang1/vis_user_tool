import sys, os
import numpy as np
from threading import Thread

# path to the rendering app libs
sys.path.append(os.path.join(os.path.dirname(sys.path[0]), 'build', 'renderingApps', 'py'))
import renderInterface

a = renderInterface.AnimationHandler()

#
# call offline render to produce video
#

#f_path = '/home/xuanhuang/projects/vis_interface/vis_user_tool/jsonSamples/example_kf_header.json'
f_path = sys.argv[1]

Thread(target = a.renderTaskOfflineVTK(f_path)).start()
