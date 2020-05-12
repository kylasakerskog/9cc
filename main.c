#include "9cc.h"

int main(int argc, char **argv){
  if (argc != 2)
    error("%s: 引数の個数が正しくありません", argv[0]);

  // tokenize and parse
  Token *tok = tokenize(argv[1]);
  Node *node = parse(tok);

  // 走査
  codegen(node);
  return 0;
}

