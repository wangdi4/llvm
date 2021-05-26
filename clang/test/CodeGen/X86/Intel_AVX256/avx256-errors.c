// REQUIRES: intel_feature_isa_avx256
// RUN: %clang %s -ffreestanding -target x86_64-unknown-unknown -march=common-avx256 -emit-llvm -fsyntax-only

#include <immintrin.h>
#include <stddef.h>

__m512d test_mm512_sqrt_pd(__m512d a)
{
  // CHECK-LABEL: @test_mm512_sqrt_pd
  // expected-error {{ '__builtin_ia32_sqrtpd512' exceeds the max legal vector width }}
  return _mm512_sqrt_pd(a);
}
