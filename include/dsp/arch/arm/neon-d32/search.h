/*
 * search.h
 *
 *  Created on: 9 окт. 2018 г.
 *      Author: sadko
 */

#ifndef DSP_ARCH_ARM_NEON_D32_SEARCH_H_
#define DSP_ARCH_ARM_NEON_D32_SEARCH_H_

namespace neon_d32
{
    #define MINMAX_SEARCH_CORE(op) \
        __ASM_EMIT("veor        q0, q0") \
        __ASM_EMIT("cmp         %[count], 0") \
        __ASM_EMIT("beq         ...f") \
        \
        __ASM_EMIT("subs        %[count], $56") \
        __ASM_EMIT("veor        q1, q1") \
        __ASM_EMIT("blo         2f") \
        /* x56 blocks */ \
        __ASM_EMIT("1:") \
        __ASM_EMIT("vldm        %[src]!, {q2-q7}") \
        __ASM_EMIT(op "         q0, q0, q2") \
        __ASM_EMIT(op "         q1, q1, q3") \
        __ASM_EMIT(op "         q0, q0, q4") \
        __ASM_EMIT(op "         q1, q1, q5") \
        __ASM_EMIT("subs        %[count], $56") \
        __ASM_EMIT("vldm        %[src]!, {q8-q15}") \
        __ASM_EMIT(op "         q0, q0, q6") \
        __ASM_EMIT(op "         q1, q1, q7") \
        __ASM_EMIT(op "         q0, q0, q8") \
        __ASM_EMIT(op "         q1, q1, q9") \
        __ASM_EMIT(op "         q0, q0, q10") \
        __ASM_EMIT(op "         q1, q1, q11") \
        __ASM_EMIT(op "         q0, q0, q12") \
        __ASM_EMIT(op "         q1, q1, q13") \
        __ASM_EMIT(op "         q0, q0, q14") \
        __ASM_EMIT(op "         q1, q1, q15") \
        __ASM_EMIT("bhs         1b") \
        /* x32 block */ \
        __ASM_EMIT("2:") \
        __ASM_EMIT("adds        %[count], $24") \
        __ASM_EMIT("blt         4f") \
        __ASM_EMIT("vldm        %[src]!, {q8-q15}") \
        __ASM_EMIT("sub         %[count], $32") \
        __ASM_EMIT(op "         q0, q0, q6") \
        __ASM_EMIT(op "         q1, q1, q7") \
        __ASM_EMIT(op "         q0, q0, q8") \
        __ASM_EMIT(op "         q1, q1, q9") \
        __ASM_EMIT(op "         q0, q0, q10") \
        __ASM_EMIT(op "         q1, q1, q11") \
        __ASM_EMIT(op "         q0, q0, q12") \
        __ASM_EMIT(op "         q1, q1, q13") \
        __ASM_EMIT(op "         q0, q0, q14") \
        __ASM_EMIT(op "         q1, q1, q15") \
        /* x16 block */ \
        __ASM_EMIT("4:") \
        __ASM_EMIT("adds        %[count], $16") \
        __ASM_EMIT("blt         6f") \
        __ASM_EMIT("vldm        %[src]!, {q2-q5}") \
        __ASM_EMIT("sub         %[count], $16") \
        __ASM_EMIT(op "         q0, q0, q2") \
        __ASM_EMIT(op "         q1, q1, q3") \
        __ASM_EMIT(op "         q0, q0, q4") \
        __ASM_EMIT(op "         q1, q1, q5") \
        /* x8 block */ \
        __ASM_EMIT("6:") \
        __ASM_EMIT("adds        %[count], $8") \
        __ASM_EMIT("blt         8f") \
        __ASM_EMIT("vldm        %[src]!, {q2-q3}") \
        __ASM_EMIT("sub         %[count], $8") \
        __ASM_EMIT(op "         q0, q0, q2") \
        __ASM_EMIT(op "         q1, q1, q3") \
        /* x4 block */ \
        __ASM_EMIT("8:") \
        __ASM_EMIT("adds        %[count], $4") \
        __ASM_EMIT("blt         10f") \
        __ASM_EMIT("vldm        %[src]!, {q2}") \
        __ASM_EMIT("sub         %[count], $4") \
        __ASM_EMIT(op "         q0, q0, q2") \
        /* x1 block */ \
        __ASM_EMIT("10:") \
        __ASM_EMIT(op "         q0, q0, q1") \
        __ASM_EMIT(op "         d0, d0, d1") \
        __ASM_EMIT("vmov        s2, s1") \
        __ASM_EMIT(op "         d0, d0, d1") \
        __ASM_EMIT("11:") \
        __ASM_EMIT("vldr        %[src]!, {s2}") \
        __ASM_EMIT(op "         d0, d0, d1")  \
        __ASM_EMIT("subs        %[count], $1") \
        __ASM_EMIT("bge         11b") \
        \
        __ASM_EMIT("vmov        %[res], s0") \
        __ASM_EMIT("12:") \

    float min(const float *src, size_t count)
    {
        float res = 0.0f;
        IF_ARCH_ARM(
            MINMAX_SEARCH_CORE("vmin")
            : [count] "+r" (count), [src]
              [res] "+t" (res)
            :
            : "q1", "q2", "q3" , "q4", "q5", "q6", "q7",
              "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
        )
        return res;
    }

    float max(const float *src, size_t count)
    {
        float res = 0.0f;
        IF_ARCH_ARM(
            MINMAX_SEARCH_CORE("vmax")
            : [count] "+r" (count), [src]
              [res] "+t" (res)
            :
            : "q1", "q2", "q3" , "q4", "q5", "q6", "q7",
              "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
        )
        return res;
    }

    #undef MINMAX_SEARCH_CORE
}

#endif /* DSP_ARCH_ARM_NEON_D32_SEARCH_H_ */
