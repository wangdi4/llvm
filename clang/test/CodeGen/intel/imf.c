// RUN: %clang_cc1 -ffreestanding -triple x86_64-unknown-linux-gnu \
// RUN:   -target-cpu skylake-avx512 -emit-llvm -fintel-compatibility \
// RUN:   "-mGLOB_imf_attr=precision:high precision:low:sinf,exp2,_mm512_sin_ps absolute-error:0.00001:sin,exp2" \
// RUN:    -o - %s | FileCheck %s
// RUN: %clang_cc1 -ffreestanding -triple x86_64-unknown-linux-gnu \
// RUN:   -target-cpu skylake-avx512 -emit-llvm -fintel-compatibility \
// RUN:   "-mGLOB_imf_attr=precision:low:sin absolute-error:0.123:sin precision:medium:sin" \
// RUN:   "-mGLOB_imf_attr=precision:medium precision:low" \
// RUN:   "-mGLOB_imf_attr=precision:medium precision:high" \
// RUN:    -o - %s | FileCheck -check-prefix CHECK-TWO %s

// RUN: %clang_cc1 -ffreestanding -triple x86_64-unknown-linux-gnu \
// RUN:   -target-cpu skylake-avx512 -emit-llvm -fintel-compatibility \
// RUN:   "-mGLOB_imf_attr=absolute-error:0.123:sin accuracy-bits:14:sqrt" \
// RUN:    -o - %s | FileCheck -check-prefix CHECK-THREE %s

#include <immintrin.h>
extern float sinf (float __x) __attribute__ ((__nothrow__ ));
extern double sin (double __x) __attribute__ ((__nothrow__ ));
extern void otherfunc(void);

__m512 foo(__m512 a, float f0, double d0)
{
  float f1;
  double d1;
  __m512 v;

  //CHECK: = call float @sinf(float noundef %{{.*}}) [[ATTR4:#[0-9]+]]
  //CHECK-TWO: = call float @sinf(float noundef %{{.*}}) [[ATTR4A:#[0-9]+]]
  //CHECK-THREE: = call float @sinf(float noundef %{{.*}}) [[ATTR4B:#[0-9]+]]
  f1 = sinf(f0);
  //CHECK: = call double @sin(double noundef %{{.*}}) [[ATTR5:#[0-9]+]]
  //CHECK-TWO: = call double @sin(double noundef %{{.*}}) [[ATTR5A:#[0-9]+]]
  //CHECK-THREE: = call double @sin(double noundef %{{.*}}) [[ATTR5B:#[0-9]+]]
  d1 = sin(d0);
  //CHECK: call void @otherfunc() [[ATTR6:#[0-9]+]]
  otherfunc();

  //CHECK: call svml_cc <16 x float> @__svml_sinf16(<16 x float> {{.*}}) [[ATTR7:#[0-9]+]]
  v = _mm512_sin_ps(a);

  //CHECK: call svml_cc <16 x float> @__svml_exp2f16(<16 x float> {{.*}}) [[ATTR8:#[0-9]+]]
  return _mm512_exp2_ps(a);
}

//CHECK-THREE: attributes [[ATTR4B]] = {{{.*}}"imf-accuracy-bits-sqrt"="14"
//CHECK-THREE: attributes [[ATTR5B]] = {{{.*}}"imf-accuracy-bits-sqrt"="14"

//CHECK-TWO: attributes [[ATTR4A]] = {{{.*}}"imf-precision"="high"
//CHECK-TWO: attributes [[ATTR5A]] = {{{.*}}"imf-absolute-error"="0.123"
//CHECK-TWO-SAME: "imf-precision"="medium"

//CHECK: attributes [[ATTR4]] = {{{.*}}"imf-precision"="low"
//CHECK: attributes [[ATTR5]] = {{{.*}}"imf-absolute-error"="0.00001"
//CHECK-SAME: "imf-precision"="high"
//CHECK: attributes [[ATTR6]] = {{{.*}}"imf-precision"="high"
//CHECK: attributes [[ATTR7]] = {{{.*}}"imf-precision"="low"
//CHECK: attributes [[ATTR8]] = {{{.*}}"imf-precision"="high"
