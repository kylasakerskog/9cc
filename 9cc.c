#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
    if (argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    char *p = argv[1];
    
    // intel記法の選択
    printf(".intel_syntax noprefix \n");

    // プログラム全体から見える関数の指定
    printf(".global main\n");

    printf("main:\n");

    /* 
        strtol(const char *s, char **endptr, int base)
          文字列sをlong型に変換し返却し，変換不可能な文字があった場合はendptrに格納
            *s: 変換対象文字列
            **endptr: 変換不可能な文字列へのポインタの格納先
            base: 進数
    */
    printf("  mov rax, %ld\n", strtol(p, &p, 10));

    while (*p){
        if (*p == '+') {
            p++;
            printf("  add rax, %ld\n", strtol(p, &p,10));
            continue;
        }

        if (*p == '-') {
            p++;
            printf("  sub rax, %ld\n", strtol(p, &p,10));
            continue;
        }

        fprintf(stderr, "予期しない文字です: '%c'\n", *p);
    }
    
    printf("  ret\n");
    return 0;
}
