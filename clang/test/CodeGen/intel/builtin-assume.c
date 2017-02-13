// RUN: %clang-cc1 -triple x86_64-unknown-unknown -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

typedef long long __m64 __attribute__((__vector_size__(8)));
typedef int __v8i __attribute__ ((__vector_size__ (32)));
typedef unsigned int __mmask32;

__m64 foo(__m64 a, __m64 b) {
  // CHECK: [[V:%[0-9]+]] = call i1 @llvm.has.feature(i64 64)
  // CHECK: call void @llvm.assume(i1 [[V]])
  // CHECK: call x86_mmx @llvm.x86.mmx.padd.q
  return (__m64)__builtin_ia32_paddq(a, b);
}

__v8i two_features (__v8i A, __v8i W, __mmask32 U) {
  // CHECK: [[V:%[0-9]+]] = call i1 @llvm.has.feature(i64 17179869184)
  // CHECK: call void @llvm.assume(i1 [[V]])
  // CHECK: [[V2:%[0-9]+]] = call i1 @llvm.has.feature(i64 274877906944)
  // CHECK: call void @llvm.assume(i1 [[V2]])
  // CHECK: call <8 x i32> @llvm.x86.avx512.mask.conflict.d.256
  return __builtin_ia32_vpconflictsi_256_mask (A, W, U);
}
