import sys, os
import numpy as np
from threading import Thread

# path to the rendering app libs
sys.path.append(os.path.join(os.path.dirname(sys.path[0]), 'build', 'renderingApps', 'py'))
#sys.path.append("/home/xuanhuang/projects/vis_interface/vis_user_tool/build/renderingApps/py/")
import renderInterface

#
# set database source
#

a = renderInterface.AnimationHandler("https://maritime.sealstorage.io/api/v0/s3/utah/nasa/dyamond/mit_output/llc2160_theta/llc2160_theta.idx?access_key=any&secret_key=any&endpoint_url=https://maritime.sealstorage.io/api/v0/s3&cached=arco")

# set render info
a.setDataDim(8640, 6480, 90, 10269);

#
# choose data region, if applicable
#

# global
t_list = np.arange(0, 10000, 2160, dtype=int).tolist()
x_range = [0, a.x_max]
y_range = [0, a.y_max]
z_range = [0, a.z_max]
quality=-6

# the mediterranean sea region
#x_range = [int(a.x_max*0.057), int(a.x_max*0.134)]
#y_range = [int(a.y_max*0.69), int(a.y_max*0.81)]
#z_range = [0, a.z_max]
#t_list = np.arange(0, 96, 48, dtype=int).tolist()

# southern sea
#x_range = [int(a.x_max*0.4), int(a.x_max*0.47)]
#y_range = [int(a.y_max*0.33), int(a.y_max*0.45)]
#z_range = [0, a.z_max]
#t_list = np.arange(0, 10000, 2160, dtype=int).tolist()

#
# scripting
#

scriptingType = "viewer"
outputName_viewer = "viewer_script"

# additional options:
# if the data need to be flipped or transposed 
flip_axis=2
transpose=False
bgImg = ''

testing_scene="flat"
#testing_scene="sphere"

if(testing_scene=="flat"):
    flip_axis=2
    transpose=False
    render_mode=0
    bgImg = ''
elif(testing_scene=="sphere"):
    flip_axis=2
    transpose=True
    render_mode=2
    bgImg = os.path.join(os.path.dirname(sys.path[0]), 'renderingApps', 'mesh', 'land.png')
    #bgImg = '/home/xuanhuang/projects/vis_interface/vis_user_tool/renderingApps/mesh/land.png'
    
#  interactive app
print ("launch interactive viewer")
Thread(target = a.renderTask(t_list=t_list, x_range=x_range, y_range=y_range,z_range=z_range, q=quality, mode=render_mode, flip_axis=flip_axis, transpose=transpose, bgImg=bgImg, outputName=outputName_viewer)).start()

#
# download data for offline render
#
#a.saveRawFilesByVisusRead(t_list=t_list, x_range=x_range, y_range=y_range,z_range=z_range, q=quality, flip_axis=flip_axis, transpose=transpose)


