//
// _gridnode_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_GRIDNODE_H_
#define _SST_GRIDNODE_H_

// clang-format off
// -- Standard Headers
#include <map>
#include <vector>
#include <queue>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// -- SST Headers
#include "SST.h"
#include <sst/core/rng/distrib.h>
#include <sst/core/rng/rng.h>
#include <sst/core/rng/mersenne.h>

// -- SubComponent API
#include "cptsubcomp.h"

// clang-format on

namespace SST::GridNode{

// -------------------------------------------------------
// GridNodeEvent
// -------------------------------------------------------
class GridNodeEvent : public SST::Event{
public:
  /// GridNodeEvent : standard constructor
  GridNodeEvent() : SST::Event() {}

  /// GridNodeEvent: constructor
  GridNodeEvent(std::vector<unsigned> d) : SST::Event(), data(d) {}

  /// GridNodeEvent: destructor
  virtual ~GridNodeEvent() {}

  /// GridNodeEvent: retrieve the data
  std::vector<unsigned> const getData() { return data; }

private:
  std::vector<unsigned> data;     ///< GridNodeEvent: data payload

  /// GridNodeEvent: serialization method
  void serialize_order(SST::Core::Serialization::serializer& ser) override{
    Event::serialize_order(ser);
    SST_SER(data)
  }

  /// GridNodeEvent: serialization implementor
  ImplementSerializable(SST::GridNode::GridNodeEvent);

};  // class GridNodeEvent

// -------------------------------------------------------
// GridNode
// -------------------------------------------------------
class GridNode : public SST::Component{
public:
  /// GridNode: top-level SST component constructor
  GridNode( SST::ComponentId_t id, const SST::Params& params );

  /// GridNode: top-level SST component destructor
  ~GridNode();

  /// GridNode: standard SST component 'setup' function
  void setup() override;

  /// GridNode: standard SST component 'finish' function
  void finish() override;

  /// GridNode: standard SST component init function
  void init( unsigned int phase ) override;

  /// GridNode: standard SST component printStatus
  void printStatus(Output& out) override;

  /// GridNode: standard SST component clock function
  bool clockTick( SST::Cycle_t currentCycle );

  // -------------------------------------------------------
  // GridNode Component Registration Data
  // -------------------------------------------------------
  /// GridNode: Register the component with the SST core
  SST_ELI_REGISTER_COMPONENT( GridNode,     // component class
                              "grid",       // component library
                              "GridNode",   // component name
                              SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
                              "GRIDNODE SST COMPONENT",
                              COMPONENT_CATEGORY_UNCATEGORIZED )

  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",         "Sets the verbosity level of output",   "0" },
    {"numBytes",        "Internal state size (4 byte increments)", "16384"},
    {"numPorts",        "Number of external ports",             "8" },
    {"minData",         "Minimum number of unsigned values",    "10" },
    {"maxData",         "Maximum number of unsigned values",    "8192" },
    {"minDelay",        "Minumum clock delay between sends",    "50" },
    {"maxDelay",        "Maximum clock delay between sends",    "100" },
    {"clocks",          "Clock cycles to execute",              "1000"},
    {"clockFreq",       "Clock frequency",                      "1GHz"},
    {"rngSeed",         "Mersenne RNG Seed",                    "1223"},
    {"demoBug",         "Induce bug for debug demo",               "0"},

  )

  // -------------------------------------------------------
  // GridNode Component Port Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_PORTS(
    {"port%(num_ports)d",
      "Ports which connect to endpoints.",
      {"chkpnt.GridNodeEvent", ""}
    }
  )

  // -------------------------------------------------------
  // GridNode SubComponent Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    { "CPTSubComp",
      "Expansion slot for more checkpoint type checking",
      "SST::CPTSubComp::CPTSubCompAPI"
    }
  )

  // -------------------------------------------------------
  // GridNode Component Statistics Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_STATISTICS()

  // -------------------------------------------------------
  // GridNode Component Checkpoint Methods
  // -------------------------------------------------------
  /// GridNode: serialization constructor
  GridNode() : SST::Component() {}

  /// GridNode: serialization
  void serialize_order(SST::Core::Serialization::serializer& ser) override;

  /// GridNode: serialization implementations
  ImplementSerializable(SST::GridNode::GridNode)

private:
  // Start of serialized members
  uint64_t cptBegin;                              ///< Mark beginning of checkpoint sequence
  // -- SST handlers
  SST::Output    output;                          ///< SST output handler
  TimeConverter* timeConverter;                   ///< SST time conversion handler
  SST::Clock::HandlerBase* clockHandler;          ///< Clock Handler
  CPTSubComp::CPTSubCompAPI* CPTSubComp=nullptr;  ///< SubComponent for additional testing
  // -- parameters
  uint64_t numBytes;                              ///< number of bytes of internal state
  unsigned numPorts;                              ///< number of ports to configure
  uint64_t minData;                               ///< minimum number of data elements
  uint64_t maxData;                               ///< maxmium number of data elements
  uint64_t minDelay;                              ///< minimum clock delay between sends
  uint64_t maxDelay;                              ///< maximum clock delay between sends
  uint64_t clocks;                                ///< number of clocks to execute
  unsigned rngSeed;                               ///< base seed for random number generator
  uint64_t curCycle;                              ///< current cycle delay
  // Bug injection
  unsigned demoBug;                               ///< induce bug for debug demonstration
  uint64_t dataMask;                              ///< send only 16 bits of data
  uint64_t dataMax;                               ///< change to inject illegal values
  // -- internal state
  uint64_t clkDelay = 0;                          ///< current clock delay
  std::vector<std::string> portname;              ///< port 0 to numPorts names
  std::vector<SST::Link *> linkHandlers;          ///< LinkHandler objects
  std::vector<unsigned> state;                    ///< internal data structure
  uint64_t initialCheck = 0;                      ///< starting state signature
  std::map< std::string, SST::RNG::Random* > rng; ///< per port mersenne twister objects
  RNG::Random* localRNG = 0;                      ///< component local random number generator                                     
  // -- End of checkpointed members
  uint64_t cptEnd;                                ///< Mark ending of checkpoint sequence             

  // -- private methods
  /// event handler
  void handleEvent(SST::Event *ev);
  /// sends data to adjacent links
  void sendData();
  /// calculates the port number for the receiver
  unsigned neighbor(unsigned n);

};  // class GridNode
}   // namespace SST::GridNode

#endif  // _SST_GRIDNODE_H_

// EOF
