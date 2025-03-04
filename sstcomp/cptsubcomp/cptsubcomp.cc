//
// _cptsubcomp_cc_
//
// Copyright (C) 2017-2025 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "cptsubcomp.h"
#include "tcldbg.h"

using namespace SST;
using namespace SST::CPTSubComp;

// cptSubCompVecInt
CPTSubCompVecInt::CPTSubCompVecInt(ComponentId_t id, Params& params) : CPTSubCompAPI(id, params), clocks(0) {
    uint32_t Verbosity = params.find< uint32_t >( "verbose", 0 );
    output.init(
      "CPTSubCompVecInt[" + getName() + ":@p:@t]: ",
      Verbosity, 0, SST::Output::STDOUT
    );
    max = params.find<size_t>("max", 1);
    seed = params.find<unsigned>("seed", 1223);
    output.verbose(CALL_INFO, 1, 0, "max=%lx seed=%" PRIu32 "\n", max, seed);
    assert(max>0);
    rng = new SST::RNG::MersenneRNG(seed);
    tut.resize(max);
    tutini.resize(max);
    for (size_t i=0; i<max; i++) {
        int32_t n = rng->generateNextInt32();
        tut[i] = n;
        tutini[i] = n;
    }
}

CPTSubCompVecInt::~CPTSubCompVecInt()
{
    if (rng) delete rng;
}

int CPTSubCompVecInt::check()
{
    for (size_t i=0;i<max; i++) {
        if (tut[i] != tutini[i] + (int32_t)clocks) 
            return 1;
    }
    return 0;
}

void CPTSubCompVecInt::update()
{
    clocks++;
    for (size_t i=0; i<max; i++)
        tut[i]++;
}

void CPTSubCompVecInt::serialize_order(SST::Core::Serialization::serializer &ser)
{
    SST_SER(output);
    SST_SER(clocks);
    SST_SER(max);
    SST_SER(seed);
    SST_SER(tut);
    SST_SER(tutini);
    SST_SER(rng);
}

// EOF
