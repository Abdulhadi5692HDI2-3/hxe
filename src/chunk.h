#pragma once
#include "common.h"

#include "value.h"

typedef enum {
    OP_MODULE,
    OP_BUILD_ARRAY,
    OP_INDEX_SUBSCRIPT,
    OP_STORE_SUBSCRIPT,
    OP_MODULO,
    OP_INHERIT,
    OP_METHOD,
    OP_CLASS,
    OP_CLOSURE,
    OP_RETURN,
    OP_NEGATE,
    OP_PRINT,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_SUPER_INVOKE,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_SET_GLOBAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_UPVALUE,
    OP_GET_UPVALUE,
    OP_CLOSE_UPVALUE,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_GET_SUPER,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_CONSTANT,
} OpCode;

typedef struct {
    int count;
    int capacity;
    int* lines;
    uint8_t* code;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);