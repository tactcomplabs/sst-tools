# Local Environment Setup for Neural Network Compilation

## Eigen
Eigen is a C++ template library for linear algebra: matrices, vectors, numerical solvers, and related algorithms.
For more information go to http://eigen.tuxfamily.org/ or https://libeigen.gitlab.io/docs/.

To clone and build Eigen source code:
```
git clone https://gitlab.com/libeigen/eigen.git
cd eigen
mkdir -p build && cd build
export EIGEN_HOME=$PWD
cmake .. -DCMAKE_INSTALL_PREFIX=$EIGEN_HOME
make install

```

## OpenCV

Ubuntu build:

https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html

```
git clone https://github.com/opencv/opencv.git
cd opencv
export OPENCV_HOME=$PWD
git checkout 4.x
mkdir -p build && cd build
cmake ..
make -j 8
ls bin
ls lib
```Z

## User Profile

Add to ~/.profile
```
export EIGEN_HOME=<eigen location>
export OPENCV_HOME=<opencv location>
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$OPENCV_HOME/lib"
```

# Image Data
To run tests, the image data needs to be downloaded.
```
cd <sst-tools>/image_data
./get-all.py
$ ls fashion_mnist_images/train/* | wc -l
   60019

```
