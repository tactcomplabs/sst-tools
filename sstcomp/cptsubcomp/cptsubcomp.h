//
// _cptsubcomp_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_CPTSUBCOMP_H_
#define _SST_CPTSUBCOMP_H_

/*
 * Modeled after src/sst/elements/simpleElementExample/basicSubComponent_subcomponent.h
 * The intent is to provides subcomponents that handle a variety of complex datatypes
 * in order to understand the nuances of generating checkpoints and their effects on
 * checkpoint size.
 * 
 * cptSubCompAPI - inherits from SST::SubComponent. Defines the API for checkpoint subcomponents.
 * cptVecInt - std:vector<int>  baseline scheme should invoke ser& once for entire vector
 * cptVecPairIntInt - std::vector<std::pair<int,int>> - need to invoke ser& for .first and .second elements
 */

// clang-format off
// -- Standard Headers
// #include <map>
// #include <vector>
// #include <queue>
// #include <random>
// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>

// -- SST Headers
#include "SST.h"
#include "sst/core/subcomponent.h"
// clang-format on

namespace SST::CPTSubComp{

// -------------------------------------------------------
// CPTSubComp
// -------------------------------------------------------

class CPTSubCompAPI : public SST::SubComponent
{
public:
  // Register subcomponent API
  SST_ELI_REGISTER_SUBCOMPONENT_API(SST::CPTSubComp::CPTSubCompAPI)

  // Constructor/Destructor
  CPTSubCompAPI(ComponentId_t id, Params& params) : SubComponent(id) { }
  virtual ~CPTSubCompAPI() {}

  // API
  // Subcomponent self-checking returns 0 if no errors
  virtual int check() = 0;
  // Update the subcomponent internal state
  virtual void update() = 0;

  // Serialization
  CPTSubCompAPI();
  ImplementVirtualSerializable(SST::CPTSubComp::CPTSubCompAPI);
};

// subcomponent implementation for std::vector<int>
class CPTSubCompVecInt : public CPTSubCompAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
    CPTSubCompVecInt,     // Class name
    "CPTSubComp",         // Library name, the 'lib' in SST's lib.name format
    "CPTSubCompVecInt",   // Name used to refer to this subcomponent, the 'name' in SST's lib.name format
    SST_ELI_ELEMENT_VERSION(1,0,0), // A version number
    "SubComponent for checkpoint type std::vector<int>", // Description
    SST::CPTSubComp::CPTSubCompAPI // Fully qualified name of the API this subcomponent implements
  )
  SST_ELI_DOCUMENT_PARAMS( 
    { "max", "Maximum number of test elements", "1024" },
    { "seed","Initial seed for data generation", "1223"}
  )

  CPTSubCompVecInt(ComponentId_t id, Params& params);
  ~CPTSubCompVecInt();

  // API members
  int check() override;
  void update() override;

  // Serialization
  void serialize_order(SST::Core::Serialization::serializer& ser) override;

private:
  size_t max;
  unsigned seed;
  std::vector<int32_t> tut;
  SST::RNG::Random* rng;

}; //class CPTSubCompAPI

} // namespace SST::CPTSubComp

#endif  // _SST_CPTSUBCOMP_H_

// EOF
