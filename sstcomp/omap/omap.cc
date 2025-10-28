//
// _omap_cc_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "omap.h"

namespace SST::OMap{

OMSimpleComponent::OMSimpleComponent(ComponentId_t id, const SST::Params& params) : SST::Component(id)
{
    // SST output
    uint32_t verbosity = params.find<uint32_t>("verbose", 2);
    sstout_.init(getName() + ":@p:@t]: ",verbosity, 0, SST::Output::STDOUT );
    
    // SST clocking
    clockHandler_  = new SST::Clock::Handler2<OMSimpleComponent,&OMSimpleComponent::clockTick>(this);
    timeConverter_ = registerClock("1GHz", clockHandler_);

    // SubComponents
    p_omsimplecomp_function0_ = loadUserSubComponent<OMSubComponentAPI>("function0");
    assert(p_omsimplecomp_function0_);

    // Links
    port0link_ = configureLink("port0",
              new Event::Handler2<OMSimpleComponent, &OMSimpleComponent::port0rcv>(this));

    // Primary Controller
    primary = params.find<bool>("primary", false);
    if (primary) {
        registerAsPrimaryComponent();
        primaryComponentDoNotEndSim();
    } else {
        // Primary will send data over link to wake up other components
        unregisterClock(timeConverter_, clockHandler_);
    }
}


void OMSimpleComponent::serialize_order(SST::Core::Serialization::serializer &ser)
{
    Component::serialize_order(ser);

    // SST Handlers
    // SST_SER(sstout_);
    // SST_SER(timeConverter_);
    // SST_SER(clockHandler_);

    // TODO function0_ is serialized with slot0 of the component but not with sst_ser::as_ptr
    // Does this actually work with checkpointing?
    #if 1
    SST_SER(p_omsimplecomp_function0_);
    #endif

}

// Link handlers
void OMSimpleComponent::port0rcv(SST::Event *ev) {
    OMEvent* omev = static_cast<OMEvent*>(ev);
    payload_port0 = omev->payload();
    reregisterClock(timeConverter_, clockHandler_);
}

// Clock handler
bool OMSimpleComponent::clockTick(SST::Cycle_t currentCycle)
{
    // Only enable clock when we recieve data.
    // Update the received payload using a slot 'function'
    // Send it back over the link

    p_omsimplecomp_function0_->update( payload_port0 );
    sstout_.verbose(CALL_INFO, 0, 0, 
        "[%s] payload_port0.data=%" PRId64 "\n", getName().c_str(), payload_port0.data);
    if (primary && getCurrentSimCycle()>=1000000) {
        sstout_.verbose(CALL_INFO,0,0,"[%s] finished after 1000000 clocks\n", getName().c_str());
        primaryComponentOKToEndSim();
        return true;
    }
    port0link_->send(new OMEvent(payload_port0));
    return true;
}

OMSubComponentAPI::OMSubComponentAPI(ComponentId_t id, Params &params) : SubComponent(id)
{
    #ifdef _CHECK_MULTIMAP_
    // some types to check
    multimap_.insert({"Harry", "Potter"});
    multimap_.insert({"Bubba", "Gump"});
    multimap_.insert({"Harry", "Truman"});
    mutilmap_.insert({"Hari", "Seldon"});
    multimap_.insert({"Betsy", "Ross"});
    #endif
}

} // namespace SST::OMap

// EOF
