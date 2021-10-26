// RUN: %clang_cc1 -triple x86_64-unknown-unknown -target-feature +sse4.2 -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -target-feature +avx2 -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -target-feature +avx512bw -fms-compatibility -emit-llvm -o - %s | FileCheck %s
// Check SVML calling conventions are assigned correctly when compiling 128-bit SVML functions.

typedef long long __m128i __attribute__((__vector_size__(16), __aligned__(16)));

// CHECK: define dso_local svml_unified_cc <2 x i64> @__svml_irem4_e9(<2 x i64> %{{.*}}, <2 x i64> %{{.*}})
__attribute__((intel_ocl_bicc))
__m128i __svml_irem4_e9(__m128i va, __m128i vb) {
    return va;
}
