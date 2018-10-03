// RUN: %clang_cc1 -emit-llvm -verify -o - %s -fopenmp -fintel-compatibility \
// RUN:  -fintel-openmp-region -fintel-driver-tempfile-name=%t \
// RUN:  -triple x86_64-unknown-linux-gnu
// RUN: grep -q $'^\xEF\xBB\xBF' %t
// RUN: FileCheck %s < %t
// expected-no-diagnostics
// REQUIRES: shell

void foo(int N, float input1[], float input2[], float result[]) {
#pragma omp target map(to: input1[:N], input2[:N]) map(from: result[:N])
#pragma omp parallel for
  for (int i = 0; i < N; i++)
    result[i] = input1[i] + input2[i];
}
// CHECK: <compiler_to_driver_communication>
// CHECK-NEXT: <gfx_offload_src>{{.+}}offload_driver_tempfile.cpp</gfx_offload_src>
// CHECK-NEXT: </compiler_to_driver_communication>
