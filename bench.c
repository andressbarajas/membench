
#include <kos.h>
#include <arch/irq.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "memfuncs.h"
#include "fastmem.h"

#define PERFECT_ALIGNMENT 1

#define SIZE 1024 * 4 //16+1
#define ITERATIONS 1

static unsigned long long total1, total2, total3, total4, total5;

static inline unsigned int get_align(void)
{
    if (PERFECT_ALIGNMENT)
        return 0;

    return rand() % 8;
}

int main(int argc, char **argv)
{
    char src[SIZE + 8]__attribute__((aligned(32)));
    char dst[SIZE + 8]__attribute__((aligned(32)));
    int v;

    srand((unsigned int)time(NULL));

    int i, j;
    unsigned int align, align2;

    printf("Bytes,Memcpy,Moop,Zcrc,Fast,DreamHal\n"); // Header for CSV format

    for(j = 0; j < SIZE; j++)
    {
        // Initialize the source with some data
        for (i = 0; i < j; i++) {
            src[i] = rand() % 256;
        }

        uint64_t first_total = 0;
        for(i = 0; i < ITERATIONS; ++i)
        {
            align = get_align();
            align2 = get_align();

	    v = irq_disable();
            uint64_t start = timer_ns_gettime64();
            memcpy(&dst[align], &src[align2], j);
            first_total += (timer_ns_gettime64() - start);
	    irq_restore(v);

            assert(!memcmp(&dst[align], &src[align2], j));
        }

        for (i = 0; i < j; i++) {
            src[i] = rand() % 256;
        }

        uint64_t second_total = 0;
        for(i = 0; i < ITERATIONS; ++i)
        {
            align = get_align();
            align2 = get_align();

	    v = irq_disable();
            uint64_t start = timer_ns_gettime64();
            memcpy_moop(&dst[align], &src[align2], j);
            second_total += (timer_ns_gettime64() - start);
	    irq_restore(v);

            assert(!memcmp(&dst[align], &src[align2], j));
        }

        for (i = 0; i < j; i++) {
            src[i] = rand() % 256;
        }

        uint64_t third_total = 0;
        for(i = 0; i < ITERATIONS; ++i)
        {
            align = get_align();
            align2 = get_align();

	    v = irq_disable();
            uint64_t start = timer_ns_gettime64();
            memcpy_zcrc(&dst[align], &src[align2], j);
            third_total += (timer_ns_gettime64() - start);
	    irq_restore(v);

            assert(!memcmp(&dst[align], &src[align2], j));
        }

        uint64_t fourth_total = 0;
        for(i = 0; i < ITERATIONS; ++i)
        {
            align = get_align();
            align2 = get_align();

	    v = irq_disable();
            uint64_t start = timer_ns_gettime64();
            memcpy_fast(&dst[align], &src[align2], j);
            fourth_total += (timer_ns_gettime64() - start);
	    irq_restore(v);

            assert(!memcmp(&dst[align], &src[align2], j));
        }

        uint64_t fifth_total = 0;
        for(i = 0; i < ITERATIONS; ++i)
        {
            align = get_align();
            align2 = get_align();

            v = irq_disable();
            uint64_t start = timer_ns_gettime64();
            memcpy_dreamhal(&dst[align], &src[align2], j);
            fifth_total += (timer_ns_gettime64() - start);
            irq_restore(v);

            assert(!memcmp(&dst[align], &src[align2], j));
        }

        printf("%d,%llu,%llu,%llu,%llu,%llu\n", j, first_total, second_total, third_total, fourth_total, fifth_total);

        total1 += first_total;
        total2 += second_total;
        total3 += third_total;
        total4 += fourth_total;
        total5 += fifth_total;
    }

    printf("TOTAL: %llu,%llu,%llu,%llu,%llu\n", total1, total2, total3, total4, total5);

    return 0;
}
