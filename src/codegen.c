#include "codegen.h"

const char *register_list[][4] = {
    [BITS_8]  = { "al",  "bl",  "cl",  "dl" },
    [BITS_16] = { "ax",  "bx",  "cx",  "dx", },
    [BITS_32] = { "eax", "ebx", "ecx", "edx" },
    [BITS_64] = { "rax", "rbx", "rcx", "rdx" }
};

bool register_free_list[] = { 0, 0, 0, 0 };

void generate_ast_assembly(FILE *file, AST *ast) {
}