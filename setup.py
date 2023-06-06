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

with open("readme.md", encoding="utf-8") as fr:
    long_description = fr.read()

setup(
    name="turbojpeg",
    version="0.0.1",
    author="Dobatymo",
    author_email="Dobatymo@users.noreply.github.com",
    url="https://github.com/Dobatymo/turbojpeg-python",
    description="Python bindungs for libjpeg-turbo using pybind11",
    long_description=long_description,
    long_description_content_type="text/markdown",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
    extras_require={
        "test": ["Pillow", "numpy", "scikit-image"],
    },
)
