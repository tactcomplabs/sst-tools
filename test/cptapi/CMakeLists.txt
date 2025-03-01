#
# sst-tools/test/grid CMake
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(testSrc 2d.py)
set(lpath "${CMAKE_BINARY_DIR}/sstcomp/grid")
set (passRegex "Simulation is complete")
get_filename_component(testName ${testSrc} NAME_WLE)
set(CHKPT_PFX ${testName}_SAVE_)

if (${mpiDisable})
  set(nmpi 1)
else()
  set(nmpi 2)
endif()

# Checkpoint Save
add_test(NAME ${testName}_SAVE
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${SCRIPTS}/sst-chkpt.sh ${nmpi} ${CHKPT_PFX} --num-threads=2 --add-lib-path=${lpath} ${testSrc} --checkpoint-period=10ns -- --x=2 --y=2)
set_tests_properties(${testName}_SAVE
  PROPERTIES
  TIMEOUT 60
  LABELS "cptapi"
  PASS_REGULAR_EXPRESSION "${passRegex}")

# Checkpoint Restore
add_test(NAME ${testName}_RESTORE
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${SCRIPTS}/sst-restore.sh ${nmpi} ${CHKPT_PFX}/${CHKPT_PFX}_100_1010000/${CHKPT_PFX}_100_1010000.sstcpt --num-threads=2 --add-lib-path=${lpath} )
set_tests_properties(${testName}_RESTORE
  PROPERTIES
  TIMEOUT 60
  LABELS "cptapi"
  DEPENDS ${testName}_SAVE
  PASS_REGULAR_EXPRESSION "${passRegex}")

# Cleanup conditionally
if( ${SST_TOOLS_CLEAN_TESTS} )
  add_test(NAME ${testName}_CLEAN
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND rm -rf ${CHKPT_PFX})
  set_tests_properties(${testName}_CLEAN
    PROPERTIES
    TIMEOUT 10
    LABELS "cptapi"
    DEPENDS ${testName}_RESTORE)
endif()

# schema testing ( requires tcl sst-core schema branch)
if( ${SST_SCHEMA_TESTING} )
  add_test(NAME SCHEMA_TEST
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ./schema-test.sh)
  set_tests_properties(SCHEMA_TEST
    PROPERTIES
    TIMEOUT 60
    LABELS "cptapi"
    PASS_REGULAR_EXPRESSION "schema-test.sh finished normally")
  # Cleanup conditionally
  if( ${SST_TOOLS_CLEAN_TESTS} )
    add_test(NAME SCHEMA_TEST_CLEAN
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND rm -rf SCHEMA_SAVE_)
    set_tests_properties(SCHEMA_TEST_CLEAN
      PROPERTIES
      TIMEOUT 10
      LABELS "cptapi"
      DEPENDS SCHEMA_TEST)
  endif()
endif()


# EOF
