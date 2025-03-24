//
// _dbgcli_cc_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "dbgcli.h"
#include "tcldbg.h"

namespace SSTDEBUG::DbgCLI{

//------------------------------------------
// DbgCLI
//------------------------------------------
DbgCLI::DbgCLI(SST::ComponentId_t id, const SST::Params& params ) :
  SST::Component( id ), timeConverter(nullptr), clockHandler(nullptr),
  numPorts(1), minData(1), maxData(2), clockDelay(1), clocks(1000),
  curCycle(0) {
  
  tcldbg::spinner("SPINNER");
  const uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
  output.init(
    "DbgCLI[" + getName() + ":@p:@t]: ",
    Verbosity, 0, SST::Output::STDOUT );
  const std::string cpuClock = params.find< std::string >("clockFreq", "1GHz");
  clockHandler  = new SST::Clock::Handler2<DbgCLI,&DbgCLI::clockTick>(this);
  timeConverter = registerClock(cpuClock, clockHandler);
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  // read the rest of the parameters
  numPorts = params.find<unsigned>("numPorts", 1);
  minData = params.find<uint64_t>("minData", 1);
  maxData = params.find<uint64_t>("maxData", 2);
  clockDelay = params.find<uint64_t>("clockDelay", 1);
  clocks = params.find<uint64_t>("clocks", 1000);
  traceMode = params.find<unsigned>("traceMode", 0);
  cliType = params.find<unsigned>("cliType", 0);
  
  output.verbose(CALL_INFO, 1, 0, "numPorts=%u\n",numPorts);
  output.verbose(CALL_INFO, 1, 0, "minData=%" PRIu64 "\n", minData);
  output.verbose(CALL_INFO, 1, 0, "maxData=%" PRIu64 "\n", maxData);
  output.verbose(CALL_INFO, 1, 0, "clockDelay=%" PRIu64 "\n", clockDelay);
  output.verbose(CALL_INFO, 1, 0, "clocks=%" PRIu64 "\n", clocks);

  output.verbose(CALL_INFO, 1, 0, "traceMode=%" PRIu32 "\n", traceMode);
  if (traceMode & 1) output.verbose(CALL_INFO, 1, 0, "tracing SEND events\n");
  if (traceMode & 2) output.verbose(CALL_INFO, 1, 0, "tracing RECV events\n");
  if (traceMode>2) 
    output.fatal(CALL_INFO, -1, "traceMode>2 not yet supported\n");

  // sanity check the params
  if( maxData < minData ){
    output.fatal(CALL_INFO, -1,
                 "%s : maxData < minData\n",
                 getName().c_str());
  }

  // setup the rng
  mersenne = new SST::RNG::MersenneRNG(params.find<unsigned int>("rngSeed", 1223));

  // setup the links
  for( unsigned i=0; i<numPorts; i++ ){
    linkHandlers.push_back(configureLink("port"+std::to_string(i),
                                         new SST::Event::Handler2<DbgCLI,
                                         &DbgCLI::handleEvent>(this)));
  }

  // Debug Probe Parameters
  int probeMode       = params.find<int>("probeMode", 0);
  int probeStartCycle = params.find<int>("probeStartCycle",0);
  int probeEndCycle   = params.find<int>("probeEndCycle", 0);
  int probeBufferSize = params.find<int>("probeBufferSize", DEFAULT_PROBE_BUFFER_SIZE);
  int probePort       = params.find<int>("probePort", 0);
  int probePostDelay  = params.find<int>("probePostDelay", 0);
  uint64_t cliControl = params.find<uint64_t>("cliControl", 0);
  // Create Probe
  probe_ = std::make_unique<DbgCLI_Probe>(
          this, &output, probeMode, 
          probeStartCycle, probeEndCycle, probeBufferSize, 
          probePort, probePostDelay, cliControl);

  // constructor completeå
  output.verbose( CALL_INFO, 5, 0, "Constructor complete\n" );
}

DbgCLI::~DbgCLI(){}

void DbgCLI::setup(){
}

void DbgCLI::finish(){
}

void DbgCLI::init( unsigned int phase ){
}

void DbgCLI::printStatus( SST::Output& out ){
}

void DbgCLI::serialize_order(SST::Core::Serialization::serializer& ser){
  SST::Component::serialize_order(ser);
  SST_SER(clockHandler);
  SST_SER(numPorts);
  SST_SER(minData);
  SST_SER(maxData);
  SST_SER(clockDelay);
  SST_SER(clocks);
  SST_SER(curCycle);
  SST_SER(mersenne);
  SST_SER(linkHandlers);

  if ( (cliType==0) && (ser.mode() == SST::Core::Serialization::serializer::PACK) ) {
    handle_chkpt_probe_action();
  }

}

void DbgCLI::handle_chkpt_probe_action()
{
  auto c = getCurrentSimCycle();
  probe_->updateSyncState(c); 
  probe_->updateProbeState(c); // ensure states update before next sim clock
}

void DbgCLI::handleEvent(SST::Event *ev){
  DbgCLIEvent *cev = static_cast<DbgCLIEvent*>(ev);
  output.verbose(CALL_INFO, 5, 0,
                 "%s: received %zu unsigned values\n",
                 getName().c_str(),
                 cev->getData().size());

  /// debug probe 
  if ((traceMode & 2) == 2) {
      uint64_t range = maxData - minData + 1;
      size_t r = cev->getData().size();
      if (probe_->triggering()) {
        probe_->trigger(r > (range-1));
      }
      if (probe_->sampling())
        probe_->capture_event_atts(getCurrentSimCycle(), r, cev);
  }

  delete ev;
}

void DbgCLI::sendData(){
  for( unsigned i=0; i<numPorts; i++ ){
    // generate a new payload
    std::vector<unsigned> data;
    uint64_t range = maxData - minData + 1;
    uint64_t r = uint64_t(rand()) % range + minData;

    /// debug probe trigger (advance to post-trigger state)
    bool trace = (traceMode & 1) == 1;
    if (trace && probe_->triggering()) probe_->trigger(r > (range-1));
    ///

    for( size_t i=0; i<(unsigned)r; i++ ){
      data.push_back((unsigned)(mersenne->generateNextUInt32()));
    }
    output.verbose(CALL_INFO, 5, 0,
                   "%s: sending %zu unsigned values on link %d\n",
                   getName().c_str(),
                   data.size(), i);
    DbgCLIEvent *ev = new DbgCLIEvent(data);
    linkHandlers[i]->send(ev);

    /// debug probe data capture
    if (trace && probe_->sampling()) 
      probe_->capture_event_atts(getCurrentSimCycle(), r, ev);
    /// 
  }
}

bool DbgCLI::clockTick( SST::Cycle_t currentCycle ){

  tcldbg::spinner("CP0_SPINNER", getName().compare("cp1")==0);

  // check to see whether we need to send data over the links
  curCycle++;
  if( curCycle >= clockDelay ){
    sendData();
    curCycle = 0;
  }

  // check to see if we've reached the completion state
  bool rc = false;
  if( (uint64_t)(currentCycle) >= clocks ){
    output.verbose(CALL_INFO, 1, 0,
                   "%s ready to end simulation\n",
                   getName().c_str());
    primaryComponentOKToEndSim();
    rc =  true;
  }

  /// Debug Probe sequencing
  if (probe_->active()) probe_->updateProbeState(currentCycle);
  ///

  return rc;
}

DbgCLI_Probe::DbgCLI_Probe(SST::Component * comp, SST::Output * out, 
  int mode, SST::SimTime_t startCycle, SST::SimTime_t endCycle, int bufferSize, 
  int port, int postDelay, uint64_t cliControl)
 : ProbeControl(comp, out, mode, startCycle, endCycle, bufferSize, port, postDelay, cliControl)
{
  probeBuffer = std::make_shared<ProbeBuffer<event_atts_t>>(bufferSize);
  setBufferControls(probeBuffer);
}

void DbgCLI_Probe::capture_event_atts(uint64_t cycle, uint64_t sz, DbgCLIEvent *ev)
{
  if (! sampling()) return;
  // copy the sample into the circular buffer  
  event_atts_t e(cycle, sz, ev);
  probeBuffer->capture(e);
  // Finally call base class to update counters
  ProbeControl::sample();
}

} // namespace SST::DbgCLI

// EOF
