#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

int int_from_str(const char* a, size_t len) {
    int tens = 1;
    int accum = 0;
    for (int i = len - 1; i >= 0; i--) {
        accum += (a[i] - '0') * tens;
        tens *= 10;
    }
    return accum;
}

float float_from_str(const char* a, size_t len) {
    float accum = 0;
    int decimal = len, exponent = -1;
    float tens = 1;

    for (int i = 0; i < len; i++) {
        if (a[i] == '.') {
            decimal = i;
        }
        if (a[i] == 'e' || a[i] == 'E') {
            exponent = i; // TODO : ADD EXPONENT support
        }
    }

    for (int i = decimal - 1; i >= 0; i--) {
        accum += (a[i] - '0') * tens;
        tens *= 10;
    }

    tens = 0.1;
    for (int i = decimal + 1; i < len; i++) {
        accum += (a[i] - '0') * tens;
        tens /= 10;
    }

    return accum;
}

bool is_number(char c) {
    return '0' <= c && c <= '9';
}

typedef enum {
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_INT_LIT,
    TOKEN_FLOAT_LIT,
    TOKEN_EOF,
    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,

    TOKEN_COUNT // Make sure this token is the last one
                // listed in the enum, as this is assumed by compile time assertions
} TokenType;

const char *token_to_string_table[] = {
    [TOKEN_PLUS] = "+",
    [TOKEN_MINUS] = "-",
    [TOKEN_MULTIPLY] = "*",
    [TOKEN_DIVIDE] = "/",
    [TOKEN_OPEN_PAREN] = "(",
    [TOKEN_CLOSE_PAREN] = ")",
    [TOKEN_INT_LIT] = "integer literal",
    [TOKEN_FLOAT_LIT] = "float literal",
    [TOKEN_EOF] = "end of file"
};

static_assert(sizeof(token_to_string_table) / sizeof(char *) == TOKEN_COUNT,
              "Every token must have a corresponding entry in the token to string table, so add an entry probably");

// TODO: add row and column information here for more useful error messages
typedef struct {
    TokenType type;
    union {
        unsigned int int_lit;
        float float_lit;
    };
} Token;

void print_token(Token tok) {
    switch (tok.type) {
        case TOKEN_PLUS:        printf("+\n"); break;
        case TOKEN_MINUS:       printf("-\n"); break;
        case TOKEN_MULTIPLY:    printf("*\n"); break;
        case TOKEN_DIVIDE:      printf("/\n"); break;
        case TOKEN_INT_LIT:     printf("%u\n", tok.int_lit); break;
        case TOKEN_FLOAT_LIT:   printf("%.2f\n", tok.float_lit); break;
        case TOKEN_EOF:         printf("EOF\n"); break;
        case TOKEN_OPEN_PAREN:  printf("(\n"); break;
        case TOKEN_CLOSE_PAREN: printf(")\n"); break;
    }
}

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenArray;

#define array_append(array, item)\
    do {\
        if (array.count >= array.capacity) {\
            if (array.capacity == 0) array.capacity = 128;\
            array.capacity *= 256;\
            array.items = realloc(array.items, array.capacity * sizeof(*array.items));\
        }\
        array.items[array.count++] = item;\
    } while (0)

typedef struct {
    TokenArray tokens;
    int current;
    bool error;
} ParserState;

typedef enum {
    AST_BINARY_OP,
    AST_INT_LIT,
    AST_FLOAT_LIT,
} ASTType;

typedef enum {
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
} BinaryOp;

typedef struct AST {
    ASTType type;
    union {
        struct {
            BinaryOp op;
            struct AST *left;
            struct AST *right;
        };
        unsigned int int_lit;
        float float_lit;
    };
} AST;

int get_token_precedence_table[] = {
    [TOKEN_PLUS] = 1,
    [TOKEN_MINUS] = 1,
    [TOKEN_MULTIPLY] = 2,
    [TOKEN_DIVIDE] = 2
};

bool token_is_operator(TokenType type) {
    return type == TOKEN_PLUS     ||
           type == TOKEN_MINUS    ||
           type == TOKEN_MULTIPLY ||
           type == TOKEN_DIVIDE;
}

int get_token_precedence(TokenType type) {
    if (token_is_operator(type)) return get_token_precedence_table[type];
    return -1;
}

TokenType peek_next_token(ParserState *parser) {
    if (parser->current >= parser->tokens.count) return TOKEN_EOF;

    return parser->tokens.items[parser->current].type;
}

bool match_next_token(ParserState *parser, TokenType type) {
    if (peek_next_token(parser) == type) {
        return true;
    } else {
        printf("parsing error: expected \"%s\" but got \"%s\" instead\n",
                token_to_string_table[type],
                token_to_string_table[peek_next_token(parser)]);
        parser->error = true;
        return false;
    }
}

Token get_next_token(ParserState *parser) {
    if (parser->current >= parser->tokens.count) return parser->tokens.items[parser->tokens.count - 1];;
    return parser->tokens.items[parser->current++];
}

AST *parse_expression(ParserState *parser, int min_prec) {
    switch (peek_next_token(parser)) {
        case TOKEN_FLOAT_LIT:
        case TOKEN_INT_LIT:
        case TOKEN_OPEN_PAREN:
            break;
        default:
            printf("parsing error: expected \"integer literal\", \"float\", or \"(\" but got \"%s\" instead\n",
                    token_to_string_table[peek_next_token(parser)]);
            parser->error = true;
            return NULL;
    }

    Token tok = get_next_token(parser);

    AST *left = (AST *)malloc(sizeof(AST));

    switch (tok.type) {
        case TOKEN_INT_LIT:
            left->type = AST_INT_LIT;
            left->int_lit = tok.int_lit;
            break;
        case TOKEN_FLOAT_LIT:
            left->type = AST_FLOAT_LIT;
            left->float_lit = tok.float_lit;
            break;
        case TOKEN_OPEN_PAREN:
            free(left);
            left = parse_expression(parser, 0);
            if (parser->error) return NULL;

            if (!match_next_token(parser, TOKEN_CLOSE_PAREN)) return NULL;
            get_next_token(parser);
            break;            
    }

    while (get_token_precedence(peek_next_token(parser)) > min_prec) {
        AST *op = (AST *)malloc(sizeof(AST));
        op->type = AST_BINARY_OP;
        op->left = left;
        tok = get_next_token(parser);
        switch (tok.type) {
            case TOKEN_PLUS:
                op->op = OP_PLUS;
                break;
            case TOKEN_MINUS:
                op->op = OP_MINUS;
                break;
            case TOKEN_MULTIPLY:
                op->op = OP_MULTIPLY;
                break;
            case TOKEN_DIVIDE:
                op->op = OP_DIVIDE;
                break;
        }

        op->right = parse_expression(parser, get_token_precedence(tok.type));
        if (parser->error) return NULL;

        left = op;
    }

    return left;
};

AST *parse_program(ParserState *parser) {
    AST *ret = parse_expression(parser, 0);
    if (parser->error) return NULL;

    if (!match_next_token(parser, TOKEN_EOF)) {
        free(ret);
        return NULL;
    }

    return ret;
}

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
    // TODO: actually check for type here later when more AST types are added
}

const char *register_list[] = { "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12" };
bool register_free_list[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static_assert(sizeof(register_list) / sizeof(char *) == sizeof(register_free_list) / sizeof(register_free_list[0]),
              "The amount of registers and the size of the register free list must be the same. "
              "Add a new spot in the free list if you added a new register");

void generate_ast_assembly(FILE *file, AST *ast) {
    
}

int main() {
    FILE *f = fopen("hello.q", "r");

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);

    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc(fsize + 1);
    fread(buf, 1, fsize, f);
    fclose(f);

    buf[fsize] = '\0';
    size_t buf_size = fsize;

    printf("%s\n", buf);

    TokenArray tokens = {0};

    size_t start = 0;
    size_t next = 0;
    while (start < buf_size) {
        char c = buf[next++];

        Token tok = {0};

        switch (c) {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                break;

            case '+':
                tok.type = TOKEN_PLUS;
                array_append(tokens, tok);
                break;
            case '-':
                tok.type = TOKEN_MINUS;
                array_append(tokens, tok);
                break;
            case '*':
                tok.type = TOKEN_MULTIPLY;
                array_append(tokens, tok);
                break;
            case '/':
                tok.type = TOKEN_DIVIDE;
                array_append(tokens, tok);
                break;
            case '(':
                tok.type = TOKEN_OPEN_PAREN;
                array_append(tokens, tok);
                break;
            case ')':
                tok.type = TOKEN_CLOSE_PAREN;
                array_append(tokens, tok);
                break;

            default:
                if (is_number(c)) {
                    tok.type = TOKEN_INT_LIT;
                    int num_decimal_points = 0;
                    while (is_number(buf[next]) || buf[next] == '.') {
                        if (buf[next] == '.') {
                            tok.type = TOKEN_FLOAT_LIT;
                            num_decimal_points++;
                        }
                        next++;
                    }

                    // TODO: need to break out of tokenization if error
                    if (num_decimal_points > 1)
                        printf("tokenizing error: too many decimal points in float literal\n");

                    switch (tok.type) {
                        case TOKEN_FLOAT_LIT:
                            tok.float_lit = float_from_str(&buf[start], next - start);
                            break;
                        case TOKEN_INT_LIT:
                            tok.int_lit = int_from_str(&buf[start], next - start);
                            break;
                    }

                    array_append(tokens, tok);
                } else {
                    printf("tokenizing error: unrecognized character \"%c\"\n", c);
                }
                break;
        }

        start = next;
    }

    Token tok_eof = {0};
    tok_eof.type = TOKEN_EOF;
    array_append(tokens, tok_eof);

    for (int i = 0; i < tokens.count; i++) {
        print_token(tokens.items[i]);
    }
    printf("\n");

    ParserState parser = {0};
    parser.tokens = tokens;

    AST *ast = parse_program(&parser);

    if (!parser.error) {
        AST result = evaluate_ast(ast);
        switch (result.type) {
            case AST_BINARY_OP:
                printf("error");
                break;
            case AST_FLOAT_LIT:
                printf("float result: %f\n", result.float_lit);
                break;
            case AST_INT_LIT:
                printf("unsigned result: %u\n", result.int_lit);
                break;
        }
    }
}
