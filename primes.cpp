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
    unsigned int sieve_size = (n / 2);
    char * sieve = new char[sieve_size];
    //sieve[0:sieve_size] = 1; //slower than cilk_for

    cilk_for(int i = 0; i < sieve_size; i++)
        sieve[i] = 1;

    int sqrtn = (int) sqrt(n);

    for(unsigned int i = 3; i <= sqrtn; i += 2)
    {
        if(sieve[i / 2] == 1)
        {
            /*
            it seems optimal to parallelize this cycle only
            outter cycle does not have enough iterations
            that do something
            */

            /*
            times two so we get only
            odd multiples of i,
            i is odd => i*i is also odd =>
            we need to skip every other multiple
            to get odd multiples
            */
            unsigned int inc = 2 * i; //its not p += 2 * i because cilk wont compile it for some reason
            cilk_for(unsigned int p = i * i; p <= n; p += inc)
            {
                sieve[p / 2] = 0;
            }
        }
    }

    unsigned int max_prime = 2;

    for(unsigned int i = n; i >= 3; i--)
    {
    	if(sieve[i / 2] == 1)
    	{
    		max_prime = i;
    		break;
    	}
    }

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
        if(sieve[i / 2] == 1)
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