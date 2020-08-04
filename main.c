#include "mycc.h"

extern char* user_input;
extern Token* token;
extern Node* code[];

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

    // prologue(allocate area for variables)
    printf(" #prologue\n");
    printf("\tpush rbp\n");
    printf("\tmov rbp, rsp\n");
    printf("\tsub rsp, %d\n", 26*8);

    user_input = argv[1];
    token = tokenize(user_input);
    program();

    int i = 0;
    while(code[i] != NULL) {
        printf(" #code[%d]\n", i);
        gen(code[i++]);
        printf("\tpop rax\n");
    }

    // epilogue
    printf(" #epilogue\n");
    printf("\tmov rsp, rbp\n");
    printf("\tpop rbp\n");
    printf("\tret\n");
    return 0;
}
