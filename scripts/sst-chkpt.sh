#!/bin/bash

# Synopsis
#   ctest driver script for tests generating checkpoints.
# Usage
#   sst-chkpt {checkpoint prefix} {rest of sst options}
# Purpose
#   This script deletes the checkpoint directory if it exists in order to
#   ensure that sst does not create another directory with a numeric suffix.

if [ -d $1 ]; then
    echo "WARNING: removing $1"
    rm -rf ${1}
fi

args=${@:2:$#}
echo sst --checkpoint-prefix=$1 $args
sst --checkpoint-prefix=$1 $args

#EOF

