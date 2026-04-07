#
# sst-tools top-level CMake
#
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

# ---------------------------------------------------
# string_contains_number
# ---------------------------------------------------
function(string_contains_number str result)
  string(REGEX MATCH "[0-9]" has_number "${str}")
  if(has_number)
    set(${result} TRUE PARENT_SCOPE)
  else()
    set(${result} FALSE PARENT_SCOPE)
  endif()
endfunction()

# ---------------------------------------------------
# main
# ---------------------------------------------------
#-- Prevent in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "DO NOT BUILD in-tree.")
endif()

# Minimum required version of CMake and project information
cmake_minimum_required(VERSION 3.19)
project(sst-tools CXX)

# Make sure Git hooks are set properly
# TODO get working with jenkins. Shouldn't need this unless committing changes ( not for testing )
# execute_process(COMMAND_ERROR_IS_FATAL ANY COMMAND_ECHO STDERR COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/test_git_hooks.sh)

#-- rpath
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# Local scripts
set(SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/scripts")

# SST Configuration Sanity Check
message(STATUS "Looking for sst-config")
find_program(SST sst)
find_program(SST_CONFIG sst-config)
if(NOT SST OR NOT SST_CONFIG)
  message(FATAL_ERROR "No SST binary or sst-config binary found in path")
endif()

#-- SST Setup
message(STATUS "Checking sst-config for sst installation directory")
execute_process(COMMAND sst-config --prefix
                OUTPUT_VARIABLE SST_INSTALL_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(SST_INCLUDE_DIR "${SST_INSTALL_DIR}/include")
if(NOT (EXISTS "${SST_INSTALL_DIR}"))
  message(FATAL_ERROR " SST_INSTALL_DIR (${SST_INSTALL_DIR}) is invalid.")
endif()
include_directories(${SST_INSTALL_DIR})

# SST Environment Derivation
message(STATUS "Checking sst-config environment")

# SST Version found in path
execute_process(COMMAND sst --version
                OUTPUT_VARIABLE SST_VERSION_STRING
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "${SST_VERSION_STRING}")

# SST Flags
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
set(LDFLAGS "${SST_LDFLAGS} -fno-stack-protector")

execute_process(COMMAND sst-config SST_ELEMENT_LIBRARY SST_ELEMENT_LIBRARY_LIBDIR
                OUTPUT_VARIABLE SST_ELEMENT_LIBRARY_LIBDIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "Checking for sst version info")
# Underscore prefix convention indicates private scope.
# Use the resolved SST_VERSION in other other cmake files
execute_process(COMMAND ${SCRIPTS}/sst-major-version.sh
                OUTPUT_VARIABLE _SST_MAJOR_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
                )
execute_process(COMMAND ${SCRIPTS}/sst-minor-version.sh
                OUTPUT_VARIABLE _SST_MINOR_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
)

string_contains_number("${_SST_MAJOR_VERSION}" contains_num)
if(NOT contains_num)
  set(_SST_MAJOR_VERSION "DEV")
  set(_SST_MINOR_VERSION "DEV")
  execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/sst-git-hash.sh
                  OUTPUT_VARIABLE SST_GIT_HASH
                  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  message(STATUS "SST GIT HASH=${SST_GIT_HASH}")
endif()

#-- Create version variables (will rewrite command line provided SST_VERSION)
message(STATUS "SST MAJOR VERSION=${_SST_MAJOR_VERSION}")
message(STATUS "SST MINOR VERSION=${_SST_MINOR_VERSION}")

# Allow override using -DSST_VERSION=n
# Important: subdirectories should only use this resolved version
if (DEFINED SST_VERSION)
  message(STATUS "SST_VERSION test selection override: ${SST_VERSION}")
elseif ("${_SST_MAJOR_VERSION}" STREQUAL "DEV")
  set(SST_VERSION "DEV")
else()
 string(CONCAT SST_VERSION  ${_SST_MAJOR_VERSION} "." ${_SST_MINOR_VERSION})
endif()
message(STATUS "RESOLVED SST VERSION=${SST_VERSION}")


#-- Compiler Options
option(SST_TOOLS_WERROR "Compile with warnings as errors" ON)
if (SST_TOOLS_WERROR)
  set(WERROR_FLAG "-Werror")
endif()

option(SST_TOOLS_ASAN "Compile using address sanitizer" OFF)
if (SST_TOOLS_ASAN)
  set (SANITIZE_FLAG "-fsanitize=address -fno-omit-frame-pointer")
endif()

#set(SST_SUGGEST_FINAL -Wsuggest-final-methods -Wsuggest-final-types")

# Compiler Options
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(FP_MODE_FLAG "-ffp-model=strict -frounding-math -ftrapping-math")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
else()
  set(FP_MODE_FLAG "-frounding-math -ftrapping-math")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-format-attribute -Wvolatile ${SST_SUGGEST_FINAL}")
endif()

# Handle deprecated API's
if( ${SST_VERSION} VERSION_GREATER_EQUAL "15.0" AND ${SST_VERSION} VERSION_LESS "16.0" )
  message(STATUS "[SST-TOOLS] USE_HANDLER2 is defined for ${SST_VERSION}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_HANDLER2")
endif()

#TODO Isolated Eigen library which has many warnings for missing-noreturn
#set(SST_MISSING_NORETURN -Wmissing-noreturn")
set(CMAKE_CXX_FLAGS "-std=c++17 ${FP_MODE_FLAG} -O2 -Wall -Wextra -Wsuggest-override ${SST_MISSING_NORETURN} -Wvla -Wuninitialized -Wfloat-conversion -Wdouble-promotion -Wsign-conversion -Wconversion -Wno-unused-parameter -Wno-deprecated-declarations -Wno-macro-redefined ${WERROR_FLAG} ${SANITIZE_FLAG} ${CMAKE_CXX_FLAGS} -I./ ${LDFLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")


#-- Include Paths
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

#-- MPI Availability
option(SST_TOOLS_MPI "Use MPI if available for testing" OFF)
if ( ${SST_TOOLS_MPI} )
  execute_process(
    COMMAND mpirun --version
    COMMAND egrep -q "mpirun .+ 4\."
    RESULT_VARIABLE mpiDisable
    )
else()
  set(mpiDisable 1)
endif()

#---------------------------------------------
# TARGET OPTIONS
#---------------------------------------------
# Install directory for APIs and Executables
set(SST_TOOLS_INSTALL_PATH ${PROJECT_SOURCE_DIR}/install)

#-- Executables
add_subdirectory(src)

#-- Testing Options
option(SST_TOOLS_CLEAN_TESTS "Clean test directories after running" ON)
option(SST_TOOLS_ENABLE_TESTING "Enable testing" OFF)
option(SST_TOOLS_SOCKETS "Enable tests using sockets" ON)

#-- Neural Net build (requires Eigen OpenCV
option(SST_TOOLS_NEURALNET "Enable neural net (requires Eigen and OpenCV)" OFF)
message(STATUS "[SST-TOOLS] SST_TOOLS_NEURALNET is ${SST_TOOLS_NEURALNET}")
#--

#-- Checkpoint Serialization build flags (temporary)
if ( ${_SST_MAJOR_VERSION} EQUAL "90" )
  set(TCL_SCHEMA ON)
  set(TCL_SCHEMA_FLAG -DTCL_SCHEMA)
else()
  set(TCL_SCHEMA OFF)
endif()
message(STATUS "[SST-TOOLS] TCL_SCHEMA is ${TCL_SCHEMA}")

if(SST_TOOLS_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

#-- Component source directories
add_subdirectory(sstcomp)

#-- Custom Commands
add_custom_target(uninstall 
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/uninstall.sh
  COMMAND ${CMAKE_COMMAND} -E echo "Deleting ${SST_TOOLS_INSTALL_PATH}"
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${SST_TOOLS_INSTALL_PATH}"
  VERBATIM
)


# EOF
