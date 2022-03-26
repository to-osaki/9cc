#include "9cc.h"

int main(int argc, char **argv)
{
    if(argc != 2) {
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    NodePtr node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // abstract syntax tree to assembly
    gen(node);
    /*
    printf("  mov rax, %d\n", expect_number());
    while(!at_eof()) {
        if(consume("+")) {
            printf("  add rax, %d\n", expect_number());
            continue;
        }

        expect("-");
        printf("  sub rax, %d\n", expect_number());
    }
    */

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}