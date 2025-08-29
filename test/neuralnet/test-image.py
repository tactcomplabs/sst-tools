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

parser.add_argument("--verbose", type=int, help="verbosity. 5=send/recv", default=1)
args = parser.parse_args()
print("configuration:")
for arg in vars(args):
  print("\t", arg, " = ", getattr(args, arg))

cp0 = sst.Component("cp0", "neuralnet.NNBatchController")
cp0.addParams({
  "verbose" : args.verbose,
  "trainingImages" : args.trainingImages,
  "testImages" : args.testImages,
  "evalImage" : args.evalImage,
  "epochs" : args.epochs,
})

# EOF
