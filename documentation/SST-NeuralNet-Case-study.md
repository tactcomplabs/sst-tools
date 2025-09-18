# A Neural Network in SST: A Development and Debug Case Study
<img src="./imgs/logo.png" alt="sst-nn" width="300"/>

## Overview
An SST design for a neural network is developed to demonstrate use cases for emerging SST features support checkpointing and debug.
Git branches are used to capture various stages of model development so that each step can be evaluated individually. 

## Getting Started

### Model Source Code

Clone the model from git hub and checkout the `sst-nn` branch

      git clone git@github.com:tactcomplabs/sst-tools.git
      cd sst-tools
      git checkout sst-nn

### Image Data
To run tests, the image data needs to be downloaded.

      $ cd <sst-tools>/image_data
      $ ./get-all.py
      $ ls fashion_mnist_images/train/* | wc -l
         60019

### Eigen Library
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


### OpenCV Library

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


### User Environment

If not using a package manager you may need to set these environment variables in your shell setup script.

      export EIGEN_HOME=<eigen location>
      export OPENCV_HOME=<opencv location>
      export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$OPENCV_HOME/lib"


### Building and Testing

      cd build
      git config core.hooksPath .githooks
      cmake -DSST_TOOLS_ENABLE_TESTING=ON -DSST_TOOLS_NEURALNET=ON ..
      make && make install
      ctest -j 8

### Manual Test Example

      cd test/neuralnet
      ./nn-basic.sh 1
   
   You should see the following predictions:

      Prediction for /Users/kgriesser/work/sst-tools/image_data/eval/tshirt.png ... Survey says ### TOP ###
      Prediction for /Users/kgriesser/work/sst-tools/image_data/eval/pants.png ... Survey says ### TROUSER ###
      Simulation is complete, simulated time: 82.2502 ms

---
<br><br><br>

# Initial Model Block Diagram

The initial model is provided in the `sst-nn-0-base` branch of the repository. This model has no special enhancements for SST debug features. It is also, essentially, and single-threaded model. Although the components can be instantiated on parallel threads their operation is serialized.

<img src="./imgs/block-diagram.png" alt="initial-model" width="800"/>

## Class Hierarchy

<img src="./imgs/nn-classhier.png" alt="class-hierarchy" width="800"/>

## Model Bring-up

The initial model was based on a reference design containing several hidden layers and training on low resolution MNIST images.  The C++ development was brought up incrementally by building and checking each object against a reference design. SST was run on a single thread using 'printf' and LLDB as the primary methods to check and debug the functionality. 

## SST SDL

The SST model structure is described in Python which provides encapsulated layers, similar to PyTorch, for easy model construction.

<img src="./imgs/sstsdl.svg" alt="sstsdl" width="800"/>

## Development Process

This work was inspired by Neural Networks from Scratch in Python, Kinsley, Kukiela, 2020. The Python reference provides a modelling API similar to other popular AI front-ends. This was converted to a functional C++ model to establish the underlying linear algebra routines. Then, the C++ model was used as a reference to check a distributed, state machine-based implementation in SST.

<img src="./imgs/port.svg" alt="porting" width="800"/>

## Reference Code for the Section

[sst-nn-0-base](https://github.com/tactcomplabs/sst-tools/tree/sst-nn-0-base)

---
<br><br><br>

# Priming the Model for SST Serialization and Debug support

At this point, we have a complete and functional model. This seems like a convenient time to add serialization support. Additing serialization not only provides checkpointing and restart capability, but also makes internal data available to SST debug features. In this section, we'll add the necessary serialization macros and provide additional tests to ensure serialization is behaving as expected.

The official guidelines for serialization in SST are located at https://sst-simulator.org/sst-docs/docs/guides/features/checkpoint.

We will deviate slightly from the guidelines by only serializing a few variables for each class. The builds the complete infrastructure and allows some use of the debug features without having to ensure the integrity of the checkpointed simulation.

## Inherit from the Serializable Class

Since all our components inherent from SST serializable SST Core base classes we can skip this step.

## Change Clock and Event Handler Functions

The clock handler in [nn_batch_controller.cc](../sstcomp/neuralnet/nn_batch_controller.cc) is already using the required 'Handler2' type.

```
clockHandler  = new SST::Clock::Handler2<NNBatchController,&NNBatchController::clockTick>(this);
```

## Add a Default Constructor

If this easily overlooked step is missed, the model will not compile when serialization macros are added. Reviewing all the classes in the [class heirarchy](#class-hierarchy) reveals nearly all default constructors were missing. For example, in [nn_batch_controller.h](../sstcomp/neuralnet/nn_batch_controller.h) the following code was added:

```
public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNBatchController() : NNLayerBase() {};
};  //class NNBatchController
```

Similar code was added to all the classes.

Notice that the serialization support is put at the end of the class. This avoids potential compiler errors due to serializiation macros changing the access specifier. See the (sst checkpointing guidelines) [https://sst-simulator.org/sst-docs/docs/guides/features/checkpoint] for more information.


## Add a Serialization Function

Add the the business end of the serialization and deserialization feature to all serializable classes. 


The serialization support section in [nn_batch_controller.h](../sstcomp/neuralnet/nn_batch_controller.h) is now:

```
public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNBatchController() : NNLayerBase() {};
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
};  //class NNBatchController
```

The body of the overridden function in [nn_batch_controller.cc](../sstcomp/neuralnet/nn_batch_controller.cc) is

```
void NNBatchController::serialize_order(SST::Core::Serialization::serializer &ser)
{
  NNLayerBase::serialize_order(ser);
  SST_SER(output);
}
```

The rest of the class members will be serialized later. 
Similar code can be added, tediously, to all the other classes but do not bother adding any data members. Be sure to call the base class `serialize_order` function for each instance.

At this point we just want to be sure this code compiles and runs. It is common to miss a instance of an overridden function causing confusing and repetitive runtime errors such as:

```
Error: unable to find "neuralnet" element library
SST-DL: Loading failed for /Users/kgriesser/work/sst-tools/sstcomp/neuralnet/libneuralnet.dylib, error: dlopen(/Users/kgriesser/work/sst-tools/sstcomp/neuralnet/libneuralnet.dylib, 0x0009): symbol not found in flat namespace '__ZTIN3SST9NeuralNet11NNLayerBaseE'
SST-DL: Loading failed for /Users/kgriesser/work/sst-tools/sstcomp/neuralnet/libneuralnet.dylib, error: dlopen(/Users/kgriesser/work/sst-tools/sstcomp/neuralnet/libneuralnet.dylib, 0x0009): symbol not found in flat namespace '__ZTIN3SST9NeuralNet11NNLayerBaseE'
```

If there are many classes and subclasses (as in this case), it may be better to do a few classes at a time and build and test with each change.

## Add the Appropriate Serialization Macro

`ImplementVirtualSerializable` for pure virtual classes.
`ImplementSerializable` for all others.

Our final code for the class definition of NNBatchController is now:
```
public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNBatchController() : NNLayerBase() {};
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  // Serialization implementation
  ImplementSerializable(SST::NeuralNet::NNBatchController)
};  //class NNBatchController
```

Note that there is no semicolon after the `ImplementSerializable` statement.

There are several pure virtual classes in this implementation.
Example: [nn_layer_base.h](../sstcomp/neuralnet/nn_layer_base.h)

```
public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  NNOptimizerAPI() : SubComponent() {}
  // Serialization function
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  // Serialization implementation
  ImplementVirtualSerializable(SST::NeuralNet::NNOptimizerAPI)
}; //class NNOptimizerAPI
```

At this point we have a partially checkpointable model (just 1 variable). The code should compile and run correctly and adding code for the rest of the variables amounts to employing the SST_SER macro judiciously.

In the next section, we'll implement more serialserialization and briefly detour into the emerging realm of interactive debug.

## Reference Code for the Section

[sst-nn-1-ser](https://github.com/tactcomplabs/sst-tools/tree/sst-nn-1-ser)

---
<br><br><br>

# Introduction to the Interactive Debug Console

In the previous section, we built the scaffolding for serialization but did not attempt to make the model fully "checkpoint-able". That is, we cannot support restarting the simulation from a checkpoint yet. As it turns out, we're just not interested in supporting restart until we have a stable design and have scaled it up significantly.  However, serialization serves a second purpose: observability and controllabily of internal state.

We will demonstrate this by first serializing the hyperparameters used by the optimizer base class and the Adam based optimizer child class.

From [nn_layer.h](../sstcomp/neuralnet/nn_layer.h)

```
void NNOptimizerAPI::serialize_order(SST::Core::Serialization::serializer &ser)
{
  SubComponent::serialize_order(ser);
  SST_SER(sstout);
  SST_SER(learning_rate_);
  SST_SER(current_learning_rate_);
  SST_SER(iterations_);
}

void NNAdamOptimizer::serialize_order(SST::Core::Serialization::serializer &ser)
{
  NNOptimizerAPI::serialize_order(ser);
  SST_SER(decay_);
  SST_SER(epsilon_);
  SST_SER(beta_1_);
  SST_SER(beta_2_);
}
```

A side benefit is that we have fully serialized these classes with a few lines of trivial code.

Now we want to observe and control the learning rate given by the following code:

```
void NNAdamOptimizer::pre_update_params() {
  if (decay_ != 0) {
    current_learning_rate_ = learning_rate_ * ( 1. / (1. + decay_ * iterations_ ));
  }
}
```

Refering back to the [top level block diagram](#initial-model-block-diagram), the loss layer feeds back loss information for back propogation. In this implementation each dense layer uses this information in it's calculations to adjust weights and biases. 

With that in mind let's, enter the debug console at time 0 using the following SST command line options:
`--interactive-console=sst.interactive.simpledebug --interactive-start=0`

For running the neural network training, the command line is conveniently embedded in the provided script below.

```
$ cd <sst-tools>/test/neuralnet
$ ./nn-interactive
```

This results in:
```
1  Running small simulation
2  sst ../test-image.py --interactive-console=sst.interactive.simpledebug --interactive-start=0 -- 
       --batchSize=1 --classImageLimit=4 --epochs=4 --evalImages=/Users/kgriesser/work/sst-tools/image_data/eval 
       --hiddenLayerSize=32 --initialWeightScaling=0.01 
       --testImages=/Users/kgriesser/work/sst-tools/image_data/fashion_mnist_images/test
       --trainingImages=/Users/kgriesser/work/sst-tools/image_data/fashion_mnist_images/train 
       --verbose=0
3  NNBatchController[batch_controller:init:0]: init phase 0
4  NNBatchController[batch_controller:setup:0]: setup
5  Reading /Users/kgriesser/work/sst-tools/image_data/fashion_mnist_images/train
6  Loaded 16 images
7  Reading /Users/kgriesser/work/sst-tools/image_data/fashion_mnist_images/test
8  Loaded 16 images
9  Reading /Users/kgriesser/work/sst-tools/image_data/eval
10 Loading image from /Users/kgriesser/work/sst-tools/image_data/eval/tshirt.png
11 Loading image from /Users/kgriesser/work/sst-tools/image_data/eval/pants.png
12 Loaded 2 images
13 NNBatchController[batch_controller:setup:0]: setup completed. Ready for first clock
14 0 
15 Entering interactive mode at time 0 
16 Interactive start at 0
17 > 
```

Observed lines 3 and which indicates the main controller is entering the SST INIT and SETUP phases.
Line 13 shows when we exit the SETUP phase.
Line 17 is the interactive console command prompt.

This illustrates a current limitation of the SST interactive console:

`The interactive console is only available during the RUN phase of SST`
  
TODO: Can we address this restriction in component debug probe

At the command prompt, type `help` for a list of available commands.

Next step, navigate the design heirarchy to find the loss layer and it's optimizer subcomponent.
```
> ls
      batch_controller/ (SST::NeuralNet::NNBatchController)
      dense1/ (SST::NeuralNet::NNLayer)
      dense2/ (SST::NeuralNet::NNLayer)
      dense3/ (SST::NeuralNet::NNLayer)
      input/ (SST::NeuralNet::NNLayer)
      loss/ (SST::NeuralNet::NNLayer)
      relu1/ (SST::NeuralNet::NNLayer)
      relu2/ (SST::NeuralNet::NNLayer)
      softmax/ (SST::NeuralNet::NNLayer)
> cd loss
> ls
      accuracy_function/ (SST::NeuralNet::NNAccuracyCategorical)
      component/ (SST::NeuralNet::NNInputLayer)
      component/ (SST::NeuralNet::NNAccuracyCategorical)
      component/ (SST::NeuralNet::NNLoss_CategoricalCrossEntropy)
      component/ (SST::NeuralNet::NNAdamOptimizer)
      component_state_ = 0 (SST::BaseComponent::ComponentState)
      link_map/ (SST::LinkMap*)
      link_map/ (SST::LinkMap*)
      link_map/ (SST::LinkMap*)
      link_map/ (SST::LinkMap*)
      loss_function/ (SST::NeuralNet::NNLoss_CategoricalCrossEntropy)
      my_info_/ ()
      my_info_/ (SST::ComponentInfo*)
      optimizer/ (SST::NeuralNet::NNAdamOptimizer)
      transfer_function/ (SST::NeuralNet::NNInputLayer)
> cd optimizer
> ls
      beta_1_ = 0.900000 (double)
      beta_2_ = 0.999000 (double)
      component_state_ = 0 (SST::BaseComponent::ComponentState)
      current_learning_rate_ = 0.001000 (double)
      decay_ = 0.001000 (double)
      epsilon_ = 0.000000 (double)
      iterations_ = 0 (unsigned int)
      learning_rate_ = 0.001000 (double)
      my_info_/ ()
      my_info_/ (SST::ComponentInfo)
      sstout_/ (SST::Output)
> pwd
      loss/optimizer (SST::NeuralNet::NNAdamOptimizer)
```
Now we can "watch" the current learning rate and the simulation will break whenever its value changes.
```
> watch current_learning_rate_
> watch
      Current watch points:
      0 - loss/optimizer/current_learning_rate_
> run
      NNBatchController[batch_controller:initTraining:1000]: Starting training phase
      epoch: 0, step: 0, acc: 0.000, loss: 2.303 (data_loss: 2.303, reg_loss: 0.000) ,lr: 0.0010000000
      Entering interactive mode at time 24025000
      Watch point loss/optimizer/current_learning_rate_ buffer  # TODO can this print value automatically?
> print current_learning_rate_
      current_learning_rate_ = 0.000999 (double)
> run
      Entering interactive mode at time 40040000
      Watch point loss/optimizer/current_learning_rate_ buffer
> print current_learning_rate_
      current_learning_rate_ = 0.000999 (double)      
#TODO we need control of precision and formatting in general, or show scientific notation by default in short-term
#TODO maintain command history buffer
#TODO Can 'watch' with no params also print the values and indicate which one triggered breaking into interactive? 
```
Now we can change the learning rate and see the effects.

```
> set learning_rate_ -1.0
> ls
      beta_1_ = 0.900000 (double)
      beta_2_ = 0.999000 (double)
      component_state_ = 0 (SST::BaseComponent::ComponentState)
      current_learning_rate_ = 0.000999 (double)
      decay_ = 0.001000 (double)
      epsilon_ = 0.000000 (double)
      iterations_ = 2 (unsigned int)
      learning_rate_ = -1.000000 (double)
      my_info_/ ()
      my_info_/ (SST::ComponentInfo)
      sstout_/ (SST::Output)
> run
      Entering interactive mode at time 40041000
      Watch point loss/optimizer/current_learning_rate_ buffer
      Watch point loss/optimizer/current_learning_rate_ buffer
> print current_learning_rate_
      current_learning_rate_ = -0.998004 (double)
> shutdown
#TODO quit should clear any triggers that will break us into interactive mode
#TODO "cd /" - bring back to top level
```

So we've demonstrated how a few lines of code can enable some powerful debug capabilities.
Of course, GDB and LLDB have very powerful source level debug capabilities that can do the
same thing. In the upcoming sections, we'll demonstrate how the built-in SST features are
differentiated from standard software debug tools.

## Reference Code for the Section

[sst-nn-2-dbg-intro](https://github.com/tactcomplabs/sst-tools/tree/sst-nn-2-dbgintro)




