#ifndef __EIGEN__
#define __EIGEN__

// clang-format off
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#if !defined( __GNUC__) || defined( __clang__ )
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
#include <Eigen/Dense>
#include <unsupported/Eigen/MatrixFunctions>
#pragma GCC diagnostic pop
// clang-format on

#endif //__EIGEN__