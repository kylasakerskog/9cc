#include "9cc.h"

// 入力プログラム
static char *current_input;

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
static void verror_at(char *loc, char *fmt, va_list ap){
  int pos = loc - current_input; // エラーが入力の何バイト目で起きたか
  fprintf(stderr, "%s\n", current_input);
  fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap); // apのデータをfmtに従ってstderrに出力 
  fprintf(stderr, "\n");
  exit(1);
}

static void error_at(char *loc, char *fmt, ...) {
  va_list ap; // 可変個の実引数を扱うための情報を保持
  va_start(ap, fmt); // 可変長引数の取得開始, fmt: 最初の仮引数の名前
  verror_at(loc, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
}

// 次のトークンが期待している記号のとき真を返す.
bool equal(Token *tok, char *op){
  return strlen(op) == tok->len && // opの長さとトークンの長さが同じ時
    !strncmp(tok->loc, op, tok->len); // opとtoken->strが一致している時
}

// 次のトークンが期待している記号の時トークンを1つ読みすすめる．
Token *skip(Token *tok, char *op) {
  if (!equal(tok, op))
    error_tok(tok, "expected '%s'", op);
  return tok->next;
}

// 新しいトークンをつないでcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len){
  // ヒープメモリからsizeバイトのブロックを1個割り当て
  Token *tok = calloc(1, sizeof(Token)); 
  tok->kind = kind;
  tok->loc = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startwith(char *p, char *q){
  return strncmp(p, q, strlen(q)) == 0;
}

// 入力文字列pをトークナイズし，それを返す
Token *tokenize(char *p) {
  current_input = p;
  Token head = {};
  Token *cur = &head;

  while (*p){
    // 空白文字をスキップ
    if isspace(*p){ // 文字pが標準空白類文字であれば真を返
        p++;
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

    // 複数文字の処理
    if (startwith(p, "==") ||
        startwith(p, "!=") ||
        startwith(p, "<=") ||
        startwith(p, ">=")){
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // 単一文字の処理
    if (ispunct(*p)){
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    error_at(p, "トークナイズ出来ません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
