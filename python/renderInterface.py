import sys, os
import numpy as np
import time
from datetime import timedelta

from OpenVisus import *
import vistool_py
import vistool_py_vtk

# common utility functions

def getRawFileName(dimsx, dimsy, dimsz, t):
    return "ocean_{}_{}_{}_t{}_float32.raw".format(dimsx, dimsy, dimsz, t)

def getRawFileNameWithPrefix(name, dimsx, dimsy, dimsz, t):
    return "{}_{}_{}_{}_t{}_float32.raw".format(name, dimsx, dimsy, dimsz, t)    

def saveFile(raw_fpath, data):
    start_time = time.monotonic()
    data.astype('float32').tofile(raw_fpath)
    end_time = time.monotonic()
    print('Download Duration: {}'.format(timedelta(seconds=end_time - start_time)))
    


# animation handler

class AnimationHandler:
    def __init__(self, path="", pathType="visus"):
        if (path != ""):
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
    # TODO local data src
    def readData(self, t=0,x_range=[0,0],y_range=[0,0],z_range=[0,0],q=-6, flip_axis=2, transpose=False):
        if (self.srcType == "visus"): # use visus to read
            d = self.dataSrc.read(time=t,x=x_range,y=y_range,z=z_range,quality=q)
            if (flip_axis >= 0): # flip data on demand
                d = np.flip(d, flip_axis)
            if (transpose): # transpose data on demend
                d = np.transpose(d, (2, 1, 0))
            return d

    # get file names to save or script
    def getRawFileNames(self, dimsx, dimsy, dimsz, t_list):
        t_names = []
        counter = 0;
        for t in t_list:
            print(t)    
            # concat all timesteps
            t_names.append(getRawFileName(dimsx, dimsy, dimsz, t))
            counter += 1
        return t_names

    def saveRawFilesByVisusRead(self, x_range=[0,0], y_range=[0,0], z_range=[0,0], q=-6, t_list=[0], flip_axis=2, transpose=False):
        dims = [100, 100, 100]
        counter = 0;
        for t in t_list:
            print(t)
            start_time = time.monotonic()
            data = self.readData(t=t, x_range=x_range, y_range=y_range, z_range=z_range, q=q, flip_axis=flip_axis, transpose=transpose)
            end_time = time.monotonic()
            print('Read Duration: {}'.format(timedelta(seconds=end_time - start_time)))
    
            # concate all timesteps
            dims = data.shape
            counter += 1

            # save 
            saveFile(getRawFileName(data.shape[2], data.shape[1], data.shape[0], t), data)    
        print("count ", counter)
        
    # generate scripts by templates
    def generateScript(self, input_names, kf_interval, dims, meshType, world_bbx_len, cam, tf_range, template="fixedCam", outfile="script", bgImg=""):
        if (template == "fixedCam"):
            print("generating fixed camera script to: ", outfile, "\n")
            # convert to strict data types
            dims = np.array(dims)
            cam = np.float32(cam)
            tf_range = np.float32(tf_range)
            vistool_py.generateScriptFixedCam(outfile, input_names, kf_interval, dims, meshType, world_bbx_len, cam, tf_range, bgImg);

    # read scripts by file path
    def readScript(self, p):
        return vistool_py.readScript(p);
        
    # launch rendering
    def renderTask(self, x_range=[0,0], y_range=[0,0], z_range=[0,0], q=-6, t_list=[0], flip_axis=2, transpose=False, mode=0, bgImg="", outputName="viewer_script"):
        dims = [100, 100, 100]
        total_data = []
        t_names = []
        counter = 0;
        for t in t_list:
            print(t)
            start_time = time.monotonic()
            data = self.readData(t=t, x_range=x_range, y_range=y_range, z_range=z_range, q=q, flip_axis=flip_axis, transpose=transpose)
            end_time = time.monotonic()
            print('Read Duration: {}'.format(timedelta(seconds=end_time - start_time)))
    
            # concate all timesteps
            dims = data.shape
            total_data = np.concatenate((total_data, data.ravel()), axis=None)
            t_names.append(getRawFileName(data.shape[2], data.shape[1], data.shape[0], t))
            counter += 1

            # save a copy of the data if needed
            if (0):
                saveFile(getRawFileName(data.shape[2], data.shape[1], data.shape[0], t))
    
        print(dims)
        print("count ", counter)
        print(t_names)
        #print(total_data.shape)

        vistool_py.init_app(sys.argv)
        vistool_py.run_app(total_data, t_names, dims[2], dims[1], dims[0], counter, mode, bgImg, outputName)
        #return dims


    # launch rendering
    def renderTaskOffline(self, jsonStr):
        vistool_py.init_app(sys.argv)
        vistool_py.run_offline_app(jsonStr, "", -2)

    # launch vtk rendering
    def renderTaskOfflineVTK(self, jsonStr):
        vistool_py_vtk.run_offline_app(jsonStr, "", -2)

    
