#
# sst-tools/src/readcpt CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

cmake_minimum_required(VERSION 3.19)
project(readcpt-grid CXX)
add_executable(readcpt-grid)
target_sources(readcpt-grid PUBLIC 
  readcpt-grid.cc
)
install(TARGETS readcpt-grid DESTINATION ${SST_TOOLS_INSTALL_PATH}/bin)

# EOF
