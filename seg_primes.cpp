#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstring>
#include <time.h>
#include <sys/time.h>

#include <cilk/cilk.h>
#include <cilk/reducer_ostream.h>
#include <cilk/reducer_opadd.h>
#include <cilk/reducer_max.h>

#include <cilk/cilk_api.h>

#include <unistd.h>
#include <ctype.h>

#include <fstream>

#define CACHE_SIZE 65536
#define PRIMES_FILE_PATH "primes.txt"

#define MIN(A, B) (A < B) ? A : B
#define MAX(A, B) (A > B) ? A : B

/*
Time measuring from Cilk Karatsuba Example
(https://www.cilkplus.org/sites/default/files/code_samples/karatsuba-v1.0.zip)
Author: Barry Tannenbaum
*/
static inline unsigned long long cilk_getticks()
{
     struct timeval t;
     gettimeofday(&t, 0);
     return t.tv_sec * 1000000ULL + t.tv_usec;
}

static inline double cilk_ticks_to_miliseconds(unsigned long long ticks)
{
     return ticks * 1.0e-3;
}

/*
    function returns first odd multiple of n, that is
    larger than m
*/
unsigned int first_odd_multiple(unsigned int n, int m)
{
    unsigned int multiples = (m / n) + 1;
    unsigned int odd_multiple = n * multiples;
    if (odd_multiple % 2 == 0)
        odd_multiple += n;
    return odd_multiple;
}


unsigned int process_segment(unsigned int from, unsigned int to, 
    cilk::reducer_ostream *hyper_out)
{
    unsigned int sieve_size = (to - from) / 2;
    char * sieve = new char[sieve_size];

    unsigned int sqrtn = (int) sqrt(to);

    for(unsigned int i = 0; i < sieve_size; i++)
        sieve[i] = 1;

    for(unsigned int i = 3; i <= sqrtn; i += 2)
    {
        /*
        We need to start crossing out multiples
        for current segment at the first odd
        multiple of i

        when i is greater than from (usualy first block)
        we start with i^2 like with classic eratosthenes
        sieve
        */
        unsigned int start = MAX(first_odd_multiple(i, from), i * i);

        unsigned int inc = 2 * i;
        for(unsigned int p = start; p <= to; p += inc)
        {
            unsigned int index = (p - from) / 2;
            sieve[index] = 0;
        }
    }

    unsigned int max_prime = 2;
    unsigned int start = from;
    if(start % 2 == 0)
        start++;


    for(unsigned int i = to; i >= start; i--)
    {
        unsigned int index = (i - from) / 2;
        if(sieve[index] == 1)
        {
            max_prime = i;
            break;
        }
    }

    for(unsigned int i = start; i <= to; i += 2)
    {
        unsigned int index = (i - from) / 2;
        if(sieve[index] == 1)
        {
            *hyper_out << i << std::endl;
        }
    }

    delete [] sieve;

    return max_prime;
}

void find_primes_segmented(unsigned int n, unsigned int block_size)
{
    /*
    note that for some reason program is noticably faster
    (1.7 s versus 1.2 s for n = 10^9, block_size = 32768 )
    when file "primes.txt" doesnt exist

    for block_size = 65536 this problem seems less apparent
    */

    cilk::reducer< cilk::op_max<unsigned int> > max_prime;
    std::ofstream primes_file(PRIMES_FILE_PATH);
    cilk::reducer_ostream hyper_out(primes_file);

    *hyper_out << 2 << std::endl;

    cilk_for(unsigned int i = 2; i <= n; i += block_size)
    {
        unsigned int to = MIN(n, i + block_size);
        max_prime->calc_max(process_segment(i, to, &hyper_out));
    }

    printf("prime: %d\n", max_prime.get_value());
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
        fprintf(stderr, "Setting worker count to %s\n", tvalue);
        if (0!= __cilkrts_set_param("nworkers", tvalue))
        {
            fprintf(stderr, "Failed to set worker count\n");
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "Using default worker count\n");
    }

    if(optind < argc)
    {
        n = atoi(argv[optind]);
    }

    fprintf(stderr, "Calculating primes up to %d (%.03e)\n", n, (double)n);

    int n_workers = __cilkrts_get_nworkers();
    fprintf(stderr, "Running with %d workers...\n", n_workers);

    clock_t start = cilk_getticks();
    find_primes_segmented(n, CACHE_SIZE);
    clock_t stop = cilk_getticks();

    double total_time = cilk_ticks_to_miliseconds(stop - start);
    printf("Time: %f ms\n", total_time);

    return 0;
}
