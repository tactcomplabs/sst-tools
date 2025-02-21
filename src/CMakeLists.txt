#
# sst-tools/src CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

#-- Include Paths
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

#-- Application Sources
message(STATUS "[SST-TOOLS] Enabling READCPT")
add_subdirectory(readcpt)

message(STATUS "[SST-TOOLS] Enabling READCPT-GRID")
add_subdirectory(readcpt-grid)

# EOF
