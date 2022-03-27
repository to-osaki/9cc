#include "9cc.h"

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
// program = statement*
NodePtr program() {
    NodePtr node = statement();
    while(!at_eof()) {
        node = statement();
    }
    return node;
}
// statement = expr ";"
NodePtr statement(){
    NodePtr node = expr();
    expect(";");
    return node;
}

// expr = mul ("+" mul | "-" mul)*
NodePtr expr() {
    return equality();
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