#include "9cc.h"

static int align_to(int n, int align) {
  return (n + align - 1) & ~(align - 1);
}

int main(int argc, char **argv){
  if (argc != 2)
    error("%s: 引数の個数が正しくありません", argv[0]);

  Token *tok = tokenize(argv[1]);
  Function *prog = parse(tok);

  // ローカル変数にoffsetを与える
  int offset = 32; // callee-savedレジスタを考慮
  for (Var *var = prog->locals; var; var = var->next) {
    offset += 8;
    var->offset = offset;
  }
  prog->stack_size = align_to(offset, 16);

  // Traverse the AST to emit assembly.
  codegen(prog);

  return 0;
}

