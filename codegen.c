#include "9cc.h"

// 関数のスコープをファイルスコープにする
static void gen(Node *node){
  if (node->kind == ND_NUM){
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind){
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
    printf("  cqo\n"); // RAXの64bitを128bitに伸ばし，RDXとRAXにセット
    printf("  idiv rdi\n"); // 引数のレジスタの64bitで割る
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n"); // 比較結果はフラグレジスタに挿入
    printf("  sete al\n"); // 比較結果をsete命令でalレジスタに代入
    // ALはRAXの下位8ビット
    printf("  movzb rax, al\n"); // movzb目入れはRAXの上位56ビットをゼロクリア
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n"); 
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n"); 
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n"); 
    break;
  }
  printf("  push rax\n");
}

void codegen(Node *node) {
  // 最初の3行
  printf(".intel_syntax noprefix \n"); // intel記法の選択
  printf(".global main\n"); // プログラム全体から見える関数の指定
  printf("main:\n");

  // 走査
  gen(node);

  // スタックポインタに結果があるのでpopする
  printf("  pop rax\n");
  printf("  ret\n");
}
