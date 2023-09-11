
#include "memfuncs.h"

//
// These all require appropriate alignment of source and destination.
//
// Using __attribute__((aligned(*size*))) where size is 2, 4, or 8 is very
// important, otherwise these functions *will* crash. Handles overlapping
// memory areas, too!
//
// From DreamHAL
//

// Default (8-bit, 1 byte at a time)
void * memmove_8bit(void *dest, const void *src, size_t len) {
    if(!len)
        return dest;

    const uint8_t *s = (uint8_t *)src;
    uint8_t *d = (uint8_t *)dest;

    if (s > d) {
        uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
        // This will underflow and be like adding a negative offset

        // Can use 'd' as a scratch reg now
        __asm__ volatile (
            "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
            ".align 2\n"
            "0:\n\t"
            "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
            "mov.b @%[in]+, %[scratch]\n\t" // scratch = *(s++) (LS 1/2)
            "bf.s 0b\n\t" // while(s != nexts) aka while(!T) (BR 1/2)
            " mov.b %[scratch], @(%[offset], %[in])\n" // *(datatype_of_s*) ((char*)s + diff) = scratch, where src + diff = dest (LS)
            : [in] "+&r" ((uint32_t)s), [scratch] "=&r" ((uint32_t)d), [size] "+&r" (len) // outputs
            : [offset] "z" (diff) // inputs
            : "t", "memory" // clobbers
        );
    }
    else // s < d
    {
        uint8_t *nextd = d + len;

        uint32_t diff = (uint32_t)s - (uint32_t)(d + 1); // extra offset because input calculation occurs before output is decremented
        // This will underflow and be like adding a negative offset

        // Can use 's' as a scratch reg now
        __asm__ volatile (
            "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
            ".align 2\n"
            "0:\n\t"
            "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
            "mov.b @(%[offset], %[out_end]), %[scratch]\n\t" // scratch = *(--nexts) where --nexts is nextd + underflow result (LS 2)
            "bf.s 0b\n\t" // while(nextd != d) aka while(!T) (BR 1/2)
            " mov.b %[scratch], @-%[out_end]\n" // *(--nextd) = scratch (LS 1/1)
            : [out_end] "+&r" ((uint32_t)nextd), [scratch] "=&r" ((uint32_t)s), [size] "+&r" (len) // outputs
            : [offset] "z" (diff) // inputs
            : "t", "memory" // clobbers
        );
    }

    return dest;
}

// 16-bit (2 bytes at a time)
// Len is (# of total bytes/2), so it's "# of 16-bits"
// Source and destination buffers must both be 2-byte aligned
void * memmove_16bit(void *dest, const void *src, size_t len) {
    if(!len)
        return dest;

    const uint16_t* s = (uint16_t*)src;
    uint16_t* d = (uint16_t*)dest;

    if (s > d) {
        uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
        // This will underflow and be like adding a negative offset

        // Can use 'd' as a scratch reg now
        __asm__ volatile (
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
    }
    else // s < d
    {
        uint16_t *nextd = d + len;

        uint32_t diff = (uint32_t)s - (uint32_t)(d + 1); // extra offset because input calculation occurs before output is decremented
        // This will underflow and be like adding a negative offset

        // Can use 's' as a scratch reg now
        __asm__ volatile (
            "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
            ".align 2\n"
            "0:\n\t"
            "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
            "mov.w @(%[offset], %[out_end]), %[scratch]\n\t" // scratch = *(--nexts) where --nexts is nextd + underflow result (LS 2)
            "bf.s 0b\n\t" // while(nextd != d) aka while(!T) (BR 1/2)
            " mov.w %[scratch], @-%[out_end]\n" // *(--nextd) = scratch (LS 1/1)
            : [out_end] "+&r" ((uint32_t)nextd), [scratch] "=&r" ((uint32_t)s), [size] "+&r" (len) // outputs
            : [offset] "z" (diff) // inputs
            : "t", "memory" // clobbers
        );
    }

    return dest;
}

// 32-bit (4 bytes at a time - 1 pixel in a 32-bit linear frame buffer)
// Len is (# of total bytes/4), so it's "# of 32-bits"
// Source and destination buffers must both be 4-byte aligned
void * memmove_32bit(void *dest, const void *src, size_t len) {
    if(!len)
        return dest;

    const uint32_t* s = (uint32_t*)src;
    uint32_t* d = (uint32_t*)dest;

    if (s > d) {
        uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
        // This will underflow and be like adding a negative offset

        // Can use 'd' as a scratch reg now
        __asm__ volatile (
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
    }
    else { // s < d
        uint32_t *nextd = d + len;

        uint32_t diff = (uint32_t)s - (uint32_t)(d + 1); // extra offset because input calculation occurs before output is decremented
        // This will underflow and be like adding a negative offset

        // Can use 's' as a scratch reg now
        __asm__ volatile (
            "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
            ".align 2\n"
            "0:\n\t"
            "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
            "mov.l @(%[offset], %[out_end]), %[scratch]\n\t" // scratch = *(--nexts) where --nexts is nextd + underflow result (LS 2)
            "bf.s 0b\n\t" // while(nextd != d) aka while(!T) (BR 1/2)
            " mov.l %[scratch], @-%[out_end]\n" // *(--nextd) = scratch (LS 1/1)
            : [out_end] "+&r" ((uint32_t)nextd), [scratch] "=&r" ((uint32_t)s), [size] "+&r" (len) // outputs
            : [offset] "z" (diff) // inputs
            : "t", "memory" // clobbers
        );
    }

    return dest;
}

// 64-bit (8 bytes at a time)
// Len is (# of total bytes/8), so it's "# of 64-bits"
// Source and destination buffers must both be 8-byte aligned
void * memmove_64bit(void *dest, const void *src, size_t len) {
    if(!len)
        return dest;

    const _Complex float* s = (_Complex float*)src;
    _Complex float* d = (_Complex float*)dest;

    _Complex float double_scratch;

    if (s > d) {
        uint32_t diff = (uint32_t)d - (uint32_t)(s + 1); // extra offset because input gets incremented before output is calculated
        // This will underflow and be like adding a negative offset

        __asm__ volatile (
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
    }
    else { // s < d
        _Complex float *nextd = d + len;

        uint32_t diff = (uint32_t)s - (uint32_t)(d + 1); // extra offset because input calculation occurs before output is decremented
        // This will underflow and be like adding a negative offset

        __asm__ volatile (
            "fschg\n\t"
            "clrs\n" // Align for parallelism (CO) - SH4a use "stc SR, Rn" instead with a dummy Rn
            ".align 2\n"
            "0:\n\t"
            "dt %[size]\n\t" // (--len) ? 0 -> T : 1 -> T (EX 1)
            "fmov.d @(%[offset], %[out_end]), %[scratch]\n\t" // scratch = *(--nexts) where --nexts is nextd + underflow result (LS 2)
            "bf.s 0b\n\t" // while(nextd != d) aka while(!T) (BR 1/2)
            " fmov.d %[scratch], @-%[out_end]\n\t" // *(--nextd) = scratch (LS 1/1)
            "fschg\n"
            : [out_end] "+&r" ((uint32_t)nextd), [scratch] "=&d" (double_scratch), [size] "+&r" (len) // outputs
            : [offset] "z" (diff) // inputs
            : "t", "memory" // clobbers
        );
    }

    return dest;
}

void * memmove_moop(void *dest, const void *src, size_t numbytes) {
    if (src == dest || numbytes == 0)
        return dest;

    void *returnval = dest;
    uint32_t offset = 0;
    uintptr_t ored = ((uintptr_t)src | (uintptr_t)dest);

    // Check 8-byte alignment for 8-byte copy
    if(!(ored & 0x07) && numbytes >= 8) {
        memmove_64bit(dest, src, numbytes >> 3);
        offset = numbytes & -8;
        dest = (char *)dest + offset;
        src = (char *)src + offset;
        numbytes &= 7; // clear the last 3 bits

        if(numbytes >= 4)
            goto fourbytes;
        else if(numbytes)
            goto singlebytes;
    }
    // Check 4-byte alignment for 4-byte copy
    else if(!(ored & 0x03) && numbytes >= 4) {
fourbytes:
        memmove_32bit(dest, src, numbytes >> 2);
        offset = numbytes & -4;
        dest = (char *)dest + offset;
        src = (char *)src + offset;
        numbytes &= 3; // clear the last 2 bits

        if(numbytes)
            goto singlebytes;
    }
    else {
        // numBytes always seems to be 1-3 when it reaches 
        // this else so lets just do it the old fashioned
        // way
singlebytes:
        memmove_8bit(dest, src, numbytes);
    }  

    return returnval;
}
