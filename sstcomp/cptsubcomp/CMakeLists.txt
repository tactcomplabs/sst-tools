#
# sst-tools/sstcomp/cptsubcomp CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(CPTSubCompSrcs
  cptsubcomp.cc
  cptsubcomp.h
)

add_library(cptsubcomp SHARED ${CPTSubCompSrcs})
target_include_directories(cptsubcomp PUBLIC ${SST_INSTALL_DIR}/include)
install(TARGETS cptsubcomp DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register cptsubcomp cptsubcomp_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

# EOF
