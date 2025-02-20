#
# sst-tools/test CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

if (${mpiDisable})
  message(STATUS "[SST-TOOLS] Testing with MPI disabled")
else()
  message(STATUS "[SST-TOOLS] Testing with MPI enabled")
endif()

message(STATUS "[SST-TOOLS] Enabling DBGCLI testing")
add_subdirectory(dbgcli)

message(STATUS "[SST-TOOLS] Enabling CPTAPI testing")
add_subdirectory(cptapi)

# EOF
