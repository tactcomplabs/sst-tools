#!/bin/bash
mkdir -p run
cd run

# SSTOPTS='--exit-after="5s"'
SSTOPTS=''

sst ../test-image.py $SSTOPTS -- \
    --batchSize=1 \
    --classImageLimit=4 \
    --epochs=4 \
    --evalImage="${HOME}/work/image_data/eval/pants.png" \
    --hiddenLayerSize=32 \
    --testImages="${HOME}/work/image_data/fashion_mnist_images/test" \
    --trainingImages="${HOME}/work/image_data/fashion_mnist_images/train" \
    --verbose=2

wait

