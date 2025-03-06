# sst-tools

## Overview

## Setup
Design and debug utilities, examples, and methods for SST developers

    git clone git@github.com:tactcomplabs/sst-tools.git
    cd build
    cmake -DSST_TOOLS_ENABLE_TESTING=ON ..
    make && make install
    ctest


CMake configuration options include:

    SST_TOOLS_MPI            - Enable MPI for testing if available (OFF)
    SST_TOOLS_SOCKETS        - Enable tests, like CLIDBG, that require sockets (ON)
    SST_TOOLS_CLEAN_TESTS    - delete saved checkpoint directories (ON)
    SST_TOOLS_ENABLE_TESTING - enable all tests (OFF)
    SST_TOOLS_WERROR         - enabled -Werror compile flag (ON)

## Checkpoint API
For information on generating json schema files for checkpoints and interacting with them using a Python API see the [cptapi documentation](test/cptapi/README.md).
