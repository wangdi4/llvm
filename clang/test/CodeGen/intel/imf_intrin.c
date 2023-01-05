// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
// RUN:   -ffinite-math-only -menable-no-infs -menable-no-nans \
// RUN:   -funsafe-math-optimizations -freciprocal-math -ffp-contract=fast \
// RUN:   -fno-signed-zeros -ffast-math -mreassociate \
// RUN:   -fdenormal-fp-math=preserve-sign,preserve-sign -fno-rounding-math \
// RUN:   -target-cpu skylake-avx512 -emit-llvm -fintel-compatibility \
// RUN:   "-mGLOB_imf_attr=precision:high precision:low:sinf absolute-error:0.00001:sin" \
// RUN:    -o - %s | FileCheck %s

extern float sinf (float __x) __attribute__ ((__nothrow__ ));
extern double sin (double __x) __attribute__ ((__nothrow__ ));
extern double pow(double __x, double __y) __attribute__ ((__nothrow__ ));
extern double fma (double __x, double __y, double __z)
                                          __attribute__ ((__nothrow__ ));
extern void otherfunc(void);

void foo(float f0, double d0)
{
  float f1;
  double d1;

  //CHECK: = call fast float @llvm.sin.f32(float %{{.*}}) [[ATTR_SINF:#[0-9]+]]
  f1 = sinf(f0);

  //CHECK: = call fast float @llvm.sin.f32(float %{{.*}}) [[ATTR_SINF]]
  f1 = __builtin_sinf(f0);

  //CHECK: = call fast double @llvm.sin.f64(double %{{.*}}) [[ATTR_SIN:#[0-9]+]]
  d1 = sin(d0);

  //CHECK: = call fast double @llvm.pow.f64(double %{{.*}}) [[ATTR_DEF:#[0-9]+]]
  d1 = pow(d0, d0);

  //CHECK: = call fast double @llvm.sin.f64(double %{{.*}}) [[ATTR_SIN]]
  //CHECK: = call fast float @llvm.sin.f32(float %{{.*}}) [[ATTR_SINF]]
  d1 = sinf((float)sin(d0));

  //CHECK: = call fast double @llvm.fma.f64(double %{{.*}}) [[ATTR_DEF]]
  d1 = fma(d0, d0, d0);

  //CHECK: call void @otherfunc() [[ATTR_DEF]]
  otherfunc();
}

//CHECK: attributes [[ATTR_SINF]] = {{{.*}}"imf-precision"="low"
//CHECK: attributes [[ATTR_SIN]] = {{{.*}}"imf-absolute-error"="0.00001"
//CHECK-SAME: "imf-precision"="high"
//CHECK: attributes [[ATTR_DEF]] = {{{.*}}"imf-precision"="high"
