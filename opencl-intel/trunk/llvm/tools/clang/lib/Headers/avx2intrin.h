/*===---- avxintrin.h - AVX intrinsics -------------------------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */


__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_mpsadbw_epu8 (__m256i a, __m256i b, const int imm)
{
  return (__m256i) __builtin_ia32_mpsadbw256 ((__v32qi)a, (__v32qi)b, imm);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_abs_epi8 (__m256i a)
{
  return (__m256i)__builtin_ia32_pabsb256 ((__v32qi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_abs_epi16 (__m256i a)
{
  return (__m256i)__builtin_ia32_pabsw256 ((__v16hi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_abs_epi32 (__m256i a)
{
  return (__m256i)__builtin_ia32_pabsd256 ((__v8si)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_packs_epi32 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_packssdw256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_packs_epi16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_packsswb256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_packus_epi32 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_packusdw256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_packus_epi16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_packuswb256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_add_epi8 (__m256i a, __m256i b)
{
    return (__m256i)((__v32qi)a + (__v32qi)b);

  //return (__m256i)__builtin_ia32_paddb256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_add_epi16 (__m256i a, __m256i b)
{
    return (__m256i)((__v16hi)a + (__v16hi)b);
//  return (__m256i)__builtin_ia32_paddw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_add_epi32 (__m256i a, __m256i b)
{
    return (__m256i)((__v8si)a + (__v8si)b);
//  return (__m256i)__builtin_ia32_paddd256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_add_epi64 (__m256i a, __m256i b)
{
    return (__m256i)((__v4di)a + (__v4di)b);
//  return (__m256i)__builtin_ia32_paddq256 ((__v4di)a, (__v4di)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_adds_epi8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_paddsb256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_adds_epi16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_paddsw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_adds_epu8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_paddusb256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_adds_epu16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_paddusw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_alignr_epi8 (__m256i a, __m256i b, const int n)
{
  return (__m256i) __builtin_ia32_palignr256 ((__v4di)a, (__v4di)b, n * 8);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_and_si256 (__m256i a, __m256i b)
{
  return a & b;
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_andnot_si256 (__m256i a, __m256i b)
{
  return ~a & b;
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_avg_epu8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pavgb256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_avg_epu16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pavgw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_blendv_epi8 (__m256i a, __m256i b, __m256i imm)
{
  return (__m256i) __builtin_ia32_pblendvb256 ((__v32qi)a,
					       (__v32qi)b,
					       (__v32qi)imm);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_blend_epi16 (__m256i a, __m256i b, const int imm)
{
  return (__m256i) __builtin_ia32_pblendw256 ((__v16hi)a, (__v16hi)b, imm);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cmpeq_epi8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pcmpeqb256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cmpeq_epi16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pcmpeqw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cmpeq_epi32 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pcmpeqd256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cmpeq_epi64 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pcmpeqq256 ((__v4di)a, (__v4di)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cmpgt_epi8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pcmpgtb256 ((__v32qi)a,
					     (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cmpgt_epi16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pcmpgtw256 ((__v16hi)a,
					     (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cmpgt_epi32 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pcmpgtd256 ((__v8si)a,
					     (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cmpgt_epi64 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pcmpgtq256 ((__v4di)a, (__v4di)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_hadd_epi16 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_phaddw256 ((__v16hi)a,
					     (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_hadd_epi32 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_phaddd256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_hadds_epi16 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_phaddsw256 ((__v16hi)a,
					      (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_hsub_epi16 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_phsubw256 ((__v16hi)a,
					     (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_hsub_epi32 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_phsubd256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_hsubs_epi16 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_phsubsw256 ((__v16hi)a,
					      (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_maddubs_epi16 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_pmaddubsw256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_madd_epi16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pmaddwd256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_max_epi8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pmaxsb256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_max_epi16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pmaxsw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_max_epi32 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pmaxsd256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_max_epu8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pmaxub256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_max_epu16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pmaxuw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_max_epu32 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pmaxud256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_min_epi8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pminsb256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_min_epi16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pminsw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_min_epi32 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pminsd256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_min_epu8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pminub256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_min_epu16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pminuw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_min_epu32 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pminud256 ((__v8si)a, (__v8si)b);
}

__inline int __attribute__ ((__always_inline__, __nodebug__))
_mm256_movemask_epi8 (__m256i a)
{
  return __builtin_ia32_pmovmskb256 ((__v32qi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cvtepi8_epi16 (__m128i a)
{
  return (__m256i) __builtin_ia32_pmovsxbw256 ((__v16qi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cvtepi8_epi32 (__m128i a)
{
  return (__m256i) __builtin_ia32_pmovsxbd256 ((__v16qi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cvtepi8_epi64 (__m128i a)
{
  return (__m256i) __builtin_ia32_pmovsxbq256 ((__v16qi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cvtepi16_epi32 (__m128i a)
{
  return (__m256i) __builtin_ia32_pmovsxwd256 ((__v8hi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cvtepi16_epi64 (__m128i a)
{
  return (__m256i) __builtin_ia32_pmovsxwq256 ((__v8hi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cvtepi32_epi64 (__m128i a)
{
  return (__m256i) __builtin_ia32_pmovsxdq256 ((__v4si)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cvtepu8_epi16 (__m128i a)
{
  return (__m256i) __builtin_ia32_pmovzxbw256 ((__v16qi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cvtepu8_epi32 (__m128i a)
{
  return (__m256i) __builtin_ia32_pmovzxbd256 ((__v16qi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cvtepu8_epi64 (__m128i a)
{
  return (__m256i) __builtin_ia32_pmovzxbq256 ((__v16qi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cvtepu16_epi32 (__m128i a)
{
  return (__m256i) __builtin_ia32_pmovzxwd256 ((__v8hi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cvtepu16_epi64 (__m128i a)
{
  return (__m256i) __builtin_ia32_pmovzxwq256 ((__v8hi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_cvtepu32_epi64 (__m128i a)
{
  return (__m256i) __builtin_ia32_pmovzxdq256 ((__v4si)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_mul_epi32 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_pmuldq256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_mulhrs_epi16 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_pmulhrsw256 ((__v16hi)a,
					       (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_mulhi_epu16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pmulhuw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_mulhi_epi16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pmulhw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_mullo_epi16 (__m256i a, __m256i b)
{
    return (__m256i)((__v16hi)a * (__v16hi)b);

  //return (__m256i)__builtin_ia32_pmullw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_mullo_epi32 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pmulld256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_mul_epu32 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_pmuludq256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_or_si256 (__m256i a, __m256i b)
{
  return a | b;
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sad_epu8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_psadbw256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_shuffle_epi8 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_pshufb256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_shuffle_epi32 (__m256i a, const int imm)
{
  return (__m256i)__builtin_ia32_pshufd256 ((__v8si)a, imm);
}

#define _mm256_shuffle_epi32(a, imm) \
  ((__m256i)__builtin_shufflevector((__v8si)(a), (__v8si) {0},   \
                                    (imm) & 0x3,                 \
                                    ((imm) & 0xc) >> 2,          \
                                    ((imm) & 0x30) >> 4,         \
                                    ((imm) & 0xc0) >> 6,         \
                                    4 + (((imm) & 0x03) >> 0),   \
                                    4 + (((imm) & 0x0c) >> 2),   \
                                    4 + (((imm) & 0x30) >> 4),   \
                                    4 + (((imm) & 0xc0) >> 6)))


#define _mm256_shufflehi_epi16(a, imm) \
  ((__m256i)__builtin_shufflevector((__v16hi)(a), (__v8hi) {0},  \
                                    0, 1, 2, 3,                  \
                                    4 + (((imm) & 0x03) >> 0),   \
                                    4 + (((imm) & 0x0c) >> 2),   \
                                    4 + (((imm) & 0x30) >> 4),   \
                                    4 + (((imm) & 0xc0) >> 6),   \
                                    8+0, 8+1, 8+2, 8+3,          \
                                    12 + (((imm) & 0x03) >> 0),  \
                                    12 + (((imm) & 0x0c) >> 2),  \
                                    12 + (((imm) & 0x30) >> 4),  \
                                    12 + (((imm) & 0xc0) >> 6))) 


#define _mm256_shufflelo_epi16(a, imm) \
  ((__m256i)__builtin_shufflevector((__v16hi)(a), (__v8hi) {0},  \
                                    (imm) & 0x3,                 \
                                    ((imm) & 0xc) >> 2,          \
                                    ((imm) & 0x30) >> 4,         \
                                    ((imm) & 0xc0) >> 6,         \
                                    4, 5, 6, 7,                  \
                                    8 + (((imm) & 0x03) >> 0),   \
                                    8 + (((imm) & 0x0c) >> 2),   \
                                    8 + (((imm) & 0x30) >> 4),   \
                                    8 + (((imm) & 0xc0) >> 6),   \
                                    12, 13, 14, 15))


__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sign_epi8 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_psignb256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sign_epi16 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_psignw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sign_epi32 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_psignd256 ((__v8si)a, (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_slli_si256 (__m256i a, const int imm)
{
  return (__m256i)__builtin_ia32_pslldqi256 (a, imm * 8);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_slli_epi16 (__m256i a, int b)
{
  return (__m256i)__builtin_ia32_psllwi256 ((__v16hi)a, b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sll_epi16 (__m256i a, __m128i b)
{
  return (__m256i)__builtin_ia32_psllw256((__v16hi)a, (__v8hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_slli_epi32 (__m256i a, int b)
{
  return (__m256i)__builtin_ia32_pslldi256 ((__v8si)a, b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sll_epi32 (__m256i a, __m128i b)
{
  return (__m256i)__builtin_ia32_pslld256((__v8si)a, (__v4si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_slli_epi64 (__m256i a, int b)
{
  return (__m256i)__builtin_ia32_psllqi256 ((__v4di)a, b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sll_epi64 (__m256i a, __m128i b)
{
  return (__m256i)__builtin_ia32_psllq256((__v4di)a, (__v2di)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_srai_epi16 (__m256i a, int b)
{
  return (__m256i)__builtin_ia32_psrawi256 ((__v16hi)a, b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sra_epi16 (__m256i a, __m128i b)
{
  return (__m256i)__builtin_ia32_psraw256 ((__v16hi)a, (__v8hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_srai_epi32 (__m256i a, int b)
{
  return (__m256i)__builtin_ia32_psradi256 ((__v8si)a, b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sra_epi32 (__m256i a, __m128i b)
{
  return (__m256i)__builtin_ia32_psrad256 ((__v8si)a, (__v4si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_srli_si256 (__m256i a, const int imm)
{
  return (__m256i)__builtin_ia32_psrldqi256 (a, imm * 8);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_srli_epi16 (__m256i a, int b)
{
  return (__m256i)__builtin_ia32_psrlwi256 ((__v16hi)a, b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_srl_epi16 (__m256i a, __m128i b)
{
  return (__m256i)__builtin_ia32_psrlw256((__v16hi)a, (__v8hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_srli_epi32 (__m256i a, int b)
{
  return (__m256i)__builtin_ia32_psrldi256 ((__v8si)a, b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_srl_epi32 (__m256i a, __m128i b)
{
  return (__m256i)__builtin_ia32_psrld256((__v8si)a, (__v4si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_srli_epi64 (__m256i a, int b)
{
  return (__m256i)__builtin_ia32_psrlqi256 ((__v4di)a, b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_srl_epi64 (__m256i a, __m128i b)
{
  return (__m256i)__builtin_ia32_psrlq256((__v4di)a, (__v2di)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sub_epi8 (__m256i a, __m256i b)
{
  return (__m256i)((__v32qi)a - (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sub_epi16 (__m256i a, __m256i b)
{
  return (__m256i)((__v16hi)a - (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sub_epi32 (__m256i a, __m256i b)
{
  return (__m256i)((__v8si)a - (__v8si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sub_epi64 (__m256i a, __m256i b)
{
  return (__m256i)((__v4di)a - (__v4di)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_subs_epi8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_psubsb256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_subs_epi16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_psubsw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_subs_epu8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_psubusb256 ((__v32qi)a, (__v32qi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_subs_epu16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_ia32_psubusw256 ((__v16hi)a, (__v16hi)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_unpackhi_epi8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_shufflevector ((__v32qi)a, (__v32qi)b, 
    16, 32+16, 17, 32+17, 18, 32+18, 19, 32+19, 20, 32+20, 21, 32+21, 22, 32+22, 23, 32+23, 
    24, 32+24, 25, 32+25, 26, 32+26, 27, 32+27, 28, 32+28, 29, 32+29, 30, 32+30, 31, 32+31);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_unpackhi_epi16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_shufflevector((__v16hi)a, (__v16hi)b, 
    8,  16+8,   9, 16+9,  10, 16+10, 11, 16+11, 
    12, 16+12, 13, 16+13, 14, 16+14, 15, 16+15);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_unpackhi_epi32 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_shufflevector ((__v8si)a, (__v8si)b, 4, 8+4, 5, 8+5, 6, 8+6, 7, 8+7);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_unpackhi_epi64 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_shufflevector ((__v4di)a, (__v4di)b, 2, 4+2, 3, 4+3);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_unpacklo_epi8 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_shufflevector ((__v32qi)a, (__v32qi)b, 
    0, 32+0, 1, 32+1, 2, 32+2, 3, 32+3, 4, 32+4, 5, 32+5, 6, 32+6, 7, 32+7, 
    8, 32+8, 9, 32+9, 10, 32+10, 11, 32+11, 12, 32+12, 13, 32+13, 14, 32+14, 15, 32+15);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_unpacklo_epi16 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_shufflevector((__v16hi)a, (__v16hi)b, 
    0, 16+0, 1, 16+1, 2, 16+2, 3, 16+3, 4, 16+4, 5, 16+5, 6, 16+6, 7, 16+7);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_unpacklo_epi32 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_shufflevector ((__v8si)a, (__v8si)b, 0, 8+0, 1, 8+1, 2, 8+2, 3, 8+3);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_unpacklo_epi64 (__m256i a, __m256i b)
{
  return (__m256i)__builtin_shufflevector ((__v4di)a, (__v4di)b, 0, 4+0, 1, 4+1);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_xor_si256 (__m256i a, __m256i b)
{
  return a ^ b;
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_stream_load_si256 (__m256i const *a)
{
  return (__m256i) __builtin_ia32_movntdqa256 ((__v4di *) a);
}

__inline __m128 __attribute__ ((__always_inline__, __nodebug__))
_mm_broadcastss_ps (__m128 a)
{
  return (__m128) __builtin_ia32_vbroadcastss_ps ((__v4sf)a);
}

__inline __m256 __attribute__ ((__always_inline__, __nodebug__))
_mm256_broadcastss_ps (__m128 a)
{
  return (__m256) __builtin_ia32_vbroadcastss_ps256 ((__v4sf)a);
}

__inline __m256d __attribute__ ((__always_inline__, __nodebug__))
_mm256_broadcastsd_pd (__m128d a)
{
  return (__m256d) __builtin_ia32_vbroadcastsd_pd256 ((__v2df)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm_broadcastsi128_si256 (__m128i a)
{
  return (__m256i) __builtin_ia32_vbroadcastsi256 ((__v2di)a);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_blend_epi32 (__m128i a, __m128i b, const int imm)
{
  return (__m128i) __builtin_ia32_pblendd128 ((__v4si)a, (__v4si)b, imm);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_blend_epi32 (__m256i a, __m256i b, const int imm)
{
  return (__m256i) __builtin_ia32_pblendd256 ((__v8si)a, (__v8si)b, imm);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_broadcastb_epi8 (__m128i a)
{
  return (__m256i) __builtin_ia32_pbroadcastb256 ((__v16qi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_broadcastw_epi16 (__m128i a)
{
  return (__m256i) __builtin_ia32_pbroadcastw256 ((__v8hi)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_broadcastd_epi32 (__m128i a)
{
  return (__m256i) __builtin_ia32_pbroadcastd256 ((__v4si)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_broadcastq_epi64 (__m128i a)
{
  return (__m256i) __builtin_ia32_pbroadcastq256 ((__v2di)a);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_broadcastb_epi8 (__m128i a)
{
  return (__m128i) __builtin_ia32_pbroadcastb128 ((__v16qi)a);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_broadcastw_epi16 (__m128i a)
{
  return (__m128i) __builtin_ia32_pbroadcastw128 ((__v8hi)a);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_broadcastd_epi32 (__m128i a)
{
  return (__m128i) __builtin_ia32_pbroadcastd128 ((__v4si)a);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_broadcastq_epi64 (__m128i a)
{
  return (__m128i) __builtin_ia32_pbroadcastq128 ((__v2di)a);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_permutevar8x32_epi32 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_permvarsi256 ((__v8si)a, (__v8si)b);
}

__inline __m256d __attribute__ ((__always_inline__, __nodebug__))
_mm256_permute4x64_pd (__m256d a, const int imm)
{
  return (__m256d) __builtin_ia32_permdf256 ((__v4df)a, imm);
}

__inline __m256 __attribute__ ((__always_inline__, __nodebug__))
_mm256_permutevar8x32_ps (__m256 a, __m256 b)
{
  return (__m256) __builtin_ia32_permvarsf256 ((__v8sf)a,(__v8sf)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_permute4x64_epi64 (__m256i a, const int imm)
{
  return (__m256i) __builtin_ia32_permdi256 ((__v4di)a, imm);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_permute2x128_si256 (__m256i a, __m256i b, const int imm)
{
  return (__m256i) __builtin_ia32_permti256 ((__v4di)a, (__v4di)b, imm);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm256_extracti128_si256 (__m256i a, const int imm)
{
  return (__m128i) __builtin_ia32_extract128i256 ((__v4di)a, imm);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_inserti128_si256 (__m256i a, __m128i b, const int imm)
{
  return (__m256i) __builtin_ia32_insert128i256 ((__v4di)a, (__v2di)b, imm);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_maskload_epi32 (int const *a, __m256i imm )
{
  return (__m256i) __builtin_ia32_maskloadd256 ((const __v8si *)a,
						(__v8si)imm);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_maskload_epi64 (long const *a, __m256i imm )
{
  return (__m256i) __builtin_ia32_maskloadq256 ((const __v4di *)a,
						(__v4di)imm);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_maskload_epi32 (int const *a, __m128i imm )
{
  return (__m128i) __builtin_ia32_maskloadd ((const __v4si *)a,
					     (__v4si)imm);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_maskload_epi64 (long const *a, __m128i imm )
{
  return (__m128i) __builtin_ia32_maskloadq ((const __v2di *)a,
					     (__v2di)imm);
}

__inline void __attribute__ ((__always_inline__, __nodebug__))
_mm256_maskstore_epi32 (int *a, __m256i imm, __m256i b )
{
  __builtin_ia32_maskstored256 ((__v8si *)a, (__v8si)imm, (__v8si)b);
}

__inline void __attribute__ ((__always_inline__, __nodebug__))
_mm256_maskstore_epi64 (long *a, __m256i imm, __m256i b )
{
  __builtin_ia32_maskstoreq256 ((__v4di *)a, (__v4di)imm, (__v4di)b);
}

__inline void __attribute__ ((__always_inline__, __nodebug__))
_mm_maskstore_epi32 (int *a, __m128i imm, __m128i b )
{
  __builtin_ia32_maskstored ((__v4si *)a, (__v4si)imm, (__v4si)b);
}

__inline void __attribute__ ((__always_inline__, __nodebug__))
_mm_maskstore_epi64 (long *a, __m128i imm, __m128i b )
{
  __builtin_ia32_maskstoreq ((__v2di *)a, (__v2di)imm, (__v2di)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sllv_epi32 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_psllv8si ((__v8si)a, (__v8si)b);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_sllv_epi32 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_psllv4si ((__v4si)a, (__v4si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_sllv_epi64 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_psllv4di ((__v4di)a, (__v4di)b);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_sllv_epi64 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_psllv2di ((__v2di)a, (__v2di)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_srav_epi32 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_psrav8si ((__v8si)a, (__v8si)b);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_srav_epi32 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_psrav4si ((__v4si)a, (__v4si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_srlv_epi32 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_psrlv8si ((__v8si)a, (__v8si)b);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_srlv_epi32 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_psrlv4si ((__v4si)a, (__v4si)b);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_srlv_epi64 (__m256i a, __m256i b)
{
  return (__m256i) __builtin_ia32_psrlv4di ((__v4di)a, (__v4di)b);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_srlv_epi64 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_psrlv2di ((__v2di)a, (__v2di)b);
}

#define ZERO_PD    __m128d{ 0, 0 };
#define ALLONES_PD __m128d{ ~0, ~0 };

#ifdef __GATHER

__inline __m128d __attribute__ ((__always_inline__, __nodebug__))
_mm_i32gather_pd (double const *base, __m128i index, const int scale)
{
  return (__m128d) __builtin_ia32_gathersiv2df (ZERO_PD, base,
              (__v4si)index, ALLONES_PD, scale);
}

__inline __m128d __attribute__ ((__always_inline__, __nodebug__))
_mm_mask_i32gather_pd (__m128d src, double const *base, __m128i index,
		       __m128d mask, const int scale)
{
  return (__m128d) __builtin_ia32_gathersiv2df ((__v2df)src,
						base, (__v4si)index, (__v2df)mask, scale);
}

__inline __m256d __attribute__ ((__always_inline__, __nodebug__))
_mm256_i32gather_pd (double const *base, __m128i index, const int scale)
{
  __v4df src = _mm256_setzero_pd ();
  __v4df mask = _mm256_set1_pd((double)(long int) -1);

  return (__m256d) __builtin_ia32_gathersiv4df (src,
						base,
						(__v4si)index,
						mask,
						scale);
}

__inline __m256d __attribute__ ((__always_inline__, __nodebug__))
_mm256_mask_i32gather_pd (__m256d src, double const *base,
			  __m128i index, __m256d mask, const int scale)
{
  return (__m256d) __builtin_ia32_gathersiv4df ((__v4df)src,
						base,
						(__v4si)index,
						(__v4df)mask,
						scale);
}

__inline __m128d __attribute__ ((__always_inline__, __nodebug__))
_mm_i64gather_pd (double const *base, __m128i index, const int scale)
{
  __v2df src = _mm_setzero_pd ();
  __v2df mask = _mm_cmpeq_pd (src, src);

  return (__m128d) __builtin_ia32_gatherdiv2df (src,
						base,
						(__v2di)index,
						mask,
						scale);
}

__inline __m128d __attribute__ ((__always_inline__, __nodebug__))
_mm_mask_i64gather_pd (__m128d src, double const *base, __m128i index,
		       __m128d mask, const int scale)
{
  return (__m128d) __builtin_ia32_gatherdiv2df ((__v2df)src,
						base,
						(__v2di)index,
						(__v2df)mask,
						scale);
}

__inline __m256d __attribute__ ((__always_inline__, __nodebug__))
_mm256_i64gather_pd (double const *base, __m256i index, const int scale)
{
  __v4df src = _mm256_setzero_pd ();
  __v4df mask = _mm256_set1_pd((double)(long int) -1);

  return (__m256d) __builtin_ia32_gatherdiv4df (src,
						base,
						(__v4di)index,
						mask,
						scale);
}

__inline __m256d __attribute__ ((__always_inline__, __nodebug__))
_mm256_mask_i64gather_pd (__m256d src, double const *base,
			  __m256i index, __m256d mask, const int scale)
{
  return (__m256d) __builtin_ia32_gatherdiv4df ((__v4df)src,
						base,
						(__v4di)index,
						(__v4df)mask,
						scale);
}

__inline __m128 __attribute__ ((__always_inline__, __nodebug__))
_mm_i32gather_ps (float const *base, __m128i index, const int scale)
{
  __v4sf src = _mm_setzero_ps ();
  __v4sf mask = _mm_cmpeq_ps (src, src);

  return (__m128) __builtin_ia32_gathersiv4sf (src,
					       base,
					       (__v4si)index,
					       mask,
					       scale);
}

__inline __m128 __attribute__ ((__always_inline__, __nodebug__))
_mm_mask_i32gather_ps (__m128 src, float const *base, __m128i index,
		       __m128 mask, const int scale)
{
  return (__m128) __builtin_ia32_gathersiv4sf ((__v4sf)src,
					       base,
					       (__v4si)index,
					       (__v4sf)mask,
					       scale);
}

__inline __m256 __attribute__ ((__always_inline__, __nodebug__))
_mm256_i32gather_ps (float const *base, __m256i index, const int scale)
{
  __v8sf src = _mm256_setzero_ps ();
  __v8sf mask = _mm256_set1_ps((float)(int) -1);

  return (__m256) __builtin_ia32_gathersiv8sf (src,
					       base,
					       (__v8si)index,
					       mask,
					       scale);
}

__inline __m256 __attribute__ ((__always_inline__, __nodebug__))
_mm256_mask_i32gather_ps (__m256 src, float const *base,
			  __m256i index, __m256 mask, const int scale)
{
  return (__m256) __builtin_ia32_gathersiv8sf ((__v8sf)src,
					       base,
					       (__v8si)index,
					       (__v8sf)mask,
					       scale);
}

__inline __m128 __attribute__ ((__always_inline__, __nodebug__))
_mm_i64gather_ps (float const *base, __m128i index, const int scale)
{
  __v4sf src = _mm_setzero_ps ();
  __v4sf mask = _mm_cmpeq_ps (src, src);

  return (__m128) __builtin_ia32_gatherdiv4sf (src,
					       base,
					       (__v2di)index,
					       mask,
					       scale);
}

__inline __m128 __attribute__ ((__always_inline__, __nodebug__))
_mm_mask_i64gather_ps (__m128 src, float const *base, __m128i index,
		       __m128 mask, const int scale)
{
  return (__m128) __builtin_ia32_gatherdiv4sf ((__v4sf)src,
						base,
						(__v2di)index,
						(__v4sf)mask,
						scale);
}

__inline __m128 __attribute__ ((__always_inline__, __nodebug__))
_mm256_i64gather_ps (float const *base, __m256i index, const int scale)
{
  __v4sf src = _mm_setzero_ps ();
  __v4sf mask = _mm_cmpeq_ps (src, src);

  return (__m128) __builtin_ia32_gatherdiv4sf256 (src,
						  base,
						  (__v4di)index,
						  mask,
						  scale);
}

__inline __m128 __attribute__ ((__always_inline__, __nodebug__))
_mm256_mask_i64gather_ps (__m128 src, float const *base,
			  __m256i index, __m128 mask, const int scale)
{
  return (__m128) __builtin_ia32_gatherdiv4sf256 ((__v4sf)src,
						  base,
						  (__v4di)index,
						  (__v4sf)mask,
						  scale);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_i32gather_epi64 (long int const *base,
		     __m128i index, const int scale)
{
  __v2di src = __extension__ (__v2di){ 0, 0 };
  __v2di mask = __extension__ (__v2di){ ~0, ~0 };

  return (__m128i) __builtin_ia32_gathersiv2di (src,
						base,
						(__v4si)index,
						mask,
						scale);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_mask_i32gather_epi64 (__m128i src, long int const *base,
			  __m128i index, __m128i mask, const int scale)
{
  return (__m128i) __builtin_ia32_gathersiv2di ((__v2di)src,
						base,
						(__v4si)index,
						(__v2di)mask,
						scale);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_i32gather_epi64 (long int const *base,
			__m128i index, const int scale)
{
  __v4di src = __extension__ (__v4di){ 0, 0, 0, 0 };
  __v4di mask = __extension__ (__v4di){ ~0, ~0, ~0, ~0 };

  return (__m256i) __builtin_ia32_gathersiv4di (src,
						base,
						(__v4si)index,
						mask,
						scale);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_mask_i32gather_epi64 (__m256i src, long int const *base,
			     __m128i index, __m256i mask, const int scale)
{
  return (__m256i) __builtin_ia32_gathersiv4di ((__v4di)src,
						base,
						(__v4si)index,
						(__v4di)mask,
						scale);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_i64gather_epi64 (long int const *base,
		     __m128i index, const int scale)
{
  __v2di src = __extension__ (__v2di){ 0, 0 };
  __v2di mask = __extension__ (__v2di){ ~0, ~0 };

  return (__m128i) __builtin_ia32_gatherdiv2di (src,
						base,
						(__v2di)index,
						mask,
						scale);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_mask_i64gather_epi64 (__m128i src, long int const *base, __m128i index,
			  __m128i mask, const int scale)
{
  return (__m128i) __builtin_ia32_gatherdiv2di ((__v2di)src,
						base,
						(__v2di)index,
						(__v2di)mask,
						scale);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_i64gather_epi64 (long int const *base,
			__m256i index, const int scale)
{
  __v4di src = __extension__ (__v4di){ 0, 0, 0, 0 };
  __v4di mask = __extension__ (__v4di){ ~0, ~0, ~0, ~0 };

  return (__m256i) __builtin_ia32_gatherdiv4di (src,
						base,
						(__v4di)index,
						mask,
						scale);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_mask_i64gather_epi64 (__m256i src, long int const *base,
			     __m256i index, __m256i mask, const int scale)
{
  return (__m256i) __builtin_ia32_gatherdiv4di ((__v4di)src,
						base,
						(__v4di)index,
						(__v4di)mask,
						scale);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_i32gather_epi32 (int const *base, __m128i index, const int scale)
{
  __v4si src = __extension__ (__v4si){ 0, 0, 0, 0 };
  __v4si mask = __extension__ (__v4si){ ~0, ~0, ~0, ~0 };

  return (__m128i) __builtin_ia32_gathersiv4si (src,
					       base,
					       (__v4si)index,
					       mask,
					       scale);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_mask_i32gather_epi32 (__m128i src, int const *base, __m128i index,
			  __m128i mask, const int scale)
{
  return (__m128i) __builtin_ia32_gathersiv4si ((__v4si)src,
						base,
						(__v4si)index,
						(__v4si)mask,
						scale);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_i32gather_epi32 (int const *base, __m256i index, const int scale)
{
  __v8si src = __extension__ (__v8si){ 0, 0, 0, 0, 0, 0, 0, 0 };
  __v8si mask = __extension__ (__v8si){ ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0 };

  return (__m256i) __builtin_ia32_gathersiv8si (src,
						base,
						(__v8si)index,
						mask,
						scale);
}

__inline __m256i __attribute__ ((__always_inline__, __nodebug__))
_mm256_mask_i32gather_epi32 (__m256i src, int const *base,
			     __m256i index, __m256i mask, const int scale)
{
  return (__m256i) __builtin_ia32_gathersiv8si ((__v8si)src,
						base,
						(__v8si)index,
						(__v8si)mask,
						scale);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_i64gather_epi32 (int const *base, __m128i index, const int scale)
{
  __v4si src = __extension__ (__v4si){ 0, 0, 0, 0 };
  __v4si mask = __extension__ (__v4si){ ~0, ~0, ~0, ~0 };

  return (__m128i) __builtin_ia32_gatherdiv4si (src,
						base,
						(__v2di)index,
						mask,
						scale);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm_mask_i64gather_epi32 (__m128i src, int const *base, __m128i index,
			  __m128i mask, const int scale)
{
  return (__m128i) __builtin_ia32_gatherdiv4si ((__v4si)src,
						base,
						(__v2di)index,
						(__v4si)mask,
						scale);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm256_i64gather_epi32 (int const *base, __m256i index, const int scale)
{
  __v4si src = __extension__ (__v4si){ 0, 0, 0, 0 };
  __v4si mask = __extension__ (__v4si){ ~0, ~0, ~0, ~0 };

  return (__m128i) __builtin_ia32_gatherdiv4si256 (src,
						  base,
						  (__v4di)index,
						  mask,
						  scale);
}

__inline __m128i __attribute__ ((__always_inline__, __nodebug__))
_mm256_mask_i64gather_epi32 (__m128i src, int const *base,
			     __m256i index, __m128i mask, const int scale)
{
  return (__m128i) __builtin_ia32_gatherdiv4si256 ((__v4si)src,
						   base,
						   (__v4di)index,
						   (__v4si)mask,
						   scale);
}
#endif // GATHER