//
// _dbgsst15_h_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SSTDEBUG_DBGSST15_H_
#define _SSTDEBUG_DBGSST15_H_

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

#define PROBE 1
#define SOCKET 0
#define TESTSER 0

#if PROBE
using namespace SSTDEBUG::Probe;
#endif
namespace SSTDEBUG::DbgSST15 {
#if PROBE
class DbgSST15_Probe;
#endif

#if TESTSER // Testing out serialization of unique and shared
class PC { // represent ProbeControl
public:
  PC(int s) :
    start(s)
  { }
  //virtual ~PC();
  int start;

  // Support for  serialization
  void serialize_order(SST::Core::Serialization::serializer& ser) { /* override */
    SST_SER(start);  
  }
};  // class PC

class DP final : public PC {  // Represent DebugSST15_Probe
public:  
  DP(int s) : PC(s) {}

  void serialize_order(SST::Core::Serialization::serializer& ser) { /* override */
    PC::serialize_order(ser);
  }
}; //class DP

class myPBC { // represent probe buffer control
public:
  myPBC(int s) :
    size(s)
    {}
  int size;

  void serialize_order(SST::Core::Serialization::serializer& ser) { /* override */
    SST_SER(size);
  }

}; // class PBC

template<typename T> class myPB final : public myPBC {
public:
  myPB(int s) : myPBC(s) {}
  void set(T v) { val = v; }   
  void serialize_order(SST::Core::Serialization::serializer& ser) { /* override */
    SST_SER(val);
  }


private: 
  T val;
  std::vector<T> mybuf;
}; // class PB

#endif


// -------------------------------------------------------
// DbgSST15Event
// -------------------------------------------------------
class DbgSST15Event : public SST::Event{
public:
  /// DbgSST15Event : standard constructor
  DbgSST15Event() : SST::Event() {}

  /// DbgSST15Event: constructor
  DbgSST15Event(std::vector<unsigned> d) : SST::Event(), data(d) {}

  /// DbgSST15Event: destructor
  ~DbgSST15Event() {}

  /// DbgSST15Event: retrieve the data
  std::vector<unsigned> const getData() { return data; }

private:
  std::vector<unsigned> data;     ///< DbgSST15Event: data payload

  /// DbgSST15Event: serialization method
  void serialize_order(SST::Core::Serialization::serializer& ser) override{
    Event::serialize_order(ser);
    SST_SER(data);
  }

  /// DbgSST15Event: serialization implementor
  ImplementSerializable(SSTDEBUG::DbgSST15::DbgSST15Event);

};  // class DbgSST15Event

// -------------------------------------------------------
// DbgSST15
// -------------------------------------------------------
class DbgSST15 : public SST::Component{
public:
  /// DbgSST15: top-level SST component constructor
  DbgSST15( SST::ComponentId_t id, const SST::Params& params );

  /// DbgSST15: top-level SST component destructor
  ~DbgSST15();

  /// DbgSST15: standard SST component 'setup' function
  void setup() override;

  /// DbgSST15: standard SST component 'finish' function
  void finish() override;

  /// DbgSST15: standard SST component init function
  void init( unsigned int phase ) override;

  /// DbgSST15: standard SST component printStatus
  void printStatus(SST::Output& out) override;

  /// DbgSST15: standard SST component clock function
  bool clockTick( SST::Cycle_t currentCycle );

  const int DEFAULT_PROBE_BUFFER_SIZE = 1024;

  // -------------------------------------------------------
  // DbgSST15 Component Registration Data
  // -------------------------------------------------------
  /// DbgSST15: Register the component with the SST core
  SST_ELI_REGISTER_COMPONENT( DbgSST15,     // component class
                              "dbgsst15",   // component library
                              "DbgSST15",   // component name
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
#if PROBE
    {"probeMode",       "0-Disabled,1-Checkpoint based, >1-rsv",    "0"},
    {"probeStartCycle", "Use with checkpoint-sim-period",           "0"},
    {"probeEndCycle",   "Cycle probing disable. 0 is no limit",     "0"},
    {"probeBufferSize", "Records in circular trace buffer",      "1024"}, // DEFAULT_PROBE_BUFFER_SIZE
    {"probePostDelay",  "post-trigger delay cycles. -1 to sample until checkpoint", "0"},
    {"probePort",       "Socket assignment for debug port",         "0"},
    {"cliControl",  "0x40 every chkpt, 0x20 chkpts when probe active, 0x10 sync state change,\n"
                    "0x04 every probe sample, 0x02 probe samples from trigger onward, 0x01 probe state change"
                    , "0"},
#endif
  )

  // -------------------------------------------------------
  // DbgSST15 Component Port Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_PORTS(
    {"port%(num_ports)d",
      "Ports which connect to endpoints.",
      {"dbgcli.DbgSST15Event", ""}
    }
  )

  // -------------------------------------------------------
  // DbgSST15 SubComponent Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()

  // -------------------------------------------------------
  // DbgSST15 Component Statistics Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_STATISTICS()

  // -------------------------------------------------------
  // DbgSST15 Component Checkpoint Methods
  // -------------------------------------------------------
  /// DbgSST15: serialization constructor
  DbgSST15() : SST::Component() {}

  /// DbgSST15: serialization
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
#if 0
  /// DbgSST15: Update debug control state object on checkpoint
  void handle_chkpt_probe_action();
#endif
  /// DbgSST15: serialization implementations
  ImplementSerializable(SSTDEBUG::DbgSST15::DbgSST15)

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
#if PROBE
// -- Component probe state object
std::unique_ptr<DbgSST15_Probe> probe_;
#endif
#if TESTSER
// SKK Test serializing unique pointer
std::unique_ptr<int> test_uptr;
std::unique_ptr<DP> DP_uptr;
std::unique_ptr<PC> PC_uptr;
std::unique_ptr<PC> PCser_uptr;
std::unique_ptr<DP> DPser_uptr;

std::shared_ptr<int> test_sptr;
std::shared_ptr<DP> DP_sptr;
std::shared_ptr<PC> PC_sptr;
std::shared_ptr<PC> PCser_sptr;
std::shared_ptr<DP> DPser_sptr;

DP* test_DP;
myPB<int>* test_myPB;
std::shared_ptr<myPB<int>> test_smyPB;


ProbeControl* test_ProbeControl;
//ProbeBufCtl* test_ProbeBufCtl;
ProbeBuffer<int>* test_ProbeBuffer;
//DbgSST15_Probe test_probe;
#endif
// -- rng objects
SST::RNG::Random* mersenne;                     ///< mersenne twister object

std::vector<SST::Link *> linkHandlers;          ///< LinkHandler objects

// -- private methods
/// event handler
void handleEvent(SST::Event *ev);

/// sends data to adjacent links
void sendData();

};  // class DbgSST15
#if PROBE
// -------------------------------------------------------
// Debug Control State 
// -------------------------------------------------------
class DbgSST15_Probe final : public ProbeControl {

public:
DbgSST15_Probe(SST::Component * comp, SST::Output * out, 
            int mode, SST::SimTime_t startCycle, SST::SimTime_t endCycle, int bufferSize, 
            int port, int postDelay, uint64_t cliControl);
// User custom sampling functions
void capture_event_atts(uint64_t cycle, uint64_t sz, DbgSST15Event *ev);
// Custom data type for samples
struct event_atts_t {
  uint64_t cycle_ = 0;
  uint64_t sz_ = 0;
  uint64_t deliveryTime_ = 0;
  int priority_ = 0;
  uint64_t orderTag_ = 0; 
  uint64_t queueOrder_ = 0;
  event_atts_t() {};
  event_atts_t(uint64_t c, uint64_t sz, DbgSST15Event *ev) : cycle_(c), sz_(sz)
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
#if TESTSER
  ProbeBuffer<int>* testPB;
#endif
  // -------------------------------------------------------
  // DbgSST15i_Probe Component Serialization Method
  // -------------------------------------------------------
  void serialize_order(SST::Core::Serialization::serializer& ser);

}; // class DbgSST15_Probe
#endif  // PROBE
}  // namespace SSTDEBUG::DbgSST15

#endif  // _SSTDEBUG_DBGSST15_H_

// EOF
