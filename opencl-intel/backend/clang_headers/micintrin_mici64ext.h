#ifndef __MICINTRIN_MICEXT_H
#define __MICINTRIN_MICEXT_H

#if !defined(__MIC__) && !defined(__MIC2__)
#error "MIC is not enabled!"
#endif

/*
 * 64-bit integer intrinsic extension for MIC
 *
 * intrinsic extension to support 'v8i64' through emulation on 'v16i32'.
 */

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_sub_epi64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_subpq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sub_epi64(__m512i v1_old, __mmask8 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_subpq512(v1_old, k1, v2, v3);
}

/* 64-bit signed integer compares. */

__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpeq_epi64(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpeqpq512(v1, v2);
}
__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpeq_epi64(__mmask8 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpeqpq512(k1, v1, v2);
}

__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_cmplt_epi64(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpltpq512(v1, v2);
}
__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmplt_epi64(__mmask8 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpltpq512(k1, v1, v2);
}

__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_cmple_epi64(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmplepq512(v1, v2);
}
__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmple_epi64(__mmask8 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmplepq512(k1, v1, v2);
}

__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpneq_epi64(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpneqpq512(v1, v2);
}
__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpneq_epi64(__mmask8 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpneqpq512(k1, v1, v2);
}

__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpnlt_epi64(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpnltpq512(v1, v2);
}
__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpnlt_epi64(__mmask8 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpnltpq512(k1, v1, v2);
}

__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpnle_epi64(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpnlepq512(v1, v2);
}
__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpnle_epi64(__mmask8 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpnlepq512(k1, v1, v2);
}

/* 64-bit unsigned integer compares. */

__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpeq_epu64(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpeqpuq512(v1, v2);
}
__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpeq_epu64(__mmask8 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpeqpuq512(k1, v1, v2);
}

__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_cmplt_epu64(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpltpuq512(v1, v2);
}
__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmplt_epu64(__mmask8 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpltpuq512(k1, v1, v2);
}

__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_cmple_epu64(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmplepuq512(v1, v2);
}
__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmple_epu64(__mmask8 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmplepuq512(k1, v1, v2);
}

__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpneq_epu64(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpneqpuq512(v1, v2);
}
__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpneq_epu64(__mmask8 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpneqpuq512(k1, v1, v2);
}

__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpnlt_epu64(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpnltpuq512(v1, v2);
}
__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpnlt_epu64(__mmask8 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpnltpuq512(k1, v1, v2);
}

__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_cmpnle_epu64(__m512i v1, __m512i v2)
{
  return __builtin_ia32_cmpnlepuq512(v1, v2);
}
__inline__ __mmask8 __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmpnle_epu64(__mmask8 k1, __m512i v1, __m512i v2)
{
  return __builtin_ia32_mask_cmpnlepuq512(k1, v1, v2);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_max_epi64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_maxpq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_max_epi64(__m512i v1_old, __mmask8 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_maxpq512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_max_epu64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_maxpuq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_max_epu64(__m512i v1_old, __mmask8 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_maxpuq512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_min_epi64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_minpq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_min_epi64(__m512i v1_old, __mmask8 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_minpq512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_min_epu64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_minpuq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_min_epu64(__m512i v1_old, __mmask8 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_minpuq512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mullo_epi64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_mullpq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_mullo_epi64(__m512i v1_old, __mmask8 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_mullpq512(v1_old, k1, v2, v3);
}

/*
 * Shift int64 vector by count modulo 64.
 *
 *    Performs an element-by-element shift of int64 vector 'v2', shifting
 *    by the number of bits, modulo 64, specified by the int64 vector 'v3'.
 *
 *    sll    logical shift left
 *    srl    logical shift right
 *    sra    arithmetic shift right
 */
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_sll_epi64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_sllpq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sll_epi64(__m512i v1_old,  __mmask8 k1, __m512i v2,__m512i v3)
{
  return __builtin_ia32_mask_sllpq512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_sra_epi64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_srapq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sra_epi64(__m512i v1_old,  __mmask8 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_srapq512(v1_old, k1, v2, v3);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_srl_epi64(__m512i v2, __m512i v3)
{
  return __builtin_ia32_srlpq512(v2, v3);
}
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_mask_srl_epi64(__m512i v1_old,  __mmask8 k1, __m512i v2, __m512i v3)
{
  return __builtin_ia32_mask_srlpq512(v1_old, k1, v2, v3);
}

#endif // __MICINTRIN_MICEXT_H
