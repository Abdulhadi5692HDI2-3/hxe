#pragma once
#include "common.h"
#include "chunk.h"
#include "value.h"
#include "table.h"
#include "object.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    ObjClosure* closure;
    uint8_t* ip;
    Value* slots;
} CallFrame;
typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;

    Value stack[STACK_MAX];
    Value* stackTop;
    Table globals;
    Table strings;


    ObjModule* lastModule;
    Table modules;

    ObjString* initString;
    ObjUpvalue* openUpvalues;

    size_t bytesAllocated;
    size_t nextGC;
    
    Obj* objects;
    int grayCount;
    int grayCapacity;
    Obj** grayStack;

    bool stdlibLoaded;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
}InterpretResult;
extern VM vm;

void initVM(VM nvm);
void freeVM();
InterpretResult interpret(const char* source, const char* file);
void push(Value value);
Value pop();
void runtimeError(const char* format, ...);