#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
    if (argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    // intel記法の選択
    printf(".intel_syntax noprefix \n");

    // プログラム全体から見える関数の指定
    printf(".global main\n");

    printf("main:\n");

    // 文字列から数値への変換
    printf("  mov rax, %d\n", atoi(argv[1]));
    printf("  ret\n");
    return 0;
}
