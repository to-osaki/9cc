#include<stdio.h>
#include<stdlib.h>

#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>

// node of abstract syntax tree
typedef enum {
    ND_ADD, 
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,

    ND_EQ,
    ND_NEQ,
    ND_LOWER,
    ND_LEQ,
} NodeKind;

// node
typedef struct _Node *NodePtr;
typedef struct _Node {
    NodeKind kind;
    NodePtr lhs;
    NodePtr rhs;
    int val; // use if ND_NUM
} Node;

// token kind
typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

// token
typedef struct _Token *TokenPtr;
typedef struct _Token {
    TokenKind kind;
    TokenPtr next;
    int val;
    int len;
    char *str;
} Token;


// forward declaration
extern NodePtr program();
extern NodePtr statement();
extern NodePtr expr();
extern NodePtr equality();
extern NodePtr relational();
extern NodePtr add();
extern NodePtr mul();
extern NodePtr unary();
extern NodePtr primary();

extern bool consume(char*);
extern void expect(char*);
extern int expect_number();
extern bool startswith(char *p, char *q);
extern bool at_eof();

extern void gen(NodePtr node);
extern Token *tokenize();

extern TokenPtr token; // current token
extern char *user_input; // source code