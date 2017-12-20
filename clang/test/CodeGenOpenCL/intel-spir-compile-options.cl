// RUN: %clang_cc1 %s -cl-std=CL1.2 -emit-llvm -o - -triple spir-unknown-unknown -cl-spir-compile-options "-cl-mad-enable -cl-denorms-are-zero" | FileCheck %s

// CHECK: !opencl.compiler.options = !{[[OPT:![0-9]+]]}
// CHECK: [[OPT]] = !{!"-cl-mad-enable", !"-cl-denorms-are-zero"}

kernel void foo(void);
