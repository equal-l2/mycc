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

int eprintf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vfprintf(stderr, fmt, ap);
    va_end(ap);
    return ret;
}

void error(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    eprintf("\n");
    exit(1);
}

char *user_input;

void error_at(char* loc, char* fmt, ...) {
  int pos = loc - user_input;
  eprintf("pos: %d\n", pos);

  eprintf("%s\n", user_input);
  eprintf("%*s", pos, ""); // pos個の空白を出力
  eprintf("^ ");

  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);

  eprintf("\n");
  exit(1);
}

void debug_tk(TokenKind tk) {
    switch (tk) {
        case TK_RESERVED: eprintf("TK_RESERVED\n"); break;
        case TK_NUM: eprintf("TK_NUM\n"); break;
        case TK_EOF: eprintf("TK_EOF\n"); break;
    }
}

bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

num_t expect_num() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "not a number");
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
    head.str = p;
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

        error_at(cur->str == head.str ? cur->str : cur->str+1, "unexpected character");
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

    user_input = argv[1];
    char* p = user_input;
    token = tokenize(p);

    printf("\tmov rax, %ld\n", expect_num());

    while (!at_eof()) {
        if (consume('+')) {
            printf("\tadd rax, %ld\n", expect_num());
        } else if (consume('-')) {
            printf("\tsub rax, %ld\n", expect_num());
        } else {
            error_at(token->str, "unexpected character");
        }
    }

    printf("\tret\n");
    return 0;
}
