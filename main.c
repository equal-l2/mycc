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

    user_input = argv[1];
    token = tokenize(user_input);
    program();

    printf("# prologue\n");
    printf("\tpush rbp       # save base ptr\n");
    printf("\tmov rbp, rsp   # set base to current stack ptr\n");
    printf("\tsub rsp, %-4d  # allocate area for variables\n", get_offset());

    int i = 0;
    while(code[i] != NULL) {
        printf("# begin code[%d]\n", i);
        gen(code[i++]);
        printf("\tpop rax\n");
        printf("# end code[%d]\n", i);
    }

    printf("# epilogue\n");
    printf("\tmov rsp, rbp # restore stack ptr\n");
    printf("\tpop rbp      # restore base ptr\n");
    printf("\tret\n");
    return 0;
}
