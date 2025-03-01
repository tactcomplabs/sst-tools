#
# sst-tools top-level CMake
#
# Copyright (C) 2024-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

cmake_minimum_required(VERSION 3.19)

#-- Prevent in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "DO NOT BUILD in-tree.")
endif()

# Minimum required version of CMake and project information
cmake_minimum_required(VERSION 3.19)
project(sst-tools CXX)

# Make sure Git hooks are set properly
execute_process(COMMAND_ERROR_IS_FATAL ANY COMMAND_ECHO STDERR COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/test_git_hooks.sh)

# Local scripts
set(SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/scripts")

#-- SST Setup
execute_process(COMMAND sst-config --prefix
                OUTPUT_VARIABLE SST_INSTALL_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(SST_INCLUDE_DIR "${SST_INSTALL_DIR}/include")
if(NOT (EXISTS "${SST_INSTALL_DIR}"))
  message(FATAL_ERROR " SST_INSTALL_DIR (${SST_INSTALL_DIR}) is invalid.")
endif()

# SST Configuration Sanity Check
find_program(SST sst)
find_program(SST_CONFIG sst-config)
if(NOT SST OR NOT SST_CONFIG)
  message(FATAL_ERROR "No SST binary or sst-config binary found in path")
endif()

# SST Environment Derivation
execute_process(COMMAND sst-config --CXX
                OUTPUT_VARIABLE CXX
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND sst-config --ELEMENT_CXXFLAGS
                OUTPUT_VARIABLE SST_CXXFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CXXFLAGS "${SST_CXXFLAGS}  -fno-stack-protector")
execute_process(COMMAND sst-config --ELEMENT_LDFLAGS
                OUTPUT_VARIABLE SST_LDFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Use SST_LDFLAGS in sstcomp only or it will affect add_executable() in other subprojects
set(LDFLAGS -fno-stack-protector)

execute_process(COMMAND sst-config SST_ELEMENT_LIBRARY SST_ELEMENT_LIBRARY_LIBDIR
                OUTPUT_VARIABLE SST_ELEMENT_LIBRARY_LIBDIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND ${SCRIPTS}/sst-major-version.sh
                OUTPUT_VARIABLE SST_MAJOR_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
                )
execute_process(COMMAND ${SCRIPTS}/sst-minor-version.sh
                OUTPUT_VARIABLE SST_MINOR_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "SST MAJOR VERSION=${SST_MAJOR_VERSION}")

#-- Compiler Options
option(WERROR "Compile with warnings as errors" ON)
if (WERROR)
  set(WERROR_FLAG "-Werror")
endif()

# Compiler Options
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(FP_MODE_FLAG "-ffp-model=strict -frounding-math -ftrapping-math")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
else()
  set(FP_MODE_FLAG "-frounding-math -ftrapping-math")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-format-attribute -Wsuggest-final-methods -Wsuggest-final-types -Wvolatile")
endif()

set(CMAKE_CXX_FLAGS "-std=c++17 ${FP_MODE_FLAG} -O2 -Wall -Wextra -Wsuggest-override -Wmissing-noreturn -Wvla -Wuninitialized -Wdouble-promotion -Wsign-conversion -Wconversion -Wno-unused-parameter -Wno-deprecated-declarations -Wno-macro-redefined ${WERROR_FLAG} ${CMAKE_CXX_FLAGS} -I./ ${LDFLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")


#-- Include Paths
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

#-- MPI Availability
execute_process(
  COMMAND mpirun --version
  COMMAND egrep -q "mpirun .+ 4\."
  RESULT_VARIABLE mpiDisable
)

#---------------------------------------------
# TARGET OPTIONS
#---------------------------------------------
# Install directory for APIs and Executables
set(SST_TOOLS_INSTALL_PATH ${PROJECT_SOURCE_DIR}/install)

#-- Executables
add_subdirectory(src)

#-- Testing Options
option(SST_TOOLS_CLEAN_TESTS "Clean test directories after running" ON)
option(SST_TOOLS_ENABLE_TESTING "Enable Testing" OFF)
#-- Checkpoint Serialization build flags (temporary)
if ( ${SST_MAJOR_VERSION} EQUAL "90" )
  set(SST_SCHEMA_TESTING ON)
endif()
if(SST_TOOLS_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

#-- Component source directories
add_subdirectory(sstcomp)

#-- Custom Commands
add_custom_target(uninstall 
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/uninstall.sh
  COMMAND ${CMAKE_COMMAND} -E echo "Deleting ${SST_TOOLS_INSTALL_PATH}/bin"
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${SST_TOOLS_INSTALL_PATH}"
  VERBATIM
)


# EOF
