#
# sst-tools/test/cptapi CMake
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(UserLibs "--add-lib-path=${CMAKE_BINARY_DIR}/sstcomp/grid")
set (passRegex "Simulation is complete")
set (failRegex "[Ee][Rr][Rr][Oo][Rr]")

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
add_test(NAME cpt-save
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${SCRIPTS}/sst-chkpt.sh ${nmpi} cpt.save --num-threads=2 ${UserLibs} 2d.py --checkpoint-period=10ns -- --x=2 --y=2
)
set_tests_properties(cpt-save
  PROPERTIES
  TIMEOUT 60
  LABELS "cptapi"
  PASS_REGULAR_EXPRESSION "${passRegex}"
  FAIL_REGULAR_EXPRESSION "${failRegex}"
)

add_test(NAME cpt-restore
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${SCRIPTS}/sst-restore.sh ${SST_TOOLS_CLEAN_TESTS} ${nmpi} cpt.save cpt.save/cpt.save_800_8010000/cpt.save_800_8010000.sstcpt --num-threads=2 ${UserLibs}
)
set_tests_properties(cpt-restore
  PROPERTIES
  TIMEOUT 60
  LABELS "cptapi"
  DEPENDS cpt-save
  PASS_REGULAR_EXPRESSION "${passRegex}"
  FAIL_REGULAR_EXPRESSION "${failRegex}"
)

#  
# Subcomponent/Type Checkpoint Save/Restore Tests
#

# TODO std:pair needs to be fixed
# set(subcomps grid.CPTSubCompVecInt grid.CPTSubCompVecStruct grid.CPTSubCompPairOfStructs)
set(subcomps grid.CPTSubCompVecInt grid.CPTSubCompVecStruct)
foreach(subcomp IN ITEMS ${subcomps})
  add_test(NAME ${subcomp}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND test-subcomp.sh ${subcomp} ${TCL_SCHEMA} ${SST_TOOLS_CLEAN_TESTS}
  )
  set_tests_properties(${subcomp}
    PROPERTIES
    TIMEOUT 60
    LABELS "cptapi"
    PASS_REGULAR_EXPRESSION "${passRegex}"
    FAIL_REGULAR_EXPRESSION "${failRegex}"
  )
endforeach()

#
# schema testing ( requires tcl sst-core schema branch, version -dev-schema)
#
if( ${TCL_SCHEMA} )
  add_test(NAME cpt-schema
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ./test-schema.sh)
  set_tests_properties(cpt-schema
    PROPERTIES
    TIMEOUT 60
    LABELS "cptapi"
    PASS_REGULAR_EXPRESSION "test-schema.sh finished normally")
  # Cleanup conditionally
  if( ${SST_TOOLS_CLEAN_TESTS} )
    add_test(NAME cpt-schema-clean
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND rm -rf cpt.schema)
    set_tests_properties(cpt-schema-clean
      PROPERTIES
      TIMEOUT 10
      LABELS "cptapi"
      DEPENDS cpt-schema)
  endif()
endif()


# EOF
