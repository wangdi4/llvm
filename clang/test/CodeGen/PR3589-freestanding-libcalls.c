// RUN: %clang_cc1 -triple i386-unknown-unknown -emit-llvm %s -o - | grep 'declare i32 @printf' | count 1
// RUN: %clang_cc1 -triple i386-unknown-unknown -O2 -emit-llvm %s -o - | grep 'declare i32 @puts' | count 1
// RUN: %clang_cc1 -triple i386-unknown-unknown -ffreestanding -O2 -emit-llvm %s -o - | not grep 'declare i32 @puts'

// INTEL: This test is broken by a workaround implemented for OpenCL build in
// LLVM repository. The workaround disables replacement of 'printf' function
// with 'puts' functions as this change breaks existing implementation of OpenCL
// 'printf' function.
// XFAIL: intel_opencl

int printf(const char *, ...);

void f0() {
  printf("hello\n");
}
