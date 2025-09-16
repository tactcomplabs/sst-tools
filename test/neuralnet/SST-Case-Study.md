# An Neural Network in SST: A Development and Debug Case Study
<img src="./imgs/logo.png" alt="sst-nn" width="300"/>

## Overview
An SST design for a neural network is developed to demonstrate use cases for emerging SST features support checkpointing and debug.

## Initial Model

The initial model is provided in the `sst-nn-0-base` branch of the `sst-tools` repository on GitHub. This model has no special enhancements for SST debug features. It is also, essentially, and single-threaded model. Although the components can be instantiated on parallel threads their operation is serialized.

<img src="./imgs/block-diagram.svg" alt="initial-model" width="800"/>

### Model Bringup

The initial model was based on a reference design containing several hidden layers and training on low resolution MNIST images.  The C++ development was brought up incrementally by building and checking each object against a reference design. SST was run on a single thread using 'printf' and LLDB as the primary methods to check and debug the functionality. 

### SST SDL

The SST model structure is described in Python which provides encapsulated layers, similar to PyTorch, for easy model construction.

<img src="./imgs/sstsdl.svg" alt="sstsdl" width="800"/>

### Development Process

This work was inspired by Neural Networks from Scratch in Python, Kinsley, Kukiela, 2020. The Python reference provides a modelling API similar to other popular AI front-ends. This was converted to a functional C++ model to establish the underlying linear algebra routines. Then, the C++ model was used as a reference to check a distributed, state machine-based implementation in SST.

<img src="./imgs/port.svg" alt="porting" width="800"/>

## Adding SST Serialization and Debug support

