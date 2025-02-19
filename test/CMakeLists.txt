#
# sst-tools/test CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

message(STATUS "[SST-BENCH] Enabling DBGCLI testing")
add_subdirectory(dbgcli)

message(STATUS "[SST-BENCH] Enabling CPTAPI testing")
add_subdirectory(cptapi)

# EOF
