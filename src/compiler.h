#pragma once

#include "vm.h"
#include "object.h"
ObjFunction* compile(const char* source, bool include);
void markCompilerRoots();