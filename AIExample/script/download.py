import numpy as np
from OpenVisus import *

db=LoadDataset("https://maritime.sealstorage.io/api/v0/s3/utah/nasa/dyamond/mit_output/llc2160_arco/visus.idx?access_key=AKIAQXOZFVQ7KUIPMHUJ&secret_key=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&endpoint_url=https://maritime.sealstorage.io/api/v0/s3&cached=arco")

t = [0, 50, 100];
q=-8
for time in t:
    data=db.read(time=time, quality=q) # compressed, quality=0 is full res

    dtype="float32"
    raw_fpath = "ocean_{}_{}_{}_t{}_{}.raw".format(*data.shape, time, dtype)
    data.astype('float32').tofile(raw_fpath)
    print("saved", raw_fpath)
