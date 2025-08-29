#ifndef __OPENCV_H__
#define __OPENCV_H__

//clang-format off
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#if !defined( __GNUC__) || defined( __clang__ )
#pragma GCC diagnostic ignored "-Wimplicit-float-conversion"
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#endif
#include "opencv2/opencv.hpp"
#include "opencv2/core/eigen.hpp"
#pragma GCC diagnostic pop
//clang-format on

#endif //__OPENCV_H__
