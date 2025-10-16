#!/bin/bash

#
# Start from a checkpoint and enter interactive mode.
#

SSTOPTS="--interactive-start=0 --load-checkpoint checkpoint/checkpoint_1_32262251000/checkpoint_1_32262251000.sstcpt"
IMAGE_DATA=$(realpath "../../image_data")

cmd="sst nn.py ${SSTOPTS}"

echo $cmd 
$cmd 
wait
