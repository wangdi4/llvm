// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s

// CHECK: define spir_func void @foo1() local_unnamed_addr #0 !max_concurrency ![[FOO1_MC:[0-9]+]] {
void __attribute((max_concurrency(4))) foo1()
{
  int A;
}

// CHECK: define spir_func void @foo2() local_unnamed_addr #0 !max_concurrency ![[FOO2_MC:[0-9]+]] {
void __attribute((max_concurrency(0))) foo2()
{
  int A;
}

// CHECK: ![[FOO1_MC]] = !{i32 4}
// CHECK: ![[FOO2_MC]] = !{i32 0}

