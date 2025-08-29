#
# sst-tools/sstcomp/sstnn CMake
#
# Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#

set(NNSrcs
  nn_batch_controller.cc
  nn_layer.cc
)

add_library(neuralnet SHARED ${NNSrcs})

#-- Eigen headers
find_path( EIGEN_INCLUDE_DIR Eigen 
  PATHS /usr/local/include/eigen3 /opt/homebrew/Cellar/eigen/3.4.0_1/include/eigen3)
if(EIGEN_INCLUDE_DIR)
  message("[CPPNNFS] EIGEN_INCLUDE_DIR=${EIGEN_INCLUDE_DIR}")
else()
  message(FATAL_ERROR "[CPPNNFS] Could not locate eigen3 include directory")
endif()

target_include_directories(neuralnet PUBLIC 
  ${SST_INSTALL_DIR}/include
  ${EIGEN_INCLUDE_DIR}
)
#-- OpenCV
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs)
target_link_libraries(neuralnet ${OpenCV_LIBS})

install(TARGETS neuralnet DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register NNBatchController neuralnet_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
install(CODE "execute_process(COMMAND sst-register NNLayer neuralnet_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

# EOF
