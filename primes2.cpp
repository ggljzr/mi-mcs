#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstring>

#include <cilk/cilk.h>
#include <vector>

//this implementation is
//faster than primes_serial.cpp
//but more memory intesive
void find_primes(int n)
{
    char * sieve = new char[n + 1];
    std::vector<unsigned int> primes;
    //sieve[0:n + 1] = 1; //slower than cilk_for

    cilk_for(int i = 0; i < n + 1; i++)
        sieve[i] = 1;

    int sqrtn = (int) sqrt(n);

    for(unsigned int i = 2; i < sqrtn; i++)
    {
        if(sieve[i] == 1)
        {
            /*
            it seems optimal to parallelize this cycle only
            outter cycle does not have enough iterations
            that do something
            */
            cilk_for(unsigned int p = i * i; p <= n; p += i)
            {
                sieve[p] = 0;
            }
        }
    }

    /*
    sequentially pushing primes to vector
    seems faster than using cilk reducer with
    append operator

    better solution would be processing sieve array
    in parallel, while pushing to local stacks,
    then sequentially merge stacks
    */
    primes.push_back(2);

    for(unsigned int i = 3; i <= n; i+= 2){
        if(sieve[i] == 1)
            primes.push_back(i);
    }

    if(n <= 1000){
        for(size_t i = 0; i < primes.size(); i++)
        {
            printf("%d\n", primes[i]);
        }
    }

    printf("%lu primes found\n", primes.size());
    printf("last prime: %d", primes.back());

    delete [] sieve;
}

int main(int argc, char ** argv){

    int n = 100000;

    if(argc > 1)
        n = atoi(argv[1]);

    find_primes(n);
    return 0;
}