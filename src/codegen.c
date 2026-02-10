#include <assert.h>

#include "codegen.h"

const char *register_list[][4] = {
    [BIT_8]  = { "al",  "bl",  "cl",  "dl" },
    [BIT_16] = { "ax",  "bx",  "cx",  "dx", },
    [BIT_32] = { "eax", "ebx", "ecx", "edx" },
    [BIT_64] = { "rax", "rbx", "rcx", "rdx" }
};

bool register_free_list[] = { 0, 0, 0, 0 };

int allocate_register() {
    for (int i = 0; i < (int)(sizeof(register_free_list) / sizeof(register_free_list[0])); i++) {
        if (register_free_list[i] == 0) {
            register_free_list[i] = 1;
            return i;
        }
    }

    assert(0);
    return -1;
}

void free_register(int reg) {
    register_free_list[reg] = 0;
}

void generate_load_register_with_int(FILE *file, int reg, int val) {
    // TODO: Perhaps it might be better to have the register allocation happen within the code generation functions
    // I'm not sure about freeing however. Let's see how codegen goes
    fprintf(file, "\tmov\t%s, %d\n", register_list[BIT_32][reg], val);
}

void generate_add_registers(FILE *file, int reg1, int reg2) {
    fprintf(file, "\tadd\t%s, %s\n", register_list[BIT_32][reg1], register_list[BIT_32][reg2]);
}

Loc generate_ast_assembly(FILE *file, AST *ast) {
    assert(ast->type != AST_FLOAT_LIT);

    if (ast->type == AST_INT_LIT) {
        int reg = allocate_register();
        generate_load_register_with_int(file, reg, ast->int_lit);
        Loc loc = {
            .type = LOC_REGISTER,
            .register_index = reg
        };
        return loc;
    }

    Loc loc_left = generate_ast_assembly(file, ast->left);
    Loc loc_right = generate_ast_assembly(file, ast->right);

    switch (ast->op) {
        case OP_PLUS:
            generate_add_registers(file, loc_left.register_index, loc_right.register_index);
            free_register(loc_right.register_index);
            break;
        case OP_MINUS:    
            break;
        case OP_MULTIPLY:
            break;
        case OP_DIVIDE: 
            break;
    }
    
    return (Loc){0};
}
