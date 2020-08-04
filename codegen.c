#include "mycc.h"

void gen_lval(Node* node) {
    if (node->kind != ND_LVAR) {
        error("variable expected");
    }
    printf("\tmov rax, rbp\n");              // load base ptr
    printf("\tsub rax, %d\n", node->offset); // sub offset
    printf("\tpush rax\n");                  // push addr of var
}

void gen(Node* node) {
    switch (node->kind) {
        case ND_NUM:
            printf("\tpush %d\n", node->num);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("\tpop rax\n");        // load addr of var
            printf("\tmov rax, [rax]\n"); // load var value
            printf("\tpush rax\n");       // push value
            return;
        case ND_ASSIGN:
            printf("  #assign\n");
            gen_lval(node->lhs);
            gen(node->rhs);
            printf("\tpop rbx\n");       // load rhs value
            printf("\tpop rax\n");       // load lhs addr
            printf("\tmov [rax], rbx\n"); // assign value to addr's pointing
            printf("\tpush rbx\n");      // push rhs (for chained assign)
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
            printf("\tcqo\n");          // extend rax to rdx:rax
            printf("\tidiv rbx\n");     // divide rdx:rax by rbx
            break;
        case ND_EQU:
            printf("\tcmp rax, rbx\n");
            printf("\tsete al\n");      // move e flag-register to al (operand must be a byte register)
            printf("\tmovzx rax, al\n");
            break;
        case ND_NEQ:
            printf("\tcmp rax, rbx\n");
            printf("\tsetne al\n");
            printf("\tmovzx rax, al\n");
            break;
        case ND_LET:
            printf("\tcmp rax, rbx\n");
            printf("\tsetl al\n");
            printf("\tmovzx rax, al\n");
            break;
        case ND_ELT:
            printf("\tcmp rax, rbx\n");
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

