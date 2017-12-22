#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstring>

#include <cilk/cilk.h>
#include <vector>

#define PRIMES_FILE_PATH "primes.txt"

//this implementation is
//faster than primes_serial.cpp
//but more memory intesive
void find_primes(unsigned int n)
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

    unsigned int max_prime = 2;
    unsigned int primes_count = 1;
    //cilk::reducer< cilk::op_add<int> > primes_count(1);

    for(unsigned int i = n; i >= 3; i--)
    {
    	if(sieve[i] == 1)
    	{
    		max_prime = i;
    		break;
    	}
    }

    FILE * primes_file = fopen(PRIMES_FILE_PATH, "w");
    fprintf(primes_file, "2\n");
    fclose(primes_file);

    primes_file = fopen(PRIMES_FILE_PATH, "a");

    for(unsigned int i = 3; i <= n; i+= 2){
        if(sieve[i] == 1)
        {
            fprintf(primes_file, "%d\n", i);
        	primes_count += 1;
       	}
    }

    fclose(primes_file);

    printf("%d primes found\n", primes_count);
    printf("last prime: %d", max_prime);

    delete [] sieve;
}

int main(int argc, char ** argv){

    int n = 100000;

    if(argc > 1)
        n = atoi(argv[1]);

    find_primes(n);
    return 0;
}