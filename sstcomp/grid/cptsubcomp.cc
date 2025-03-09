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
    subcompBegin = 0xcccb00000000bccc;
    subcompEnd = 0xccce00000000eccc;
}

CPTSubCompVecInt::~CPTSubCompVecInt()
{
    if (rng) delete rng;
}

void SST::CPTSubComp::CPTSubCompVecInt::setup()
{
    output.verbose(CALL_INFO, 2, 0, "setup() clocks %d check 0x%x\n", clocks, tut[0]);
}

void CPTSubCompVecInt::finish()
{
    output.verbose(CALL_INFO, 2, 0, "finish() clocks %d check 0x%x\n", clocks, tut[0]);
    if (check())
        output.fatal(CALL_INFO, -1, "final check failed\n");
}

int CPTSubCompVecInt::check()
{
    for (size_t i=0;i<max; i++) {
        output.verbose(CALL_INFO, 3, 0, "Comparing %x %x\n", tut[i], tutini[i]);
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
    SST_SER(subcompBegin);
    SST_SER(output);
    SST_SER(clocks);
    SST_SER(max);
    SST_SER(seed);
    SST_SER(tut);
    SST_SER(tutini);
    SST_SER(rng);
    SST_SER(subcompEnd);
}



SST::CPTSubComp::CPTSubCompPairOfStructs::CPTSubCompPairOfStructs(ComponentId_t id, Params &params)
{
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
        tut[i].first = struct_t{n};
        tut[i].second = struct_t{n*n};
        tutini[i].first = tut[i].first;
        tutini[i].second = tut[i].second;
    }
    subcompBegin = 0xcccb00000000bccc;
    subcompEnd = 0xccce00000000eccc;
}

SST::CPTSubComp::CPTSubCompPairOfStructs::~CPTSubCompPairOfStructs()
{
    if (rng) delete rng;
}

void SST::CPTSubComp::CPTSubCompPairOfStructs::setup()
{
    output.verbose(CALL_INFO, 2, 0, "setup() clocks %d check {%s} : {%s}\n", 
        clocks, tut[0].first.toString().c_str(), tut[0].second.toString().c_str());
}

void SST::CPTSubComp::CPTSubCompPairOfStructs::finish()
{
    output.verbose(CALL_INFO, 2, 0, 
        "finish() clocks %d check {%s} : {%s}\n", 
        clocks, tut[0].first.toString().c_str(), tut[0].second.toString().c_str()
    );
    if (check())
        output.fatal(CALL_INFO, -1, "final check failed\n");  
}

int SST::CPTSubComp::CPTSubCompPairOfStructs::check()
{
    for (size_t i=0;i<max; i++) {
        output.verbose(CALL_INFO, 3, 0, "Comparing %s %s\n", tut[i].first.toString().c_str(), tutini[i].first.toString().c_str());
        if (tut[i].first.u8 != tutini[i].first.u8 + (uint8_t)clocks) 
            return 1;
        if (tut[i].first.u16 != tutini[i].first.u16 + (uint16_t)clocks) 
            return 1;
        if (tut[i].first.u32 != tutini[i].first.u32 + (uint32_t)clocks) 
            return 1;
        if (tut[i].first.u64 != tutini[i].first.u64 + (uint32_t)clocks) 
            return 1;
        output.verbose(CALL_INFO, 3, 0, "Comparing %s %s\n", tut[i].second.toString().c_str(), tutini[i].second.toString().c_str());
        if (tut[i].second.u8 != tutini[i].second.u8 + (uint8_t)clocks) 
            return 1;
        if (tut[i].second.u16 != tutini[i].second.u16 + (uint16_t)clocks) 
            return 1;
        if (tut[i].second.u32 != tutini[i].second.u32 + (uint32_t)clocks) 
            return 1;
        if (tut[i].second.u64 != tutini[i].second.u64 + (uint32_t)clocks) 
            return 1;

    }
    return 0;
}

void SST::CPTSubComp::CPTSubCompPairOfStructs::update()
{
    clocks++;
    for (size_t i=0; i<max; i++) {
        tut[i].first++;
        tut[i].second++;
    }
}

void SST::CPTSubComp::CPTSubCompPairOfStructs::serialize_order(SST::Core::Serialization::serializer &ser)
{
    SST_SER(subcompBegin);
    SST_SER(output);
    SST_SER(clocks);
    SST_SER(max);
    SST_SER(seed);
    assert(tut.size()==tutini.size());
    for (size_t i=0;i<tut.size();i++) {
        #if 1
        // TODO how to deal with [-Werror,-Wpotentially-evaluated-expression] for schema
        // This will serialize but not deserialize
        // We can add type as an argument to the macro.
        struct_t tfirst = tut[i].first;
        struct_t tsecond = tut[i].second;
        struct_t tifirst = tut[i].first;
        struct_t tisecond = tut[i].second;
        SST_SER(tfirst);
        SST_SER(tsecond);
        SST_SER(tifirst);
        SST_SER(tisecond);
        #else
        SST_SER(tut[i].first);
        SST_SER(tut[i].second);
        SST_SER(tutini[i].first);
        SST_SER(tutini[i].second);
        #endif
    }
    SST_SER(rng);
    SST_SER(subcompEnd);
}
