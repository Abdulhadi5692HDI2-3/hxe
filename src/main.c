#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "chunk.h"
#include "debug.h"
#include "vm.h"

static void repl() {
    printf("hxe (built from crafting interpreters) %s built binary: %s at %s\n", hxe_version, hxe_blddate, hxe_bldtime);
    char line[1024];
    for (;;) {
        printf(REPL_PROMPT);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        interpret(line, "<repl>");
    }
}

char* readFile(const char* path, bool exitOnFail) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file %s\n", path);
        if (exitOnFail) {
            exit(74);
        }
        return NULL;
    }
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read file!\n");
        if (exitOnFail) {
            exit(74);
        }
        return NULL;
        
    }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        if (exitOnFail) {
            exit(74);
        }
        return NULL;
    }
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

void runFile(const char* path) {
    char* source = readFile(path, path);
    InterpretResult result = interpret(source, false);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}


int main(int argc, const char* argv[]) {
    VM root;
    initVM(root);
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: hexec <path>\nNote: passing no arguments will drop you into the REPL ;)\n");
    }
    freeVM();
    return 0;
}

