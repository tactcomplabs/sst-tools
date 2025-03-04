//
// _gridnode_cc_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "gridnode.h"
#include "tcldbg.h"

namespace SST::GridNode{

//------------------------------------------
// GridNode
//------------------------------------------
GridNode::GridNode(SST::ComponentId_t id, const SST::Params& params ) :
  SST::Component( id ), timeConverter(nullptr), clockHandler(nullptr),
  numPorts(8), minData(10), maxData(256), minDelay(20), maxDelay(100), clocks(1000),
  curCycle(0), demoBug(0), dataMask(0x1ffffff), dataMax(0x1ffffff) 
{
  tcldbg::spinner("GRID_SPINNER");
  uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
  output.init(
    "GridNode[" + getName() + ":@p:@t]: ",
    Verbosity, 0, SST::Output::STDOUT );
  const std::string cpuClock = params.find< std::string >("clockFreq", "1GHz");
  clockHandler  = new SST::Clock::Handler2<GridNode,&GridNode::clockTick>(this);
  timeConverter = registerClock(cpuClock, clockHandler);

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
  int checkSlot = params.find<int>("checkSlot", 0);

  // Load optional subcomponent in the cpt_check slot
  CPTSubComp = loadUserSubComponent<CPTSubComp::CPTSubCompAPI>("CPTSubComp");
  if (checkSlot && !CPTSubComp)
    output.fatal(CALL_INFO, -1, "SubComponent did not load properly\n");

  // Complete construction
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  // bug injection
  dataMax += demoBug;
  
  // Checkpoint markers
  cptBegin = 0xffb0000000000000UL | (id&0xffffffff)<<16 | 0xb1ffUL;
  cptEnd = 0xffe0000000000000UL | (id&0xffffffff)<<16 | 0xe1ffUL;
  output.verbose(CALL_INFO, 1, 0, 
    "Checkpoint markers for component id %" PRId64 " = [ 0x%" PRIx64 " 0x%" PRIx64 " ]\n",
    id, cptBegin, cptEnd );

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
                                         new Event::Handler2<GridNode,
                                         &GridNode::handleEvent>(this)));
    
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
  localRNG = new SST::RNG::MersenneRNG(unsigned(id) + rngSeed);
  clkDelay = localRNG->generateNextUInt32() % (maxDelay-minDelay+1) + minDelay;

  // constructor complete
  output.verbose( CALL_INFO, 5, 0, "Constructor complete\n" );
}

GridNode::~GridNode(){
  if (localRNG) delete localRNG;
  for (unsigned i=0;i<numPorts; i++) {
    if (rng[portname[i]]) 
      delete rng[portname[i]];
  } 
}

void GridNode::setup(){
}

void GridNode::finish(){
}

void GridNode::init( unsigned int phase ){
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

void GridNode::printStatus( Output& out ){
}

void GridNode::serialize_order(SST::Core::Serialization::serializer& ser){
  SST::Component::serialize_order(ser);
  SST_SER(cptBegin)
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
  SST_SER(cptEnd)
}

void GridNode::handleEvent(SST::Event *ev){
  GridNodeEvent *cev = static_cast<GridNodeEvent*>(ev);
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
  size_t range = maxData - minData + 1;
  uint32_t r = portRNG->generateNextUInt32() % uint32_t(range) + uint32_t(minData);
  if (r != data.size()) {
    output.fatal(CALL_INFO, -1,
                  "%s expected data size %" PRIu32 " does not match actual size %zu\n",
                  getName().c_str(), r, data.size());
  }
  if (r != data[1]) {
    output.fatal(CALL_INFO, -1,
              "%s expected data[0] %" PRIu32 " does not match actual %" PRIu32 "\n",
              getName().c_str(), r, data[0]);
  }
  for (unsigned i=2; i<r; i++){
    // checked is slightly different from how send data is generated to induce an error.
    unsigned d = (unsigned)portRNG->generateNextUInt32() & (unsigned)dataMask; 
    if ( d != data[i] ) {
      output.fatal(CALL_INFO, -1,
          "%s expected data[%" PRIu32 "] %" PRIu32 " does not match actual %" PRIu32 "\n",
          getName().c_str(), i, d, data[i]);
    }
  }
  
  delete ev;

  // Periodic subcomponent self-checking
  if (CPTSubComp && CPTSubComp->check())
    output.fatal(CALL_INFO, -1, "%s subcomponent self-check failed\n", getName().c_str());

}

void GridNode::sendData(){
  // Iterate over sending ports.
  // Treating links as unidirectional so the recieve port RNG tracks the send port.
  // TODO: create 2 RNG's per port, one for send, one for recieve
  for( unsigned port=0; port<(numPorts/2); port++ ){
    // generate a new payload
    std::vector<unsigned> data;
    size_t range = maxData - minData + 1;
    unsigned r = rng[portname[port]]->generateNextUInt32() % (uint32_t)range + (uint32_t)minData;
    // Outbound data sequence
    // [0] sending port number
    // [1] number of ints
    // [2:r-1] random data
    data.push_back(port);
    data.push_back(r);
    for( unsigned i=2; i<r; i++ ){
      uint64_t d = (uint64_t)(rng[portname[port]]->generateNextUInt32());
      // This is to introduce an infrequent mismatch between sender and receiver
      d = d & ( 0xfULL | (dataMask<<4) );
      if (d > dataMax) d = d & dataMask;
      data.push_back(unsigned(d));
    }
    output.verbose(CALL_INFO, 5, 0,
                   "%s: sending %zu unsigned values on link %d\n",
                   getName().c_str(),
                   data.size(), port);
    GridNodeEvent *ev = new GridNodeEvent(data);
    linkHandlers[port]->send(ev);
  }
}

unsigned GridNode::neighbor(unsigned n)
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

bool GridNode::clockTick( SST::Cycle_t currentCycle ){

  // sanity check the array
  for( uint64_t i = 0; i < (numBytes/4ull); i++ ){
    if( state[i] != ((unsigned)(i) + rngSeed) ){
      // found a mismatch
      output.fatal( CALL_INFO, -1,
                    "Error : found a mismatch data element: element %" PRIu64 " was %d and should have been %d\n",
                    i, state[i], ((unsigned)(i) + rngSeed));
    }
  }

  // Perform checkpoint subcomponent update every clock.
  if (CPTSubComp)
    CPTSubComp->update();

  // check to see whether we need to send data over the links
  curCycle++;
  if( curCycle >= clkDelay ){
    sendData();
    curCycle = 0;
    clkDelay = localRNG->generateNextUInt32() % (maxDelay-minDelay+1) + minDelay;
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

} // namespace SST::GridNode

// EOF
