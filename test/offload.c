#include <stdio.h>
#include <stdlib.h>

int offload(int n) {
    long long sum = 0;
    //int*a = (int*)malloc(sizeof(int)*100);
    int a[n];

#pragma omp target data map(a[n])
#pragma omp target teams distribute parallel for 
    for (int i = 0; i < n; i++) {
        a[i] = i;
    }

    for (int i =0; i < n; i++) {
        sum += a[i];
    }
    printf("SUM: %lld\n", sum);
    return  0;
}
void f(int *a) {

}

int main() {
    int a[500], a2[233];

    char string[] = "string";
    f(a);
	offload(100);
	return 0;
}
