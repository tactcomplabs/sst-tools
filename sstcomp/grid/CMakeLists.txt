#
# sst-tools/sstcomp/grid CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(GridSrcs
  gridnode.cc
  gridnode.h
  cptsubcomp.cc
  cptsubcomp.h
)

add_library(grid SHARED ${GridSrcs})
target_include_directories(grid PUBLIC ${SST_INSTALL_DIR}/include)
install(TARGETS grid DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register grid                       grid_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompPair             grid_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompPairOfStructs    grid_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompVecInt           grid_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompVecPair          grid_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompVecPairOfStructs grid_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register CPTSubCompVecStruct        grid_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
# EOF
