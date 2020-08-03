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

bool consume(char c) {
    if (token->kind != TK_RESERVED || token->str[0] != c) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char c) {
    if (!consume(c)) {
        error_at(token->str, "not '%c'", c);
    }
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
        switch (*p) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '(':
            case ')':
                cur = new_token(TK_RESERVED, cur, p++);
                break;
            default:
                error_at(cur->str == head.str ? cur->str : cur->str+1, "unexpected character");
        }
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
mul     = unary ("*" unary | "/" unary)*
unary   = ('+' | '-')? primary
primary = num | "(" expr ")"
 */

Node* expr(void);

Node* num(void) {
    return new_node_num(expect_num());
}

Node* primary(void) {
    Node* node;
    if (consume('(')) {
        node = expr();
        expect(')');
    } else {
        node = num();
    }
    return node;
}

Node* unary(void) {
    if (consume('+')) {
        return primary();
    } else if (consume('-')) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}

Node* mul(void) {
    Node* node = unary();
    while (true) {
        if (consume('*')) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume('/')) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node* expr(void) {
    Node* node = mul();

    while (true) {
        if (consume('+')) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume('-')) {
            node = new_node(ND_SUB, node, mul());
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
        case ND_MUL:
            printf("\timul rax, rbx\n");
            break;
        case ND_DIV:
            printf("\tcqo\n");
            printf("\tidiv rbx\n");
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
