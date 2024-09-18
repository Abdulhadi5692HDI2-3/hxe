#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "gen/autogen.h"

#define MAX_STDLIB_DIR_PATH 255
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXCEPTION
#define INCLUDE_STDLIB_HC
//#define DEBUG_STRESS_GC
// #define DEBUG_LOG_GC

#define FAST_MODULO(a1, a2) a1 & (a2 - 1)
#define NORMAL_MODULO(a1, a2) a1 % a2

#define UINT8_COUNT UINT8_MAX + 1