#include "mycc.h"

void gen_lval(Node* node) {
    if (node->kind != ND_LVAR) {
        error("variable expected");
    }
    printf("\tmov rax, rbp      # load base ptr\n");
    printf("\tsub rax, %d       # substract offset\n", node->offset);
    printf("\tpush rax          # push resulting address\n");
}

static int cnt_else;
static int cnt_end;
static int cnt_begin;

void gen(Node* node) {
    int _cnt_else = cnt_else;
    int _cnt_end = cnt_end;
    int _cnt_begin = cnt_begin;
    switch (node->kind) {
        case ND_NUM:
            printf("\tpush %d\n", node->num);
            return;
        case ND_LVAR:
            puts("# begin ND_LVAR");
            gen_lval(node);
            printf("\tpop rax           # load address\n");
            printf("\tmov rax, [rax]    # load value\n");
            printf("\tpush rax          # push resulting value\n");
            puts("# end ND_LVAR");
            return;
        case ND_ASSIGN:
            puts("# begin ND_ASSIGN");
            gen_lval(node->lhs);
            gen(node->rhs);
            printf("\tpop rcx           # load lhs value\n");
            printf("\tpop rax           # load rhs value\n");
            printf("\tmov [rax], rcx    # place value to addr's pointing\n");
            printf("\tpush rcx          # push rhs (for chained assignment)\n");
            puts("# end ND_ASSIGN");
            return;
        case ND_RET:
            puts("# begin ND_RET");
            gen(node->lhs);
            printf("\tpop rax      # load expr value\n");
            printf("\tmov rsp, rbp # restore stack ptr\n");
            printf("\tpop rbp      # restore base ptr\n");
            printf("\tret\n");
            puts("# end ND_RET");
            return;
        case ND_IF:
            gen(node->cond);
            cnt_else++;
            cnt_end++;
            printf("\tpop rax\n");
            printf("\tcmp rax, 0\n");
            printf("\tje .Lelse%d\n", _cnt_else);
            printf("  #If body\n");
            gen(node->lhs);
            printf("\tjmp .Lend%d\n", _cnt_end);
            printf(".Lelse%d:\n", _cnt_else);
            if (node->rhs) gen(node->rhs);
            printf(".Lend%d:\n", _cnt_end);
            return;
        case ND_WHILE:
            puts("# begin ND_WHILE");
            cnt_begin++;
            cnt_end++;
            printf(".Lbegin%d:\n", _cnt_begin);
            gen(node->cond);
            printf("\tpop rax\n");
            printf("\tcmp rax, 0\n");
            printf("\tje .Lend%d\n", _cnt_end);
            gen(node->lhs);
            printf("\tjmp .Lbegin%d\n", _cnt_begin);
            printf(".Lend%d:\n", _cnt_end);
            puts("# end ND_WHILE");
            return;
        case ND_FOR:
            puts("# begin ND_FOR");
            cnt_begin++;
            cnt_end++;
            if (node->init) gen(node->init);
            printf(".Lbegin%d:\n", _cnt_begin);
            if (node->cond) {
                gen(node->cond);
                printf("\tpop rax\n");
                printf("\tcmp rax, 0\n");
                printf("\tje .Lend%d\n", _cnt_end);
            }
            gen(node->lhs);
            if (node->iter) gen(node->iter);
            printf("\tjmp .Lbegin%d\n", _cnt_begin);
            printf(".Lend%d:\n", _cnt_end);
            puts("# end ND_FOR");
            return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("\tpop rcx           # load lhs value\n");
    printf("\tpop rax           # load rhs value\n");

    switch (node->kind) {
        case ND_ADD:
            printf("\tadd rax, rcx\n");
            break;
        case ND_SUB:
            printf("\tsub rax, rcx\n");
            break;
        case ND_MUL:
            printf("\timul rax, rcx\n");
            break;
        case ND_DIV:
            printf("\tcqo\n");          // extend rax to rdx:rax
            printf("\tidiv rcx\n");     // divide rdx:rax by rcx
            break;
        case ND_REM:
            printf("\tcqo\n");
            printf("\tidiv rcx\n");
            printf("\tmov rax, rdx\n");
            break;
        case ND_EQU:
            printf("\tcmp rax, rcx\n");
            printf("\tsete al\n");       // move e flag-register to al (operand must be a byte register)
            printf("\tmovzx rax, al\n"); // store al to rax with zero expansion
            break;
        case ND_NEQ:
            printf("\tcmp rax, rcx\n");
            printf("\tsetne al\n");
            printf("\tmovzx rax, al\n");
            break;
        case ND_LET:
            printf("\tcmp rax, rcx\n");
            printf("\tsetl al\n");
            printf("\tmovzx rax, al\n");
            break;
        case ND_ELT:
            printf("\tcmp rax, rcx\n");
            printf("\tsetle al\n");
            printf("\tmovzx rax, al\n");
            break;
        default:
            debug_node(node->kind);
            error("unimplemented");
            break;
    }
    printf("\tpush rax\n");
}

