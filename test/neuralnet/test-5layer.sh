#!/bin/bash

#-- Command line options                                                                                                                                           
largesim=0
if [ $# -ne 0 ]; then
 largesim=$1
fi

mkdir -p run
cd run

# SSTOPTS='--exit-after="5s"'
SSTOPTS=''

verbose=0
if [ ! -z "${VERBOSE}" ]; then
    verbose=${VERBOSE}
fi

hiddenLayers=5

if [ $largesim -eq 0 ]; then
    echo "Running small simulation"
    sst ../test-image.py ${SSTOPTS} -- \
        --batchSize=1 \
        --classImageLimit=4 \
        --epochs=4 \
        --evalImage="${HOME}/work/image_data/eval/pants.png" \
        --evalImages="${HOME}/work/image_data/eval" \
        --hiddenLayers=${hiddenLayers} \
        --hiddenLayerSize=32 \
        --initialWeightScaling=0.01 \
        --testImages="${HOME}/work/image_data/fashion_mnist_images/test" \
        --trainingImages="${HOME}/work/image_data/fashion_mnist_images/train" \
        --verbose=${verbose}
else
    echo "Running large simulation"
    sst ../test-image.py ${SSTOPTS} -- \
        --batchSize=128 \
        --epochs=10 \
        --evalImage="${HOME}/work/image_data/eval/pants.png" \
        --evalImages="${HOME}/work/image_data/eval" \
        --hiddenLayers=${hiddenLayers} \
        --hiddenLayerSize=128 \
        --initialWeightScaling=0.01 \
        --testImages="${HOME}/work/image_data/fashion_mnist_images/test" \
        --trainingImages="${HOME}/work/image_data/fashion_mnist_images/train" \
        --verbose=${verbose}
fi

wait

