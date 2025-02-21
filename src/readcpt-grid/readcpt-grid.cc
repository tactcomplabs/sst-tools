//clang-format off
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

#include <kgdbg.h>
//clang-format on

using namespace std;

// Declare variables for know types and placeholders of an appropriate size for unknown types.

// Serializing the base class for our test case is consistently 195 bytes.
// SST::Component::serialize_order(ser);
uint8_t padding0[195];
// SST_SER(cptBegin)
uint64_t cptBegin;
// SST_SER(clockHandler)
// SST::Clock::HandlerBase* clockHandler;
void* clockHandler;
// SST_SER(numBytes)
// SST_SER(numPorts)
// SST_SER(minData)
// SST_SER(maxData)
// SST_SER(minDelay)
// SST_SER(maxDelay)
uint64_t numBytes;
unsigned numPorts;
uint64_t minData;
uint64_t maxData;
uint64_t minDelay;
uint64_t maxDelay;
// SST_SER(clkDelay)
// SST_SER(clocks)
// SST_SER(rngSeed)
uint64_t clkDelay;
uint64_t clocks; 
unsigned rngSeed;
// // SST_SER(state)
// std::vector<unsigned> state;
// SST_SER(curCycle)
uint64_t curCycle;
// SST_SER(portname)
std::vector<std::string> portname;
// SST_SER(rng)
//std::map< std::string, SST::RNG::Random* > rng;
std::map< std::string, void* > rng;
// SST_SER(localRNG)
//SST::RNG::Random* localRNG;
void* localRNG;
// SST_SER(linkHandlers)
//std::vector<SST::Link *> linkHandlers;
std::vector<void*> linkHandlers;
// SST_SER(demoBug)
// SST_SER(dataMask)
// SST_SER(dataMax)
unsigned demoBug;
uint64_t dataMask;
uint64_t dataMax; 
// SST_SER(cptEnd)
uint64_t cptEnd;

// file read buffer
char buffer[4096];

int main(int argc, char* argv[])
{
    // parse command line
    if (argc != 3) {
        cout << "Usage: readcpt-grid checkpoint-file component-offset" << endl;
        return 1;
    }
    string cptFileName(argv[1]);\
    unsigned compOffset = unsigned(stoul(argv[2]));

    // open checkpoint file
    ifstream cpt(cptFileName);
    if ( !cpt.is_open() ) {
        cerr << "Error: Cannot open " << cptFileName << endl;
        return 1;
    }
    cout << "Opened checkpoint file " << cptFileName << endl;

    // Advance to start of component data
    cpt.seekg(compOffset);

    // SST::Component::serialize_order(ser);
    cpt.read(buffer, sizeof(padding0)); memcpy(&padding0, buffer, sizeof(padding0));
    // SST_SER(cptBegin)
    cpt.read(buffer, sizeof(cptBegin)); memcpy(&cptBegin, buffer, sizeof(cptBegin));
    // SST_SER(clockHandler)
    cpt.read(buffer, sizeof(clockHandler)); memcpy(&clockHandler, buffer, sizeof(clockHandler));
    // SST_SER(numBytes)
    cpt.read(buffer, sizeof(numBytes)); memcpy(&numBytes, buffer, sizeof(numBytes));
    // SST_SER(numPorts)
    cpt.read(buffer, sizeof(numPorts)); memcpy(&numPorts, buffer, sizeof(numPorts));
    // SST_SER(minData)
    cpt.read(buffer, sizeof(minData)); memcpy(&minData, buffer, sizeof(minData));
    // SST_SER(maxData)
    cpt.read(buffer, sizeof(maxData)); memcpy(&maxData, buffer, sizeof(maxData));
    // SST_SER(minDelay)
    cpt.read(buffer, sizeof(minDelay)); memcpy(&minDelay, buffer, sizeof(minDelay));
    // SST_SER(maxDelay)
    cpt.read(buffer, sizeof(maxDelay)); memcpy(&maxDelay, buffer, sizeof(maxDelay));
    // SST_SER(clkDelay)
    cpt.read(buffer, sizeof(clkDelay)); memcpy(&clkDelay, buffer, sizeof(clkDelay));
    // SST_SER(clocks)
    cpt.read(buffer, sizeof(clocks)); memcpy(&clocks, buffer, sizeof(clocks));
    // SST_SER(rngSeed)
    cpt.read(buffer, sizeof(rngSeed)); memcpy(&rngSeed, buffer, sizeof(rngSeed));
    // // SST_SER(state)
    // state.resize(1);
    // cpt.read(buffer, sizeof(state)); memcpy(&state, buffer, sizeof(state));
    // SST_SER(curCycle)
    cpt.read(buffer, sizeof(curCycle)); memcpy(&curCycle, buffer, sizeof(curCycle));
    // // SST_SER(portname)
    // cpt.read(buffer, sizeof(portname)); memcpy(&portname, buffer, sizeof(portname));
    // // SST_SER(rng)
    // cpt.read(buffer, sizeof(rng)); memcpy(&rng, buffer, sizeof(rng));
    // // SST_SER(localRNG)
    // cpt.read(buffer, sizeof(localRNG)); memcpy(&localRNG, buffer, sizeof(localRNG));
    // // SST_SER(linkHandlers)
    // cpt.read(buffer, sizeof(linkHandlers)); memcpy(&linkHandlers, buffer, sizeof(linkHandlers));
    // SST_SER(demoBug)
    cpt.read(buffer, sizeof(demoBug)); memcpy(&demoBug, buffer, sizeof(demoBug));
    // SST_SER(dataMask)
    cpt.read(buffer, sizeof(dataMask)); memcpy(&dataMask, buffer, sizeof(dataMask));
    // SST_SER(dataMax)
    cpt.read(buffer, sizeof(dataMax)); memcpy(&dataMax, buffer, sizeof(dataMax));
    // SST_SER(cptEnd)
    cpt.read(buffer, sizeof(cptEnd)); memcpy(&cptEnd, buffer, sizeof(cptEnd));

    // Done
    cpt.close();

    kgdbg::spinner("SPINNER");
    cout << "cptBegin <- 0x" << hex << setfill('0') << setw(16) << cptBegin << endl;
    cout << "cptEnd <- 0x" << hex << setfill('0') << setw(16) << cptEnd << endl;
    cout << "readcpt-grid completed normally" << endl;
    return 0;
}