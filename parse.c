#include "9cc.h"

TokenPtr token; // current token
char *user_input; // source code

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    fprintf(stderr, "%s\n", user_input);
    if(false) {
        int pos = loc - user_input;
        fprintf(stderr, "%*s", pos, " ");
        fprintf(stderr, "^ ");
    }
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

TokenPtr consume_kind(TokenKind kind) {
    if(token->kind != kind)
    {
        return NULL;
    }
    else
    {
        TokenPtr ident = token;
        token = token->next;
        return ident;
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

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

int ident_length(char *p) {
    char *q = p;
    while(is_alnum(*q)) {
        ++q;
    }
    return q - p;
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
        // Number
        else if(isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }
        // reserved word : return
        else if(strncmp(p, "return", 6) == 0 && isspace(p[6])) {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
        }
        // Identifier
        else if(is_alnum(*p)) {
            int len = ident_length(p);
            cur = new_token(TK_IDENT, cur, p, len);
            p += len;
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
        else {
            error_at(cur->str, "failed to tokenize.");
        }
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}