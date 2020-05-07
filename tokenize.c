#include "9cc.h"

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
