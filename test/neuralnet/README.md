# SST Neural Network

<img src="../../documentation/imgs/logo.png" alt="sst-nn" width="300"/>

## Model Source Code

Clone the model from git hub and checkout the `sst-nn` branch

      git clone git@github.com:tactcomplabs/sst-tools.git
      cd sst-tools
      git checkout sst-nn

## Image Data
To run tests, the image data needs to be downloaded.

      $ cd <sst-tools>/image_data
      $ ./get-all.py
      $ ls fashion_mnist_images/train/* | wc -l
         60019

## Eigen
Eigen is a C++ template library for linear algebra: matrices, vectors, numerical solvers, and related algorithms.
For more information go to http://eigen.tuxfamily.org/ or https://libeigen.gitlab.io/docs/.

To clone and build Eigen from the source code:

      git clone https://gitlab.com/libeigen/eigen.git
      cd eigen
      mkdir -p build && cd build
      export EIGEN_HOME=$PWD
      cmake .. -DCMAKE_INSTALL_PREFIX=$EIGEN_HOME
      make install

It is available on MacOS Homebrew as well.

      brew install eigen


## OpenCV

Ubuntu build:

https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html

      git clone https://github.com/opencv/opencv.git
      cd opencv
      export OPENCV_HOME=$PWD
      git checkout 4.x
      mkdir -p build && cd build
      cmake ..
      make -j 8
      ls bin
      ls lib

It is available on MacOS Homebrew as well.

      brew install opencv


## User Profile

If not using a package manager you may need to set these environment variables in your shell setup script.

      export EIGEN_HOME=<eigen location>
      export OPENCV_HOME=<opencv location>
      export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$OPENCV_HOME/lib"


# Building and Testing

      cd build
      git config core.hooksPath .githooks
      cmake -DSST_TOOLS_ENABLE_TESTING=ON -DSST_TOOLS_NEURALNET=ON ..
      make && make install
      ctest -j 8

# Manual Test Example

      cd test/neuralnet
      ./nn-basic.sh 1
   
   You should see the following predictions:

      Prediction for /Users/kgriesser/work/sst-tools/image_data/eval/tshirt.png ... Survey says ### TOP ###
      Prediction for /Users/kgriesser/work/sst-tools/image_data/eval/pants.png ... Survey says ### TROUSER ###
      Simulation is complete, simulated time: 82.2502 ms