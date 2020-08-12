#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
int eprintf(const char* fmt, ...);
void error(const char* fmt, ...);

// parser
typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token* next;
    int num;
    char* str;
    int len;
};

Token* tokenize(char* p);
void debug_token(TokenKind);

int get_offset(void);

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_REM,
    ND_NUM,
    ND_EQU,
    ND_NEQ,
    ND_ELT,
    ND_LET,
    ND_ASSIGN,
    ND_LVAR,
    ND_RET,
    ND_IF,
    ND_WHILE,
    ND_FOR,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node* lhs;
    Node* rhs;
    int num;
    int offset;

    Node* cond;

    Node* init;
    Node* iter;

};

void program(void);
void debug_node(NodeKind);

// codegen
void gen(Node*);
