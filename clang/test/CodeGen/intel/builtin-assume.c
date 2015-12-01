// RUN: %clang-cc1 -triple x86_64-unknown-unknown -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

typedef long long __m64 __attribute__((__vector_size__(8)));
typedef char __v32qi __attribute__ ((__vector_size__ (32)));
typedef short __v32hi __attribute__ ((__vector_size__ (64)));
typedef unsigned int __mmask32;

__m64 foo(__m64 a, __m64 b) {
  // CHECK: [[V:%[0-9]+]] = call i1 @llvm.has.feature(i64 64)
  // CHECK: call void @llvm.assume(i1 [[V]])
  // CHECK: call x86_mmx @llvm.x86.mmx.padd.q
  return (__m64)__builtin_ia32_paddq(a, b);
}

__v32qi two_features (__v32qi A, __v32qi W, __mmask32 U) {
  // CHECK: [[V:%[0-9]+]] = call i1 @llvm.has.feature(i64 274877906944)
  // CHECK: call void @llvm.assume(i1 [[V]])
  // CHECK: [[V2:%[0-9]+]] = call i1 @llvm.has.feature(i64 137438953472)
  // CHECK: call void @llvm.assume(i1 [[V2]])
  // CHECK: call <32 x i8> @llvm.x86.avx512.mask.pabs.b.256
  return __builtin_ia32_pabsb256_mask (A, W, U);  
}