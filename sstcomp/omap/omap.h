//
// _SST_OMAP_H
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_OMAP_H
#define _SST_OMAP_H

// clang-format off
// -- Standard Headers
#include "SST.h"
#include <map>
// clang-format on

// #define _CHECK_MULTIMAP_

namespace SST::OMap {

// ---------------------------------------------
// payload_t
// ---------------------------------------------
struct payload_t {
  uint64_t data = 0;
}; //struct payload_t

// ---------------------------------------------
// OMEvent
// ---------------------------------------------
class OMEvent : public SST::Event {
public:
  OMEvent(const payload_t& p) : SST::Event(), payload_(p) {}
  virtual ~OMEvent() {}
  const payload_t& payload(){ return payload_; }
private:
  payload_t payload_;
public:
  // Serialization
  OMEvent() : Event() {}
  void serialize_order(SST::Core::Serialization::serializer& ser) override {
    SST::Event::serialize_order(ser);
    // SST_SER(payload_); //TODO can I see in-flight events in debugger?
  }
  ImplementSerializable(SST::OMap::OMEvent);
}; // class OMEvent

// -------------------------------------------------------
// OMSubComponentAPI (not registered)
// -------------------------------------------------------
class OMSubComponentAPI : public SST::SubComponent {
public:
  SST_ELI_REGISTER_SUBCOMPONENT_API(SST::OMap::OMSubComponentAPI);
  // SST_ELI_DOCUMENT_PARAMS(
  //   {"verbose",         "Sets the verbosity level of output",   "0" }
  // )
  OMSubComponentAPI(ComponentId_t id, Params& params);
  virtual ~OMSubComponentAPI() {}
  virtual void update(payload_t& p) {
    for (size_t i=0; i<8; i++) 
      array_[i] = subcompapi_counter_ + i;
    subcompapi_counter_++;
  };
protected:
  SST::Output sstout_;
  uint64_t subcompapi_counter_ = 0;        // ObjectMapFundamental
  uint64_t array_[8] = {0};     // ObjectMapArray

  #ifdef _CHECK_MULTIMAP_
  std::multimap<std::string, std::string> multimap_; // ObjectMapContainer
  #endif

public:
  // serialization support
  OMSubComponentAPI() : SubComponent() {}
  void serialize_order(SST::Core::Serialization::serializer& ser) override {
    SubComponent::serialize_order(ser);
    // SST_SER(sstout_);
    SST_SER(subcompapi_counter_);
    // SST_SER(array_);
    #ifdef _CHECK_MULTIMAP_
    SST_SER(multimap_);
    #endif
  }
  ImplementVirtualSerializable(SST::OMap::OMSubComponentAPI)
}; //class OMSubComponentAPI

// -------------------------------------------------------
// OMSimpleSubComponent
// -------------------------------------------------------
class OMSimpleSubComponent : public OMSubComponentAPI {
public:
  SST_ELI_REGISTER_SUBCOMPONENT(
        OMSimpleSubComponent,            // Class name
        "omap",                          // Library name
        "OMSimpleSubComponent",          // Subcomponent name
        SST_ELI_ELEMENT_VERSION(1,0,0),  // A version number
        "Simple subcomponent for object map evaluation", 
        SST::OMap::OMSimpleSubComponent) // Fully qualified API name
  // SST_ELI_DOCUMENT_PARAMS(
  //   {"param",    "desc", "default" }
  // )
  OMSimpleSubComponent(ComponentId_t id, Params& params) : OMSubComponentAPI(id,params) {}
  virtual ~OMSimpleSubComponent() {}
  virtual void update(payload_t& p) final {
    OMSubComponentAPI::update(p);
    p.data += 1;
  }
public:
    // serialization support
    OMSimpleSubComponent() : OMSubComponentAPI() {}
    void serialize_order(SST::Core::Serialization::serializer& ser) override {
      OMSubComponentAPI::serialize_order(ser);
    }
    ImplementSerializable(SST::OMap::OMSimpleSubComponent)
}; //class OMSimpleSubComponent

// ---------------------------------------------
// OMSimpleComponent
// ---------------------------------------------
class OMSimpleComponent : public SST::Component{

public:
  SST_ELI_REGISTER_COMPONENT( OMSimpleComponent,   // component class
                            "omap",       // component library
                            "OMSimpleComponent",   // component name
                            SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
                            "Simple Object Map Evalulation Component",
                            COMPONENT_CATEGORY_UNCATEGORIZED )
  SST_ELI_DOCUMENT_PARAMS(
    {"verbose", "Sets the verbosity level of output",   "1" },
    {"primary", "Sets component as primary controller", "0" },
  )
  SST_ELI_DOCUMENT_PORTS(
    { "port0",  "generic port 0",   {"omap.OMEvent"} },
  )
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    { "function0", 
      "generic function 0",
      "SST::OMap::OMSubComponentAPI" },
  )

  explicit OMSimpleComponent(ComponentId_t id, const SST::Params& params);
  ~OMSimpleComponent() {}

  // Component Lifecycle
  void init( unsigned int phase ) override {};     // post-construction, polled events
  void setup() override {};                        // pre-simulation, called once per component
  void complete( unsigned int phase ) override {}; // post-simulation, polled events
  void finish() override {};                       // pre-destruction, called once per component
  void emergencyShutdown() override {};            // SIGINT, SIGTERM
  void printStatus(Output& out) override {};       // SIGUSR2

  // Clocking
  bool clockTick( SST::Cycle_t currentCycle );  // return true if clock should be disabled

private:
  // Subcomponent pointers
  OMSubComponentAPI* p_omsimplecomp_function0_ = nullptr;

  // SST Handlers
  SST::Output sstout_;
  SST::TimeConverter* timeConverter_;
  SST::Clock::HandlerBase* clockHandler_;

  // Links
  SST::Link* port0link_;

  // Link handlers
  void port0rcv(SST::Event *ev);
  
  // Internals
  bool primary = false;
  payload_t payload_port0;

public:
  // -------------------------------------------------------
  // Serialization support
  // -------------------------------------------------------
  // Default constructor required for serialization
  OMSimpleComponent() : SST::Component() {}
  // Serialization function 
  void serialize_order(SST::Core::Serialization::serializer& ser) override;
  // Serialization implementation
  ImplementSerializable(SST::OMap::OMSimpleComponent)

}; //class OMSimpleComponent

}   //namespace SST::OMap

#endif  // _SST_OMAP_H

// EOF
