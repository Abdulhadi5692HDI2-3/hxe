#include "stdlibhxe.h"
#include "common.h"
#include "compiler.h"
#include "object.h"
#include "main.h"

#include <stdio.h>
#include <string.h>

extern char* currentFile;
// @param loadFrom The function expects the param to have a forward slash '/' at teh end
void InitalizeStdLib(const char* loadFrom) {
    char arr[2048];
    char* save = currentFile;
    strcpy(arr, "");
    strcat(arr, loadFrom);

    // load the  library System (provides System.*)
    strcat(arr, "system.hxe");
    currentFile = "system.hxe";
    int index = makeConstant(OBJ_VAL(copyString(arr, strlen(arr))));
    emitBytes(OP_MODULE, index);
    emitByte(OP_POP);
    currentFile = save;

    // load the library Array (provides function to append and to delete from Arrays)
    strcpy(arr, "");
    strcat(arr, loadFrom);
    strcat(arr, "array.hxe");
    currentFile = "array.hxe";
    index = makeConstant(OBJ_VAL(copyString(arr, strlen(arr))));
    emitBytes(OP_MODULE, index);
    emitByte(OP_POP);
    currentFile = save;
}
