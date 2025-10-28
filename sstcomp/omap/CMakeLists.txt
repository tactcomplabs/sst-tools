#
# sst-tools/omap CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(OMapSrcs
  OMap.cc
)

add_library(omap SHARED ${OMapSrcs})

target_include_directories(omap PUBLIC 
  ${SST_INSTALL_DIR}/include
)

install(TARGETS omap DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register OMSimpleComponent omap_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register OMSimpleSubComponent omap_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

# EOF
