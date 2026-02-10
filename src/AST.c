#include "AST.h"

AST evaluate_ast(AST *ast) {
    switch (ast->type) {
        case AST_BINARY_OP:
            {
            AST ret = {0};
            AST left = evaluate_ast(ast->left);
            AST right = evaluate_ast(ast->right);

            ret.type = (left.type == AST_INT_LIT && right.type == AST_FLOAT_LIT || left.type == AST_FLOAT_LIT && right.type) ? AST_FLOAT_LIT : AST_INT_LIT; // promotion

            switch (ret.type) {
                case AST_INT_LIT:
                switch (ast->op) {
                    case OP_PLUS:
                        ret.int_lit = (unsigned int) ((left.type == AST_FLOAT_LIT ? left.float_lit : left.int_lit) + (right.type == AST_FLOAT_LIT ? right.float_lit : right.int_lit));
                        break;
                    case OP_MINUS:
                        ret.int_lit = (unsigned int) ((left.type == AST_FLOAT_LIT ? left.float_lit : left.int_lit) - (right.type == AST_FLOAT_LIT ? right.float_lit : right.int_lit));
                        break;
                    case OP_MULTIPLY:
                        ret.int_lit = (unsigned int) ((left.type == AST_FLOAT_LIT ? left.float_lit : left.int_lit) * (right.type == AST_FLOAT_LIT ? right.float_lit : right.int_lit));
                        break;
                    case OP_DIVIDE:
                        ret.int_lit = (unsigned int) ((left.type == AST_FLOAT_LIT ? left.float_lit : left.int_lit) / (right.type == AST_FLOAT_LIT ? right.float_lit : right.int_lit));
                        break;
                }
                break;
                case AST_FLOAT_LIT:
                switch (ast->op) {
                    case OP_PLUS:
                        ret.float_lit = (float) ((left.type == AST_FLOAT_LIT ? left.float_lit : left.int_lit) + (right.type == AST_FLOAT_LIT ? right.float_lit : right.int_lit));
                        break;
                    case OP_MINUS:
                        ret.float_lit = (float) ((left.type == AST_FLOAT_LIT ? left.float_lit : left.int_lit) - (right.type == AST_FLOAT_LIT ? right.float_lit : right.int_lit));
                        break;
                    case OP_MULTIPLY:
                        ret.float_lit = (float) ((left.type == AST_FLOAT_LIT ? left.float_lit : left.int_lit) * (right.type == AST_FLOAT_LIT ? right.float_lit : right.int_lit));
                        break;
                    case OP_DIVIDE:
                        ret.float_lit = (float) ((left.type == AST_FLOAT_LIT ? left.float_lit : left.int_lit) / (right.type == AST_FLOAT_LIT ? right.float_lit : right.int_lit));
                        break;
                }
                break;
                }
                return ret;
            }
        case AST_FLOAT_LIT:
        case AST_INT_LIT:
            return (*ast);
        default:
            return (AST){0};
    }
}
