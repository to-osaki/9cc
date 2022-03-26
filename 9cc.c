#include<stdio.h>
#include<stdlib.h>

#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct _Token *TokenPtr;

typedef struct _Token {
    TokenKind kind;
    TokenPtr next;
    int val;
    char *str;
} Token;

TokenPtr token; // current token
char *user_input; // source code

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;

    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char op) {
    if(token->kind != TK_RESERVED || token->str[0] != op) 
        return false;

    token = token->next;
    return true;
}

void expect(char op) {
    if(token->kind != TK_RESERVED || token->str[0] != op) 
        error_at(token->str, "'%c' is not '%c'", token->str[0], op);
    token = token->next;
}

int expect_number() {
    if(token->kind != TK_NUM)
        error_at(token->str, "'%c' is not a number", token->kind);
    
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p) {
        if(isspace(*p)) {
            ++p;
            continue;
        }
        else if(*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }
        else if(isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }
        else {
            error_at(cur->str, "failed to tokenize.");
        }
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv)
{
    if(argc != 2) {
        return 1;
    }

    token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    printf("  mov rax, %d\n", expect_number());

    while(!at_eof()) {
        if(consume('+')) {
            printf("  add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("  sub rax, %d\n", expect_number());
    }

    printf("  ret\n");
    return 0;
}