#include "mycc.h"
#include <ctype.h>
#include <string.h>

char *user_input;

void error_at(const char* loc, char* fmt, ...) {
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


Token* token;

void debug_tk(TokenKind tk) {
    switch (tk) {
        case TK_RESERVED: eprintf("TK_RESERVED\n"); break;
        case TK_NUM: eprintf("TK_NUM\n"); break;
        case TK_EOF: eprintf("TK_EOF\n"); break;
    }
}

bool consume(char* s) {
    if (token->kind != TK_RESERVED || strlen(s) != token->len || memcmp(s, token->str, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char* s) {
    if (!consume(s)) {
        error_at(token->str, "not '%s'", s);
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

Token* new_token(TokenKind kind, Token* cur, char* str, size_t len) {
    //debug_tk(kind);
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

Token* tokenize(char* p) {
    Token head;
    head.next = NULL;
    head.str = p;
    Token* cur = &head;

    while (*p) {
        const char * const pos = cur->str == head.str ? cur->str : cur->str+1;
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->num = strtol(p, &p, 10);
            continue;
        }
        switch (*p) {
            case '=':
            case '!':
                if (*(p+1) != '=') {
                    error_at(pos, "unexpected character");
                }
                cur = new_token(TK_RESERVED, cur, p, 2);
                p += 2;
                break;
            case '<':
            case '>':
                if (*(p+1) == '=') {
                    cur = new_token(TK_RESERVED, cur, p, 2);
                    p += 2;
                    break;
                }
            case '+':
            case '-':
            case '*':
            case '/':
            case '(':
            case ')':
                cur = new_token(TK_RESERVED, cur, p++, 1);
                break;
            default:
                error_at(pos, "unexpected character");
        }
    }

    new_token(TK_EOF, cur, p, 0);

    return head.next;
}

void debug_node(NodeKind k) {
    switch (k) {
        case ND_ADD: eprintf("ND_ADD\n");break;
        case ND_SUB: eprintf("ND_SUB\n");break;
        case ND_MUL: eprintf("ND_MUL\n");break;
        case ND_DIV: eprintf("ND_DIV\n");break;
        case ND_NUM: eprintf("ND_NUM\n");break;
        case ND_EQU: eprintf("ND_EQU\n");break;
        case ND_NEQ: eprintf("ND_NEQ\n");break;
        case ND_ELT: eprintf("ND_ELT\n");break;
        case ND_LET: eprintf("ND_LET\n");break;
    }
}

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
expr    = equ
equ     = rel ("==" rel | "!=" rel)*
rel     = add ("<=" add | ">=" add | "<" add | ">" add)*
add     = mul ("+" mul | "-" mul)*
mul     = unary ("*" unary | "/" unary)*
unary   = ('+' | '-')? primary
primary = num | "(" expr ")"
 */

Node* add(void);

Node* rel(void) {
    Node* node = add();

    while (true) {
        if (consume(">=")) {
            // a >= b -> b <= a
            node = new_node(ND_ELT, add(), node);
        } else if (consume(">")) {
            // a > b -> b < a
            node = new_node(ND_LET, add(), node);
        } else if (consume("<=")) {
            node = new_node(ND_ELT, node, add());
        } else if (consume("<")) {
            node = new_node(ND_LET, node, add());
        } else {
            return node;
        }
    }
}

Node* equ(void) {
    Node* node = rel();

    while (true) {
        if (consume("==")) {
            node = new_node(ND_EQU, node, rel());
        } else if (consume("!=")) {
            node = new_node(ND_NEQ, node, rel());
        } else {
            return node;
        }
    }
}

Node* expr(void) {
    return equ();
}

Node* num(void) {
    return new_node_num(expect_num());
}

Node* primary(void) {
    Node* node;
    if (consume("(")) {
        node = expr();
        expect(")");
    } else {
        node = num();
    }
    return node;
}

Node* unary(void) {
    if (consume("+")) {
        // +x => x
        return primary();
    } else if (consume("-")) {
        // -x => 0-x
        return new_node(ND_SUB, new_node_num(0), primary());
    } else {
        // x => x
        return primary();
    }
}

Node* mul(void) {
    Node* node = unary();
    while (true) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node* add(void) {
    Node* node = mul();

    while (true) {
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

