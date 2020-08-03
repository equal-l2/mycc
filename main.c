#include "mycc.h"

extern char* user_input;
extern Token* token;

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
