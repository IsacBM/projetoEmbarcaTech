#include <stdio.h>
int main(){

    // int a[] = {1,2,3,4,5};
    // int *p = a;
    // printf("%d", *(p + 3));
    int i;
    for(i = 1; i <= 5; i++){
        if (i % 2 == 0) continue;
        printf("%d", i); 
    } 
}