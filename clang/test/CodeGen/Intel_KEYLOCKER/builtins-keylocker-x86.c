// REQUIRES: intel_feature_isa_keylocker
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -target-feature +keylocker -emit-llvm -o %t %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -target-feature +keylocker -fsyntax-only -o %t %s

typedef signed int V4i __attribute__((vector_size(16)));
void f0() {
  signed int          tmp_i;
  unsigned int        tmp_Ui;
  V4i    tmp_V4i;
  V4i    tmp_V4array[16];

  //  _mm_loadiwkey (unsigned int __ctl, __m128i __intkey,
  //                 __m128i __enkey_lo, __m128i __enkey_hi)
  __builtin_ia32_loadiwkey(tmp_i, tmp_V4i, tmp_V4i, tmp_V4i);

  //  _mm_encodekey128_u32(unsigned int __htype, __m128i __key, void *__h)
  V4i *results = (V4i*)&tmp_V4array[0];
  __builtin_ia32_encodekey128(tmp_Ui, tmp_V4i,
                                      results,
                                      results + 1,
                                      results + 2,
                                      results + 3,
                                      results + 4,
                                      results + 5);

  //_mm_encodekey256_u32(unsigned int __htype, __m128i __key_lo,
  //                      __m128i __key_hi, void *__h)
  V4i    key_lo;
  V4i    key_hi;
  __builtin_ia32_encodekey256(tmp_Ui, key_lo, key_hi,
                                      results,
                                      results + 1,
                                      results + 2,
                                      results + 3,
                                      results + 4,
                                      results + 5,
                                      results + 6);

  //_mm_aesenc128kl_u8(__m128i *__odata, __m128i __idata, const void *__h)
  __builtin_ia32_aesenc128kl(&tmp_V4i, tmp_V4i, (const void*)results);

  //_mm_aesenc256kl_u8(__m128i *__odata, __m128i __idata, const void *__h)
  __builtin_ia32_aesenc256kl(&tmp_V4i, tmp_V4i, (const void*)results);

  //_mm_aesdec128kl_u8(__m128i *__odata, __m128i __idata, const void *__h)
  __builtin_ia32_aesdec128kl(&tmp_V4i, tmp_V4i, (const void*)results);

  //_mm_aesdec256kl_u8(__m128i *__odata, __m128i __idata, const void *__h)
  __builtin_ia32_aesdec256kl(&tmp_V4i, tmp_V4i, (const void*)results);

  //_mm_aesencwide128kl_u8(__m128i __odata[8], const __m128i __idata[8],
  //                    const void* __h)
  V4i odata[8], idata[8];
  __builtin_ia32_aesencwide128kl((const void*)results,
                                odata,
                                odata + 1,
                                odata + 2,
                                odata + 3,
                                odata + 4,
                                odata + 5,
                                odata + 6,
                                odata + 7,
                                idata[0],
                                idata[1],
                                idata[2],
                                idata[3],
                                idata[4],
                                idata[5],
                                idata[6],
                                idata[7]);

  //_mm_aesencwide256kl_u8(__m128i __odata[8], const __m128i __idata[8],
  //                    const void* __h)
  __builtin_ia32_aesencwide256kl((const void*)results,
                                 odata,
                                 odata + 1,
                                 odata + 2,
                                 odata + 3,
                                 odata + 4,
                                 odata + 5,
                                 odata + 6,
                                 odata + 7,
                                 idata[0],
                                 idata[1],
                                 idata[2],
                                 idata[3],
                                 idata[4],
                                 idata[5],
                                 idata[6],
                                 idata[7]);

  //_mm_aesdecwide128kl_u8(__m128i __odata[8], const __m128i __idata[8],
  //                    const void* __h)
  __builtin_ia32_aesdecwide128kl((const void*)results,
                                 odata,
                                 odata + 1,
                                 odata + 2,
                                 odata + 3,
                                 odata + 4,
                                 odata + 5,
                                 odata + 6,
                                 odata + 7,
                                 idata[0],
                                 idata[1],
                                 idata[2],
                                 idata[3],
                                 idata[4],
                                 idata[5],
                                 idata[6],
                                 idata[7]);

  //_mm_aesdecwide256kl_u8(__m128i __odata[8], const __m128i __idata[8],i
  //                    const void* __h)
  __builtin_ia32_aesdecwide256kl((const void*)results,
                                 odata,
                                 odata + 1,
                                 odata + 2,
                                 odata + 3,
                                 odata + 4,
                                 odata + 5,
                                 odata + 6,
                                 odata + 7,
                                 idata[0],
                                 idata[1],
                                 idata[2],
                                 idata[3],
                                 idata[4],
                                 idata[5],
                                 idata[6],
                                 idata[7]);
}
