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

#define KG_SERIALIZE

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

#ifdef KG_SERIALIZE
  // Serialization
  CPTSubCompAPI() {};
  ImplementVirtualSerializable(SST::CPTSubComp::CPTSubCompAPI);
#endif
};

// subcomponent implementation for std::vector<int>
class CPTSubCompVecInt final : public CPTSubCompAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
    CPTSubCompVecInt,     // Class name
    "grid",               // Library name, the 'lib' in SST's lib.name format
    "CPTSubCompVecInt",   // Name used to refer to this subcomponent, the 'name' in SST's lib.name format
    SST_ELI_ELEMENT_VERSION(1,0,0), // A version number
    "SubComponent for checkpoint type std::vector<int>", // Description
    SST::CPTSubComp::CPTSubCompAPI // Fully qualified name of the API this subcomponent implements
  )
  SST_ELI_DOCUMENT_PARAMS( 
    {"verbose", "Sets the verbosity level of output", "0" },
    { "max", "Maximum number of test elements", "100" },
    { "seed","Initial seed for data generation", "1223"}
  )

  CPTSubCompVecInt(ComponentId_t id, Params& params);
  virtual ~CPTSubCompVecInt();

  // subcomponent overrides
  virtual void setup() override;
  virtual void finish() override;

  // API members
  int check() override;
  void update() override;

#ifdef KG_SERIALIZE
  // Serialization
  CPTSubCompVecInt() : CPTSubCompAPI() {};
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  ImplementSerializable(SST::CPTSubComp::CPTSubCompVecInt);
#endif

private:
  uint64_t  subcompBegin;
  SST::Output    output;        ///< SST output handler
  unsigned clocks;
  size_t max;
  unsigned seed;
  std::vector<int32_t> tut;     // type under test
  std::vector<int32_t> tutini;  // initial values for type under test
  SST::RNG::Random* rng;
  uint64_t subcompEnd;

}; //class CPTSubCompAPI

// test struct
struct struct_t : public SST::Core::Serialization::serializable {
  uint8_t u8;
  uint16_t u16;
  uint32_t u32;
  uint64_t u64;
  std::string toString() {
    std::stringstream s; 
    s << std::hex << "0x" << u8 << " 0x" << u16 << " 0x" << u32 << " 0x " << u64;
    return s.str();
  };
  struct_t() {};
  void serialize_order(SST::Core::Serialization::serializer& ser) override {
    SST_SER(u8);
    SST_SER(u16);
    SST_SER(u32);
    SST_SER(u64);
  };
  ImplementSerializable(SST::CPTSubComp::struct_t) ;
};

// subcomponent implementation for std::pair<struct,struct>
class CPTSubCompPairOfStructs final : public CPTSubCompAPI {

  public:
    SST_ELI_REGISTER_SUBCOMPONENT(
      CPTSubCompPairOfStructs,     // Class name
      "grid",               // Library name, the 'lib' in SST's lib.name format
      "CPTSubCompPairOfStructs",   // Name used to refer to this subcomponent, the 'name' in SST's lib.name format
      SST_ELI_ELEMENT_VERSION(1,0,0), // A version number
      "SubComponent for checkpoint type std::vector<int>", // Description
      SST::CPTSubComp::CPTSubCompAPI // Fully qualified name of the API this subcomponent implements
    )
    SST_ELI_DOCUMENT_PARAMS( 
      {"verbose", "Sets the verbosity level of output", "0" },
      { "max", "Maximum number of test elements", "100" },
      { "seed","Initial seed for data generation", "1223"}
    )
  
    CPTSubCompPairOfStructs(ComponentId_t id, Params& params);
    virtual ~CPTSubCompPairOfStructs();
  
    // subcomponent overrides
    virtual void setup() override;
    virtual void finish() override;
  
    // API members
    int check() override;
    void update() override;
  
  #ifdef KG_SERIALIZE
    // Serialization
    CPTSubCompPairOfStructs() : CPTSubCompAPI() {};
    void serialize_order(SST::Core::Serialization::serializer& ser) override;
    ImplementSerializable(SST::CPTSubComp::CPTSubCompPairOfStructs);
  #endif
  
  private:
    uint64_t  subcompBegin;
    SST::Output    output;        ///< SST output handler
    unsigned clocks;
    size_t max;
    unsigned seed;
    std::pair<struct_t, struct_t> tut;     // type under test
    std::pair<struct_t, struct_t> tutini;  // initial values for type under test
    SST::RNG::Random* rng;
    uint64_t subcompEnd;
  
  }; //class CPTSubCompAPI
  
} // namespace SST::CPTSubComp

#endif  // _SST_CPTSUBCOMP_H_

// EOF
