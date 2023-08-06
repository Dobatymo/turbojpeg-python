# turbojpeg-python

Python bindings for libjpeg-turbo using pybind11. It's using Version 3 of the TurboJPEG C API.

It implements the three functions `transform`, `compress` and `decompress`. For more details see the function docstrings or the TurboJPEG C API docs <https://rawcdn.githack.com/libjpeg-turbo/libjpeg-turbo/main/doc/html/group___turbo_j_p_e_g.html>

## Install

- `pip install turbojpeg`

## Example

Rotate JPEG file losslessly by 90 degrees.

```python
import turbojpeg

with open("image.jpg", "rb") as fr:
    img_data = fr.read()

# lossless rotation. fails if lossless rotation is not possible
# for example when the image dimensions are not multiples of 16
img_data_rot = turbojpeg.transform(img_data, op=turbojpeg.OP.ROT90)

with open("image-rotated.jpg", "xb") as fw:
    fw.write(img_data_rot)
```

Save random image as JPEG with arithmetic coding.

```python
import numpy as np
import turbojpeg
arr = np.random.randint(0, 256, (256, 256, 3), dtype=np.uint8)  # random color image
quality = 90
result = turbojpeg.compress(arr, quality, turbojpeg.SAMP.Y420, arithmetic=True)
with open("arithmetic.jpg", "xb") as fw:
    fw.write(result)
```

## Build

The Python `setup.py` expects the `libjpeg-turbo` binaries in `libjpeg-turbo-build`. You can either build it yourself or download precompiled binaries. For more details see <https://github.com/libjpeg-turbo/libjpeg-turbo/blob/main/BUILDING.md>.

GitHub actions CI builds wheels for Windows 32 and 64-bit, manylinux 64-bit, musllinux 64-bit and MacOS 64-bit. 32-bit Linux builds fail due some weird NASM error. Not all wheels are tested, since some of the test dependencies are not available for all platforms.

## Run tests

`python -m unittest discover -s tests`
