#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <cilk/cilk.h>
#include <vector>

//this implementation is
//faster than primes_serial.cpp
//but more memory intesive
void find_primes(int n)
{
    char * sieve = new char[n + 1];
    std::vector<int> primes;
    //sieve[0:n + 1] = 1; //it segfaults on compilation when using cilk_for in function

    cilk_for(int i = 0; i < n + 1; i++)
        sieve[i] = 1;

    int sqrtn = (int) sqrt(n);

    cilk_for(int i = 2; i < sqrtn; i++)
    {
        if(sieve[i] == 1)
        {
            cilk_for(int p = i * i; p <= n; p += i)
            {
                sieve[p] = 0;
            }
        }
    }

    primes.push_back(2);

    for(int i = 3; i <= n; i+= 2){
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