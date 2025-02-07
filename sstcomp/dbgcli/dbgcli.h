//
// _dbgcli_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SSTDEBUG_DBGCLI_H_
#define _SSTDEBUG_DBGCLI_H_

// -- Standard Headers
#include <vector>
#include <queue>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

// -- SST Headers
#include "SST.h"

// -- Debug Probe
#include "probe.h"

using namespace SSTDEBUG::Probe;

namespace SSTDEBUG::DbgCLI {

class DbgCLI_Probe;

// -------------------------------------------------------
// DbgCLIEvent
// -------------------------------------------------------
class DbgCLIEvent : public SST::Event{
public:
  /// DbgCLIEvent : standard constructor
  DbgCLIEvent() : SST::Event() {}

  /// DbgCLIEvent: constructor
  DbgCLIEvent(std::vector<unsigned> d) : SST::Event(), data(d) {}

  /// DbgCLIEvent: destructor
  ~DbgCLIEvent() {}

  /// DbgCLIEvent: retrieve the data
  std::vector<unsigned> const getData() { return data; }

private:
  std::vector<unsigned> data;     ///< DbgCLIEvent: data payload

  /// DbgCLIEvent: serialization method
  void serialize_order(SST::Core::Serialization::serializer& ser) override{
    Event::serialize_order(ser);
    SST_SER(data)
  }

  /// DbgCLIEvent: serialization implementor
  ImplementSerializable(SSTDEBUG::DbgCLI::DbgCLIEvent);

};  // class DbgCLIEvent

// -------------------------------------------------------
// DbgCLI
// -------------------------------------------------------
class DbgCLI : public SST::Component{
public:
  /// DbgCLI: top-level SST component constructor
  DbgCLI( SST::ComponentId_t id, const SST::Params& params );

  /// DbgCLI: top-level SST component destructor
  ~DbgCLI();

  /// DbgCLI: standard SST component 'setup' function
  void setup() override;

  /// DbgCLI: standard SST component 'finish' function
  void finish() override;

  /// DbgCLI: standard SST component init function
  void init( unsigned int phase ) override;

  /// DbgCLI: standard SST component printStatus
  void printStatus(SST::Output& out) override;

  /// DbgCLI: standard SST component clock function
  bool clockTick( SST::Cycle_t currentCycle );

  const int DEFAULT_PROBE_BUFFER_SIZE = 1024;

  // -------------------------------------------------------
  // DbgCLI Component Registration Data
  // -------------------------------------------------------
  /// DbgCLI: Register the component with the SST core
  SST_ELI_REGISTER_COMPONENT( DbgCLI,     // component class
                              "dbgcli",   // component library
                              "DbgCLI",   // component name
                              SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
                              "CHKPNT SST COMPONENT",
                              COMPONENT_CATEGORY_UNCATEGORIZED )

  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",         "Sets the verbosity level of output",      "0" },
    {"numPorts",        "Number of external ports",                "1" },
    {"minData",         "Minimum number of unsigned values",       "1" },
    {"maxData",         "Maximum number of unsigned values",       "2" },
    {"clockDelay",      "Clock delay between sends",               "1" },
    {"clocks",          "Clock cycles to execute",               "1000"},
    {"rngSeed",         "Mersenne RNG Seed",                     "1223"},
    {"clockFreq",       "Clock frequency",                       "1GHz"},
    // component specific probe controls
    {"traceMode",       "0-none, 1-send, 2-recv",                   "0"},
    // TODO Should get rest into base class. Component extends Probe instead of instantiating it
    {"probeMode",       "0-Disabled,1-Checkpoint based, >1-rsv",    "0"},
    {"probeStartCycle", "Use with checkpoint-sim-period",           "0"},
    {"probeEndCycle",   "Cycle probing disable. 0 is no limit",     "0"},
    {"probeBufferSize", "Records in circular trace buffer",      "1024"}, // DEFAULT_PROBE_BUFFER_SIZE
    {"probePostDelay",  "post-trigger delay cycles. -1 to sample until checkpoint", "0"},
    {"probePort",       "Socket assignment for debug port",         "0"},
    {"cliControl",  "0x40 every chkpt, 0x20 chkpts when probe active, 0x10 sync state change,\n"
                    "0x04 every probe sample, 0x02 probe samples from trigger onward, 0x01 probe state change"
                    , "0"},
  )

  // -------------------------------------------------------
  // DbgCLI Component Port Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_PORTS(
    {"port%(num_ports)d",
      "Ports which connect to endpoints.",
      {"dbgcli.DbgCLIEvent", ""}
    }
  )

  // -------------------------------------------------------
  // DbgCLI SubComponent Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()

  // -------------------------------------------------------
  // DbgCLI Component Statistics Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_STATISTICS()

  // -------------------------------------------------------
  // DbgCLI Component Checkpoint Methods
  // -------------------------------------------------------
  /// DbgCLI: serialization constructor
  DbgCLI() : SST::Component() {}

  /// DbgCLI: serialization
  void serialize_order(SST::Core::Serialization::serializer& ser) override;

  /// DbgCLI: Update debug control state object on checkpoint
  void handle_chkpt_probe_action();
  
  /// DbgCLI: serialization implementations
  ImplementSerializable(SSTDEBUG::DbgCLI::DbgCLI)

private:
  // -- internal handlers
  SST::Output    output;                          ///< SST output handler
  SST::TimeConverter* timeConverter;                   ///< SST time conversion handler
  SST::Clock::HandlerBase* clockHandler;          ///< Clock Handler

  // -- parameters
  unsigned numPorts;                              ///< number of ports to configure
  uint64_t minData;                               ///< minimum number of data elements
  uint64_t maxData;                               ///< maxmium number of data elements
  uint64_t clockDelay;                            ///< clock delay between sends
  uint64_t clocks;                                ///< number of clocks to execute
  uint64_t curCycle;                              ///< current cycle delay
  // -- probing
  unsigned traceMode;                             ///< 0-none, 1-send, 2-recv, 3-both
  unsigned cliType;                               ///< 0-serializer-entry, 1-initiateInteractive

  // -- Component probe state object
 std::unique_ptr<DbgCLI_Probe> probe_;

  // -- rng objects
  SST::RNG::Random* mersenne;                     ///< mersenne twister object

  std::vector<SST::Link *> linkHandlers;          ///< LinkHandler objects

  // -- private methods
  /// event handler
  void handleEvent(SST::Event *ev);

  /// sends data to adjacent links
  void sendData();

};  // class DbgCLI

// -------------------------------------------------------
// Debug Control State 
// -------------------------------------------------------
class DbgCLI_Probe : public ProbeControl {

public:
  DbgCLI_Probe(SST::Component * comp, SST::Output * out, 
              int mode, SST::SimTime_t startCycle, SST::SimTime_t endCycle, int bufferSize, 
              int port, int postDelay, uint64_t cliControl);
  // User custom sampling functions
  void capture_event_atts(uint64_t cycle, uint64_t sz, DbgCLIEvent *ev);
  // Custom data type for samples
  struct event_atts_t {
    uint64_t cycle_ = 0;
    uint64_t sz_ = 0;
    uint64_t deliveryTime_ = 0;
    int priority_ = 0;
    uint64_t orderTag_ = 0; 
    uint64_t queueOrder_ = 0;
    event_atts_t() {};
    event_atts_t(uint64_t c, uint64_t sz, DbgCLIEvent *ev) : cycle_(c), sz_(sz)
    { 
      deliveryTime_ = ev->getDeliveryTime();
      priority_ = ev->getPriority();
      orderTag_ = ev->getOrderTag();
      queueOrder_ = ev->getQueueOrder();
    };
    friend std::ostream & operator<<(std::ostream &os, const event_atts_t& e) {
      os << std::dec << "cycle=" << e.cycle_ 
        << " sz=" << e.sz_ 
        << " deliveryTime=" << e.deliveryTime_ 
        << " priority=" << e.priority_
        << " orderTag=" << e.orderTag_ 
        << " queueOrder=" << e.queueOrder_;
      return os;
    }
  };
  // trace buffer
  std::shared_ptr<ProbeBuffer<event_atts_t>> probeBuffer;

}; // class DbgCLI_Probe

}  // namespace SSTDEBUG::DbgCLI

#endif  // _SSTDEBUG_DBGCLI_H_

// EOF
