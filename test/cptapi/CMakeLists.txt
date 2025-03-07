#
# sst-tools/test/cptapi CMake
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

# All tests generating checkpoints have a dependent test that 
# checks them and conditionally deletes the checkpoint files.

set(testSrc 2d.py)
set(UserLibs "--add-lib-path=${CMAKE_BINARY_DIR}/sstcomp/grid")
set (passRegex "Simulation is complete")
set (failRegex "[Ee][Rr][Rr][Oo][Rr]")
get_filename_component(testName ${testSrc} NAME_WLE)
set(CHKPT_PFX ${testName}_SAVE_)
set(SUBCOMP "--subcomp=grid.CPTSubCompVecInt")

# For simple ctest just running MPI with 2 ranks.
# May need to add RUN_SERIAL property for threaded tests as some point.
if (${mpiDisable})
  set(nmpi 1)
else()
  set(nmpi 2)
endif()

#
# Checkpoint Save/Restore - no subcomponent
#
add_test(NAME ${testName}_SAVE
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${SCRIPTS}/sst-chkpt.sh ${nmpi} ${CHKPT_PFX} --num-threads=2 ${UserLibs} ${testSrc} --checkpoint-period=10ns -- --x=2 --y=2
)
set_tests_properties(${testName}_SAVE
  PROPERTIES
  TIMEOUT 60
  LABELS "cptapi"
  PASS_REGULAR_EXPRESSION "${passRegex}"
  FAIL_REGULAR_EXPRESSION "${failRegex}"
)

add_test(NAME ${testName}_RESTORE
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${SCRIPTS}/sst-restore.sh ${SST_TOOLS_CLEAN_TESTS} ${nmpi} ${CHKPT_PFX}/${CHKPT_PFX}_800_8010000/${CHKPT_PFX}_800_8010000.sstcpt --num-threads=2 ${UserLibs}
)
set_tests_properties(${testName}_RESTORE
  PROPERTIES
  TIMEOUT 60
  LABELS "cptapi"
  DEPENDS ${testName}_SAVE
  PASS_REGULAR_EXPRESSION "${passRegex}"
  FAIL_REGULAR_EXPRESSION "${failRegex}"
)

#  
# Checkpoint Save -- --subcomp=CPTSubComp.CPTSubCompVecInt
#
add_test(NAME ${testName}_SAVE_VECINT
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${SCRIPTS}/sst-chkpt.sh ${nmpi} ${CHKPT_PFX}VecInt --num-threads=2 ${UserLibs} ${testSrc} --checkpoint-period=10ns -- --x=2 --y=2 ${SUBCOMP}
)
set_tests_properties(${testName}_SAVE_VECINT
  PROPERTIES
  TIMEOUT 60
  LABELS "cptapi"
  PASS_REGULAR_EXPRESSION "${passRegex}"
  FAIL_REGULAR_EXPRESSION "${failRegex}"
)

add_test(NAME ${testName}_RESTORE_VECINT
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${SCRIPTS}/sst-restore.sh ${SST_TOOLS_CLEAN_TESTS} ${nmpi} ${CHKPT_PFX}VecInt/${CHKPT_PFX}VecInt_800_8010000/${CHKPT_PFX}VecInt_800_8010000.sstcpt --num-threads=2 ${UserLibs}
)
set_tests_properties(${testName}_RESTORE_VECINT
  PROPERTIES
  TIMEOUT 60
  LABELS "cptapi"
  DEPENDS ${testName}_SAVE_VECINT
  PASS_REGULAR_EXPRESSION "${passRegex}"
  FAIL_REGULAR_EXPRESSION "${failRegex}"
)

#
# schema testing ( requires tcl sst-core schema branch)
#
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
