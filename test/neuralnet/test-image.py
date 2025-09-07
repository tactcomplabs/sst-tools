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
parser.add_argument("--verbose",              type=int,   help="verbosity. 5=send/recv", default=1)  

args = parser.parse_args()
print("configuration:")
for arg in vars(args):
  print("\t", arg, " = ", getattr(args, arg))


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

# Primary controller
batch_controller = sst.Component("batch_controller", "neuralnet.NNBatchController")
batch_controller.addParams(batch_controller_params)

# Layers
image_size = 28 * 28
input = sst.Component("input",   "neuralnet.NNLayer")
input.setSubComponent("transfer_function", "neuralnet.NNInputLayer")
dense1 = DenseLayer("dense1", image_size, args.hiddenLayerSize)
relu1 = ReLU("relu1")
dense2 = DenseLayer("dense2", args.hiddenLayerSize, args.hiddenLayerSize)
relu2 = ReLU("relu2")
dense3 = DenseLayer("dense3", args.hiddenLayerSize, 10)
softmax = Softmax("softmax")
loss = Loss_CategoricalCrossEntropy("loss")

# Connections
components = []
forward_links = []
backward_links = []

components.append(batch_controller)
components.append(input)
components.append(dense1.comp)
components.append(relu1.comp)
components.append(dense2.comp)
components.append(relu2.comp)
components.append(dense3.comp)
components.append(softmax.comp)
components.append(loss.comp)

for i in range(len(components)):
  components[i].addParams( { "verbose" : args.verbose } )
  if i < len(components)-1:
    forward_links.append(sst.Link(f"F{i}"))
    backward_links.append(sst.Link(f"B{i}"))
    forward_links[i].connect(  (components[i],  "forward_o", "1us"),     (components[i+1], "forward_i", "1us") )
    backward_links[i].connect( (components[i+1], "backward_o", "1us"),  (components[i], "backward_i", "1us") )
  else:
    # loss layer feeds results back into the primary controller 
    monitor_link = sst.Link("M")
    monitor_link.connect( (components[i], "monitor", "1us"), (components[0], "monitor", "1us") )

#EOF