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


NodePtr expr();
NodePtr equality();
NodePtr relational();
NodePtr add();
NodePtr mul();
NodePtr unary();
NodePtr primary();
bool consume(char*);
void expect(char*);
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
    return equality();
}
// equality = relational ("==" relational | "!=" relational)*
NodePtr equality()
{
    NodePtr node = relational();
    for(;;) {
        if(consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if(consume("!="))
            node = new_node(ND_NEQ, node, relational());
        else
            return node;
    }
}
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
NodePtr relational()
{
    NodePtr node = add();
    for(;;) {
        if(consume("<="))
            node = new_node(ND_LEQ, node, add());
        else if(consume("<"))
            node = new_node(ND_LOWER, node, add());
        else if(consume(">="))
            node = new_node(ND_LEQ, add(), node);
        else if(consume(">"))
            node = new_node(ND_LOWER, add(), node);
        else
            return node;
    }
}
// add = mul ("+" mul | "-" mul)*
NodePtr add()
{
    NodePtr node = mul();
    for (;;) {
        if(consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if(consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}
// mul = primary ("*" primary | "/" primary)*
NodePtr mul() {
    NodePtr node = unary();
    for (;;) {
        if(consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if(consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}
// unary = ("+" | "-")? primary
NodePtr unary() {
    if(consume("+"))
        return unary();
    else if(consume("-"))
        return new_node(ND_SUB, new_node_num(0), unary());
    else
        return primary();
}
// primary = "(" expr ")" | num
NodePtr primary() {
    if(consume("(")) {
        NodePtr node = expr();
        expect(")");
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
        case ND_EQ:
            printf("  cmp rax, rdi\n"); // compare
            printf("  sete al\n"); // to al flag
            printf("  movzx rax, al\n");
            break;
        case ND_NEQ:
            printf("  cmp rax, rdi\n"); // compare
            printf("  setne al\n"); // to al flag
            printf("  movzx rax, al\n");
            break;
        case ND_LOWER:
            printf("  cmp rax, rdi\n"); // compare
            printf("  setl al\n"); // to al flag
            printf("  movzx rax, al\n");
            break;
        case ND_LEQ:
            printf("  cmp rax, rdi\n"); // compare
            printf("  setle al\n"); // to al flag
            printf("  movzx rax, al\n");
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
    int len;
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
        else if(strchr("+-*/()<>", *p)) {
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