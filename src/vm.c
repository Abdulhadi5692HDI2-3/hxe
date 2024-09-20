#include "vm.h"
#include "compiler.h"
#include <stdio.h>
#ifdef WIN32
    #include <windows.h>
#endif
#ifdef Linux
    #include <unistd.h>
#endif
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "mem.h"

#include "object.h"

#include <stdarg.h>
#include "debug.h"
#include "main.h"

#define YOUFUCKEDUP NUMBER_VAL(1)

VM vm;

static void runtimeError(const char* format, ...);

// alias (due to some idiotic mistake i made in early dev of this)
#define nativeFuncError runtimeError
static void resetStack();

// clock
static Value clockNative(int argCount, Value* args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

// useless
static Value _testNative(int argCount, Value* args) {
    return *args;
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
    if (!isValidArrayIndex(args, index)) {
        nativeFuncError("Array index is invalid! (assumption: probably an out of\nbounds value you are trying to access)");
    }
    deleteFromArray(arr, index);
    return NULL_VAL;
}
// returns the error code from the program being run.
static Value runprocNative(int argCount, Value* args) {
    int result = system(AS_CSTRING(*args));
    return NUMBER_VAL(result);
}

// use the C inbuilt exit() function with the status code
static Value exitNative(int argCount, Value* args) {
    if (args == NULL) {
        printf("warning: exiting with status 0 since no arguments received!\n");
        exit(0);
    } else {
        exit(AS_NUMBER(*args));
    }
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
// file operations

// usage: newFile(filename)
static Value newFileNative(int argCount, Value* args) {
    FILE* fptr;
    fptr = fopen(AS_CSTRING(args[0]), "w");
    fclose(fptr);
}

// usage: writeFile(filename, <stuff to write>)
// note: this just appends to a file! (or creates one if it hasnt already existed)
// if you want to overwrite use unsafeWriteFileNative (todo: implement that)
static Value writeFileNative(int argCount, Value* args) {
    FILE* fptr;
    fptr = fopen(AS_CSTRING(args[0]), "a");
    fprintf(fptr, AS_CSTRING(args[1]));
    fclose(fptr);
}

static Value writeFilewithNewlineNative(int argCount, Value* args) {
    FILE* fptr;
    fptr = fopen(AS_CSTRING(args[0]), "a");
    char* b = strcat(AS_CSTRING(args[1]), "\n");
    fprintf(fptr, b);
    fclose(fptr);
}
// usage: readFile(filename)
// note: this returns the characters read in a string
static Value readFileNative(int argCount, Value* args) {
    FILE* fptr;
    fptr = fopen(AS_CSTRING(args[0]), "r");
    if (fptr == NULL) {
        return NULL_VAL;
    }
    char* resultBuffer;
    fgets(resultBuffer, strlen(AS_CSTRING(args[0])), fptr);
    fclose(fptr);
    Obj* a = copyString(resultBuffer, strlen(resultBuffer));
    return OBJ_VAL(a);
}

// uses sleep() to pause for an amount of seconds
static Value waitNative(int argCount, Value* args){
    sleep(AS_NUMBER(*args));
    return NULL_VAL;
}

static void resetStack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.openUpvalues = NULL;
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }
    resetStack();
}

static void defineNative(const char* name, NativeFn func) {
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(func)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

static void rmQuotes(char* line, int lineLength) {
    int j = 0;
    for (int i = 0; i < lineLength; i ++) {
        if (line[i] != '"' && line[i] != '\\') { 
            line[j++] = line[i];
        } else if (line[i+1] == '"' && line[i] == '\\') { 
            line[j++] = '"';
    } else if (line[i+1] != '"' && line[i] == '\\') { 
            line[j++] = '\\';
        }
    }
    if(j>0) line[j]=0;
}

static void nativeFunctions() {
    defineNative("clock", clockNative);
    defineNative("_test", _testNative);
    defineNative("getk", readKeyNative);
    defineNative("syswait", waitNative);
    defineNative("sysrun", runprocNative);
    defineNative("sysexit", exitNative);
    defineNative("writeFile", writeFileNative);
    defineNative("writeFileWithnewline", writeFilewithNewlineNative);
    defineNative("readFile", readFileNative);
    defineNative("newFile", newFileNative);
    defineNative("append", ArrayAppend);
    defineNative("delete", ArrayDelete);
}


void initVM(VM nvm) {
    if (!&nvm) {
        fprintf(stderr, "\nerror: invalid vm param passed over!");
        exit(74);
    }
    vm = nvm;
    resetStack();
    vm.objects = NULL;
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;
    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;
    vm.stdlibLoaded = false;

    initTable(&vm.globals);
    initTable(&vm.strings);
    initTable(&vm.modules);

    vm.initString = NULL;
    vm.initString = copyString("init", 4);
    nativeFunctions();
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    freeTable(&vm.modules);
    vm.initString = NULL;
    freeObjects();
}

void push(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}
Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

static Value peek(int dist) {
    return vm.stackTop[-1 - dist];
}

static bool call(ObjClosure* closure, int argCount) {
    if (argCount != closure->function->arity) {
        runtimeError("Expected %d params but got %d.", closure->function->arity, argCount);
        return false;
    }
    if (vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }
    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}
static bool callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch(OBJ_TYPE(callee)) {
            case OBJ_BOUND_METHOD: {
                ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
                vm.stackTop[-argCount - 1] = bound->receiver;
                return call(bound->method, argCount);
            }
            case OBJ_CLASS: {
                ObjClass* kclass = AS_CLASS(callee);
                vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(kclass));
                Value initalizer;
                if (tableGet(&kclass->methods, vm.initString, &initalizer)) {
                    return call(AS_CLOSURE(initalizer), argCount);
                } else if (argCount != 0) {
                    runtimeError("Expected 0 args but got %d args!", argCount);
                    return false;
                }
                return true;
            }
            case OBJ_CLOSURE:
                return call(AS_CLOSURE(callee), argCount);
            case OBJ_NATIVE:
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
                vm.stackTop -= argCount - 1;
                push(result);
                return true;
            default:
                break;
        }
    }
    runtimeError("Can only call functions and classes!");
    return false;
}

static bool invokeFromClass(ObjClass* kclass, ObjString* name, int argCount) {
    Value method;
    if (!tableGet(&kclass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }
    return call(AS_CLOSURE(method), argCount);
}
static bool invoke(ObjString* name, int argCount) {
    Value receiver = peek(argCount);

    if (!IS_INSTANCE(receiver)) {
        runtimeError("Only instances have methods!");
        return false;
    }
    ObjInstance* instance = AS_INSTANCE(receiver);
    Value value;
    if (tableGet(&instance->fields, name, &value)) {
        vm.stackTop[-argCount - 1] = value;
        return callValue(value, argCount);
    }
    return invokeFromClass(instance->kclass, name, argCount);
}
static bool bindMethod(ObjClass* kclass, ObjString* name) {
    Value method;
    if (!tableGet(&kclass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }
    ObjBoundMethod* bound = newBoundMethod(peek(0), AS_CLOSURE(method));
    pop();
    push(OBJ_VAL(bound));
    return true;
}
static ObjUpvalue* captureUpvalue(Value* local) {
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    ObjUpvalue* createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;
    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }
    return createdUpvalue;
}

static void closeUpvalues(Value* last) {
    while (vm.openUpvalues != NULL && vm.openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}

static void defineMethod(ObjString* name) {
    Value method = peek(0);
    ObjClass* kclass = AS_CLASS(peek(1));
    tableSet(&kclass->methods, name, method);
    pop();
}
static bool isFalsey(Value value) {
    return IS_NULL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
    ObjString* b = AS_STRING(peek(0));
    ObjString* a = AS_STRING(peek(1));

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}

static InterpretResult run();

InterpretResult interpret(const char* source, bool include) {
    ObjString* name = copyString("__script__", strlen("__script__"));
    push(OBJ_VAL(name));
    ObjModule* module = newModule(name);
    pop();

    push(OBJ_VAL(module));
    module->path = "";
    pop();

    ObjFunction* function = compile(module, source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    call(closure, 0);
    return run();
}

static InterpretResult run() {
    CallFrame* frame  = &vm.frames[vm.frameCount - 1];
    #define READ_BYTE() (*frame->ip++)
    #define READ_CONSTANT() \
        (frame->closure->function->chunk.constants.values[READ_BYTE()])
    #define READ_SHORT() \
        (frame->ip += 2, \
        (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
    #define READ_STRING() AS_STRING(READ_CONSTANT())
    #define BINARY_OP(valueType, op) \
        do { \
            if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
                runtimeError("Operands must be NUMBERS!"); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            double b = AS_NUMBER(pop()); \
            double a = AS_NUMBER(pop()); \
            push(valueType(a op b)); \
        } while(false)

    for (;;) {
        #ifdef DEBUG_TRACE_EXCEPTION
            printf("          ");
            for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
                printf("[ ");
                printValue(*slot);
                printf(" ]");
            }
            printf("\n");
            disassembleInstruction(&frame->closure->function->chunk,
                (int)(frame->ip - frame->closure->function->chunk.code));
        #endif
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NULL: push(NULL_VAL); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    runtimeError("Must be two numbers or two strings!");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_DIVIDE: BINARY_OP(NUMBER_VAL, /); break;
            case OP_NOT: {
                push(BOOL_VAL(isFalsey(pop())));
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_NEGATE: {
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number!");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            }
            case OP_PRINT: {
                printValue(pop());
                printf("\n");
                break;
            }
            case OP_POP: pop(); break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                if (tableSet(&vm.globals, name, peek(0))) {
                    tableDelete(&vm.globals, name);
                    runtimeError("Undefined variable %s", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Undefined variable %s", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                tableSet(&vm.globals, name, peek(0));
                pop();
                break;
            }
            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push(*frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek(0);
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) frame->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }
            case OP_CALL: {
                int argCount = READ_BYTE();
                if (!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                if (!invoke(method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_SUPER_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                ObjClass* superclass = AS_CLASS(pop());
                if (!invokeFromClass(superclass, method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_CLOSURE: {
                ObjFunction* func = AS_FUNCTION(READ_CONSTANT());
                ObjClosure* closure = newClosure(func);
                push(OBJ_VAL(closure));
                for (int i = 0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if (isLocal) {
                        closure->upvalues[i] = captureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }
            case OP_CLASS: {
                push(OBJ_VAL(newClass(READ_STRING())));
                break;
            }
            case OP_CLOSE_UPVALUE: {
                closeUpvalues(vm.stackTop - 1);
                pop();
                break;
            }
            case OP_GET_PROPERTY: {
                if (!IS_INSTANCE(peek(0))) {
                    runtimeError("Only instances have properties!");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek(0));
                ObjString* name = READ_STRING();

                Value value;
                if (tableGet(&instance->fields, name, &value)) {
                    pop();
                    push(value);
                    break;
                }
                if (!bindMethod(instance->kclass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SET_PROPERTY: {
                if (!IS_INSTANCE(peek(1))) {
                    runtimeError("Only instances have fields!");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek(1));
                tableSet(&instance->fields, READ_STRING(), peek(0));
                Value value = pop();
                pop();
                push(value);
                break;
            }
            case OP_GET_SUPER: {
                ObjString* name = READ_STRING();
                ObjClass* superclass = AS_CLASS(pop());

                if (!bindMethod(superclass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_METHOD: {
                defineMethod(READ_STRING());
                break;
            }
            case OP_INHERIT: {
                Value superclass = peek(1);
                if (!IS_CLASS(superclass)) {
                    runtimeError("Superclass must be a class!");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjClass* subclass = AS_CLASS(peek(0));
                tableAddAll(&AS_CLASS(superclass)->methods, &subclass->methods);
                pop();
                break;
            }
            case OP_MODULE: {
                ObjString* filename = READ_STRING();
                Value moduleVal;

                char path[2048];
                strcpy(path, filename->chars);
                // remove the god damn quotes
                rmQuotes(path, strlen(path));

                ObjString* pathObj = copyString(path, strlen(path));
                push(OBJ_VAL(pathObj));

                if (tableGet(&vm.modules, pathObj, &moduleVal)) {
                    pop();
                    vm.lastModule = AS_MODULE(moduleVal);
                    push(NULL_VAL);
                    break;
                }

                char* source = readFile(path);
                if (!source) {
                    runtimeError("Failed to open and read module!");
                }

                ObjModule* mod = newModule(pathObj);
                mod->path = path;
                vm.lastModule = mod;
                pop();
                push(OBJ_VAL(mod));
                ObjFunction* function = compile(mod, source);
                pop();

                free(source);

                if (function == NULL) return INTERPRET_COMPILE_ERROR;
                push(OBJ_VAL(function));
                ObjClosure* closure = newClosure(function);
                pop();
                push(OBJ_VAL(closure));

                call(closure, 0);
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_MODULO: {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    runtimeError("Operands must be NUMBERS!");
                    return INTERPRET_RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                int c = FAST_MODULO((int)a, (int)b);
                push(NUMBER_VAL(c));
                break;
            }
            case OP_BUILD_ARRAY: {
                ObjArray* array = newArray();
                uint8_t itemCount = READ_BYTE();

                push(OBJ_VAL(array));
                for (int i = itemCount; i > 0; i--) {
                    appendToArray(array, peek(i));
                }
                pop();

                while(itemCount-- > 0) {
                    pop();
                }
                push(OBJ_VAL(array));
                break;
            }
            case OP_INDEX_SUBSCRIPT: {
                Value index = pop();
                Value list = pop();
                Value result;

                if (!IS_ARRAY(list)) {
                    runtimeError("Invalid type to index into.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjArray* array = AS_ARRAY(list);

                if (!IS_NUMBER(index)) {
                    runtimeError("List index is not a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                int index_ = AS_NUMBER(index);
                /*
                if (!isValidArrayIndex(array, index_)) {
                    runtimeError("List index out of range.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                */
               if (!isValidArrayIndex(array, index_)) {
                    push(NULL_VAL);
                    break;
                }

                result = getArrayItem(array, AS_NUMBER(index));
                push(result);
                break;
            }
            case OP_STORE_SUBSCRIPT: {
                Value item = pop();
                Value index = pop();
                Value list = pop();

                if (!IS_ARRAY(list)) {
                    runtimeError("Cannot store value in a non-array.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjArray* array = AS_ARRAY(list);

                if (!IS_NUMBER(index)) {
                    runtimeError("Array index is not a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                int index_ = AS_NUMBER(index);

                if (!isValidArrayIndex(array, index_)) {
                    runtimeError("Invalid array index.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                storeToArray(array, index_, item);
                push(item);
                break;
            }
            case OP_RETURN:
                Value result = pop();
                closeUpvalues(frame->slots);
                vm.frameCount--;
                if (vm.frameCount == 0) {
                    pop();
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                push(result);
                frame = &vm.frames[vm.frameCount - 1];
                break;
        }
    }

    #undef READ_BYTE
    #undef READ_SHORT
    #undef READ_CONSTANT
    #undef READ_STRING
    #undef BINARY_OP
    #undef DEBUG_BINARY_OP
}