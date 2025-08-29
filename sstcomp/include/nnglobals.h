#ifndef _NNGLOBALS_H
#define _NNGLOBALS_H

#ifndef INITIAL_WEIGHT_SCALING
#define INITIAL_WEIGHT_SCALING 0.1
#endif

#ifndef DEFAULT_LEARNING_RATE
#define DEFAULT_LEARNING_RATE 0.001
#endif

static const bool g_debug = false;
static const bool g_useDropout = false;
static const bool g_smallsim = false;
static const bool g_shuffle = true;
static const bool g_randFromFile = true;

#endif //_NNGLOBALS_H