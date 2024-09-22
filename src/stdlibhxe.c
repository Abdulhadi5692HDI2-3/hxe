#include "stdlibhxe.h"
#include "common.h"
#include "compiler.h"
#include "object.h"
#include "main.h"

#include <stdio.h>
#include <string.h>

extern char* currentFile;


// @param loadFrom note: needs to have a forward slash
// @param fileName file name of the library
void LoadHxeFile(const char* loadFrom, const char* fileName) {
    char arr[2048];
    char* save = currentFile;
    strcpy(arr, "");
    strcat(arr, loadFrom);

    // load the library
    strcat(arr, fileName);
    currentFile = fileName;
    int index = makeConstant(OBJ_VAL(copyString(arr, strlen(arr))));
    emitBytes(OP_MODULE, index);
    emitByte(OP_POP);
    currentFile = save;
}

// @param loadFrom The function expects the param to have a forward slash '/' at teh end
void InitalizeStdLib(const char* loadFrom) {
    LoadHxeFile(loadFrom, "system.hxe");
    LoadHxeFile(loadFrom, "array.hxe");
    LoadHxeFile(loadFrom, "file.hxe");
    LoadHxeFile(loadFrom, "random.hxe");
    LoadHxeFile(loadFrom, "string.hxe");
    LoadHxeFile(loadFrom, "math.hxe");
}
