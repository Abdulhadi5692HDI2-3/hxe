#pragma once

#include "vm.h"
#include "object.h"

ObjFunction* compile(ObjModule* module,  char* source);
void markCompilerRoots();