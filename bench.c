
#include <kos.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "memfuncs.h"
#include "fastmem.h"

#define SIZE 1024*8 + 1//16+1
#define ITERATIONS 1

int main(int argc, char **argv)
{
    char src[SIZE]__attribute__((aligned(8)));
    char dst[SIZE]__attribute__((aligned(8)));

    srand((unsigned int)time(NULL));

    int i, j;

    printf("Bytes,Memcpy,Memcpy_Moop,Memcpy_Fast\n"); // Header for CSV format

    for(j = 0; j < SIZE; j++)
    {
        // Initialize the source with some data
        for (i = 0; i < j; i++) {
            src[i] = rand() % 256;
        }

        uint64_t first_total = 0;
        for(i = 0; i < ITERATIONS; ++i)
        {
            uint64_t start = timer_ns_gettime64();
            memcpy(dst,src,j);
            first_total += (timer_ns_gettime64() - start);
            assert(!memcmp(dst, src, j));  
        }

        for (i = 0; i < j; i++) {
            src[i] = rand() % 256;
        }

        uint64_t second_total = 0;
        for(i = 0; i < ITERATIONS; ++i)
        {
            uint64_t start = timer_ns_gettime64();
            memcpy_moop(dst,src,j);
            second_total += (timer_ns_gettime64() - start);
            assert(!memcmp(dst, src, j));  
        }

        for (i = 0; i < j; i++) {
            src[i] = rand() % 256;
        }

        uint64_t third_total = 0;
        for(i = 0; i < ITERATIONS; ++i)
        {
            uint64_t start = timer_ns_gettime64();
            memcpy_fast(dst,src,j);
            third_total += (timer_ns_gettime64() - start);
            assert(!memcmp(dst, src, j));  
        }

        printf("%d,%llu,%llu,%llu\n", j, first_total, second_total, third_total);
    }

    return 0;
}
