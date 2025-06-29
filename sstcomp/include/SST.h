//
// _SST_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

// Header file to include all SST headers, so that Rev compiler warnings can be
// turned off during third-party SST header inclusion.

#ifndef _SST_H_
#define _SST_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wsign-conversion"

#if defined( __GNUC__ ) && !defined( __clang__ )
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsuggest-final-methods"
#pragma GCC diagnostic ignored "-Wsuggest-final-types"
#else
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#pragma GCC diagnostic ignored "-Winconsistent-missing-override"
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#endif

// The #include order is important, so we prevent clang-format from reordering
// clang-format off
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include <sst/core/event.h>
#include <sst/core/interfaces/simpleNetwork.h>
#include <sst/core/link.h>
#include <sst/core/output.h>
#include <sst/core/statapi/stataccumulator.h>
#include <sst/core/subcomponent.h>
#include <sst/core/timeConverter.h>
#include <sst/core/model/element_python.h>
#include <sst/core/rng/distrib.h>
#include <sst/core/rng/rng.h>
#include <sst/core/rng/mersenne.h>
#include <sst/core/serialization/serialize.h>
#if 1 // Used for interactive console
#include <sst/core/baseComponent.h>
#include <sst/core/stringize.h>
#include <sst/core/interactiveConsole.h>
#include <sst/core/serialization/objectMapDeferred.h>
//#include <sst/core/watchPoint.h>
#include "watchPoint.h"
#endif
// clang-format on

#pragma GCC diagnostic pop

#endif
