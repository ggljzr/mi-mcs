#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstring>

#include <cilk/cilk.h>
#include <cilk/reducer_ostream.h>
#include <cilk/reducer_opadd.h>

#include <cilk/cilk_api.h>

#include <unistd.h>
#include <ctype.h>

#include <fstream>

#define PRIMES_FILE_PATH "primes.txt"

//this implementation is
//faster than primes_serial.cpp
//but more memory intesive
void find_primes(unsigned int n)
{

    char * sieve = new char[n + 1];
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

    for(unsigned int i = n; i >= 3; i--)
    {
    	if(sieve[i] == 1)
    	{
    		max_prime = i;
    		break;
    	}
    }

    /* 
    program takes about 3.2 seconds for 10^9 primes up to this part
    (creating the primes sieve and findinx max prime)

    iterating trough sieve array and writing primes takes 
    about 5 to 6 seconds -- main bottleneck is file operations
    (total time = 9.2 seconds)

    iterating through sieve array without writing primes to
    a file, only counting them, takes about 0.6 seconds
    (total time = 3.8 seconds)

    should probalby focus on creating sieve array faster,
    since little can be done abou file operations (maybe
    ostream reducer)

    for cycle that iterates through sieve array is not a
    good candidate for paralellization with cilk_for
    since few iterations do some actual work

    using cilk_for and op_add reducer to count primes
    (without writing them into a file) is slightly faster
    than sequential solution
    (total time = 3.3 seconds)

    writing primes with cilk_for, while using op_add reducer
    to count primes and ostream reducer to write them
    seems most efficient
    (total time = 4.11 seconds, may vary more than other cases)
    */

    cilk::reducer< cilk::op_add<unsigned int> > primes_count(1);

    std::ofstream primes_file(PRIMES_FILE_PATH);
    cilk::reducer_ostream hyper_out(primes_file);

    /*
    note that example from: 
    https://www.cilkplus.org/tutorial-reducer-ostream
    wont compile, use example described in reducer_ostream.h file
    on rsj1.fit.cvut.cz -- its probably older version

    (/usr/lib/gcc/x86_64-linux-gnu/5/include/cilk/reducer_ostream.h)
    */

    *hyper_out << 2 << std::endl;
    cilk_for(unsigned int i = 3; i <= max_prime; i+= 2){
        if(sieve[i] == 1)
        {
            *hyper_out << i << std::endl;
            *primes_count += 1;
       	}
    }

    primes_file.close();

    printf("%d primes found\n", primes_count.get_value());
    printf("prime: %d", max_prime);

    delete [] sieve;
}

int main(int argc, char ** argv){
    
    char * tvalue = NULL;
    int c;
    int n = 100000;

    while ((c = getopt (argc, argv, "t:")) != -1)
    {
        switch(c)
        {
            case 't':
                tvalue = optarg;
                break;
            default:
                break;
        }
    }

    if(tvalue != NULL)
    {
        printf("Setting worker count to %s\n", tvalue);
        if (0!= __cilkrts_set_param("nworkers", tvalue))
        {
            printf("Failed to set worker count\n");
            return 1;
        }
    }
    else
    {
        printf("Using default worker count\n");
    }

    if(optind < argc)
    {
        n = atoi(argv[optind]);
    }

    printf("Calculating primes up to %d (%.03e)\n", n, (double)n);

    int n_workers = __cilkrts_get_nworkers();
    printf("Running with %d workers...\n", n_workers);

    find_primes(n);
    return 0;
}