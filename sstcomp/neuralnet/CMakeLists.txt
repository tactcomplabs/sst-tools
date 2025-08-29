#
# sst-tools/sstcomp/sstnn CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(NNSrcs
  nn_batch_controller.cc
)

add_library(neuralnet SHARED ${NNSrcs})
target_include_directories(neuralnet PUBLIC ${SST_INSTALL_DIR}/include)
install(TARGETS neuralnet DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register NNBatchController neuralnet_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

# EOF
