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
} NodeKind;

// node
typedef struct _Node *NodePtr;
typedef struct _Node {
    NodeKind kind;
    NodePtr lhs;
    NodePtr rhs;
    int val; // use if ND_NUM
} Node;


NodePtr expr();
NodePtr mul();
NodePtr unary();
NodePtr primary();
bool consume(char);
void expect(char);
int expect_number();

// node generator methods
NodePtr new_node(NodeKind kind, NodePtr lhs, NodePtr rhs) {
    NodePtr node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}
NodePtr new_node_num(int val) {
    NodePtr node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// methods by production-rule
// expr = mul ("+" mul | "-" mul)*
NodePtr expr() {
    NodePtr node = mul();
    for (;;) {
        if(consume('+'))
            node = new_node(ND_ADD, node, mul());
        else if(consume('-'))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}
// mul = primary ("*" primary | "/" primary)*
NodePtr mul() {
    NodePtr node = unary();
    for (;;) {
        if(consume('*'))
            node = new_node(ND_MUL, node, unary());
        else if(consume('/'))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}
// unary = ("+" | "-")? primary
NodePtr unary() {
    if(consume('+'))
        return primary();
    else if(consume('-'))
        return new_node(ND_SUB, new_node_num(0), primary());
    else
        return primary();
}
// primary = "(" expr ")" | num
NodePtr primary() {
    if(consume('(')) {
        NodePtr node = expr();
        expect(')');
        return node;
    }

    return new_node_num(expect_number());
}

// generate code
void gen(NodePtr node) {
    if(node->kind == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch(node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("  cqo\n"); // convert quad word to octo word
            printf("  idiv rdi\n"); // rdx&rax / rdi = quotient:rax remainder:rdx
            break;
        default:
            break;
    }

    printf("  push rax\n");
}


// token
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
        // blank
        if(isspace(*p)) {
            ++p;
            continue;
        }
        // Punctuator
        else if(strchr("+-*/()", *p)) {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }
        // Number
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

    user_input = argv[1];
    token = tokenize(user_input);
    NodePtr node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // abstract syntax tree to assembly
    gen(node);
    /*
    printf("  mov rax, %d\n", expect_number());
    while(!at_eof()) {
        if(consume('+')) {
            printf("  add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("  sub rax, %d\n", expect_number());
    }
    */

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}