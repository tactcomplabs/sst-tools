#
# sst-tools/test/omap CMake
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

add_test(
  NAME omsimple
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
  COMMAND ./omsimple.sh
)
set_tests_properties(omsimple PROPERTIES
  LABELS "omap"
  TIMEOUT 5
  PASS_REGULAR_EXPRESSION "Simulation is complete, simulated time: 10 ns"
)

# EOF
