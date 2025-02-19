#
# sst-tools/test CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

if (mpiOK)
  message(STATUS "[SST-TOOLS] Testing with MPI enabled")
else()
  message(STATUS "[SST-TOOLS] Testing with MPI disabled")
endif()

message(STATUS "[SST-TOOLS] Enabling DBGCLI testing")
add_subdirectory(dbgcli)

message(STATUS "[SST-TOOLS] Enabling CPTAPI testing")
add_subdirectory(cptapi)

# EOF
