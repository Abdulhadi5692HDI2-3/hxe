#pragma once

#include "common.h"
typedef struct Obj Obj;
typedef struct ObjString ObjString;




#define AS_OBJ(val) ((val).as.obj)
#define AS_BOOL(val) ((val).as.boolean)
#define AS_NUMBER(val) ((val).as.number)

#define IS_BOOL(val) ((val).type == VAL_BOOL)
#define IS_NULL(val) ((val).type == VAL_NULL)
#define IS_NUMBER(val) ((val).type == VAL_NUMBER)
#define IS_OBJ(val) ((val).type == VAL_OBJ)


#define BOOL_VAL(val) ((Value){VAL_BOOL, {.boolean = val}})
#define NULL_VAL ((Value){VAL_NULL, {.number = 0}})
#define NUMBER_VAL(val) ((Value){VAL_NUMBER, {.number = val}})
#define OBJ_VAL(val) ((Value){VAL_OBJ, {.obj = val}})

typedef enum {
    VAL_BOOL,
    VAL_NULL,
    VAL_NUMBER,
    VAL_OBJ,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
} Value;


typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;


void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);
char* typeofValue(Value value);
bool valuesEqual(Value a, Value b);