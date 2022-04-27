// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-linux-gnu -target-feature +fma -O2 -emit-llvm -o - | FileCheck %s

#include <immintrin.h>

void compute(const float* in, float* out) {
  const float k[16] =
  {1.0f, 2.0f, 3.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f};

  __m256 a = _mm256_loadu_ps(k);
  __m256 b = _mm256_loadu_ps(in);
  __m256 c = _mm256_loadu_ps(in + 16);

  __m256 x = _mm256_fmadd_ps(a, b, c);
  __m256 y = _mm256_fnmadd_ps(a, b, c);

  _mm256_storeu_ps(out, x);
  _mm256_storeu_ps(out + 16, y);
}

//CHECK-LABEL: compute
//CHECK: %{{.*}} = tail call <8 x float> @llvm.fma.v8f32(<8 x float> %{{.*}}, <8 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 3.000000e+00, float 2.000000e+00, float 1.000000e+00, float 0.000000e+00>, <8 x float> %{{.*}})
//CHECK:   %{{.*}} = fneg <8 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 3.000000e+00, float 2.000000e+00, float 1.000000e+00, float 0.000000e+00>
//CHECK:   %{{.*}} = tail call <8 x float> @llvm.fma.v8f32(<8 x float> %{{.*}}, <8 x float> %{{.*}}, <8 x float> %{{.*}})
