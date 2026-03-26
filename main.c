#include "Allocator.h"
#include <stdio.h>
int main(){
    int *a = my_malloc(sizeof(int));
    if (a == NULL) {
        printf("my_malloc failed to allocate the bytes.\n ");
        return 1;
    }
    *a = 45;

    int *b = my_malloc(sizeof(int));
    if (b == NULL) {
    printf("my_malloc failed for b also. \n");
    my_free(a);
    return 1;
    }
    *b = 2000;

    printf("value of a is %d \n", *a);
    printf("value of b is %d \n", *b);

    my_free(a);
    my_free(b);

    int *arr = my_calloc(5, sizeof(int));
    if (arr == NULL) {
        printf("my_calloc is failed to allocate. \n");
        return 1;
    }

    for (int i = 0; i < 5; i++) {
        printf("Index: %d, Value: %d \n", i, arr[i]);
    }
    printf("\n");

    my_free(arr);

    int *x = my_malloc(sizeof(int));
    if (x == NULL) {
        printf("my_malloc fialed to for x \n");
        return 1;
    }
    *x = 677;

    x = my_realloc(x, sizeof(int) * 10);
    if (x == NULL) {
        printf("my_realloc is faled to X again. \n");
        return 1;
    }

    printf("%d \n", x[0]);

    my_free(x);
    return 0;
}
