#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stdbool.h>
#include "AST.h"

typedef enum {
    BITS_8,
    BITS_16,
    BITS_32,
    BITS_64,
} RegisterSizes;

extern const char *register_list[][4];
extern bool register_free_list[];

void generate_ast_assembly(FILE *file, AST *ast);

#endif