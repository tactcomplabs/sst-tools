//
// _SST_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

// Header file to include all SST headers, so that compiler warnings can be
// turned off during third-party SST header inclusion.

#ifndef _SST_H
#define _SST_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"

#if defined( __GNUC__ ) && !defined( __clang__ )
#pragma GCC diagnostic ignored "-Wsuggest-final-methods"
#pragma GCC diagnostic ignored "-Wsuggest-final-types"
#endif

// The #include order is important, so we prevent clang-format from reordering
// clang-format off
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include <sst/core/event.h>
// #include <sst/core/interfaces/stdMem.h>
#include <sst/core/output.h>
#include <sst/core/statapi/stataccumulator.h>
#include <sst/core/subcomponent.h>
#include <sst/core/timeConverter.h>
// #include <sst/core/params.h>
// #include <sst/core/rng/marsaglia.h>
// #include <sst/core/shared/sharedArray.h>
// #include <sst/core/unitAlgebra.h>
// clang-format on

#pragma GCC diagnostic pop

#endif
