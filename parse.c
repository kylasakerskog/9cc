#include "9cc.h"

// 新しいノードの作成(符号，カッコ)
Node *new_node(NodeKind kind){
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs){
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// 新しいノードの作成(数)
Node *new_num(int val){
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *equality(void);
Node *relational(void);
Node *add(void);
Node *mul(void);
Node *unary(void);
Node *primary(void);

// expr = equality
Node *expr(void){
  return equality();
}

// equality   = relational ("==" relational | "!=" relational)*
Node *equality(void){
  Node *node = relational();
  for (;;){
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational(void){
  Node *node = add();
  for (;;){
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
     return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add(void){
  Node *node = mul();
  for (;;){
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul(void){
  Node *node = unary();
  for (;;){
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;   
  }
}

// unary = ("+" | "-")? primary
Node *unary(void){
  // +xをxに置き換える
  if (consume("+"))
    return unary();
  // -xを0-xに置き換える
  else if (consume("-"))
    return new_binary(ND_SUB, new_num(0), unary());
  return primary();
}

// primary = "(" expr ")" | num
Node *primary(void){
  // 次のトークンが"("なら"(" expr ")"のはず
  if (consume("(")){
    Node *node = expr();
    expect(")");
    return node;
  }

  // それ以外ならnum
  return new_num(expect_number());
}
