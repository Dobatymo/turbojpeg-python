yum makecache
yum -y install nasm devtoolset-11-gcc
python -m pip install -U pip wheel
python -m pip install -U cmake
mkdir libjpeg-turbo-build
cd libjpeg-turbo-build
cmake -DCMAKE_C_COMPILER=/usr/bin/gcc-11 -G"Unix Makefiles" ../libjpeg-turbo
make
cd ..
ls libjpeg-turbo-build
