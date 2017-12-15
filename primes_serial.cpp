#include <cstdio>
#include <math.h>

#include <cilk/cilk.h>

int* sieve(int n){
    int * primes = new int[n];
    int i = 1;
    int p = 3;
    primes[0] = 2;

    while(i < n){
        for(int j = 0; j < i; j++){
            if (p % primes[j] == 0) break;
            if (p < primes[j] * primes[j]){
                primes[i] = p;
                i++;
                break;
            }
        }
        p += 2;
    }

    return primes;
}

int main(int argc, char ** argv){

    int n = 1000000;

    int * primes = sieve(n);

    printf("%d\n", primes[464]);
    return 0;
}
