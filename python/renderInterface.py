import sys, os
import numpy as np
import time
from datetime import timedelta

from OpenVisus import *
import vistool_py

# common utility functions

def getFileName(dimsx, dimsy, dimsz, t):
    return "ocean_{}_{}_{}_t{}_Transpose_float32.raw".format(dimsx, dimsy, dimsz, t)

def getFileNameWithPrefix(name, dimsx, dimsy, dimsz, t):
    return "{}_{}_{}_{}_t{}_Transpose_float32.raw".format(name, dimsx, dimsy, dimsz, t)

def saveFile(raw_fpath, data):
    start_time = time.monotonic()
    data.astype('float32').tofile(raw_fpath)
    end_time = time.monotonic()
    print('Download Duration: {}'.format(timedelta(seconds=end_time - start_time)))
    


# animation handler

class AnimationHandler:
    def __init__(self, path, pathType="visus"):
        self.srcType = pathType
        if (self.srcType == "visus"): # use visus to load
            self.dataSrc = LoadDataset(path)
            print(self.dataSrc.getDatasetBody().toString())
        elif (pathType == "local"): # set local file path
            self.dataSrc = path

    def setDataDim(self, x_max=0, y_max=0, z_max=0, t_max=0):
        self.x_max = x_max
        self.y_max = y_max
        self.z_max = z_max
        self.t_max = t_max
        print("set data dims [0, {}] [0, {}] [0, {}] t=[0, {}]".format(x_max, y_max, z_max, t_max))

    # get data from source
    def readData(self, t=0,x_range=[0,0],y_range=[0,0],z_range=[0,0],q=-6):
        if (self.srcType == "visus"): # use visus to read
            return self.dataSrc.read(time=t,x=x_range,y=y_range,z=z_range,quality=q)
            
    # launch rendering
    def renderTask(self, x_range=[0,0], y_range=[0,0], z_range=[0,0], q=-6, t_list=[0], mode=0):
        dims = [100, 100, 100]
        total_data = []
        t_names = []
        counter = 0;
        for t in t_list:
            print(t)
            start_time = time.monotonic()
            #data=db.read(time=t,x=x_range,y=y_range,z=[0,90],quality=q) # compressed, quality=0 is full res
            data = self.readData(t=t, x_range=x_range, y_range=y_range, z_range=z_range, q=q)
            end_time = time.monotonic()
            print('Read Duration: {}'.format(timedelta(seconds=end_time - start_time)))

            # prepare raw data for rendering
            data = np.flip(data, 2)
            if (mode == 2): # special treatment for spherical rendering
                data = np.transpose(data, (2, 1, 0))
            
            # concate all timesteps
            dims = data.shape
            total_data = np.concatenate((total_data, data.ravel()), axis=None)
            t_names.append(getFileName(*data.shape, t))
            counter += 1

            # save a copy of the data if needed
            if (0):
                saveFile(getFileName(*data.shape, t))
    
        print(dims)
        print("count ", counter)
        print(t_names)
        #print(total_data.shape)

        vistool_py.init_app(sys.argv)
        vistool_py.run_app(total_data, t_names, dims[2], dims[1], dims[0], counter, mode)
        #return dims
    
