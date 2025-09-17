#!/bin/bash

#-- Command line options                                                                                                                                           
largesim=0
if [ $# -ne 0 ]; then
 largesim=$1
fi

mkdir -p run
cd run

# SSTOPTS='--exit-after="5s"'
SSTOPTS='-n 3 --output-json=exp.json --parallel-output=1'
SSTOPTS2='-n 2 --parallel-load'

# TODO debug running with MPI
#MPIOPTS='mpirun -N 2'
#MPIOPTS2='mpirun -N 2'

IMAGE_DATA=$(realpath "../../../image_data")

verbose=10
if [ ! -z "${VERBOSE}" ]; then
    verbose=${VERBOSE}
fi

hiddenLayers=5

if [ $largesim -eq 0 ]; then
    echo "Running small simulation"
    $MPIOPTS sst ../test-image.py ${SSTOPTS} -- \
        --batchSize=1 \
        --classImageLimit=4 \
        --epochs=4 \
        --evalImages="${IMAGE_DATA}/eval" \
        --hiddenLayers=${hiddenLayers} \
        --hiddenLayerSize=32 \
        --initialWeightScaling=0.01 \
        --testImages="${IMAGE_DATA}/fashion_mnist_images/test" \
        --trainingImages="${IMAGE_DATA}/fashion_mnist_images/train" \
        --verbose=${verbose} | tee run.log
else
    echo "Running large simulation"
    $MPIOPTS sst ../test-image.py ${SSTOPTS} -- \
        --batchSize=128 \
        --epochs=10 \
        --evalImages="${IMAGE_DATA}/eval" \
        --hiddenLayers=${hiddenLayers} \
        --hiddenLayerSize=128 \
        --initialWeightScaling=0.01 \
        --testImages="${IMAGE_DATA}/fashion_mnist_images/test" \
        --trainingImages="${IMAGE_DATA}/fashion_mnist_images/train" \
        --verbose=${verbose} | tee run.log
fi

## reload configuration and run again
if [ $largesim -eq 0 ]; then
    echo "Running small simulation"
    $MPIOPTS2 sst exp.json ${SSTOPTS2} -- \
        --batchSize=1 \
        --classImageLimit=4 \
        --epochs=4 \
        --evalImages="${IMAGE_DATA}/eval" \
        --hiddenLayers=${hiddenLayers} \
        --hiddenLayerSize=32 \
        --initialWeightScaling=0.01 \
        --testImages="${IMAGE_DATA}/fashion_mnist_images/test" \
        --trainingImages="${IMAGE_DATA}/fashion_mnist_images/train" \
        --verbose=${verbose} | tee run2.log
else
    echo "Running large simulation"
    $MPIOPTS2 sst exp.json ${SSTOPTS2} -- \
        --batchSize=128 \
        --epochs=10 \
        --evalImages="${IMAGE_DATA}/eval" \
        --hiddenLayers=${hiddenLayers} \
        --hiddenLayerSize=128 \
        --initialWeightScaling=0.01 \
        --testImages="${IMAGE_DATA}/fashion_mnist_images/test" \
        --trainingImages="${IMAGE_DATA}/fashion_mnist_images/train" \
        --verbose=${verbose} | tee run2.log
fi

echo
echo 'comparing sim times'
grep 'simulated time:' run.log
grep 'simulated time:' run2.log

wait

