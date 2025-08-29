#!/bin/bash
mkdir -p run
cd run

sst ../test-image.py -- \
    --trainingImages="${HOME}/work/image_data/fashion_mnist_images/train" \
    --testImages="${HOME}/work/image_data/fashion_mnist_images/test" \
    --evalImage="${HOME}/work/image_data/eval/pants.png" \
    --epochs=1050

wait

