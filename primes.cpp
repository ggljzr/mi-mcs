#include <cstdio>
#include <math.h>
#include <stdint.h>

#include <cilk/cilk.h>

#define CACHE_SIZE 327680

int64_t* sieve(int64_t n){
    int64_t * primes = new int64_t[n];
    int i = 1;
    int64_t p = 3;
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

    int n = 1000;

    int64_t * primes = sieve(n);

    printf("%d\n", primes[464]);
    return 0;
}
