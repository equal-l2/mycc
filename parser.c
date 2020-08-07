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

Token* consume_ident() {
    if (token->kind == TK_IDENT) {
        Token* tk = token;
        token = token->next;
        return tk;
    }
    return NULL;
}

void expect(char* s) {
    if (!consume(s)) {
        error_at(token->str, "not '%s'", s);
    }
}

int expect_num() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "not a number");
    }
    int ret = token->num;
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
        const char * const pos = cur->str == head.str ? cur->str : cur->str+1;
        if (isspace(*p)) {
            p++;
            continue;
        }
        if ('a' <= *p && *p <= 'z') {
            int len = 1;
            while ('a' <= *(p+len) && *(p+len) <= 'z') len++;
            cur = new_token(TK_IDENT, cur, p);
            cur->len = len;
            p += len;
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->num = strtol(p, &p, 10);
            continue;
        }
        switch (*p) {
            case '=':
                if (*(p+1) == '=') {
                    cur = new_token(TK_RESERVED, cur, p);
                    cur->len = 2;
                    p += 2;
                } else {
                    cur = new_token(TK_RESERVED, cur, p++);
                    cur->len = 1;
                }
                break;
            case '!':
                if (*(p+1) != '=') {
                    error_at(pos, "not '='");
                } else {
                    cur = new_token(TK_RESERVED, cur, p);
                    cur->len = 2;
                    p += 2;
                }
                break;
            case '<':
            case '>':
                if (*(p+1) == '=') {
                    cur = new_token(TK_RESERVED, cur, p);
                    cur->len = 2;
                    p += 2;
                    break;
                }
            case '+':
            case '-':
            case '*':
            case '/':
            case '(':
            case ')':
            case ';':
                cur = new_token(TK_RESERVED, cur, p++);
                cur->len = 1;
                break;
            default:
                error_at(pos, "unexpected character");
        }
    }

    new_token(TK_EOF, cur, p);

    return head.next;
}

void debug_node(NodeKind k) {
    switch (k) {
        case ND_ADD:    eprintf("ND_ADD\n");    break;
        case ND_SUB:    eprintf("ND_SUB\n");    break;
        case ND_MUL:    eprintf("ND_MUL\n");    break;
        case ND_DIV:    eprintf("ND_DIV\n");    break;
        case ND_NUM:    eprintf("ND_NUM\n");    break;
        case ND_EQU:    eprintf("ND_EQU\n");    break;
        case ND_NEQ:    eprintf("ND_NEQ\n");    break;
        case ND_ELT:    eprintf("ND_ELT\n");    break;
        case ND_LET:    eprintf("ND_LET\n");    break;
        case ND_ASSIGN: eprintf("ND_ASSIGN\n"); break;
        case ND_LVAR:   eprintf("ND_LVAR\n");   break;
    }
}

Node* new_node(NodeKind kind, Node* lhs, Node* rhs) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node* new_node_num(int num) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->num = num;
    return node;
}

typedef struct LVar LVar;

struct LVar {
    LVar* next;
    char* name;
    int len;
    int offset;
};

LVar* locals;

int get_offset(void) {
    return (locals ? locals->offset : 0);
}

LVar* find_lvar(Token* tk) {
    LVar* p = locals;
    while (p) {
        if (p->len == tk->len && !memcmp(p->name, tk->str, tk->len)) return p;
        p = p->next;
    }
    return NULL;
}

/*
program    = stmt*
stmt       = expr ";"
expr       = assign
assign     = equ ("=" assign)?
equ        = rel ("==" rel | "!=" rel)*
rel        = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | ident | "(" expr ")"
 */

Node* expr(void);

Node* primary(void) {
    Node* node;
    if (consume("(")) {
        node = expr();
        expect(")");
    } else {
        Token* tk = consume_ident();
        if (tk) {
            node = calloc(1, sizeof(Node));
            node->kind = ND_LVAR;

            LVar* lvar = find_lvar(tk);

            if (lvar) {
                node->offset = lvar->offset;
            } else {
                lvar = calloc(1, sizeof(LVar));
                lvar->name = tk->str;
                lvar->len = tk->len;
                lvar->offset = node->offset = (locals ? locals->offset + 8 : 8);
                lvar->next = locals;
                locals = lvar;
            }
        } else {
            node = new_node_num(expect_num());
        }
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

Node* assign(void) {
    Node* node = equ();

    while (true) {
        if (consume("=")) {
            node = new_node(ND_ASSIGN, node, assign());
        } else {
            return node;
        }
    }
}

Node* expr(void) {
    return assign();
}

Node* stmt(void) {
    Node* node = expr();
    expect(";");
    return node;
}

Node* code[100];

void program(void) {
    int i = 0;
    while (!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

