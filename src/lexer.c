#include "lexer.h"

const char *token_to_string_table[] = {
    [TOKEN_PLUS] = "+",
    [TOKEN_MINUS] = "-",
    [TOKEN_MULTIPLY] = "*",
    [TOKEN_DIVIDE] = "/",
    [TOKEN_OPEN_PAREN] = "(",
    [TOKEN_CLOSE_PAREN] = ")",
    [TOKEN_INT_LIT] = "integer literal",
    [TOKEN_FLOAT_LIT] = "float literal",
    [TOKEN_EQUALS] = "=",
    [TOKEN_EOF] = "end of file"
};

static_assert(sizeof(token_to_string_table) / sizeof(char *) == TOKEN_COUNT,
              "Every token must have a corresponding entry in the token to string table, so add an entry probably");

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
        case TOKEN_EQUALS:      printf("=\n"); break;
    }
}