#include "9cc.h"

static int top;
static int labelseq = 1;

// レジスタ関数
static char *reg(int idx){
  static char *r[] = {"r10", "r11", "r12", "r13", "r14", "r15"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
    error("register out of range: %d, idx");
  return r[idx];
}

static void gen_addr(Node *node){
  if (node->kind == ND_VAR){
    printf("  lea %s, [rbp-%d]\n", reg(top++), node->var->offset);
    return;
  }

  error("not an lvalue");
}

static void load(void){
  printf("  mov %s, [%s]\n", reg(top - 1), reg(top - 1));
}

static void store(void) {
  printf("  mov [%s], %s\n", reg(top - 1), reg(top - 2));
  top--;
}

// static : 関数のスコープをファイルスコープにする
static void gen_expr(Node *node){
  switch (node->kind){
  case ND_NUM:
    printf("  mov %s, %lu\n", reg(top++), node->val);
    return;
  case ND_VAR:
    gen_addr(node);
    load();
    return;
  case ND_ASSIGN:
    gen_expr(node->rhs);
    gen_addr(node->lhs);
    store();
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
  case ND_IF: {
    int seq = labelseq++; // if文の出現回数
    if (node->els){ // if(A) B else C
      gen_expr(node->cond); // Aをコンパイルしたコード // スタックのトップに結果がある
      printf("  cmp %s, 0\n", reg(--top));
      printf("  je .L.else.%d\n", seq);
      gen_stmt(node->then); // Bをコンパイルしたコード
      printf("  jmp .L.end.%d\n", seq);
      printf(".L.else.%d:\n", seq);
      gen_stmt(node->els); // Cをコンパイルしたコード
      printf(".L.end.%d:\n", seq);
    }
    else { // if (A) B
      gen_expr(node->cond); // Aをコンパイルしたコード //スタックのトップに結果がある
      printf("  cmp %s, 0\n", reg(--top));
      printf("  je .L.end.%d\n", seq);
      gen_stmt(node->then); // Bをコンパイルしたコード
      printf(".L.end.%d:\n", seq);
    }
    return;
  }
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


void codegen(Function *prog) {
  // 最初の3行
  printf(".intel_syntax noprefix \n"); // intel記法の選択
  printf(".global main\n"); // プログラム全体から見える関数の指定
  printf("main:\n");

  // Prologue. r12-15 : callee-saved レジスタ
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", prog->stack_size);
  printf("  mov [rbp-8], r12\n");
  printf("  mov [rbp-16], r13\n");
  printf("  mov [rbp-24], r14\n");
  printf("  mov [rbp-32], r15\n");

  // 走査
  for (Node *n = prog->node; n; n = n->next){
    gen_stmt(n);
    assert(top == 0);
  }

  // Epilogue
  printf(".L.return:\n");
  printf("  mov r12, [rbp-8]\n");
  printf("  mov r13, [rbp-16]\n");
  printf("  mov r14, [rbp-24]\n");
  printf("  mov r15, [rbp-32]\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}
