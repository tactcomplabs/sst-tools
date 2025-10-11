#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# test-image.py
#

import argparse
import sst
import sys
from enum import Enum

parser = argparse.ArgumentParser(description="test image")

parser.add_argument("--batchSize",            type=int,   help="number of images for each training batch", default=128)
parser.add_argument("--classImageLimit",      type=int,   help="limited the number of images loaded per class", default=100000)
parser.add_argument("--epochs",               type=int,   help="number of training rounds", default=1)
parser.add_argument("--evalImage",            type=str,   help="path to a single evaluation image", default="")
parser.add_argument("--evalImages",           type=str,   help="path to a collection of evaluation images", default="")
parser.add_argument("--hiddenLayers",         type=int,   help="number of hidden layers (3 minimum)", default=3)
parser.add_argument("--hiddenLayerSize",      type=int,   help="number of neurons in each hidden layer", default=128)
parser.add_argument("--initialWeightScaling", type=float, help="scaling factor for random weights", default=0.1)
parser.add_argument("--testImages",           type=str,   help="path to test data organized in class subdirectories", default="")
parser.add_argument("--trainingImages",       type=str,   help="path to training data organized in class subdirectories", default="")
parser.add_argument("--verbose",              type=int,   help="verbosity", default=1)

args = parser.parse_args()

if args.verbose==10:
  print("configuration:")
  for arg in vars(args):
    print("\t", arg, " = ", getattr(args, arg))

if (args.hiddenLayers < 3):
  print("hiddenLayers requires minimum value of 3", file=sys.stderr)
  sys.exit(1)

# current simple test images are 28x28
image_size = 28 * 28

batch_controller_params = {
  "batchSize" : args.batchSize,
  "classImageLimit" : args.classImageLimit,
  "epochs" : args.epochs,
  "evalImage" : args.evalImage,
  "evalImages" : args.evalImages,
  "testImages" : args.testImages,
  "trainingImages" : args.trainingImages,
  "verbose" : args.verbose,
}

# Optimizer parameters
optimizer_params = {
  "learningRate" : 0.001,
  "decay" : 1e-3,
  "epsilon" : 1e-7,
  "beta_1" : 0.9,
  "beta_2" : 0.999,
  "verbose" : args.verbose,
}

class DenseLayer():
  def __init__(self, name, inputs, neurons):
    self.comp =  sst.Component(name,  "neuralnet.NNLayer")
    self.transfer_function = self.comp.setSubComponent( 
      "transfer_function", "neuralnet.NNDenseLayer" )
    self.transfer_function.addParams({ 
      "nInputs" : inputs, "nNeurons" : neurons,
      "initialWeightScaling" : args.initialWeightScaling,
      "verbose" : args.verbose })
    self.optimizer = self.comp.setSubComponent(
      "optimizer", "neuralnet.NNAdamOptimizer" )
    self.optimizer.addParams({ "verbose" : args.verbose })
    self.optimizer.addParams( optimizer_params )

class ReLU():
  def __init__(self, name):
    self.comp = sst.Component(name,   "neuralnet.NNLayer")
    self.comp.addParams( {"verbose" : args.verbose} )
    self.transfer_function = self.comp.setSubComponent( 
      "transfer_function", "neuralnet.NNActivationReLULayer")
    self.transfer_function.addParams( { "verbose" : args.verbose })

class Softmax():
  def __init__(self, name):
    self.comp = sst.Component(name, "neuralnet.NNLayer")
    self.comp.addParams( {"verbose" : args.verbose} )
    self.tranfer_function = self.comp.setSubComponent("transfer_function", "neuralnet.NNActivationSoftmaxLayer")
    self.tranfer_function.addParams( {"verbose" : args.verbose} )

class Loss_CategoricalCrossEntropy:
  def __init__(self, name):
    self.comp = sst.Component(name,    "neuralnet.NNLayer")
    self.comp.addParams({ 
      "lastComponent" : 1,
      "verbose" : args.verbose })
    self.transfer_function = self.comp.setSubComponent( "transfer_function", "neuralnet.NNInputLayer")
    self.transfer_function.addParams({ "verbose" : args.verbose })
    self.accuracy_function = self.comp.setSubComponent( "accuracy_function", "neuralnet.NNAccuracyCategorical")
    self.accuracy_function.addParams( {"verbose" : args.verbose} );
    self.loss_function = self.comp.setSubComponent( "loss_function", "neuralnet.NNLoss_CategoricalCrossEntropy")
    self.loss_function.addParams( {"verbose" : args.verbose} )
    self.optimizer = self.comp.setSubComponent( "optimizer", "neuralnet.NNAdamOptimizer" )
    self.optimizer.addParams( optimizer_params )

# Model construction
comps = []
forward_links = []
backward_links = []

# Primary controller
comps.append(sst.Component("batch_controller", "neuralnet.NNBatchController"))
comps[-1].addParams(batch_controller_params)

# Input Layer
input = sst.Component("input",   "neuralnet.NNLayer")
input.setSubComponent("transfer_function", "neuralnet.NNInputLayer")
comps.append(input)

# Hidden Layers
comps.append(DenseLayer("dense1", image_size, args.hiddenLayerSize).comp)
comps.append(ReLU("relu1").comp)
for i in range(2, args.hiddenLayers):
  comps.append(DenseLayer(f"dense{i}", args.hiddenLayerSize, args.hiddenLayerSize).comp)
  comps.append(ReLU(f"relu{i}").comp)
comps.append(DenseLayer(f"dense{args.hiddenLayers}", args.hiddenLayerSize, 10).comp)

# Output Layers
comps.append(Softmax("softmax").comp)
comps.append(Loss_CategoricalCrossEntropy("loss").comp)

# Connections
for i in range(len(comps)):
  if i < len(comps)-1:
    forward_links.append(sst.Link(f"F{i}"))
    backward_links.append(sst.Link(f"B{i}"))
    forward_links[i].connect(  (comps[i],  "forward_o", "1us"),     (comps[i+1], "forward_i", "1us") )
    backward_links[i].connect( (comps[i+1], "backward_o", "1us"),  (comps[i], "backward_i", "1us") )
  else:
    # loss layer feeds results back into the primary controller
    monitor_link = sst.Link("M")
    monitor_link.connect( (comps[i], "monitor", "1us"), (comps[0], "monitor", "1us") )

#EOF
