#!/bin/bash
#
# Unregisters Rev from the SST infrastructure
#

#-- unregister it
sst-register -u CPTSubCompPairOfStructs
sst-register -u CPTSubCompPair
sst-register -u CPTSubCompVecPair
sst-register -u CPTSubCompVecPairOfStructs
sst-register -u CPTSubCompVecInt
sst-register -u CPTSubCompVecStruct
sst-register -u dbgcli
sst-register -u gridtest
sst-register -u NNBatchController

#-- forcible remove it from the local script
CONFIG=~/.sst/sstsimulator.conf
if test -f "$CONFIG"; then
  echo "Removing configuration from local config=$CONFIG"
  sed -i.bak '/CPTSubCompPairOfStructs/d' $CONFIG
  sed -i.bak '/CPTSubCompPair/d' $CONFIG
  sed -i.bak '/CPTSubCompPairOfStructs/d' $CONFIG
  sed -i.bak '/CPTSubCompVecPair/d' $CONFIG
  sed -i.bak '/CPTSubCompVecPairOfStructs/d' $CONFIG
  sed -i.bak '/CPTSubCompVecInt/d' $CONFIG
  sed -i.bak '/dbgcli/d' $CONFIG
  sed -i.bak '/gridtest/d' $CONFIG
  sed -i.back'/NNBatchController/d' $CONFIG
fi

