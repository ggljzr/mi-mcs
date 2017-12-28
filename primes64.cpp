#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstring>
#include <unistd.h>
#include <stdint.h>

#include <cilk/cilk.h>
#include <cilk/reducer_ostream.h>
#include <cilk/reducer_opadd.h>

#include <cilk/cilk_api.h>

#include <fstream>

#define PRIMES_FILE_PATH "primes.txt"

//this implementation is
//faster than primes_serial.cpp
//but more memory intesive
void find_primes(int64_t n)
{
    int64_t sieve_size = (n / 2);
    char * sieve = new char[sieve_size];
    //sieve[0:sieve_size] = 1; //slower than cilk_for

    cilk_for(int i = 0; i < sieve_size; i++)
        sieve[i] = 1;

    int64_t sqrtn = (int64_t) sqrt(n);

    for(int64_t i = 3; i <= sqrtn; i += 2)
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
            int64_t inc = 2 * i; //its not p += 2 * i because cilk wont compile it for some reason
            cilk_for(int64_t p = i * i; p <= n; p += inc)
            {
                sieve[p / 2] = 0;
            }
        }
    }

    int64_t max_prime = 2;

    for(int64_t i = n; i >= 3; i--)
    {
    	if(sieve[i / 2] == 1)
    	{
    		max_prime = i;
    		break;
    	}
    }

    cilk::reducer< cilk::op_add<int64_t> > primes_count(1);

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
    cilk_for(int64_t i = 3; i <= max_prime; i+= 2){
        if(sieve[i / 2] == 1)
        {
            *hyper_out << i << std::endl;
            *primes_count += 1;
       	}
    }

    primes_file.close();

    printf("%ld primes found\n", primes_count.get_value());
    printf("prime: %ld", max_prime);

    delete [] sieve;
}

int main(int argc, char ** argv){
    
    char * tvalue = NULL;
    int c;
    int64_t n = 100000;

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
        std::istringstream string_parser(argv[optind]);
        string_parser >> n;
    }

    printf("Calculating primes up to %ld (%.03e)\n", n, (double)n);

    int n_workers = __cilkrts_get_nworkers();
    printf("Running with %d workers...\n", n_workers);

    find_primes(n);
    return 0;
}