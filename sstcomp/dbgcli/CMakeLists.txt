#
# sst-tools/sstcomp/dbgcli CMake
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(DbgCLISrcs
  dbgcli.cc
  dbgcli.h
  probe.cc
  probe.h
)

add_library(dbgcli SHARED ${DbgCLISrcs})
target_include_directories(dbgcli PUBLIC ${SST_INSTALL_DIR}/include)
install(TARGETS dbgcli DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register dbgcli dbgcli_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

# EOF
