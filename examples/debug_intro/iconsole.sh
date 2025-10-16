#!/bin/bash

sst nn.py --interactive-start=0 -- \
    --classImageLimit=2000 --batchSize=128 --epochs=10 \
    --hiddenLayerSize=128 --initialWeightScaling=0.01 \
    --trainingImages=/Users/kgriesser/work/sst-tools/image_data/fashion_mnist_images/train \
    --testImages=/Users/kgriesser/work/sst-tools/image_data/fashion_mnist_images/test \
    --evalImages=/Users/kgriesser/work/sst-tools/image_data/eval \
    --verbose=2

wait
