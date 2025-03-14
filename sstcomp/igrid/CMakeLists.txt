#
# sst-tools/sstcomp/igrid CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(IGridSrcs
  igridnode.cc
  igridnode.h
)

add_library(igrid SHARED ${IGridSrcs})
target_include_directories(igrid PUBLIC ${SST_INSTALL_DIR}/include)
install(TARGETS igrid DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register igrid igrid_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

# EOF
