#
# sst-tools/test/dbgcli CMake
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

add_test(
  NAME clidbg-sanity
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
  COMMAND ./run-sanity.bash
)
set_tests_properties(clidbg-sanity PROPERTIES
  LABELS "probe"
  TIMEOUT 30
  # TODO this cycle varies between macos(5900000) and ubuntu(4500000)
  #  PASS_REGULAR_EXPRESSION "#T cycle=5900000"
  PASS_REGULAR_EXPRESSION "#T cycle="
)

# EOF
