//
// _dbgsst15_cc_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "dbgsst15.h"
#include "tcldbg.h"

//#define PROBE 1
//#define SOCKET 1

namespace SSTDEBUG::DbgSST15{

//------------------------------------------
// DbgSST15
//------------------------------------------
DbgSST15::DbgSST15(SST::ComponentId_t id, const SST::Params& params ) :
  SST::Component( id ), timeConverter(nullptr), clockHandler(nullptr),
  numPorts(1), minData(1), maxData(2), clockDelay(1), clocks(1000),
  curCycle(0) {
  
  tcldbg::spinner("SPINNER");
  const uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
  output.init(
    "DbgSST15[" + getName() + ":@p:@t]: ",
    Verbosity, 0, SST::Output::STDOUT );
  const std::string cpuClock = params.find< std::string >("clockFreq", "1GHz");
  clockHandler  = new SST::Clock::Handler2<DbgSST15,&DbgSST15::clockTick>(this);
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
                                         new SST::Event::Handler2<DbgSST15,
                                         &DbgSST15::handleEvent>(this)));
  }
#if PROBE
  // Debug Probe Parameters
  int probeMode       = params.find<int>("probeMode", 0);
  int probeStartCycle = params.find<int>("probeStartCycle",0);
  int probeEndCycle   = params.find<int>("probeEndCycle", 0);
  int probeBufferSize = params.find<int>("probeBufferSize", DEFAULT_PROBE_BUFFER_SIZE);
  int probePort       = params.find<int>("probePort", 0);
  int probePostDelay  = params.find<int>("probePostDelay", 0);
  uint64_t cliControl = params.find<uint64_t>("cliControl", 0);
  // Create Probe
  probe_ = std::make_unique<DbgSST15_Probe>(
          this, &output, probeMode, 
          probeStartCycle, probeEndCycle, probeBufferSize, 
          probePort, probePostDelay, cliControl);
#endif
#if TESTSER
  test_uptr = std::make_unique<int>(5);
  DP_uptr = std::make_unique<DP>(7);
  PC_uptr = std::make_unique<PC>(8);
  PCser_uptr = std::make_unique<PC>(9);
  DPser_uptr = std::make_unique<DP>(10);

  test_sptr = std::make_shared<int>(15);
  DP_sptr = std::make_shared<DP>(17);
  PC_sptr = std::make_shared<PC>(18);
  PCser_sptr = std::make_shared<PC>(19);
  DPser_sptr = std::make_shared<DP>(20);
  
  test_DP = new DP(21);
  test_myPB = new myPB<int>(22);
  test_myPB->set(122);
  test_smyPB = std::make_shared<myPB<int>>(23);
  test_smyPB->set(123);
 
  test_ProbeControl = new ProbeControl(this, &output, 1, 1, 1, 34, 1, 1, 1);
  test_ProbeBuffer = new ProbeBuffer<int>(24);

  std::cout << "Runtime type test_myPB: " << typeid(test_myPB).name() << std::endl;
  std::cout << "Runtime type *test_myPB: " << typeid(*test_myPB).name() << std::endl;
  std::cout << "Runtime type test_ProbeBuffer: " << typeid(test_ProbeBuffer).name() << std::endl;
  std::cout << "Runtime type *test_ProbeBuffer: " << typeid(*test_ProbeBuffer).name() << std::endl;

#if 0
  test_probe = DbgSST15_Probe(
                this, &output, probeMode,
                 probeStartCycle, probeEndCycle, probeBufferSize,
          probePort, probePostDelay, cliControl);
  #endif
#endif
  // constructor completeå
  output.verbose( CALL_INFO, 5, 0, "Constructor complete\n" );
}

DbgSST15::~DbgSST15(){}

void DbgSST15::setup(){
}

void DbgSST15::finish(){
}

void DbgSST15::init( unsigned int phase ){
}

void DbgSST15::printStatus( SST::Output& out ){
}

void DbgSST15::serialize_order(SST::Core::Serialization::serializer& ser){
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

  SST_SER(traceMode);
  SST_SER(cliType);
  SST_SER(*probe_);

#if TESTSER
  SST_SER(*test_uptr);
  SST_SER(PC_uptr->start);
  SST_SER(DP_uptr->start);
  SST_SER(*PCser_uptr);
  SST_SER(*DPser_uptr);

  SST_SER(*test_sptr);
  SST_SER(PC_sptr->start);
  SST_SER(DP_sptr->start);
  SST_SER(*PCser_sptr);
  SST_SER(*DPser_sptr);

  SST_SER(*test_DP);
  
  SST_SER(*test_myPB); 
  SST_SER(*test_smyPB);
  SST_SER(*test_ProbeBuffer);  // SKK ERROR caused here
#endif

#if 0
  if ( (cliType==0) && (ser.mode() == SST::Core::Serialization::serializer::PACK) ) {
    handle_chkpt_probe_action();
  }
#endif

}

#if 1
void DbgSST15::handle_chkpt_probe_action()
{
  auto c = getCurrentSimCycle();
  printf("handle chkpt probe action @ cycle %ld\n", c);

  probe_->updateSyncState(c); 
  probe_->updateProbeState(c); // ensure states update before next sim clock
}
#endif

void DbgSST15::handleEvent(SST::Event *ev){
  DbgSST15Event *cev = static_cast<DbgSST15Event*>(ev);
  output.verbose(CALL_INFO, 5, 0,
                 "%s: received %zu unsigned values\n",
                 getName().c_str(),
                 cev->getData().size());
#if 1
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
#endif
  delete ev;
}

void DbgSST15::sendData(){
  for( unsigned i=0; i<numPorts; i++ ){
    // generate a new payload
    std::vector<unsigned> data;
    uint64_t range = maxData - minData + 1;
    uint64_t r = uint64_t(rand()) % range + minData;
#if 1
    /// debug probe trigger (advance to post-trigger state)
    bool trace = (traceMode & 1) == 1;
    if (trace && probe_->triggering()) probe_->trigger(r > (range-1));
    ///
#endif
    for( size_t i=0; i<(unsigned)r; i++ ){
      data.push_back((unsigned)(mersenne->generateNextUInt32()));
    }
    output.verbose(CALL_INFO, 5, 0,
                   "%s: sending %zu unsigned values on link %d\n",
                   getName().c_str(),
                   data.size(), i);
    DbgSST15Event *ev = new DbgSST15Event(data);
    linkHandlers[i]->send(ev);
#if 1
    /// debug probe data capture
    if (trace && probe_->sampling()) 
      probe_->capture_event_atts(getCurrentSimCycle(), r, ev);
    /// 
#endif
  }
}

bool DbgSST15::clockTick( SST::Cycle_t currentCycle ){

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
#if 1
  /// Debug Probe sequencing
  if (probe_->active()) probe_->updateProbeState(currentCycle);
  ///
#endif

  return rc;
}


//------------------------------------------
// DbgSST15_Probe
//------------------------------------------

#if PROBE
DbgSST15_Probe::DbgSST15_Probe(SST::Component * comp, SST::Output * out, 
  int mode, SST::SimTime_t startCycle, SST::SimTime_t endCycle, int bufferSize, 
  int port, int postDelay, uint64_t cliControl)
 : ProbeControl(comp, out, mode, startCycle, endCycle, bufferSize, port, postDelay, cliControl)
{
  probeBuffer = std::make_shared<ProbeBuffer<event_atts_t>>(bufferSize);
  setBufferControls(probeBuffer);
}

void DbgSST15_Probe::capture_event_atts(uint64_t cycle, uint64_t sz, DbgSST15Event *ev)
{
  if (! sampling()) return;
  // copy the sample into the circular buffer  
  event_atts_t e(cycle, sz, ev);
  probeBuffer->capture(e);
  // Finally call base class to update counters
  ProbeControl::sample();
}

 
void DbgSST15_Probe::serialize_order(SST::Core::Serialization::serializer& ser) {
    ProbeControl::serialize_order(ser);
    SST_SER(*probeBuffer);
}


#endif

} // namespace SSTDEBUG::DbgSST15

// EOF
