#!/bin/bash
#
# Unregisters Rev from the SST infrastructure
#

#--remove files from cmake install manifest
xargs rm < install_manifest.txt

#-- unregister it
sst-register -u CPTSubCompPairOfStructs
sst-register -u CPTSubCompListPairOfStructs
sst-register -u CPTSubCompPair
sst-register -u CPTSubCompVecPair
sst-register -u CPTSubCompVecPairOfStructs
sst-register -u CPTSubCompVecInt
sst-register -u CPTSubCompVecStruct
sst-register -u dbgcli
sst-register -u gridtest
sst-register -u NNBatchController
sst-register -u NNLayer
sst-register -u NNInputLayer
sst-register -u NNDenseLayer
sst-register -u NNActivationReLULayer
sst-register -u NNActivationSoftmaxLayer
sst-register -u NNLoss_CategoricalCrossEntropy
sst-register -u NNAccuracyCategorical
sst-register -u NNAdamOptimizer
sst-register -u OMSimpleComponent
sst-register -u OMSimpleSubComponent

#-- forcible remove it from the local script
CONFIG=~/.sst/sstsimulator.conf
if test -f "$CONFIG"; then
  echo "Removing configuration from local config=$CONFIG"
  sed -i.bak '/CPTSubCompPairOfStructs/d' $CONFIG
  sed -i.bak '/CPTSubCompListPairOfStructs/d' $CONFIG
  sed -i.bak '/CPTSubCompPair/d' $CONFIG
  sed -i.bak '/CPTSubCompPairOfStructs/d' $CONFIG
  sed -i.bak '/CPTSubCompVecPair/d' $CONFIG
  sed -i.bak '/CPTSubCompVecPairOfStructs/d' $CONFIG
  sed -i.bak '/CPTSubCompVecInt/d' $CONFIG
  sed -i.bak '/dbgcli/d' $CONFIG
  sed -i.bak '/gridtest/d' $CONFIG
  sed -i.bak '/NNBatchController/d' $CONFIG
  sed -i.bak '/NNLayer/d' $CONFIG
  sed -i.bak '/NNInputLayer/d' $CONFIG
  sed -i.bak '/NNDenseLayer/d' $CONFIG
  sed -i.bak '/NNActivationReLULayer/d' $CONFIG
  sed -i.bak '/NNActivationSoftmaxLayer/d' $CONFIG
  sed -i.bak '/NNLoss_CategoricalCrossEntropy/d' $CONFIG
  sed -i.bak '/NNAccuracyCategorical/d' $CONFIG
  sed -i.bak '/NNAdamOptimizer/d' $CONFIG
  sed -i.bak '/OMSimpleComponent/d' $CONFIG
  sed -i.bak '/OMSimpleSubComponent/d' $CONFIG
fi

