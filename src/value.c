#include "value.h"
#include "mem.h"
#include "string.h"
#include "object.h"
#include <stdio.h>


void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}


void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;    
}

void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}


void printValue(Value value) {
    switch(value.type) {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NULL: printf("null"); break;
        case VAL_NUMBER: {
            if (__builtin_isnan(AS_NUMBER(value))) {
                printf("NaN");
            } else if (__builtin_isinf(AS_NUMBER(value))) {
                printf("Inf");
            } else {
                printf("%g", AS_NUMBER(value)); 
            }
            break;
        }
        case VAL_OBJ: printObject(value); break;
    }
}

char* typeofValue(Value value) {
    switch(value.type) {
        case VAL_BOOL:
            return "boolean";
            break;
        case VAL_NULL: return "null"; break;
        case VAL_NUMBER: {
            return "number";
            break;
        }
        case VAL_OBJ: return typeofObject(value);
    }
    return "n/a";
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NULL: return true;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ: return AS_OBJ(a) == AS_OBJ(b);
        default: return false;
    }
}