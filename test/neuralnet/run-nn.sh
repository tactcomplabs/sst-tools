#!/bin/bash
mkdir -p run
cd run

classImageLimit=4

sst ../test-image.py --exit-after="5s" -- \
    --trainingImages="${HOME}/work/image_data/fashion_mnist_images/train" \
    --testImages="${HOME}/work/image_data/fashion_mnist_images/test" \
    --evalImage="${HOME}/work/image_data/eval/pants.png" \
    --epochs=10 \
    --classImageLimit=$classImageLimit

wait

