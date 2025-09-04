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

parser.add_argument("--batchSize", type=int, help="number of images for each training batch", default=128)
parser.add_argument("--classImageLimit", type=int, help="limited the number of images loaded per class", default=100000)
parser.add_argument("--epochs", type=int, help="number of training rounds", default=1)
parser.add_argument("--evalImage", type=str, help="path to a single evaluation image", default="")
parser.add_argument("--hiddenLayerSize", type=int, help="number of neurons in each hidden layer", default=128)
parser.add_argument("--initialWeightScaling", type=float, help="scaling factor for random weights", default=0.1)
parser.add_argument("--testImages", type=str, help="path to test data organized in class subdirectories", default="")
parser.add_argument("--trainingImages", type=str, help="path to training data organized in class subdirectories", default="")
parser.add_argument("--verbose", type=int, help="verbosity. 5=send/recv", default=1)

args = parser.parse_args()
print("configuration:")
for arg in vars(args):
  print("\t", arg, " = ", getattr(args, arg))

# Primary controller
batch_controller = sst.Component("batch_controller", "neuralnet.NNBatchController")
batch_controller.addParams({
  "batchSize" : args.batchSize,
  "classImageLimit" : args.classImageLimit,
  "epochs" : args.epochs,
  "evalImage" : args.evalImage,
  "testImages" : args.testImages,
  "trainingImages" : args.trainingImages,
  "verbose" : args.verbose,
})

# Optimizer parameters
optimizer_params = {
  "verbose" : args.verbose,
  "learningRate" : 0.001,
  "decay" : 1e-3,
  "epsilon" : 1e-7,
  "beta_1" : 0.9,
  "beta_2" : 0.999
 }

# Instantiate layers
input   = sst.Component("input",   "neuralnet.NNLayer")
input.setSubComponent(  "transfer_function", "neuralnet.NNInputLayer")

# Limit input images to 28 x 28.
dense1  = sst.Component("dense1",  "neuralnet.NNLayer")
subd1 = dense1.setSubComponent( "transfer_function", "neuralnet.NNDenseLayer")
subd1.addParams( {"nInputs" : 28*28, "nNeurons" : args.hiddenLayerSize,
                 "initialWeightScaling" : args.initialWeightScaling,
                 "verbose" : args.verbose } )
subd1Opt = dense1.setSubComponent( "optimizer", "neuralnet.NNAdamOptimizer")
subd1Opt.addParams( optimizer_params )

relu1   = sst.Component("relu1",   "neuralnet.NNLayer")
relu1.setSubComponent(  "transfer_function", "neuralnet.NNActivationReLULayer")
relu1.addParams( {"verbose" : args.verbose} )

dense2  = sst.Component("dense2",  "neuralnet.NNLayer")
subd2 = dense2.setSubComponent( "transfer_function", "neuralnet.NNDenseLayer")
subd2.addParams( {"nInputs" : args.hiddenLayerSize, "nNeurons" : args.hiddenLayerSize,
                 "initialWeightScaling" : args.initialWeightScaling,
                 "verbose" : args.verbose } )
subd2Opt = dense2.setSubComponent( "optimizer", "neuralnet.NNAdamOptimizer")
subd2Opt.addParams( optimizer_params )

relu2   = sst.Component("relu2",   "neuralnet.NNLayer")
relu2.setSubComponent(  "transfer_function", "neuralnet.NNActivationReLULayer")
relu2.addParams( {"verbose" : args.verbose} )

dense3  = sst.Component("dense3",  "neuralnet.NNLayer")
subd3 = dense3.setSubComponent("transfer_function", "neuralnet.NNDenseLayer" )
subd3.addParams( {"nInputs" : args.hiddenLayerSize, "nNeurons" : 10,
                 "initialWeightScaling" : args.initialWeightScaling,
                 "verbose" : args.verbose } )
subd3Opt = dense3.setSubComponent( "optimizer", "neuralnet.NNAdamOptimizer")
subd3Opt.addParams( optimizer_params )

softmax = sst.Component("softmax", "neuralnet.NNLayer")
subsoftmax = softmax.setSubComponent("transfer_function", "neuralnet.NNActivationSoftmaxLayer")
subsoftmax.addParams( {"verbose" : args.verbose} )

# Final stage: Loss Layer

loss    = sst.Component("loss",    "neuralnet.NNLayer")
loss.addParams( { "lastComponent" : 1, "verbose" : args.verbose} )

loss_xfr = loss.setSubComponent( "transfer_function", "neuralnet.NNInputLayer")
loss_xfr.addParams( {"verbose" : args.verbose} );

loss_acc=loss.setSubComponent( "accuracy_function", "neuralnet.NNAccuracyCategorical")
loss_acc.addParams( {"verbose" : args.verbose} );

loss_fnc = loss.setSubComponent( "loss_function", "neuralnet.NNLoss_CategoricalCrossEntropy")
loss_fnc.addParams( {"verbose" : args.verbose} );

lossOpt = loss.setSubComponent( "optimizer", "neuralnet.NNAdamOptimizer" )
lossOpt.addParams( optimizer_params )

# Ordered lists
components = []
forward_links = []
backward_links = []

components.append(batch_controller)
components.append(input)
components.append(dense1)
components.append(relu1)
components.append(dense2)
components.append(relu2)
components.append(dense3)
components.append(softmax)
components.append(loss)

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