#
# sst-bench/test/grid CMake
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

# Checkpoint Save
add_test(NAME ${testName}_SAVE
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${SCRIPTS}/sst-chkpt.sh ${CHKPT_PFX} --num-threads=2 --checkpoint-period=10ns --add-lib-path=${lpath} ${testSrc})
set_tests_properties(${testName}_SAVE
  PROPERTIES
  TIMEOUT 60
  LABELS "cptapi"
  PASS_REGULAR_EXPRESSION "${passRegex}")

# Checkpoint Restore
add_test(NAME ${testName}_RESTORE
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND sst --add-lib-path=${lpath} --num-threads=2 --load-checkpoint ${CHKPT_PFX}/${CHKPT_PFX}_100_1010000/${CHKPT_PFX}_100_1010000.sstcpt)
set_tests_properties(${testName}_RESTORE
  PROPERTIES
  TIMEOUT 60
  LABELS "cptapi"
  DEPENDS ${testName}_SAVE
  PASS_REGULAR_EXPRESSION "${passRegex}")


# EOF
