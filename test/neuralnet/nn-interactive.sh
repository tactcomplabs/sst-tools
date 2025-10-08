#!/bin/bash

#-- Command line options                                                                                                                                           
largesim=0
if [ $# -ne 0 ]; then
 largesim=$1
fi

# SSTOPTS="--interactive-console=sst.interactive.simpledebug --interactive-start=0"
SSTOPTS="--interactive-start=0"

mkdir -p run
cd run

IMAGE_DATA=$(realpath "../../../image_data")

verbose=0
if [ ! -z "${VERBOSE}" ]; then
    verbose=${VERBOSE}
fi

if [ $largesim -eq 0 ]; then
    echo "Running small simulation"
    cmd="sst ../test-image.py ${SSTOPTS} -- \
        --batchSize=1 \
        --classImageLimit=4 \
        --epochs=4 \
        --evalImages="${IMAGE_DATA}/eval" \
        --hiddenLayerSize=32 \
        --initialWeightScaling=0.01 \
        --testImages="${IMAGE_DATA}/fashion_mnist_images/test" \
        --trainingImages="${IMAGE_DATA}/fashion_mnist_images/train" \
        --verbose=${verbose}"
else
    echo "Running large simulation"
    cmd="sst ../test-image.py ${SSTOPTS} -- \
        --batchSize=128 \
        --epochs=10 \
        --evalImages="${IMAGE_DATA}/eval" \
        --hiddenLayerSize=128 \
        --initialWeightScaling=0.01 \
        --testImages="${IMAGE_DATA}/fashion_mnist_images/test" \
        --trainingImages="${IMAGE_DATA}/fashion_mnist_images/train" \
        --verbose=${verbose}"
fi

echo $cmd
$cmd

wait

