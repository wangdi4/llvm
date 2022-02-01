// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -target-feature +sse4.2 -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -target-feature +avx2 -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -target-feature +avx512bw -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-windows -target-feature +sse4.2 -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-windows -target-feature +avx2 -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-windows -target-feature +avx512bw -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-linux -target-feature +sse4.2 -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-linux -target-feature +avx2 -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-linux -target-feature +avx512bw -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-win32 -target-feature +sse4.2 -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-win32 -target-feature +avx2 -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-win32 -target-feature +avx512bw -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// Check SVML calling conventions are assigned correctly when compiling 128-bit SVML functions.

typedef long long __m128i __attribute__((__vector_size__(16), __aligned__(16)));
typedef double __m128d __attribute__((__vector_size__(16), __aligned__(16)));

typedef struct __m128ix2_t   { __m128i v1; __m128i v2; } __m128ix2;

typedef struct { __m128i v1; } int3_sse;
typedef struct { __m128d v1; __m128d v2; } double3_sse;
typedef struct { __m128i v1; __m128i v2; } int8_sse;
typedef struct { __m128d v1; __m128d v2; __m128d v3; __m128d v4; } double8_sse;

// CHECK: define dso_local svml_unified_cc <2 x i64> @__svml_irem4_e9(<2 x i64> noundef %{{.*}}, <2 x i64> noundef %{{.*}})
__attribute__((intel_ocl_bicc))
__m128i __svml_irem4_e9(__m128i va, __m128i vb) {
    return va;
}

// CHECK: define dso_local svml_unified_cc <2 x i64> @__svml_i8div16_e9(<2 x i64> noundef %{{.*}}, <2 x i64> noundef %{{.*}})
__attribute__((intel_ocl_bicc))
__m128i __svml_i8div16_e9(__m128i va, __m128i vb) {
  return va;
}

// CHECK: define dso_local svml_unified_cc %struct.__m128ix2_t @__svml_idivrem4(<2 x i64> noundef %{{.*}}, <2 x i64> noundef %{{.*}})
__attribute__((intel_ocl_bicc))
__m128ix2 __svml_idivrem4(__m128i a, __m128i b) {
  __m128ix2 ret;
  ret.v1 = a;
  ret.v2 = b;
  return ret;
}

// CHECK: define dso_local svml_unified_cc %struct.double3_sse @__ocl_svml_h8_ldexp3(<2 x double> %{{.*}}, <2 x double> %{{.*}}, <2 x i64> %{{.*}})
__attribute__((intel_ocl_bicc))
double3_sse __ocl_svml_h8_ldexp3 (double3_sse a, int3_sse b) {
  return a;
}
