#
# sst-tools/sstcomp/dbgcli CMake
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(DbgSST15Srcs
  dbgsst15.cc
  dbgsst15.h
  ICDbgSST15.cc
  ICDbgSST15.h
  probe.cc
  probe.h
)

add_library(dbgsst15 SHARED ${DbgSST15Srcs})
target_include_directories(dbgsst15 PUBLIC ${SST_INSTALL_DIR}/include)
install(TARGETS dbgsst15 DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register dbgsst15 dbgsst15_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

#add_library(icdbg SHARED ${ICDbgSrcs})
#target_include_directories(icdbg PUBLIC ${SST_INSTALL_DIR}/include)
#install(TARGETS icdbg DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
#install(CODE "execute_process(COMMAND sst-register icdbg icdbg_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

install(CODE "execute_process(COMMAND sst-register ICDebugSST15 dgbsst15_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")


# EOF
