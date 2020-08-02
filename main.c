#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "not enough arguments, expected 1\n");
        return 1;
    }


    printf(".intel_syntax noprefix\n");
    printf(".globl _main\n");
    printf("_main:\n");

    char* p = argv[1];
    printf("\tmov rax, %ld\n", strtol(p, &p, 10));

    while (*p != '\0') {
        switch (*p) {
            case '+': {
                          p++;
                          printf("\tadd rax, %ld\n", strtol(p, &p, 10));
                          break;
                      }
            case '-': {
                          p++;
                          printf("\tsub rax, %ld\n", strtol(p, &p, 10));
                          break;
                      }
            default: {
                         fprintf(stderr, "unimplemented operator \"%c\"\n", *p);
                         return 1;
                     }
        }
    }

    printf("\tret\n");
    return 0;
}
