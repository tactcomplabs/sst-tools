//
// _igridnode_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_IGRIDNODE_H_
#define _SST_IGRIDNODE_H_

// -- Standard Headers
#include <map>
#include <vector>
#include <queue>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

// -- SST Headers
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

namespace SST::IGridNode{

// -------------------------------------------------------
// IGridNodeEvent
// -------------------------------------------------------
class IGridNodeEvent : public SST::Event{
public:
  /// IGridNodeEvent : standard constructor
  IGridNodeEvent() : SST::Event() {}

  /// IGridNodeEvent: constructor
  IGridNodeEvent(std::vector<unsigned> d) : SST::Event(), data(d) {}

  /// IGridNodeEvent: destructor
  ~IGridNodeEvent() {}

  /// IGridNodeEvent: retrieve the data
  std::vector<unsigned> const getData() { return data; }

private:
  std::vector<unsigned> data;     ///< IGridNodeEvent: data payload

  /// IGridNodeEvent: serialization method
  void serialize_order(SST::Core::Serialization::serializer& ser) override{
    Event::serialize_order(ser);
    SST_SER(data)
  }

  /// IGridNodeEvent: serialization implementor
  ImplementSerializable(SST::IGridNode::IGridNodeEvent);

};  // class IGridNodeEvent

// -------------------------------------------------------
// IGridNode
// -------------------------------------------------------
class IGridNode : public SST::Component{
public:
  /// IGridNode: top-level SST component constructor
  IGridNode( SST::ComponentId_t id, const SST::Params& params );

  /// IGridNode: top-level SST component destructor
  ~IGridNode();

  /// IGridNode: standard SST component 'setup' function
  void setup() override;

  /// IGridNode: standard SST component 'finish' function
  void finish() override;

  /// IGridNode: standard SST component init function
  void init( unsigned int phase ) override;

  /// IGridNode: standard SST component printStatus
  void printStatus(Output& out) override;

  /// IGridNode: standard SST component clock function
  bool clockTick( SST::Cycle_t currentCycle );

  // -------------------------------------------------------
  // IGridNode Component Registration Data
  // -------------------------------------------------------
  /// IGridNode: Register the component with the SST core
  SST_ELI_REGISTER_COMPONENT( IGridNode,     // component class
                              "igrid",       // component library
                              "IGridNode",   // component name
                              SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
                              "INTERACTIVE GRIDNODE SST COMPONENT",
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
    {"breakEnable",     "Enables break to interactive console from code", "0"},

  )

  // -------------------------------------------------------
  // IGridNode Component Port Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_PORTS(
    {"port%(num_ports)d",
      "Ports which connect to endpoints.",
      {"chkpnt.IGridNodeEvent", ""}
    }
  )

  // -------------------------------------------------------
  // IGridNode SubComponent Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()

  // -------------------------------------------------------
  // IGridNode Component Statistics Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_STATISTICS()

  // -------------------------------------------------------
  // IGridNode Component Checkpoint Methods
  // -------------------------------------------------------
  /// IGridNode: serialization constructor
  IGridNode() : SST::Component() {}

  /// IGridNode: serialization
  void serialize_order(SST::Core::Serialization::serializer& ser) override;

  /// IGridNode: serialization implementations
  ImplementSerializable(SST::IGridNode::IGridNode)

private:
  // -- internal handlers
  SST::Output    output;                          ///< SST output handler
  TimeConverter* timeConverter;                   ///< SST time conversion handler
  SST::Clock::HandlerBase* clockHandler;          ///< Clock Handler

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
  std::map< std::string, SST::RNG::Random* > rng; ///< per port mersenne twister objects
  SST::RNG::Random* localRNG = 0;                 ///< component local random number generator

  // Interactive Console Debug Example
  // Break into interactive console for debug
  // Can be enabled with cmd line parameter or modified from interactive console
  volatile int breakEnable;

  // -- private methods
  /// event handler
  void handleEvent(SST::Event *ev);
  /// sends data to adjacent links
  void sendData();
  /// calculates the port number for the receiver
  unsigned neighbor(unsigned n);

};  // class IGridNode
}   // namespace SST::IGridNode

#endif  // _SST_IGRIDNODE_H_

// EOF
