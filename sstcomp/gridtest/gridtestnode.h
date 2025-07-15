//
// _gridtestnode_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_GRIDTESTNODE_H_
#define _SST_GRIDTESTNODE_H_

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

// -- SubComponent API
#include "cptsubcomp.h"

// clang-format on

namespace SST::GridTestNode{

// -------------------------------------------------------
// GridTestNodeEvent
// -------------------------------------------------------
class GridTestNodeEvent : public SST::Event{
public:
  /// GridTestNodeEvent : standard constructor
  GridTestNodeEvent() : SST::Event() {}

  /// GridTestNodeEvent: constructor
  GridTestNodeEvent(std::vector<unsigned> d) : SST::Event(), data(d) {}

  /// GridTestNodeEvent: destructor
  virtual ~GridTestNodeEvent() {}

  /// GridTestNodeEvent: retrieve the data
  std::vector<unsigned> const getData() { return data; }

private:
  std::vector<unsigned> data;     ///< GridTestNodeEvent: data payload

  /// GridTestNodeEvent: serialization method
  void serialize_order(SST::Core::Serialization::serializer& ser) override{
    Event::serialize_order(ser);
    SST_SER(data);
  }

  /// GridTestNodeEvent: serialization implementor
  ImplementSerializable(SST::GridTestNode::GridTestNodeEvent);

};  // class GridTestNodeEvent

// -------------------------------------------------------
// GridTestNode
// -------------------------------------------------------
class GridTestNode : public SST::Component{
public:
  /// GridTestNode: top-level SST component constructor
  GridTestNode( SST::ComponentId_t id, const SST::Params& params );

  /// GridTestNode: top-level SST component destructor
  ~GridTestNode();

  /// GridTestNode: standard SST component 'setup' function
  void setup() override;

  /// GridTestNode: standard SST component 'finish' function
  void finish() override;

  /// GridTestNode: standard SST component init function
  void init( unsigned int phase ) override;

  /// GridTestNode: standard SST component printStatus
  void printStatus(Output& out) override;

  /// GridTestNode: standard SST component clock function
  bool clockTick( SST::Cycle_t currentCycle );

  // -------------------------------------------------------
  // GridTestNode Component Registration Data
  // -------------------------------------------------------
  /// GridTestNode: Register the component with the SST core
  SST_ELI_REGISTER_COMPONENT( GridTestNode,     // component class
                              "gridtest",       // component library
                              "GridTestNode",   // component name
                              SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
                              "GRIDTESTNODE SST COMPONENT",
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
  // GridTestNode Component Port Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_PORTS(
    {"port%(num_ports)d",
      "Ports which connect to endpoints.",
      {"chkpnt.GridTestNodeEvent", ""}
    }
  )

  // -------------------------------------------------------
  // GridTestNode SubComponent Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    { "CPTSubComp",
      "Expansion slot for more checkpoint type checking",
      "SST::CPTSubComp::CPTSubCompAPI"
    }
  )

  // -------------------------------------------------------
  // GridTestNode Component Statistics Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_STATISTICS()

  // -------------------------------------------------------
  // GridTestNode Component Checkpoint Methods
  // -------------------------------------------------------
  /// GridTestNode: serialization constructor
  GridTestNode() : SST::Component() {}

  /// GridTestNode: serialization
  void serialize_order(SST::Core::Serialization::serializer& ser) override;

  /// GridTestNode: serialization implementations
  ImplementSerializable(SST::GridTestNode::GridTestNode)

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

};  // class GridTestNode
}   // namespace SST::GridTestNode

#endif  // _SST_GRIDTESTNODE_H_

// EOF
