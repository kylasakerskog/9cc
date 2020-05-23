// usage #include "9cc.h" => カレントディレクトリからヘッダファイルを探してくれる
#define _POSIX_C_SOURCE 200809L // 謎?
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
              TK_IDENT, // 識別子(変数)
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

typedef struct Var Var;

// ローカル変数の型
struct Var {
  Var *next; // 次の変数かNULL
  char *name; // 変数の名前
  int offset; // RBPからのオフセット
};

// AST node
typedef enum {
              ND_ADD, // +
              ND_SUB, // -
              ND_MUL, // *
              ND_DIV, // /
              ND_EQ, // ==
              ND_NE, // !=
              ND_LT, // <
              ND_LE, // <=
              ND_ASSIGN, // =
              ND_RETURN, // return
              ND_IF, // if文
              ND_FOR, // for文
              ND_EXPR_STMT, // Statement
              ND_VAR, // 変数
              ND_NUM, // 整数
} NodeKind;

// struct NodeをNodeという型で定義
typedef struct Node Node;
struct Node {
  NodeKind kind; // ノードの型
  Node *next; // 次のノード
  
  Node *lhs; // 左辺
  Node *rhs; // 右辺

  // kindがND_IF, ND_FORの場合のみ使う
  Node *cond; // 条件 
  Node *then; // 条件が真のとき
  Node *els; // 条件が偽のとき
  Node *init; // 初期化
  Node *inc; // インクリメント

  Var *var; // kindがND_VARの場合のみ使う
  long val; // kindがND_NUMの場合のみ使う
};

typedef struct Function Function;
struct Function{
  Node *node;
  Var *locals;
  int stack_size;
};

// parseのときの返り値を構造体Functionで返す
Function *parse(Token *tok);


//
// codegen.c
//

void codegen(Function *prog);
