#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "gen/autogen.h"


// terminal text coloring stuff
#define CReset "\x1B[0m"
#define CRed  "\x1B[31m"
#define CGreen  "\x1B[32m"
#define CYellow  "\x1B[33m"
#define CBlue  "\x1B[34m"
#define CMagenta  "\x1B[35m"
#define CCyan  "\x1B[36m"
#define CWhite  "\x1B[37m"
#define CBrightRed "\x1B[91m"
#define CBrightYellow "\x1B[93m"
#define CBold "\x1B[1m"

// other

#define REPL_PROMPT "> "
#define AS_SOON_AS_WE_CAN 1

//#define DEBUG_PRINT_CODE
//#define DEBUG_TRACE_EXCEPTION
//#define DEBUG_STRESS_GC
// #define DEBUG_LOG_GC

#define FAST_MODULO(a1, a2) a1 & (a2 - 1)
#define NORMAL_MODULO(a1, a2) a1 % a2

#define UINT8_COUNT UINT8_MAX + 1