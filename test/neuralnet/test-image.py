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

parser = argparse.ArgumentParser(description="test image")
parser.add_argument("--trainingImages", type=str, help="path to training data organized in class subdirectories", default="")
parser.add_argument("--testImages", type=str, help="path to test data organized in class subdirectories", default="")
parser.add_argument("--evalImage", type=str, help="path to a single evaluation image", default="")
parser.add_argument("--epochs", type=int, help="epochs duh", default=0)
parser.add_argument("--classImageLimit", type=int, help="limited the number of images loaded per class", default=100000)

parser.add_argument("--verbose", type=int, help="verbosity. 5=send/recv", default=1)
args = parser.parse_args()
print("configuration:")
for arg in vars(args):
  print("\t", arg, " = ", getattr(args, arg))

batch_controller = sst.Component("batch_controller", "neuralnet.NNBatchController")
batch_controller.addParams({
  "verbose" : args.verbose,
  "trainingImages" : args.trainingImages,
  "testImages" : args.testImages,
  "evalImage" : args.evalImage,
  "epochs" : args.epochs,
  "classImageLimit" : args.classImageLimit,
})

layer = sst.Component("layer", "neuralnet.NNLayer")
layer.addParams({
  "verbose" : args.verbose,
})

link0 = sst.Link("link0")
link0.connect( (batch_controller, "forward", "1us"), (layer, "forward", "1us") )

# EOF
