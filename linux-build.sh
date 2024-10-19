yum makecache
yum -y install nasm devtoolset-11-gcc
apk update
apk add --upgrade gcc nasm
python -m pip install -U pip wheel
python -m pip install -U 'cmake>=3.5,<4.0'
mkdir libjpeg-turbo-build
cd libjpeg-turbo-build
cmake -G"Unix Makefiles" ../libjpeg-turbo
make
cd ..
