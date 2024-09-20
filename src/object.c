#include "object.h"
#include <stdio.h>
#include <string.h>

#include "mem.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* obj = (Obj*)reallocate(NULL, 0, size);
    obj->type = type;

    obj->next = vm.objects;
    obj->isMarked = false;

    vm.objects = obj;
#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)obj, size, type);
#endif
    return obj;
}


ObjModule* newModule(ObjString* name) {
    Value moduleVal;
    if (tableGet(&vm.modules, name, &moduleVal)) {
        return AS_MODULE(moduleVal);
    }

    ObjModule* module = ALLOCATE_OBJ(ObjModule, OBJ_MODULE);
    initTable(&module->values);
    module->name = name;
    module->path = NULL;
    push(OBJ_VAL(module));
    ObjString* __file__ = copyString("__file__", 8);
    push(OBJ_VAL(__file__));

    tableSet(&module->values, __file__, OBJ_VAL(name));
    tableSet(&vm.modules, name, OBJ_VAL(module));
    pop();
    pop();
    return module;
}
// if i'm being honest
// i feel like i should've just used an ValueArray*
// instead implementing this all over again
// TODO: fix that once school is over
// also i did mostly use code from: https://calebschoepp.com/blog/2020/adding-a-list-data-type-to-lox/#:~:text=To%20implement%20lists%20we%20will,do%20not%20add%20more%20opcodes.
// TODO: try and organize this. perhaps even move this to a new file.
ObjArray* newArray() {
    ObjArray* arr = ALLOCATE_OBJ(ObjArray, OBJ_ARRAY);
    arr->elements = NULL;
    arr->count = 0;
    arr->capacity = 0;
    return arr;
}

void appendToArray(ObjArray* arr, Value value) {
    if (arr->capacity < arr->count + 1) {
        int oldCapacity = arr->capacity;
        arr->capacity = GROW_CAPACITY(oldCapacity);
        arr->elements = GROW_ARRAY(Value, arr->elements, oldCapacity, arr->capacity);
    }
    arr->elements[arr->count] = value;
    arr->count++;
    return;
}

void storeToArray(ObjArray* arr, int loc,  Value value) {
    arr->elements[loc]= value;
}

// this gets an value from the array from loc
Value getArrayItem(ObjArray* arr, int loc) {
    return arr->elements[loc];
}

void deleteFromArray(ObjArray* arr, int loc) {
    for (int i = loc; i < arr->count - 1; i++) {
        arr->elements[i] = arr->elements[i + 1];
    }
    arr->elements[arr->count - 1] = NULL_VAL;
    arr->count--;
}

bool isValidArrayIndex(ObjArray* arr, int loc) {
    if (loc < 0 || loc > arr->count - 1) {
        return false;
    }
    return true;
}

ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method) {
    ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}
ObjClass* newClass(ObjString* name) {
    ObjClass* kclass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    kclass->name = name;
    initTable(&kclass->methods);
    return kclass;
}
ObjInstance* newInstance(ObjClass* kclass) {
    ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->kclass = kclass;
    initTable(&instance->fields);
    return instance;
}
ObjNative* newNative(NativeFn func) {
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = func;
    return native;
}

ObjClosure* newClosure(ObjFunction* function) {
    ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upValueCount);
    for (int i = 0; i < function->upValueCount; i++) {
        upvalues[i] = NULL;
    }
    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upValueCount;
    closure->function = function;
    return closure;
}

ObjFunction* newFunction() {
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->name = NULL;
    function->upValueCount = 0;
    initChunk(&function->chunk);
    return function;
}

bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

static ObjString* allocateString(char* chars, int length, uint32_t hash) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    push(OBJ_VAL(string));
    tableSet(&vm.strings, string, NULL_VAL);
    pop();
    return string;
}

static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

ObjString* takeString(char* chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString* interned = tableFindString(&vm.strings, chars, length,
                                        hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}
ObjString* copyString(const char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = tableFindString(&vm.strings, chars, length,
                                        hash);
    if (interned != NULL) return interned;
    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash);
}

ObjUpvalue* newUpvalue(Value* slot) {
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->closed = NULL_VAL;
    upvalue->location = slot;
    upvalue->next = NULL;
    return upvalue;
}

static void printFunction(ObjFunction* function) {
    if (function->name == NULL) {
        printf("<program>");
        return;
    }
    printf("<func %s>", function->name->chars);
}
void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_ARRAY: {
            printf("<array>");
            break;
        }
        case OBJ_BOUND_METHOD:
            printFunction(AS_BOUND_METHOD(value)->method->function);
            break;
        case OBJ_CLASS:
            printf("%s", AS_CLASS(value)->name->chars);
            break;
        case OBJ_CLOSURE:
            printFunction(AS_CLOSURE(value)->function);
            break;
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;
        case OBJ_INSTANCE:
            printf("%s instance", AS_INSTANCE(value)->kclass->name->chars);
            break;
        case OBJ_NATIVE:
            printf("<inbuilt func>");
            break;
        case OBJ_UPVALUE:
            printf("<upvalue>");
            break;
    }
}