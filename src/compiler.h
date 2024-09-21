#pragma once

#include "vm.h"
#include "object.h"

ObjFunction* compile(ObjModule* module,  char* source);
void emitByte(uint8_t byte);
void emitBytes(uint8_t byte1, uint8_t byte2);
void markCompilerRoots();
uint8_t makeConstant(Value value);