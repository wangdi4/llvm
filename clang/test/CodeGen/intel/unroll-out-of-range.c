// CQ#366562
// REQUIRES: llvm-backend
// RUN: %clang_cc1 -fintel-compatibility -O0 -DTEST1 -emit-llvm %s -o - | FileCheck -check-prefix CHECK1 %s
// RUN: %clang_cc1 -fintel-compatibility -O0 -DTEST2 -emit-llvm %s -o - | FileCheck -check-prefix CHECK2 %s
// RUN: %clang_cc1 -fintel-compatibility -O0 -DTEST3 -emit-llvm %s -o - | FileCheck -check-prefix CHECK2 %s
// REQUIRES: llvm-backend

int main(void) {
  int i = 0, s = 0;

#if TEST1

#pragma unroll(2)

#elif TEST2

#pragma unroll(0)

#elif TEST3

#pragma unroll

#else

#error Unknown test mode

#endif

  for (i = 0; i < 10; ++i)
    s = s + i;

  // CHECK1: !{!"llvm.loop.unroll.count", i32 2}
  // CHECK2: !{!"llvm.loop.unroll.full"}
  return s;
}
