#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# dbgcli-test1.py
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

hiddenLayerSize = args.hiddenLayerSize

# Instantiate layers
input   = sst.Component("input",   "neuralnet.NNLayer")
input.setSubComponent(  "transfer_function", "neuralnet.NNInputLayer")

# Limit input images to 28 x 28.
dense1  = sst.Component("dense1",  "neuralnet.NNLayer")
subd1 = dense1.setSubComponent( "transfer_function", "neuralnet.NNDenseLayer")
subd1.addParams( {"nInputs" : 28*28, "nNeurons" : args.hiddenLayerSize} )

relu1   = sst.Component("relu1",   "neuralnet.NNLayer")
relu1.setSubComponent(  "transfer_function", "neuralnet.NNActivationReLULayer")

dense2  = sst.Component("dense2",  "neuralnet.NNLayer")
subd2 = dense2.setSubComponent( "transfer_function", "neuralnet.NNDenseLayer")
subd2.addParams( {"nInputs" : args.hiddenLayerSize, "nNeurons" : args.hiddenLayerSize} )

relu2   = sst.Component("relu2",   "neuralnet.NNLayer")
relu2.setSubComponent(  "transfer_function", "neuralnet.NNActivationReLULayer")

dense3  = sst.Component("dense3",  "neuralnet.NNLayer")
subd3 = dense3.setSubComponent( "transfer_function", "neuralnet.NNDenseLayer")
subd3.addParams( {"nInputs" : args.hiddenLayerSize, "nNeurons" : 10} )

softmax = sst.Component("softmax", "neuralnet.NNLayer")
softmax.setSubComponent("transfer_function", "neuralnet.NNActivationSoftmaxLayer")

loss    = sst.Component("loss",    "neuralnet.NNLayer")
loss.setSubComponent(   "transfer_function", "neuralnet.NNLossLayer")
loss.addParams( { "lastComponent" : 1 } )

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