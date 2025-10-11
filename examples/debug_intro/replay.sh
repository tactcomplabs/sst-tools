#!/bin/bash

#
# Drive interactive mode from an external replay file
#

SSTOPTS="--interactive-start=0 --replay=sst-console.in"
IMAGE_DATA=$(realpath "../../image_data")

cmd="sst nn.py ${SSTOPTS} -- \
    --classImageLimit=2000 \
    --batchSize=128 \
    --epochs=10 \
    --evalImages="${IMAGE_DATA}/eval" \
    --hiddenLayerSize=128 \
    --initialWeightScaling=0.01 \
    --testImages="${IMAGE_DATA}/fashion_mnist_images/test" \
    --trainingImages="${IMAGE_DATA}/fashion_mnist_images/train" \
    --verbose=2"

echo $cmd 
$cmd 
wait
