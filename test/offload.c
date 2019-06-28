#include <stdio.h>
#include <stdlib.h>

int offload() {
    long long sum = 0;
    int*a = (int*)malloc(sizeof(int)*100);

#pragma omp target data map(a[0:100])                                                                                                                                                         
#pragma omp target teams distribute parallel for 
    for (int i = 0; i < 100; i++) {
        a[i] = i;
    }

    for (int i =0; i < 100; i++) {
        sum += a[i];
    }
    printf("SUM: %lld\n", sum);
    return  0;
}

int main() {
	offload();
	return 0;
}
