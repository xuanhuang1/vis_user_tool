import sys, os
from threading import Thread

# path to the rendering app libs
sys.path.append("/home/xuanhuang/projects/vis_interface/vis_user_tool/build/renderingApps/py/")
import renderInterface

a = renderInterface.AnimationHandler("https://maritime.sealstorage.io/api/v0/s3/utah/nasa/dyamond/mit_output/llc2160_theta/llc2160_theta.idx?access_key=AKIAQXOZFVQ7KUIPMHUJ&secret_key=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&endpoint_url=https://maritime.sealstorage.io/api/v0/s3&cached=arco")

a.setDataDim(8640, 6480, 90, 10269);

t_list = [0, 100]
x_range = [0, a.x_max]
y_range = [0, a.y_max]
z_range = [0, a.z_max]

Thread(target = a.renderTask(t_list=t_list, x_range=x_range, y_range=y_range,z_range=z_range, mode=0)).start()

