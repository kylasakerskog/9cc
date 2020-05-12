#include "9cc.h"

static int top;

// レジスタ関数
static char *reg(int idx){
  static char *r[] = {"r10", "r11", "r12", "r13", "r14", "r15"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
    error("register out of range: %d, idx");
  return r[idx];
}

// static : 関数のスコープをファイルスコープにする
static void gen_expr(Node *node){
  if (node->kind == ND_NUM){
    printf("  mov %s, %lu\n", reg(top++), node->val);
    return;
  }

  gen_expr(node->lhs);
  gen_expr(node->rhs);

  char *rd = reg(top - 2);
  char *rs = reg(top - 1);
  top--;

  switch (node->kind){
  case ND_ADD:
    printf("  add %s, %s\n", rd, rs);
    return;
  case ND_SUB:
    printf("  sub %s, %s\n", rd, rs);
    return;
  case ND_MUL:
    printf("  imul %s, %s\n", rd, rs);
    return;
  case ND_DIV:
    printf("  mov rax, %s\n", rd);
    printf("  cqo\n"); // RAXの64bitを128bitに伸ばし，RDXとRAXにセット
    printf("  idiv %s\n", rs); // 引数のレジスタの64bitで割る
    printf("  mov %s, rax\n", rd);
    return;
  case ND_EQ:
    printf("  cmp %s, %s\n", rd, rs); // 比較結果はフラグレジスタに挿入
    printf("  sete al\n"); // 比較結果をsete命令でalレジスタに代入
    // ALはRAXの下位8ビット
    printf("  movzx %s, al\n", rd); // movzx命令はRAXの上位56ビットをゼロクリア
    return;
  case ND_NE:
    printf("  cmp %s, %s\n", rd, rs);
    printf("  setne al\n");
    printf("  movzx %s, al\n", rd); 
    return;
  case ND_LT:
    printf("  cmp %s, %s\n", rd, rs);
    printf("  setl al\n");
    printf("  movzx %s, al\n", rd); 
    return;
  case ND_LE:
    printf("  cmp %s, %s\n", rd, rs);
    printf("  setle al\n");
    printf("  movzx %s, al\n", rd); 
    return;
  default:
    error("invalid statement");
  }
}

static void gen_stmt(Node *node) {
  switch (node->kind) {
  // returnを読む
  case ND_RETURN:
    gen_expr(node->lhs);
    printf("  mov rax, %s\n", reg(--top));
    printf("  jmp .L.return\n");    
    return;
  case ND_EXPR_STMT:
    gen_expr(node->lhs);
    top--;
    return;
  default:
    error("invalid statement");
  }
}


void codegen(Node *node) {
  // 最初の3行
  printf(".intel_syntax noprefix \n"); // intel記法の選択
  printf(".global main\n"); // プログラム全体から見える関数の指定
  printf("main:\n");

  // Save callee-saved registers.
  printf("  push r12\n");
  printf("  push r13\n");
  printf("  push r14\n");
  printf("  push r15\n");

  // 走査
  for (Node *n = node; n; n = n->next){
    gen_stmt(n);
    assert(top == 0);
  }

  printf(".L.return:\n");
  // スタックポインタに結果があるのでpopする
  printf("  pop r15\n");
  printf("  pop r14\n");
  printf("  pop r13\n");
  printf("  pop r12\n");
  printf("  ret\n");
}
