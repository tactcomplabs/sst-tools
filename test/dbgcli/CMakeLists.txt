#
# sst-bench/test/dbgcli CMake
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

file(GLOB DBGCLI_TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} dbgcli-test*.py)

if(SSTBENCH_ENABLE_TESTING)
  set (passRegex "Simulation is complete")

  foreach(testSrc ${DBGCLI_TEST_SRCS})
    get_filename_component(testName ${testSrc} NAME_WE)
    add_test(NAME ${testName}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND sst --checkpoint-sim-period=1us --checkpoint-prefix=${testName}_SAVE_ --add-lib-path=${CMAKE_BINARY_DIR}/sst-bench/dbgcli ${testSrc})
    set_tests_properties(${testName}
      PROPERTIES
      TIMEOUT 60
      LABELS "all"
      PASS_REGULAR_EXPRESSION "${passRegex}")
  endforeach(testSrc)
endif()

# EOF
