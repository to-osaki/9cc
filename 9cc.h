#include<stdio.h>
#include<stdlib.h>

#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>

// node of abstract syntax tree
typedef enum {
    // operators
    ND_ADD, 
    ND_SUB,
    ND_MUL,
    ND_DIV,
    // number literal
    ND_NUM,
    // = 
    ND_ASSIGN,
    // varialbe
    ND_LOCALVAR,
    // comparison operators
    ND_EQ,
    ND_NEQ,
    ND_LOWER,
    ND_LEQ,
    // reserved word
    ND_RETURN, // return statement
    ND_IF, // if statement
    // goto label
    ND_LABEL, // define label
    ND_GOTO, // goto statement
} NodeKind;

// node
typedef struct _Node *NodePtr;
typedef struct _Node {
    NodeKind kind;
    NodePtr lhs;
    NodePtr rhs;
    int val; // use if ND_NUM
    int offset; // use if ND_LOCALVAR, offset from BP
    int gotoindex; // use if ND_GOTO
} Node;

// local var
typedef struct _LVar *LVarPtr;
typedef struct _LVar {
    LVarPtr next;
    char *name;
    int len;
    int offset;
} LVar;

// local var
typedef struct _Label *LabelPtr;
typedef struct _Label {
    LabelPtr next;
    char *name;
} Label;

// token kind
typedef enum {
    TK_RESERVED,
    TK_RETURN, // reserved word : return
    TK_IF, // reserved word : if
    TK_GOTO, // reserved word : goto
    TK_DEFINED_LABEL, // label:

    TK_IDENT,
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
extern void program();
extern NodePtr statement();
extern NodePtr assign();
extern NodePtr expr();
extern NodePtr equality();
extern NodePtr relational();
extern NodePtr add();
extern NodePtr mul();
extern NodePtr unary();
extern NodePtr primary();

extern void error_at(char*, char*, ...);
extern bool consume(char*);
extern TokenPtr consume_kind(TokenKind kind);
extern void expect(char*);
extern int expect_number();
extern bool startswith(char *p, char *q);
extern bool at_eof();

extern void gen(NodePtr node);
extern Token *tokenize();

extern TokenPtr token; // current token
extern Node *g_code[100];
extern char *user_input; // source code
extern LVar *g_locals; // local vars