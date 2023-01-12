// REQUIRES: intel_feature_isa_avx256p
// RUN: not %clang -S %s -ffreestanding -target x86_64-unknown-unknown -march=common-avx256 -emit-llvm -o - 2>&1 | FileCheck %s
// RUN: not %clang -S %s -ffreestanding -target x86_64-unknown-unknown -mavx256p -emit-llvm -o - 2>&1 | FileCheck %s
#include <immintrin.h>
#include <stddef.h>

__m512d test_mm512_sqrt_pd(__m512d a)
{
  //CHECK: error: always_inline function '_mm512_sqrt_pd' requires target feature 'avx512f', but would be inlined into function 'test_mm512_sqrt_pd' that is compiled without support for 'avx512f'
  //CHECK: error: AVX vector argument of type '__m512d' (vector of 8 'double' values) without 'avx512f' enabled changes the ABI
  return _mm512_sqrt_pd(a);
}
