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
// test struct
// -------------------------------------------------------
struct struct_t : public SST::Core::Serialization::serializable {
  uint8_t u8;
  uint16_t u16;
  uint32_t u32;
  uint64_t u64;
  struct_t(uint64_t n) {
    u8 = uint8_t(n);
    u16 = uint16_t(n);
    u32 = uint32_t(n);
    u64 = uint64_t(n);
  }
  // helpers
  std::string toString() {
    std::stringstream s; 
    s << std::hex << "0x" << (uint16_t)u8 << " 0x" << u16 << " 0x" << u32 << " 0x" << u64;
    return s.str();
  };
  struct_t operator++(int) {
    struct_t old = *this;
    u8++; u16++; u32++; u64++;
    return old;
  }
  struct_t operator+=(const struct_t& rhs) {
    u8 += rhs.u8;
    u16 += rhs.u16;
    u32 += rhs.u32;
    u64 += rhs.u64;
    return *this;
  }
  friend struct_t operator+(struct_t lhs, const uint32_t& rhs) {
    lhs.u8 += uint8_t(rhs);
    lhs.u16 += uint16_t(rhs);
    lhs.u32 += uint32_t(rhs);
    lhs.u64 += uint64_t(rhs);
    return lhs;
  }
  inline bool operator==(const struct_t& rhs) {
    bool e8 = (u8==rhs.u8);
    bool e16 = (u16==rhs.u16);
    bool e32 = (u32==rhs.u32);
    bool e64 = (u64==rhs.u64);
    return e8 && e16 && e32 && e64;
  }
  inline bool operator!=(const struct_t& rhs) {
    return !(*this == rhs);
  }
  // serialization
  struct_t() {};
  void serialize_order(SST::Core::Serialization::serializer& ser) override {
    SST_SER(u8);
    SST_SER(u16);
    SST_SER(u32);
    SST_SER(u64);
  };
  // This has public and private sections. Put last!
  ImplementSerializable(SST::CPTSubComp::struct_t) ;
}; // struct struct_t


// -------------------------------------------------------
// CPTSubCompAPI
// -------------------------------------------------------

class CPTSubCompAPI : public SST::SubComponent
{
public:
  // Register subcomponent API
  SST_ELI_REGISTER_SUBCOMPONENT_API(SST::CPTSubComp::CPTSubCompAPI)

  // Constructor/Destructor
  CPTSubCompAPI(ComponentId_t id, Params& params);
  virtual ~CPTSubCompAPI() {}

  // API
  // Subcomponent self-checking returns 0 if no errors
  virtual int check() = 0;
  // Update the subcomponent internal state
  virtual void update() = 0;

  // Serialization
  CPTSubCompAPI() {};
  ImplementVirtualSerializable(SST::CPTSubComp::CPTSubCompAPI);
}; // class CTPSubCompAPI

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

  // Serialization
  CPTSubCompVecInt() : CPTSubCompAPI() {};
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  ImplementSerializable(SST::CPTSubComp::CPTSubCompVecInt);

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

}; //class CPTSubCompVecInt
 
// subcomponent implementation for std::vector<struct>
class CPTSubCompVecStruct final : public CPTSubCompAPI {

  public:
    SST_ELI_REGISTER_SUBCOMPONENT(
      CPTSubCompVecStruct,            // Class name
      "grid",                         // Library name, the 'lib' in SST's lib.name format
      "CPTSubCompVecStruct",          // Name used to refer to this subcomponent, the 'name' in SST's lib.name format
      SST_ELI_ELEMENT_VERSION(1,0,0), // A version number
      "SubComponent for checkpoint type std::vector<struct_t>", // Description
      SST::CPTSubComp::CPTSubCompAPI  // Fully qualified name of the API this subcomponent implements
    )
    SST_ELI_DOCUMENT_PARAMS( 
      {"verbose", "Sets the verbosity level of output", "0" },
      { "max", "Maximum number of test elements", "100" },
      { "seed","Initial seed for data generation", "1223"}
    )
  
    CPTSubCompVecStruct(ComponentId_t id, Params& params);
    virtual ~CPTSubCompVecStruct();
  
    // subcomponent overrides
    virtual void setup() override;
    virtual void finish() override;
  
    // API members
    int check() override;
    void update() override;
  
    // Serialization
    CPTSubCompVecStruct() : CPTSubCompAPI() {};
    void serialize_order(SST::Core::Serialization::serializer& ser) override;
    ImplementSerializable(SST::CPTSubComp::CPTSubCompVecStruct);
  
  private:
    uint64_t  subcompBegin;
    SST::Output    output;        ///< SST output handler
    unsigned clocks;
    size_t max;
    unsigned seed;
    std::vector<struct_t> tut;     // type under test
    std::vector<struct_t> tutini;  // initial values for type under test
    SST::RNG::Random* rng;
    uint64_t subcompEnd;
  
  }; //class CPTSubCompVecStruct

// subcomponent implementation for std::vector<std::pair<struct,struct>>
class CPTSubCompVecPairOfStructs final : public CPTSubCompAPI {

  public:
    SST_ELI_REGISTER_SUBCOMPONENT(
      CPTSubCompVecPairOfStructs,     // Class name
      "grid",               // Library name, the 'lib' in SST's lib.name format
      "CPTSubCompVecPairOfStructs",   // Name used to refer to this subcomponent, the 'name' in SST's lib.name format
      SST_ELI_ELEMENT_VERSION(1,0,0), // A version number
      "SubComponent for checkpoint type std::vector<std::pair<struct_t, struct_t>>", // Description
      SST::CPTSubComp::CPTSubCompAPI // Fully qualified name of the API this subcomponent implements
    )
    SST_ELI_DOCUMENT_PARAMS( 
      {"verbose", "Sets the verbosity level of output", "0" },
      { "max", "Maximum number of test elements", "100" },
      { "seed","Initial seed for data generation", "1223"}
    )
  
    CPTSubCompVecPairOfStructs(ComponentId_t id, Params& params);
    virtual ~CPTSubCompVecPairOfStructs();
  
    // subcomponent overrides
    virtual void setup() override;
    virtual void finish() override;
  
    // API members
    int check() override;
    void update() override;
  
    // Serialization
    CPTSubCompVecPairOfStructs() : CPTSubCompAPI() {};
    void serialize_order(SST::Core::Serialization::serializer& ser) override;
    ImplementSerializable(SST::CPTSubComp::CPTSubCompVecPairOfStructs);
  
  private:
    uint64_t  subcompBegin;
    SST::Output    output;        ///< SST output handler
    unsigned clocks;
    size_t max;
    unsigned seed;
    std::vector<std::pair<struct_t, struct_t>> tut;     // type under test
    std::vector<std::pair<struct_t, struct_t>> tutini;  // initial values for type under test
    SST::RNG::Random* rng;
    uint64_t subcompEnd;
  
  }; //class CPTSubCompPairOfStructs

// subcomponent implementation for std:pair<unsigned,unsigned>
class CPTSubCompPair final : public CPTSubCompAPI {
  public:
    SST_ELI_REGISTER_SUBCOMPONENT(
      CPTSubCompPair,     // Class name
      "grid",             // Library name, the 'lib' in SST's lib.name format
      "CPTSubCompPair",   // Name used to refer to this subcomponent, the 'name' in SST's lib.name format
      SST_ELI_ELEMENT_VERSION(1,0,0), // A version number
      "SubComponent for simple std::pair<unsigned,unsigned>", // Description
      SST::CPTSubComp::CPTSubCompAPI // Fully qualified name of the API this subcomponent implements
    )
    SST_ELI_DOCUMENT_PARAMS( 
      {"verbose", "Sets the verbosity level of output", "0" },
      { "seed","Initial seed for data generation", "1223"}
    )
  
    CPTSubCompPair(ComponentId_t id, Params& params);
    virtual ~CPTSubCompPair();
  
    // subcomponent overrides
    virtual void setup() override;
    virtual void finish() override;
  
    // API members
    int check() override;
    void update() override;
  
    // Serialization
    CPTSubCompPair() : CPTSubCompAPI() {};
    void serialize_order(SST::Core::Serialization::serializer& ser) override;
    ImplementSerializable(SST::CPTSubComp::CPTSubCompPair);
  
  private:
    uint64_t  subcompBegin;
    SST::Output    output;        ///< SST output handler
    unsigned clocks;
    //size_t max;
    unsigned seed;
    std::pair<unsigned,unsigned> tut;      // type under test
    std::pair<unsigned,unsigned> tutini;   // type under test
    SST::RNG::Random* rng;
    uint64_t subcompEnd;
  
  }; //class CPTSubCompVecInt
 
// subcomponent implementation for std::pair<struct,struct>
class CPTSubCompPairOfStructs final : public CPTSubCompAPI {

  public:
    SST_ELI_REGISTER_SUBCOMPONENT(
      CPTSubCompPairOfStructs,     // Class name
      "grid",               // Library name, the 'lib' in SST's lib.name format
      "CPTSubCompPairOfStructs",   // Name used to refer to this subcomponent, the 'name' in SST's lib.name format
      SST_ELI_ELEMENT_VERSION(1,0,0), // A version number
      "SubComponent for checkpoint type std::pair<struct_t, struct_t>", // Description
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
  
    // Serialization
    CPTSubCompPairOfStructs() : CPTSubCompAPI() {};
    void serialize_order(SST::Core::Serialization::serializer& ser) override;
    ImplementSerializable(SST::CPTSubComp::CPTSubCompPairOfStructs);
  
  private:
    uint64_t  subcompBegin;
    SST::Output    output;        ///< SST output handler
    unsigned clocks;
    unsigned seed;
    std::pair<struct_t, struct_t> tut;     // type under test
    std::pair<struct_t, struct_t> tutini;  // initial values for type under test
    SST::RNG::Random* rng;
    uint64_t subcompEnd;
  
  }; //class CPTSubCompPairOfStructs

// subcomponent implementation for std::vector<std::pair<unsigned,unsigned>>
class CPTSubCompVecPair final : public CPTSubCompAPI {

  public:
    SST_ELI_REGISTER_SUBCOMPONENT(
      CPTSubCompVecPair,     // Class name
      "grid",               // Library name, the 'lib' in SST's lib.name format
      "CPTSubCompVecPair",   // Name used to refer to this subcomponent, the 'name' in SST's lib.name format
      SST_ELI_ELEMENT_VERSION(1,0,0), // A version number
      "SubComponent for checkpoint type std::vector<std::pair<struct_t, struct_t>>", // Description
      SST::CPTSubComp::CPTSubCompAPI // Fully qualified name of the API this subcomponent implements
    )
    SST_ELI_DOCUMENT_PARAMS( 
      {"verbose", "Sets the verbosity level of output", "0" },
      { "max", "Maximum number of test elements", "100" },
      { "seed","Initial seed for data generation", "1223"}
    )
  
    CPTSubCompVecPair(ComponentId_t id, Params& params);
    virtual ~CPTSubCompVecPair();
  
    // subcomponent overrides
    virtual void setup() override;
    virtual void finish() override;
  
    // API members
    int check() override;
    void update() override;
  
    // Serialization
    CPTSubCompVecPair() : CPTSubCompAPI() {};
    void serialize_order(SST::Core::Serialization::serializer& ser) override;
    ImplementSerializable(SST::CPTSubComp::CPTSubCompVecPair);
  
  private:
    uint64_t  subcompBegin;
    SST::Output    output;        ///< SST output handler
    unsigned clocks;
    size_t max;
    unsigned seed;
    std::vector<std::pair<unsigned, unsigned>> tut;     // type under test
    std::vector<std::pair<unsigned, unsigned>> tutini;  // initial values for type under test
    SST::RNG::Random* rng;
    uint64_t subcompEnd;
  
  }; //class CPTSubCompPair

} // namespace SST::CPTSubComp

#endif  // _SST_CPTSUBCOMP_H_

// EOF
