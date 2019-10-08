// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir -emit-llvm %s -o - | FileCheck %s

// CHECK-NOT: wchar_size
kernel void foo(void);
