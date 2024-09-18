#pragma once


#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"

#define OBJ_TYPE(val) (AS_OBJ(val)->type)

#define IS_ARRAY(val) isObjType(val, OBJ_ARRAY)
#define IS_BOUND_METHOD(val) isObjType(val, OBJ_BOUND_METHOD)
#define IS_CLASS(val) isObjType(val, OBJ_CLASS)
#define IS_NATIVE(val) isObjType(val, OBJ_NATIVE)
#define IS_CLOSURE(val) isObjType(val, OBJ_CLOSURE)
#define IS_FUNCTION(val) isObjType(val, OBJ_FUNCTION)
#define IS_INSTANCE(val) isObjType(val, OBJ_INSTANCE)
#define IS_STRING(val) isObjType(val, OBJ_STRING)

#define AS_ARRAY(val) ((ObjArray*)AS_OBJ(val))
#define AS_BOUND_METHOD(val) ((ObjBoundMethod*)AS_OBJ(val))
#define AS_CLASS(val) ((ObjClass*)AS_OBJ(val))
#define AS_CLOSURE(val) ((ObjClosure*)AS_OBJ(val))
#define AS_NATIVE(val) (((ObjNative*)AS_OBJ(val))->function)
#define AS_FUNCTION(val) ((ObjFunction*)AS_OBJ(val))
#define AS_INSTANCE(val) ((ObjInstance*)AS_OBJ(val))

#define AS_STRING(val) ((ObjString*)AS_OBJ(val))
#define AS_CSTRING(val) (((ObjString*)AS_OBJ(val))->chars)

typedef enum {
    OBJ_BOUND_METHOD,
    OBJ_CLASS,
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_INSTANCE,
    OBJ_NATIVE,
    OBJ_CLOSURE,
    OBJ_UPVALUE,
    OBJ_ARRAY,
} ObjType;

struct Obj {
    ObjType type;
    struct Obj* next;
    bool isMarked;
};

typedef struct {
    Obj obj;
    int arity;
    Chunk chunk;
    int upValueCount;
    ObjString* name;
} ObjFunction;
typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
    Obj obj;
    NativeFn function;
}ObjNative;

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

typedef struct _ObjUpvalue {
    Obj obj;
    Value* location;
    Value closed;
    struct _ObjUpvalue* next;
} ObjUpvalue;

typedef struct {
    Obj obj;
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalueCount;
} ObjClosure;

typedef struct {
    Obj obj;
    ObjString* name;
    Table methods;
} ObjClass;

typedef struct {
    Obj obj;
    ObjClass* kclass;
    Table fields;
} ObjInstance;

typedef struct {
    Obj obj;
    Value receiver;
    ObjClosure* method;
} ObjBoundMethod;

typedef struct {
    Obj obj;
    int count;
    int capacity;
    Value* elements;
} ObjArray;

ObjArray* newArray();
void appendToArray(ObjArray* arr, Value value);
void storeToArray(ObjArray* arr, int loc,  Value value);
Value getArrayItem(ObjArray* arr, int loc);
void deleteFromArray(ObjArray* arr, int loc);
bool isValidArrayIndex(ObjArray* arr, int loc);

ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
ObjClass* newClass(ObjString* name);
ObjInstance* newInstance(ObjClass* kclass);
ObjUpvalue* newUpvalue(Value* slot);
ObjClosure* newClosure(ObjFunction* function);
ObjNative* newNative(NativeFn func);
ObjFunction* newFunction();
ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);

bool isObjType(Value value, ObjType type);
void printObject(Value value);