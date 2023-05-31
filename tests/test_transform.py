import turbojpeg
import numpy as np
from matplotlib import pyplot as plt

def do_transform(inpath, outpath, **kwargs):

    with open(inpath, "rb") as fr:
        data = fr.read()

    result = turbojpeg.transform(data, **kwargs)

    with open(outpath, "wb") as fw:
        fw.write(result)

def do_decompress(inpath, **kwargs):
    with open(inpath, "rb") as fr:
        data = fr.read()

    result = turbojpeg.decompress(data, **kwargs)

    arr = np.frombuffer(result, dtype=np.uint8).reshape((360, 480, 3))

    plt.imshow(arr)
    plt.show()

inpath = "test.jpg"
do_transform(inpath, "test-none.jpg", op=turbojpeg.OP.NONE)
do_transform(inpath, "test-rot90.jpg", op=turbojpeg.OP.ROT90, perfect=False)
do_transform(inpath, "test-rot180.jpg", op=turbojpeg.OP.ROT180, perfect=False)
do_transform(inpath, "test-transpose.jpg", op=turbojpeg.OP.TRANSPOSE)
do_transform(inpath, "test-transverse.jpg", op=turbojpeg.OP.TRANSVERSE, perfect=False)
do_transform(inpath, "test-crop.jpg", crop=True, x=0, y=0, w=64, h=64)

do_decompress(inpath)
