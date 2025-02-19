#!/bin/bash

# Synopsis
#   ctest driver script for tests generating checkpoints.
# Usage
#   sst-chkpt  {mpi-ranks} {checkpoint-prefix} {rest of sst options}
# Purpose
#   This script provides mpirun prefix if needed and deletes the checkpoint
#   directory if it exists in order to ensure that sst does not create another
#   directory with a numeric suffix.

ranks=$1
pfx=$2


if [ -d ${pfx} ]; then
    echo "WARNING: removing ${pfx}"
    rm -rf ${pfx}
fi

if (( $ranks > 1)); then
    mpipfx="mpirun -n ${ranks}"
fi

args=${@:3:$#}
cmd="${mpipfx} sst --checkpoint-prefix=${pfx} $args"
echo $cmd
eval "${cmd}"

#EOF

