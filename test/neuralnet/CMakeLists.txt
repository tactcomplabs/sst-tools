#
# sst-tools/test/neuralnet CMake
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

add_test(
  NAME nn-basic
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
  COMMAND ./nn-basic.sh
)
set_tests_properties(nn-basic PROPERTIES
  LABELS "neuralnet"
  TIMEOUT 5
  PASS_REGULAR_EXPRESSION "Simulation is complete, simulated time: 1.61963 ms"
)

add_test(
  NAME nn-basic-1
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
  COMMAND ./nn-basic.sh 1
)
set_tests_properties(nn-basic-1 PROPERTIES
  LABELS "neuralnet"
  TIMEOUT 120
  PASS_REGULAR_EXPRESSION ".*Survey says ### TOP ###.*Survey says ### TROUSER ###.*Simulation is complete"
)

add_test(
  NAME nn-5layers
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
  COMMAND ./nn-5layers.sh
)
set_tests_properties(nn-5layers PROPERTIES
  LABELS "neuralnet"
  TIMEOUT 5
  PASS_REGULAR_EXPRESSION "Simulation is complete, simulated time: 2.3964 ms"
)

add_test(
  NAME nn-5layers-1
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
  COMMAND ./nn-5layers.sh 1
)
set_tests_properties(nn-5layers-1 PROPERTIES
  LABELS "neuralnet"
  TIMEOUT 120
  PASS_REGULAR_EXPRESSION ".*Survey says ### TOP ###.*Survey says ### TROUSER ###.*Simulation is complete"
)

# EOF
