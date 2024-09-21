#include "common.h"
#include "native.h"
#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>


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

// use the C inbuilt exit() function with the status code
static Value exitNative(int argCount, Value* args) {
    if (args == NULL) {
        fprintf(stdout, "warning: exiting with status 0 since no arguments received!\n");
        exit(0);
    } else {
        exit(AS_NUMBER(*args));
    }
}


// returns the program status that it executed
static Value runProgramNative(int argCount, Value* args) {
    int result = system(AS_CSTRING(args[0]));
    return NUMBER_VAL(result);
}

// read from the keyboard
static Value readKeyNative(int argCount, Value* args) {
    char in[1024];
    if (!fgets(in, sizeof(in), stdin)) {
        printf("\n");
    }
    Obj* a = copyString(in, (strlen(in) - 1));
    return OBJ_VAL(a);
}

void InitalizeBuiltins() {
    defineNative("_array_append", ArrayAppend);
    defineNative("_array_delete", ArrayDelete);
    defineNative("_inbuilt_exit", exitNative);
    defineNative("_inbuilt_binrun", runProgramNative);
    defineNative("_inbuilt_readk", readKeyNative);
}