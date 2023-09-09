#include "memfuncs.h"

#include <stdio.h>

//
// Special functions to speed up packet processing
//
// These all require appropriate alignment of source and destination.
//
// Using __attribute__((aligned(*size*))) where size is 2, 4, or 8 is very
// important, otherwise these functions *will* crash. If the crash is in a
// critical pathway, the exception handler may also crash and you'll need to use
// a slow-motion camera to see where the program counter (PC) is referring to as
// the faulting instruction site (if you're lucky enough to even have that show
// up at all).
//

//
// From DreamHAL
//

// 8-bit (1 bytes at a time)
// Len is (# of total bytes/1), so it's "# of 8-bits"
// Source and destination buffers must both be 1-byte aligned (aka no alignment)
void * memcpy_8bit(void *dest, const void *src, uint32_t len)
{
    if(!len)
        return dest;

    const char *s = (char *)src;
    char *d = (char *)dest;

    uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
    // Underflow would be like adding a negative offset

    // Can use 'd' as a scratch reg now
    asm volatile (
        "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
    ".align 2\n"
    "0:\n\t"
        "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
        "mov.b @%[in]+, %[scratch]\n\t" // scratch = *(s++) (LS 1/2)
        "bf.s 0b\n\t" // while(s != nexts) aka while(!T) (BR 1/2)
        " mov.b %[scratch], @(%[offset], %[in])\n" // *(datatype_of_s*) ((char*)s + diff) = scratch, where src + diff = dest (LS 1)
        : [in] "+&r" ((uint32_t)s), [scratch] "=&r" ((uint32_t)d), [size] "+&r" (len) // outputs
        : [offset] "z" (diff) // inputs
        : "t", "memory" // clobbers
    );

    return dest;
}

// 16-bit (2 bytes at a time)
// Len is (# of total bytes/2), so it's "# of 16-bits"
// Source and destination buffers must both be 2-byte aligned
void * memcpy_16bit(void *dest, const void *src, uint32_t len)
{
    if(!len)
        return dest;

    const unsigned short* s = (unsigned short*)src;
    unsigned short* d = (unsigned short*)dest;

    uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
    // Underflow would be like adding a negative offset

    // Can use 'd' as a scratch reg now
    asm volatile (
        "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
        ".align 2\n"
        "0:\n\t"
        "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
        "mov.w @%[in]+, %[scratch]\n\t" // scratch = *(s++) (LS 1/2)
        "bf.s 0b\n\t" // while(s != nexts) aka while(!T) (BR 1/2)
        " mov.w %[scratch], @(%[offset], %[in])\n" // *(datatype_of_s*) ((char*)s + diff) = scratch, where src + diff = dest (LS 1)
        : [in] "+&r" ((uint32_t)s), [scratch] "=&r" ((uint32_t)d), [size] "+&r" (len) // outputs
        : [offset] "z" (diff) // inputs
        : "t", "memory" // clobbers
    );

    return dest;
}

// 32-bit (4 bytes at a time)
// Len is (# of total bytes/4), so it's "# of 32-bits"
// Source and destination buffers must both be 4-byte aligned

void * memcpy_32bit(void *dest, const void *src, uint32_t len)
{
    if(!len)
        return dest;

    const uint32_t* s = (uint32_t*)src;
    uint32_t* d = (uint32_t*)dest;

    uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
    // Underflow would be like adding a negative offset

    // Can use 'd' as a scratch reg now
    asm volatile (
        "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
        ".align 2\n"
        "0:\n\t"
        "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
        "mov.l @%[in]+, %[scratch]\n\t" // scratch = *(s++) (LS 1/2)
        "bf.s 0b\n\t" // while(s != nexts) aka while(!T) (BR 1/2)
        " mov.l %[scratch], @(%[offset], %[in])\n" // *(datatype_of_s*) ((char*)s + diff) = scratch, where src + diff = dest (LS 1)
        : [in] "+&r" ((uint32_t)s), [scratch] "=&r" ((uint32_t)d), [size] "+&r" (len) // outputs
        : [offset] "z" (diff) // inputs
        : "t", "memory" // clobbers
    );

    return dest;
}

// 64-bit (8 bytes at a time)
// Len is (# of total bytes/8), so it's "# of 64-bits"
// Source and destination buffers must both be 8-byte aligned

void * memcpy_64bit(void *dest, const void *src, uint32_t len)
{
    if(!len)
        return dest;

    const _Complex float* s = (_Complex float*)src;
    _Complex float* d = (_Complex float*)dest;

    _Complex float double_scratch;

    uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
    // Underflow would be like adding a negative offset

    asm volatile (
        "fschg\n\t"
        "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
        ".align 2\n"
        "0:\n\t"
        "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
        "fmov.d @%[in]+, %[scratch]\n\t" // scratch = *(s++) (LS 1/2)
        "bf.s 0b\n\t" // while(s != nexts) aka while(!T) (BR 1/2)
        " fmov.d %[scratch], @(%[offset], %[in])\n\t" // *(datatype_of_s*) ((char*)s + diff) = scratch, where src + diff = dest (LS 1)
        "fschg\n"
        : [in] "+&r" ((uint32_t)s), [scratch] "=&d" (double_scratch), [size] "+&r" (len) // outputs
        : [offset] "z" (diff) // inputs
        : "t", "memory" // clobbers
    );

    return dest;
}

// 32 Bytes at a time
// Len is (# of total bytes/32), so it's "# of 32 Bytes"
// Source and destination buffers must both be 8-byte aligned
void * memcpy_64bit_32Bytes(void *dest, const void *src, uint32_t len)
{
    void * ret_dest = dest;

    if(!len)
        return ret_dest;

    _Complex float double_scratch;
    _Complex float double_scratch2;
    _Complex float double_scratch3;
    _Complex float double_scratch4;

    asm volatile (
        "fschg\n\t" // Switch to pair move mode (FE)
        "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
        ".align 2\n"
        "1:\n\t"
        // *dest++ = *src++
        "fmov.d @%[in]+, %[scratch]\n\t" // (LS)
        "fmov.d @%[in]+, %[scratch2]\n\t" // (LS)
        "fmov.d @%[in]+, %[scratch3]\n\t" // (LS)
        "add #32, %[out]\n\t" // (EX)
        "fmov.d @%[in]+, %[scratch4]\n\t" // (LS)
        "dt %[size]\n\t" // while(--len) (EX)
        "fmov.d %[scratch4], @-%[out]\n\t" // (LS)
        "fmov.d %[scratch3], @-%[out]\n\t" // (LS)
        "fmov.d %[scratch2], @-%[out]\n\t" // (LS)
        "fmov.d %[scratch], @-%[out]\n\t" // (LS)
        "bf.s 1b\n\t" // (BR)
        " add #32, %[out]\n\t" // (EX)
        "fschg\n" // Switch back to single move mode (FE)
        : [in] "+&r" ((uint32_t)src), [out] "+&r" ((uint32_t)dest), [size] "+&r" (len),
        [scratch] "=&d" (double_scratch), [scratch2] "=&d" (double_scratch2), [scratch3] "=&d" (double_scratch3), [scratch4] "=&d" (double_scratch4) // outputs
        : // inputs
        : "t", "memory" // clobbers
    );

    return ret_dest;
}

// Set 8 bytes of 0 at a time
// Len is (# of total bytes/8), so it's "# of 64-bits"
// Destination must be 8-byte aligned
void * memset_zeroes_64bit(void *dest, uint32_t len)
{
    if(!len)
        return dest;

    _Complex float * d = (_Complex float*)((uint32_t)dest & 0x1fffffff);
    _Complex float * nextd = d + len;

    asm volatile (
        "fldi0 fr0\n\t"
        "fldi0 fr1\n\t"
        "fschg\n\t" // Switch to pair move mode (FE)
        "dt %[size]\n\t" // Decrement and test size here once to prevent extra jump (EX 1)
        "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
        ".align 2\n"
        "1:\n\t"
        // *--nextd = val
        "fmov.d DR0, @-%[out]\n\t" // (LS 1/1)
        "bf.s 1b\n\t" // (BR 1/2)
        " dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
        "fschg\n" // Switch back to single move mode (FE)
        : [out] "+r" ((uint32_t)nextd), [size] "+&r" (len) // outputs
        : // inputs
        : "t", "fr0", "fr1", "memory" // clobbers
    );

    return dest;
}

void * memcpy_moop(void *dest, void *src, uint32_t numbytes)
{
    if (src == dest || numbytes == 0)
        return dest;

    void *returnval = dest;
    uint32_t offset = 0;

    while(numbytes)
    {
        // Check 8-byte alignment for 32-byte copy
        if((!(((uintptr_t)src | (uintptr_t)dest) & 0x07) ) &&
           (numbytes >= 32))
        {
            //printf("memcpy_64bit_32Bytes\n");
            memcpy_64bit_32Bytes(dest, src, numbytes >> 5);
            offset = numbytes & -32;
            dest = (char *)dest + offset;
            src = (char *)src + offset;
            numbytes -= offset;
        }
        // Check 8-byte alignment for 64-bit copy
        else if((!(((uintptr_t)src | (uintptr_t)dest) & 0x07) ) &&
                (numbytes >= 8))
        {
            //printf("memcpy_64bit\n");
            memcpy_64bit(dest, src, numbytes >> 3);
            offset = numbytes & -8;
            dest = (char *)dest + offset;
            src = (char *)src + offset;
            numbytes -= offset;
        }
        // Check 4-byte alignment
        else if((!(((uintptr_t)src | (uintptr_t)dest) & 0x03) ) &&
                (numbytes >= 4))
        {
            //printf("memcpy_32bit\n");
            memcpy_32bit(dest, src, numbytes >> 2);
            offset = numbytes & -4;
            dest = (char *)dest + offset;
            src = (char *)src + offset;
            numbytes -= offset;
        }
        // Check 2-byte alignment
        else if((!(((uintptr_t)src | (uintptr_t)dest) & 0x01) ) &&
                (numbytes >= 2))
        {
            //printf("memcpy_16bit\n");
            memcpy_16bit(dest, src, numbytes >> 1);
            offset = numbytes & -2;
            dest = (char *)dest + offset;
            src = (char *)src + offset;
            numbytes -= offset;
        }
        else if(numbytes) // No alignment? Well, that really stinks!
        {
            //printf("memcpy_8bit\n");
            memcpy_8bit(dest, src, numbytes);
            numbytes = 0;
        }

        //printf("numBytes: %ld\n", numbytes);
    }

    return returnval;
}