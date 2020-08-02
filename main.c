#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>


typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

typedef long num_t;

struct Token {
    TokenKind kind;
    Token* next;
    num_t num;
    char* str;
};

Token* token;

void error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

void debug_tk(TokenKind tk) {
    switch (tk) {
        case TK_RESERVED: fputs("TK_RESERVED\n", stderr); break;
        case TK_NUM: fputs("TK_NUM\n", stderr); break;
        case TK_EOF: fputs("TK_EOF\n", stderr); break;
    }
}

bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char op) {
    if (!consume(op)) {
        error("not '%c'", op);
    }
}

num_t expect_num() {
    if (token->kind != TK_NUM) {
        error("not a number");
    }
    num_t ret = token->num;
    token = token->next;
    return ret;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Token* new_token(TokenKind kind, Token* cur, char* str) {
    //debug_tk(kind);
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token* tokenize(char* p) {
    Token head;
    head.next = NULL;
    Token* cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->num = strtol(p, &p, 10);
            continue;
        }
        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        error("unexpected character '%c'", *p);
    }

    new_token(TK_EOF, cur, p);

    return head.next;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        error("not enough arguments, expected 1");
    }

    printf(".intel_syntax noprefix\n");
    printf(".globl _main\n");
    printf("_main:\n");

    char* p = argv[1];
    token = tokenize(p);


    printf("\tmov rax, %ld\n", expect_num());

    while (!at_eof()) {
        if (consume('+')) {
            printf("\tadd rax, %ld\n", expect_num());
        } else if (consume('-')) {
            printf("\tsub rax, %ld\n", expect_num());
        } else {
            error("unexpected character '%c'", token->str[0]);
        }
    }

    printf("\tret\n");
    return 0;
}
