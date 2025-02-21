#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
    // parse command line
    if (argc != 4) {
        cout << "Usage: checkpoint-file component-offset num-words" << endl;
        return 1;
    }
    string cptFileName(argv[1]);\
    unsigned compOffset = unsigned(stoul(argv[2]));
    unsigned numWords = unsigned(stoul(argv[3]));

    // open checkpoint file
    ifstream cpt(cptFileName);
    if ( !cpt.is_open() ) {
        cerr << "Error: Cannot open " << cptFileName << endl;
        return 1;
    }
    cout << "Opened checkpoint file " << cptFileName << endl;

    // Advance to start of component data
    cpt.seekg(compOffset);

    // Read the component data
    char buffer[8];
    uint64_t data = 0;
    for (size_t i=0; i<numWords*8;i+=8) {
        cpt.read(buffer, 8);
        memcpy(&data, buffer, 8);
        cout << hex << "0x" << i << " [0x" << compOffset + i << "] <- 0x" << data << endl; 
    }

    // Done
    cpt.close();
    cout << "readcpt completed normally" << endl;
    return 0;
}