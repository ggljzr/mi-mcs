#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <cilk/cilk.h>
#include <vector>

void find_primes(int n)
{
    char * sieve = new char[n + 1];
    std::vector<int> primes;
    sieve[0:n + 1] = 1;

    for(int i = 2; i < sqrt(n); i += 1)
    {
        if(sieve[i] == 1)
        {
            primes.push_back(i);
            for(int p = i * i; p <= n; p += i)
            {
                sieve[p] = 0;
            }
        }
    }

    for(int i = sqrt(n); i <= n; i++){
        if(sieve[i] == 1)
            primes.push_back(i);
    }

    if(n <= 1000){
        for(size_t i = 0; i < primes.size(); i++)
        {
            printf("%d\n", primes[i]);
        }
    }

    printf("found %d primes\n", primes.size());

    delete [] sieve;
}

int main(int argc, char ** argv){

    int n = 100000;

    if(argc > 1)
        n = atoi(argv[1]);

    find_primes(n);
    return 0;
}