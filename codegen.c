#include "9cc.h"

Node *g_code[100];

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
NodePtr new_node_localvar(int offset) {
    NodePtr node = calloc(1, sizeof(Node));
    node->kind = ND_LOCALVAR;
    node->offset = offset;
    return node;
}

// methods by production-rule
// program = statement*
void program() {
    int len = sizeof(g_code) / sizeof(g_code[0]);
    int i = 0;
    for(; i < len - 1 && !at_eof(); ++i) {
        g_code[i] = statement();
    }
    g_code[i] = NULL;
}
// statement = expr ";"
NodePtr statement() {
    NodePtr node = expr();
    expect(";");
    return node;
}
// expr = assign
NodePtr expr() {
    return assign();
}
// assign = equality ("=" assign)?
NodePtr assign() {
    NodePtr node = equality();
    if(consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}
// equality = relational ("==" relational | "!=" relational)*
NodePtr equality()
{
    NodePtr node = relational();
    for(;;) {
        if(consume("==")) // if token'==' is consumable, 
            node = new_node(ND_EQ, node, relational()); // then it's equation
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
// primary = "(" expr ")" | num | ident
NodePtr primary() {
    if(consume("(")) {
        NodePtr node = expr();
        expect(")");
        return node;
    }

    TokenPtr ident = consume_kind(TK_IDENT);
    if(ident != NULL) {
        return new_node_localvar((ident->str[0] - 'a' + 1) * 8);
    }
    else {
        return new_node_num(expect_number());
    }
}

void gen_lval(NodePtr node) {
    if(node->kind != ND_LOCALVAR) {
        error_at(NULL, "NodeKind [%d] is not a lvalue", node->kind);
    }

    printf("  mov rax, rbp\n"); // rax = base pointer
    printf("  sub rax, %d\n", node->offset); // base pointer - offset => var address
    printf("  push rax\n");
}

// generate code
void gen(NodePtr node) {
    // treat lvalue
    switch(node->kind) {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LOCALVAR:
            gen_lval(node);
            printf("  pop rax\n"); // rax is var address
            printf("  mov rax, [rax]\n"); // store var value to rax
            printf("  push rax\n"); // push var value
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);
            printf("  pop rdi\n"); // rdi is rvalue
            printf("  pop rax\n"); // rax is var address
            printf("  mov [rax], rdi\n"); // *rax = rdi
            printf("  push rdi\n");
            return;
        default:
            break;
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