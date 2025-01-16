#include "background_prog.h"

#include <stdio.h>

int main(int argc, char** argv){
    printf("Hello, from OsTasks!\n");
    int ans = addition(7, 8);
    printf("%d", ans);
    return 0;
}
