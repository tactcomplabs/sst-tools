# sst-tools

## Overview

## Setup
Design and debug utilities, examples, and methods for SST developers

    git clone git@github.com:tactcomplabs/sst-tools.git
    cd build
    cmake -DSST_TOOLS_ENABLE_TESTING=ON ..
    make && make install
    ctest


Additional cmake configuration options include:

    ALLOW_MPI - Enable MPI for testing if available (ON)
    SST_TOOLS_CLEAN_TESTS - delete saved checkpoint directories (ON)
    WERROR - enabled -Werror compile flag (ON)

## Checkpoint API
For information on generating json schema files for checkpoints and interacting with them using a Python API see the [cptapi documentation](test/cptapi/README.md).
