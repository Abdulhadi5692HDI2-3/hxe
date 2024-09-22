#include "common.h"
#include "native.h"
#include "unistd.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// alias (due to some idiotic mistake i made in early dev of this)
#define nativeFuncError(n) fprintf(stderr, "\n%s\n", n)

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

// SYSTEM FUNCTIONS

// use the C inbuilt exit() function with the status code
static Value exitNative(int argCount, Value* args) {
    if (args == NULL) {
        fprintf(stdout, "warning: exiting with status 0 since no arguments received!\n");
        exit(0);
    } else {
        exit(AS_NUMBER(*args));
    }
}

// uses sleep() to pause for an amount of seconds
static Value waitNative(int argCount, Value* args){
    sleep(AS_NUMBER(*args));
    return NULL_VAL;
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

// FILE FUNCTIONS

// usage: readFile(filename)
// note: this returns the characters read in a string
static Value readFileNative(int argCount, Value* args) {
    char* content = readFile(AS_CSTRING(args[0]), true);
    ObjString* returner = copyString(content, strlen(content));
    free(content);
    return OBJ_VAL(returner);
}

// usage: writeFile(filename, <stuff to write>)
static Value writeFileNative(int argCount, Value* args) {
    FILE* fptr;
    fptr = fopen(AS_CSTRING(args[0]), "w");
    fprintf(fptr, AS_CSTRING(args[1]));
    fclose(fptr);
}

// usage: appendFile(filename, <stuff to write>)
static Value appendFileNative(int argCount, Value* args) {
    FILE* fptr;
    fptr = fopen(AS_CSTRING(args[0]), "a");
    fprintf(fptr, AS_CSTRING(args[1]));
    fclose(fptr);
}


// random functions
static Value randNative(int argCount, Value* args) {
    return NUMBER_VAL(rand());
}

static Value srandNative(int argCount, Value* args) {
    srand(AS_NUMBER(args[0]));
    return NULL_VAL;
}

// time functions
static Value timecNative(int argCount, Value* args) {
    if (argCount != 0) {
        nativeFuncError("Incorrect number of arguments!");
        return NULL_VAL;
    }
    return NUMBER_VAL(time((long)AS_NUMBER(args[0])));
}

// other
static Value chkzeroNative(int argCount, Value* args) {
    if (argCount != 1) {
        nativeFuncError("Incorrect number of arguments!");
        return NULL_VAL;
    }
    return NUMBER_VAL(strlen(AS_CSTRING(args[0])));
}

// math helpers
static Value getNaN_native(int argCount, Value* args) {
    return NUMBER_VAL(__builtin_nan("0xf0f0f0"));
}

static Value isNaN_native(int argCount, Value* args) {
    return BOOL_VAL(__builtin_isnan(AS_NUMBER(args[0])));
}

static Value isInf_native(int argCount, Value* args) {
    return BOOL_VAL(__builtin_isinf(AS_NUMBER(args[0])));
}

static Value trunc_native(int argCount, Value* args) {
    long unsigned input;
    memcpy(&input, &AS_NUMBER(args[0]), 8);
    int exop = ((input >> 52) & 0x7ff) - 1023;
    if (exop >= 0) {
        int fraction = 52 - exop;
        if (fraction > 0) {
            long unsigned int integral_mask = 0xffffffffffffffff << fraction;
            long unsigned int output = input & integral_mask;
            memcpy(&AS_NUMBER(args[0]), &output, 8);
        }
    } else {
        AS_NUMBER(args[0]) = AS_NUMBER(args[0]) < 0 ? -0.0 : 0.0;
    }
    return args[0];
}

static Value asineplNative(int argCount, Value* args) {
    return NUMBER_VAL(1e-16l);
}

static Value powepsNative(int argCount, Value* args) {
    return NUMBER_VAL(1e-100l);
}

void InitalizeBuiltins() {
    defineNative("_array_append", ArrayAppend);
    defineNative("_array_delete", ArrayDelete);
    defineNative("_inbuilt_exit", exitNative);
    defineNative("_inbuilt_wait", waitNative);
    defineNative("_inbuilt_binrun", runProgramNative);
    defineNative("_inbuilt_readk", readKeyNative);
    defineNative("_inbuilt_fwrite", writeFileNative);
    defineNative("_inbuilt_fappend", appendFileNative);
    defineNative("_inbuilt_fread", readFileNative);
    defineNative("_inbuilt_rand", randNative);
    defineNative("_inbuilt_srand", srandNative);
    defineNative("_inbuilt_time", timecNative);
    defineNative("_inbuilt_strlen", chkzeroNative);
    defineNative("_inbuilt_nan", getNaN_native);
    defineNative("_inbuilt_isnan", isNaN_native);
    defineNative("_inbuilt_isinf", isInf_native);
    defineNative("_inbuilt_trunc", trunc_native);
    defineNative("_inbuilt_asin_epl", asineplNative);
    defineNative("_inbuilt_pow_eps", powepsNative);
}