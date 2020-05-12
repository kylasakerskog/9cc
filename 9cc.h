// usage #include "9cc.h" => カレントディレクトリからヘッダファイルを探してくれる
#include <assert.h>
#include <ctype.h> // typedef
#include <stdarg.h> // va_*
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// tokenize.c
//

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
  Token *next; // 次の入力トークン
  long val; // kind = TK_NUM の場合，その数値
  char *loc; // トークンの位置
  int len; // トークンの長さ
};

void error(char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
Token *tokenize(char *input);

//
// parse.c
//

typedef enum {
              ND_ADD, // +
              ND_SUB, // -
              ND_MUL, // *
              ND_DIV, // /
              ND_EQ, // ==
              ND_NE, // !=
              ND_LT, // <
              ND_LE, // <=
              ND_RETURN, // return
              ND_EXPR_STMT, // ?
              ND_NUM, // 整数
} NodeKind;

// struct NodeをNodeという型で定義
typedef struct Node Node;
struct Node {
  NodeKind kind; // ノードの型
  Node *next; // 次のノード
  Node *lhs; // 左辺
  Node *rhs; // 右辺
  long val; // kindがND_NUMの場合のみ使う
};

Node *parse(Token *tok);

//
// codegen.c
//

void codegen(Node *node);
