#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
int eprintf(const char* fmt, ...);
void error(const char* fmt, ...);

typedef long num_t;

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token* next;
    num_t num;
    char* str;
    size_t len;
};

Token* tokenize(char* p);
void debug_token(TokenKind);

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_EQU,
    ND_NEQ,
    ND_ELT,
    ND_LET,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node* lhs;
    Node* rhs;
    num_t num;
};

Node* expr(void);
void debug_node(NodeKind);

void gen(Node*);
