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

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node* lhs;
    Node* rhs;
    num_t num;
};

Node* new_node(NodeKind kind, Node* lhs, Node* rhs) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node* new_node_num(num_t num) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->num = num;
    return node;
}

/*
expr    = mul ("+" mul | "-" mul)*
mul     = primary ("*" primary | "/" primary)*
primary = num | "(" expr ")"
 */

/*
expr = num ("+" num | "-" num)*
 */

Node* expr() {
    Node* node = new_node_num(expect_num());

    while (true) {
        if (consume('+')) {
            node = new_node(ND_ADD, node, new_node_num(expect_num()));
        } else if (consume('-')) {
            node = new_node(ND_SUB, node, new_node_num(expect_num()));
        } else {
            return node;
        }
    }
}

void gen(Node* node) {
    if (node->kind == ND_NUM) {
        printf("\tpush %ld\n", node->num);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("\tpop rbx\n");
    printf("\tpop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("\tadd rax, rbx\n");
            break;
        case ND_SUB:
            printf("\tsub rax, rbx\n");
            break;
        default:
            error("unimplemented");
            break;
    }
    printf("\tpush rax\n");
}

int main(int argc, char** argv) {
    if (argc != 2) {
        error("not enough arguments, expected 1");
    }

    printf(".intel_syntax noprefix\n");
    printf(".globl _main\n");
    printf("_main:\n");

    user_input = argv[1];
    token = tokenize(user_input);
    Node* node = expr();

    gen(node);

    printf("\tpop rax\n");
    printf("\tret\n");
    return 0;
}
