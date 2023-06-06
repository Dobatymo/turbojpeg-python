brew install nasm
brew uninstall --ignore-dependencies jpeg-turbo
python -m pip install -U pip wheel
python -m pip install -U cmake
mkdir libjpeg-turbo-build
cd libjpeg-turbo-build
cmake -G"Unix Makefiles" ../libjpeg-turbo
make
cd ..
