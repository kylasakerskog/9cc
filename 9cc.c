#include <ctype.h> // typedef
#include <stdarg.h> // va_*
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
列挙型: 定数のリストを定義
TK_RESERVED = 0 // 記号
TK_NUM = 1 // 整数トークン
TK_EOF = 2 // 入力の終わりを表すトークン
*/
// Tokenのリストを定義
typedef enum {
              TK_RESERVED, // 記号
              TK_NUM, //整数トークン
              TK_EOF, //入力の終わりを表すトークン
} TokenKind;

// struct TokenをTokenという型で定義
typedef struct Token Token;
struct Token {
  TokenKind kind; // トークンの型
  Token *next; //次の入力トークン
  int val; // kind = TK_NUM の場合，その数値
  char *str; // トークン文字列;
  int len; // トークンの長さ
};

// 入力プログラム
char *user_input;

// 現在着目しているトークン
Token *token;

// エラーを報告するための関数
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告するための関数
// printfと同じ引数を取る
void error_at(char *loc, char *fmt, ...){
  va_list ap; // 可変個の実引数を扱うための情報を保持
  va_start(ap, fmt); // 可変長引数の取得開始, fmt: 最初の仮引数の名前

  int pos = loc - user_input; // エラーが入力の何バイト目で起きたか
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap); // apのデータをfmtに従ってstderrに出力 
  fprintf(stderr, "\n");
  exit(1);
}

/* 
次のトークンが期待している記号のとき，
トークンを1つ読み進めて真を返す．それ以外は偽を返す．
*/
bool consume(char *op){
  if (token->kind != TK_RESERVED || // トークンが期待していない記号の時
      strlen(op) != token->len || // opの長さとトークンの長さが違う時
      memcmp(token->str, op, token->len)) // opとtoken->strが異なる時(一致(0)だとif文は通らない)
    return false;
  token = token->next;
  return true;
}

/*
次のトークンが期待している記号の時，
トークンを1つ読みすすめる．それ以外はエラーを返す．
 */
void expect(char *op){
  if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "'%s' ではありません", op);
  token = token->next;
}

/*
次のトークンが数値の時，トークンを1つ読みすすめる．
それ以外はエラーを返す．
*/
int expect_number(){
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof(){
  return token->kind == TK_EOF;
}

// 新しいトークンをつないでcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len){
  // ヒープメモリからsizeバイトのブロックを1個割り当て
  Token *tok = calloc(1, sizeof(Token)); 
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}


bool startwith(char *p, char *q){
  return memcmp(p, q, strlen(q)) == 0;
}

// 入力文字列pをトークナイズし，それを返す
Token *tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p){
    // 空白文字をスキップ
    if isspace(*p){ // 文字pが標準空白類文字であれば真を返
        p++;
        continue;
    }

    if (startwith(p, "==") ||
        startwith(p, "!=") ||
        startwith(p, "<=") ||
        startwith(p, ">=")){
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // 文字列"+-*/()<>"から*pに一致する文字を探索
    if (strchr("+-*/()<>", *p)){
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // 数値型の処理
    if (isdigit(*p)){
      cur = new_token(TK_NUM, cur, p, 0); 
      // 文字列pをlong型に変換し, 変換不可能な文字は&pに挿入(10進数)
      // これにより数値の繋がりをまとまりで取得可能
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q; // Todo: ポインタの引き算？
      continue;
    }

    error_at(token->str, "トークナイズ出来ません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

// 抽象構文木のノードの種類
typedef enum {
              ND_ADD, // +
              ND_SUB, // -
              ND_MUL, // *
              ND_DIV, // /
              ND_EQ, // ==
              ND_NE, // !=
              ND_LT, // <
              ND_LE, // <=
              ND_NUM, // 整数
} NodeKind;

// struct NodeをNodeという型で定義
typedef struct Node Node;
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs; // 左辺
  Node *rhs; // 右辺
  int val; // kindがND_NUMの場合のみ使う
};

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

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// expr = equality
Node *expr(){
  return equality();
}

// equality   = relational ("==" relational | "!=" relational)*
Node *equality(){
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
Node *relational(){
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
Node *add(){
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
Node *mul(){
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
Node *unary(){
  // +xをxに置き換える
  if (consume("+"))
    return unary();
  // -xを0-xに置き換える
  else if (consume("-"))
    return new_binary(ND_SUB, new_num(0), unary());
  return primary();
}

// primary = "(" expr ")" | num
Node *primary(){
  // 次のトークンが"("なら"(" expr ")"のはず
  if (consume("(")){
    Node *node = expr();
    expect(")");
    return node;
  }

  // それ以外ならnum
  return new_num(expect_number());
}

void gen(Node *node){
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

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: 引数の個数が正しくありません", argv[0]);

  user_input = argv[1];
  token = tokenize();
  Node *node = expr();

  // 最初の3行
  printf(".intel_syntax noprefix \n"); // intel記法の選択
  printf(".global main\n"); // プログラム全体から見える関数の指定
  printf("main:\n");

  // 走査
  gen(node);

  // スタックポインタに結果があるのでpopする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
