from glob import glob

from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

ext_modules = [
    Pybind11Extension(
        "turbojpeg",
        sorted(glob("turbojpeg/*.cpp")),
        include_dirs=["libjpeg-turbo", "libjpeg-turbo-build"],
        library_dirs=["libjpeg-turbo-build"],
        libraries=["turbojpeg"],
        cxx_std=17,
    ),
]

setup(
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
