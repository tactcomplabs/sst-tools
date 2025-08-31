#
# sst-tools/test/dbgcli CMake
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

add_test(
  NAME neuralnet
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
  COMMAND ./run-nn.sh
)
set_tests_properties(neuralnet PROPERTIES
  LABELS "neuralnet"
  TIMEOUT 5
  PASS_REGULAR_EXPRESSION "Simulation is complete, simulated time: 64.069 us"
)

# EOF
