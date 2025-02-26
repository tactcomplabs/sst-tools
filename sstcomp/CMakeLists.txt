#
# sst-tools/sstcomp CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

#-- Include Paths
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

#-- SST Compile Options
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SST_LDFLAGS}" )

#-- Components
message(STATUS "[SST-TOOLS] Enabling dbgcli")
add_subdirectory(dbgcli)
add_subdirectory(grid)


# EOF
