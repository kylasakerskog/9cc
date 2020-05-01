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

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next; //次の入力トークン
  int val; // kind = TK_NUM の場合，その数値
  char *str; // トークン文字列;  
};

// 入力プログラム
char *user_input;

// 現在着目しているトークン
Token *token;

// エラーを報告するための関数
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
bool consume(char op){
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

/*
次のトークンが期待している記号の時，
トークンを1つ読みすすめる．それ以外はエラーを返す．
 */
void expect(char op){
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error_at(token->str, "'%c' ではありません", op);
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
Token *new_token(TokenKind kind, Token *cur, char *str){
  // ヒープメモリからsizeバイトのブロックを1個割り当て
  Token *tok = calloc(1, sizeof(Token)); 
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// 入力文字列pをトークナイズし，それを返す
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p){
    // 空白文字をスキップ
    if isspace(*p){ // 文字pが標準空白類文字であれば真を返
        p++;
        continue;
    }

    if (*p == '+' || *p == '-'){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)){
      cur = new_token(TK_NUM, cur, p);
      // 文字列pをlong型に変換し, 変換不可能な文字は&pに挿入(10進数)
      // これにより数値の繋がりをまとまりで取得可能
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(token->str, "トークナイズ出来ません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2){
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }
  
  token = tokenize(argv[1]);
  
  // intel記法の選択
  printf(".intel_syntax noprefix \n");
  
  // プログラム全体から見える関数の指定
  printf(".global main\n");
  
  printf("main:\n");
  
  /*
    式の最初は数でなくてはいけないので，それをチェックして
    最初のmov命令を出力
  */
  printf("  mov rax, %d\n", expect_number());
  
  /*
    `+ <数>`，あるいは`- <数>`というトークンの並びを
    消費しつつアセンブリを出力
  */
  while (!at_eof()){
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }
    
    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
