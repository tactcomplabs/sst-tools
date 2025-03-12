#
# sst-tools/src/dbgapi CMake
#
# Copyright (C) 2024-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(zeromq
  GIT_REPOSITORY https://github.com/zeromq/libzmq
  GIT_TAG v4.3.5
)

FetchContent_Declare(cppzmq
  GIT_REPOSITORY https://github.com/zeromq/cppzmq
  GIT_TAG v4.10.0
)

FetchContent_GetProperties(zeromq)
if(NOT zeromq_POPULATED)
  FetchContent_Populate(zeromq)
  add_subdirectory(${zeromq_SOURCE_DIR} ${zeromq_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_GetProperties(cppzmq)
if(NOT cppzmq_POPULATED)
  FetchContent_Populate(cppzmq)
  add_subdirectory(${cppzmq_SOURCE_DIR} ${cppzmq_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(DBGAPI_SRCS
  SSTDbgAPI.h
  SSTDbgAPI.cpp
)

add_library(sstdbgapi SHARED ${DBGAPI_SRCS})

target_link_libraries(sstdbgapi cppzmq)

# EOF
