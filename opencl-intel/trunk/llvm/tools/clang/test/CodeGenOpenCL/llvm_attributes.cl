// RUN: %clang_cc1 %s -emit-llvm -o - | FileCheck %s

void __attribute__((llvm_alwaysinline)) __attribute__((llvm_inlinehint)) __attribute__((llvm_readnone)) foo(void)
{
}

// CHECK: foo()
// CHECK: readnone
// CHECK: inlinehint
// CHECK: alwaysinline

void __attribute__((llvm_readonly)) __attribute__((llvm_ssp)) __attribute__((llvm_sspreq)) foo2(void)
{
}

// CHECK: foo2()
// CHECK: readonly
// CHECK: ssp
// CHECK: sspreq

void __attribute__((llvm_optsize)) __attribute__((llvm_nounwind)) __attribute__((llvm_noreturn)) foo3(void)
{
}

// CHECK: foo3()
// CHECK: noreturn
// CHECK: nounwind
// CHECK: optsize

void __attribute__((llvm_noredzone)) __attribute__((llvm_noinline)) __attribute__((llvm_noimplicitfloat)) foo4(void)
{
}

// CHECK: foo4()
// CHECK: noinline
// CHECK: noredzone
// CHECK: noimplicitfloat


void __attribute__((llvm_naked)) __attribute__((llvm_alignstack(16))) foo5(void)
{
}

// CHECK: foo5()
// CHECK: naked
// CHECK: alignstack(16)

void __attribute__((llvm_alignstack(64))) foo6(void)
{
}

// CHECK: foo6()
// CHECK: alignstack(64)
