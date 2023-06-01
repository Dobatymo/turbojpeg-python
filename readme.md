# turbojpeg-python

Python bindings for libjpeg-turbo using pybind11. It's using Version 3 of the TurboJPEG C API.

## Install

- `pip install turbojpeg`

## Example

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

## Build

The Python `setup.py` expects the `libjpeg-turbo` binaries in `libjpeg-turbo-build`. You can either build it yourself or download precompiled binaries. For more details see <https://github.com/libjpeg-turbo/libjpeg-turbo/blob/main/BUILDING.md>.

GitHub actions CI builds wheels for Windows 32 and 64-bit, manylinux 64-bit and MacOS 64-bit. 32-bit Linux builds fail due some weird NASM error and musllinux builds are not possible since gcc-11 is not available on that platform. gcc-11 is required because C++20 is required for the bindings.
