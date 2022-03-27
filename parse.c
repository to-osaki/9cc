#include "9cc.h"

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

bool consume(char *op) {
    if(
        token->kind != TK_RESERVED || 
        strlen(op) != token->len || 
        memcmp(token->str, op, token->len) != 0)
    {
        return false;
    }
    else
    {
        token = token->next;
        return true;
    }
}

void expect(char *op) {
    if(
        token->kind != TK_RESERVED || 
        strlen(op) != token->len || 
        memcmp(token->str, op, token->len) != 0)
    {
        error_at(token->str, "TokenKind [%d] is not [%s] v=%d l=%d s=%s", token->kind, op, token->val, token->len, token->str);
    }
    token = token->next;
}

int expect_number() {
    if(token->kind != TK_NUM)
        error_at(token->str, "TokenKind [%d] is not a number. v=%d l=%d s=%s", token->kind, token->val, token->len, token->str);
    
    int val = token->val;
    token = token->next;
    return val;
}

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;


    while(*p) {
        // blank
        if(isspace(*p)) {
            ++p; // skip
            continue;
        }
        // Punctuator
        else if(
            startswith(p, "==") || 
            startswith(p, "!=") || 
            startswith(p, "<=") || 
            startswith(p, ">="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        else if(strchr("+-*/()<>=;", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        // Number
        else if(isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }
        else {
            error_at(cur->str, "failed to tokenize.");
        }
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}