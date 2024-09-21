#include "common.h"
#include "native.h"


// alias (due to some idiotic mistake i made in early dev of this)
#define nativeFuncError runtimeError

void defineNative(const char* name, NativeFn func) {
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(func)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

// array etc functions
static Value ArrayAppend(int argCount, Value* args) {
    if (argCount != 2 || !IS_ARRAY(args[0])) {
        bool a = argCount != 2;
        if (a) {nativeFuncError("argument Count is not correct!");return NULL_VAL;}
        a = !IS_ARRAY(args[0]);
        if (a) {nativeFuncError("argument isn't an array!"); return NULL_VAL;}
    }
    ObjArray* arr = AS_ARRAY(args[0]);
    Value item = args[1];
    appendToArray(arr, item);
    return NULL_VAL;
}

static Value ArrayDelete(int argCount, Value* args) {
    if (argCount != 2 || !IS_ARRAY(args[0]) || !IS_NUMBER(args[1])) {
        nativeFuncError("Invalid arguments! (can't determine wtf went wrong tho)");
        return NULL_VAL;
    }
    ObjArray* arr = AS_ARRAY(args[0]);
    int index = AS_NUMBER(args[1]);
    if (!isValidArrayIndex(AS_ARRAY(args[0]), index)) {
        nativeFuncError("Array index is invalid! (assumption: probably an out of\nbounds value you are trying to access)");
    }
    deleteFromArray(arr, index);
    return NULL_VAL;
}


void InitalizeBuiltins() {
    defineNative("_array_append", ArrayAppend);
    defineNative("_array_delete", ArrayDelete);
}