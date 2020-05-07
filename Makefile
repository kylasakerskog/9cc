# Makefileのインデントはタブ文字でないといけない
# コロンの前の名前をターゲット，コロンの後ろの0個以上のファイル名を依存ファイル

# 変数定義
# Cコンパイラに渡すコマンドラインオプション

# -std=c11: Cの最新規格であるC11で書かれたソースコードということを伝える
CFLAGS=-std=c11 -g -static
# -g: デバッグ情報を出力する
# wildcardというのはmakeが提供している関数で関数の引数でマッチするファイル名が展開される
SRCS=$(wildcard *.c)
# -static: スタティックリンクする
# .cをoに置き換えるという変数の置換ルール
OBJS=$(SRCS:.c=.o)

# 9ccの依存ファイルはカレントディレクトリにある.cファイルに対応する.oファイル
# .oファイルが存在しないか，.cファイルが新しい場合にのみコンパイラを実行して.oファイルを生成
9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

# すべての.oファイルが9cc.hに依存
$(OBJS): 9cc.h

test: 9cc
	./test.sh

clean:
	  rm -f 9cc *.o *~ tmp*

# ダミーのターゲットを表すための特別な名前
# make fooではfooというファイルを生成しようとする
# make testやmake cleanでtestやcleanというファイルがあっても
# testやcleanというファイルを作成しないようにする
.PHONY: test clean
