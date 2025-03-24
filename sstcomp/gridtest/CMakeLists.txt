#
# sst-tools/sstcomp/grid CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(GridSrcs
  gridtestnode.cc
  gridtestnode.h
  cptsubcomp.cc
  cptsubcomp.h
)

add_library(gridtest SHARED ${GridSrcs})
target_include_directories(gridtest PUBLIC ${SST_INSTALL_DIR}/include)
install(TARGETS gridtest DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register gridtest                    gridtest_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompPair              gridtest_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompPairOfStructs     gridtest_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompVecInt            gridtest_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompVecPair           gridtest_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompVecPairOfStructs  gridtest_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompVecStruct         gridtest_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompListPairOfStructs gridtest_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
# EOF
