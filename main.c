#include "9cc.h"

int main(int argc, char **argv)
{
    if(argc != 2) {
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    program(); // into g_code;

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // prologue : reserve localvar
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n"); // extend stack 8 * (a to z)

    for(int i = 0; g_code[i] != NULL; ++i) {
        gen(g_code[i]);
        printf("  pop rax\n"); // pop value of statement evaluated
    }

    // abstract syntax tree to assembly
    //gen(node);
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

    // epilogue
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}