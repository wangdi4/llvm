// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -target-feature +avx2 -fms-compatibility -emit-llvm -opaque-pointers -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -target-feature +avx512bw -fms-compatibility -emit-llvm -opaque-pointers -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-windows -target-feature +avx2 -fms-compatibility -emit-llvm -opaque-pointers -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-windows -target-feature +avx512bw -fms-compatibility -emit-llvm -opaque-pointers -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-linux -target-feature +avx2 -fms-compatibility -emit-llvm -opaque-pointers -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-linux -target-feature +avx512bw -fms-compatibility -emit-llvm -opaque-pointers -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-win32 -target-feature +avx2 -fms-compatibility -emit-llvm -opaque-pointers -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-win32 -target-feature +avx512bw -fms-compatibility -emit-llvm -opaque-pointers -o - %s | FileCheck %s
// Check SVML calling conventions are assigned correctly when compiling 256-bit SVML functions.

typedef long long __m128i __attribute__((__vector_size__(16), __aligned__(16)));
typedef float __m256 __attribute__ ((__vector_size__ (32), __aligned__(32)));
typedef long long __m256i __attribute__((__vector_size__(32), __aligned__(32)));
typedef double __m256d __attribute__ ((__vector_size__ (32), __aligned__(32)));

typedef struct __m256x2_t    { __m256 v1; __m256 v2; } __m256x2;

typedef struct { __m128i v1; } int4_avx;
typedef struct { __m256d v1; } double4_avx;
typedef struct { int4_avx r1; int4_avx r2; } int4x2_avx;
typedef struct { int r1; int r2; } int1x2_avx;

__attribute__((intel_ocl_bicc))
extern __m128i __svml_urem4(__m128i x, __m128i y );

// CHECK: define dso_local svml_unified_cc_256 <4 x i64> @__svml_urem8_e9(<4 x i64> noundef %{{.*}}, <4 x i64> noundef %{{.*}})
// CHECK: declare{{.*}} svml_unified_cc <2 x i64> @__svml_urem4(<2 x i64> noundef, <2 x i64> noundef)
__attribute__((intel_ocl_bicc))
__m256i __svml_urem8_e9(__m256i x, __m256i y) {
  __declspec(align(32)) __m128i x1[2];
  __declspec(align(32)) __m128i y1[2];
  __declspec(align(32)) __m128i r1[2];

  __m256i dst;
  *((__m256i *)x1) = x;
  *((__m256i *)y1) = y;

  r1[0] = __svml_urem4(x1[0], y1[0]);
  r1[1] = __svml_urem4(x1[1], y1[1]);

  dst = *((__m256i *)r1);
  return dst;
}

// CHECK: define dso_local svml_unified_cc_256 <4 x i64> @__svml_i64div4_l9(<4 x i64> noundef %{{.*}}, <4 x i64> noundef %{{.*}})
__attribute__((intel_ocl_bicc))
__m256i __svml_i64div4_l9(__m256i vas, __m256i vbs) {
    return vas;
}

// CHECK: define dso_local svml_unified_cc_256 <4 x double> @__svml_clog2_ha_l9(<4 x double> noundef %{{.*}})
__attribute__((intel_ocl_bicc))
__m256d __svml_clog2_ha_l9 (__m256d a) {
  return a;
}

// CHECK: define dso_local svml_unified_cc_256 %struct.__m256x2_t @__svml_sincospif8_mask(<8 x float> noundef %{{.*}}, <8 x float> noundef %{{.*}})
__attribute__((intel_ocl_bicc))
__m256x2 __svml_sincospif8_mask(__m256 a, __m256 mask) {
  __m256x2 ret;
  ret.v1 = a;
  ret.v2 = mask;
  return ret;
}

// CHECK: define dso_local svml_unified_cc_256 double @__ocl_svml_l9_ldexp1(double noundef %{{.*}}, i32 noundef %{{.*}})
__attribute__((intel_ocl_bicc))
double __ocl_svml_l9_ldexp1 (double a, int b) {
  return a;
}

// CHECK: define dso_local svml_unified_cc_256 %struct.double4_avx @__ocl_svml_l9_sincos4(<4 x double> %{{.*}}, ptr noundef %{{.*}})
__attribute__((intel_ocl_bicc))
double4_avx __ocl_svml_l9_sincos4 (double4_avx a, double4_avx* c) {
  return a;
}

// CHECK: define dso_local svml_unified_cc_256 %struct.int4x2_avx @__ocl_svml_e9_udivrem4(<2 x i64> %{{.*}}, <2 x i64> %{{.*}})
__attribute__((intel_ocl_bicc))
int4x2_avx __ocl_svml_e9_udivrem4 (int4_avx a, int4_avx b) {
  int4x2_avx ret;
  ret.r1 = a;
  ret.r2 = b;
  return ret;
}

// CHECK: define dso_local svml_unified_cc_256 i32 @__ocl_svml_e9_idiv1(i32 noundef %{{.*}}, i32 noundef %{{.*}})
__attribute__((intel_ocl_bicc))
int __ocl_svml_e9_idiv1 (int a, int b) {
  return a;
}

// CHECK: define dso_local svml_unified_cc_256 double @__ocl_svml_e9_div1(double noundef %{{.*}}, double noundef %{{.*}})
__attribute__((intel_ocl_bicc))
double __ocl_svml_e9_div1 (double a, double b) {
  return a;
}

// CHECK: define dso_local svml_unified_cc_256 %struct.int1x2_avx @__ocl_svml_e9_idivrem1(i32 noundef %{{.*}}, i32 noundef %{{.*}})
__attribute__((intel_ocl_bicc))
int1x2_avx __ocl_svml_e9_idivrem1 (int a, int b) {
  int1x2_avx ret;
  ret.r1 = a;
  ret.r2 = b;
  return ret;
}
