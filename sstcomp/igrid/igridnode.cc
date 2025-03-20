//
// _igridnode_cc_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "igridnode.h"
#include "kgdbg.h"

namespace SST::IGridNode{

//------------------------------------------
// IGridNode
//------------------------------------------
IGridNode::IGridNode(SST::ComponentId_t id, const SST::Params& params ) :
  SST::Component( id ), timeConverter(nullptr), clockHandler(nullptr),
  numPorts(8), minData(10), maxData(256), minDelay(20), maxDelay(100), clocks(1000),
  curCycle(0), demoBug(0), dataMask(0x1ffffff), dataMax(0x1ffffff) {
  
  kgdbg::spinner("GRIDSPINNER");

  const unsigned Verbosity = params.find< unsigned >( "verbose", 0 );
  output.init(
    "IGridNode[" + getName() + ":@p:@t]: ",
    Verbosity, 0, SST::Output::STDOUT );
  const std::string cpuClock = params.find< std::string >("clockFreq", "1GHz");
  clockHandler  = new SST::Clock::Handler2<IGridNode,&IGridNode::clockTick>(this);
  timeConverter = registerClock(cpuClock, clockHandler);
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  // read the rest of the parameters
  numBytes = params.find<uint64_t>("numBytes", 16384);
  numPorts = params.find<unsigned>("numPorts", 8);
  minData = params.find<uint64_t>("minData", 10);
  maxData = params.find<uint64_t>("maxData", 256);
  minDelay = params.find<uint64_t>("minDelay", 50);
  maxDelay = params.find<uint64_t>("maxDelay", 100);
  clocks = params.find<uint64_t>("clocks", 1000);
  rngSeed = params.find<unsigned>("rngSeed", 1223);
  demoBug = params.find<unsigned>("demoBug", 0);
  breakEnable = (int) params.find<bool>("breakEnable", 0);
  // bug injection
  dataMax += demoBug;

  // sanity check the params
  if (minData < 10) {
    output.verbose(CALL_INFO, 1, 0, 
    "Warning: User specified minData < 10. Setting to 10\n");
    minData = 10;
  }
  if( maxData < minData ){
    output.fatal(CALL_INFO, -1,
                 "%s : maxData < minData\n",
                 getName().c_str());
  }
 if( maxDelay < minDelay ){
    output.fatal(CALL_INFO, -1,
                 "%s : maxDelay < minDelay\n",
                 getName().c_str());
  }

  // setup the port links and their random generators
  assert(numPorts==8); // TODO generalize this
  portname.resize(numPorts);
  for( unsigned i=0; i<numPorts; i++ ){
    portname[i] = "port" + std::to_string(i);
    linkHandlers.push_back(configureLink("port"+std::to_string(i),
                                         new Event::Handler2<IGridNode,
                                         &IGridNode::handleEvent>(this)));
    
    // The sending link and receiving links must have the same seed for the checking to work
    // send: up=0, down=1, left=2, right=3
    // rcv:  up=4, down=5, left=6, right=7
    #if 0
    // Keep same random sequence for all ports
    rng.insert( {portname[i], new SST::RNG::MersenneRNG(params.find<unsigned int>("rngSeed",rngSeed))} );
    #else
    // Each port has unique random sequence.
    // However, across components the data for each corresponding port will be the same.
    // To add the complimentary port's component info to the seed could be a future enhancement.
    unsigned n = i<4 ? i : neighbor(i);
    rng.insert( {portname[i], new SST::RNG::MersenneRNG(n + rngSeed)} );
    #endif
  }
  
  // local random number generator. These can run independently for each component.
  localRNG = new SST::RNG::MersenneRNG((uint32_t) (id + rngSeed));
  clkDelay = localRNG->generateNextUInt32() % (maxDelay-minDelay+1) + minDelay;

  // constructor complete
  output.verbose( CALL_INFO, 5, 0, "Constructor complete\n" );
}

IGridNode::~IGridNode(){
  if (localRNG) delete localRNG;
  for (unsigned i=0;i<numPorts; i++) {
    if (rng[portname[i]]) 
      delete rng[portname[i]];
  } 
}

void IGridNode::setup(){
}

void IGridNode::finish(){
}

void IGridNode::init( unsigned int phase ){
  if( phase == 0 ){
    // setup the initial data
    output.verbose(CALL_INFO, 5, 0,
                   "%s: initializing internal data at init phase=0\n",
                   getName().c_str());
    for( uint64_t i = 0; i < (numBytes/4ull); i++ ){
      state.push_back( (unsigned)(i) + rngSeed );
    }
  }
}

void IGridNode::printStatus( Output& out ){
}

void IGridNode::serialize_order(SST::Core::Serialization::serializer& ser){
  SST::Component::serialize_order(ser);
  SST_SER(clockHandler)
  SST_SER(numBytes)
  SST_SER(numPorts)
  SST_SER(minData)
  SST_SER(maxData)
  SST_SER(minDelay)
  SST_SER(maxDelay)
  SST_SER(clkDelay)
  SST_SER(clocks)
  SST_SER(rngSeed)
  SST_SER(state)
  SST_SER(curCycle)
  SST_SER(portname)
  SST_SER(rng)
  SST_SER(localRNG)
  SST_SER(linkHandlers)
  SST_SER(demoBug)
  SST_SER(dataMask)
  SST_SER(dataMax)
  SST_SER(breakEnable)
}

void IGridNode::handleEvent(SST::Event *ev){
  IGridNodeEvent *cev = static_cast<IGridNodeEvent*>(ev);
  auto data = cev->getData();
  output.verbose(CALL_INFO, 5, 0,
                 "%s: received %zu unsigned values\n",
                 getName().c_str(),
                 data.size());
  // Inbound data sequence
  // [0] sending port number
  // [1] r
  // [2:(r-1)] random data
  // Check the incoming data


  unsigned send_port = data[0];
  assert(send_port < (portname.size()/2)); // TODO unrestrict bidirectional links
  unsigned rcv_port = neighbor(send_port);
  auto portRNG = rng[portname[rcv_port]];
  uint64_t range = maxData - minData + 1;
  uint64_t r = portRNG->generateNextUInt32() % range + minData;
  if (r != data.size()) {
    output.fatal(CALL_INFO, -1,
                  "%s expected data size %" PRIu64 " does not match actual size %" PRIu64 "\n",
                  getName().c_str(), r, data.size());
  }
  if (r != data[1]) {
    output.fatal(CALL_INFO, -1,
              "%s expected data[0] %" PRIu64 " does not match actual %" PRIu32 "\n",
              getName().c_str(), r, data[0]);
  }
  for (unsigned i=2; i<r; i++){
    // checked is slightly different from how send data is generated to induce an error.
    uint64_t d = (unsigned)portRNG->generateNextUInt32() & dataMask; 
    if ( d != data[i] ) {
      output.fatal(CALL_INFO, -1,
          "%s expected data[%" PRIu32 "] %" PRIu64 " does not match actual %" PRIu32 "\n",
          getName().c_str(), i, d, data[i]);
    }
  }

  // Interactive Console Debug Example
  // breakEnable can be set from interactive console to enable/disable as long it is serialized 
  // Could also add triggers etc to control when to break
  if (breakEnable) {
    std::string message = "\tBreak to interactive console from event handler\n";
    SST::BaseComponent::initiateInteractive(message.c_str());
  }

  delete ev;
}

void IGridNode::sendData(){
  // Iterate over sending ports.
  // Treating links as unidirectional so the recieve port RNG tracks the send port.
  // TODO: create 2 RNG's per port, one for send, one for recieve
  for( unsigned port=0; port<(numPorts/2); port++ ){
    // generate a new payload
    std::vector<unsigned> data;
    uint64_t range = maxData - minData + 1;
    uint64_t r = rng[portname[port]]->generateNextUInt32() % range + minData;
    // Outbound data sequence
    // [0] sending port number
    // [1] number of ints
    // [2:r-1] random data
    data.push_back(port);
    data.push_back((uint32_t) r);
    for( unsigned i=2; i<r; i++ ){
      uint64_t d = (unsigned)(rng[portname[port]]->generateNextUInt32());
      // This is to introduce an infrequent mismatch between sender and receiver
      d = d & ( 0xfULL | (dataMask<<4) );
      if (d > dataMax) d = d & dataMask;
      data.push_back((uint32_t) d);
    }
    output.verbose(CALL_INFO, 5, 0,
                   "%s: sending %zu unsigned values on link %d\n",
                   getName().c_str(),
                   data.size(), port);
    IGridNodeEvent *ev = new IGridNodeEvent(data);
    linkHandlers[port]->send(ev);
  }
}

unsigned IGridNode::neighbor(unsigned n)
{
  // send: up=0, down=1, left=2, right=3
  // rcv:  up=4, down=5, left=6, right=7
  switch (n) {
    case 0:
      return 5;
    case 1:
      return 4;
    case 2:
      return 7;
    case 3:
      return 6;
    case 4:
      return 1;
    case 5:
      return 0;
    case 6:
      return 3;
    case 7:
      return 2;
    default:
      output.fatal(CALL_INFO, -1, "invalid port number\n");
  }
  assert(false);
  return 8; // invalid
}

bool IGridNode::clockTick( SST::Cycle_t currentCycle ){

  // sanity check the array
  for( uint64_t i = 0; i < (numBytes/4ull); i++ ){
    if( state[i] != ((unsigned)(i) + rngSeed) ){
      // found a mismatch
      output.fatal( CALL_INFO, -1,
                    "Error : found a mismatch data element: element %" PRIu64 " was %d and should have been %d\n",
                    i, state[i], ((unsigned)(i) + rngSeed));
    }
  }

  // check to see whether we need to send data over the links
  curCycle++;
  if( curCycle >= clkDelay ){
    sendData();
    curCycle = 0;
    clkDelay = localRNG->generateNextUInt32() % (maxDelay-minDelay+1) + minData;
  }

  // check to see if we've reached the completion state
  if( (uint64_t)(currentCycle) >= clocks ){
    output.verbose(CALL_INFO, 1, 0,
                   "%s ready to end simulation\n",
                   getName().c_str());
    primaryComponentOKToEndSim();
    return true;
  }

  return false;
}

} // namespace SST::IGridNode

// EOF
