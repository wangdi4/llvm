// RUN: %clang_cc1 -triple spir64-unknown-unknown-intelfpga -O0 -cl-std=CL2.0 -fsyntax-only -verify -ast-dump -pedantic %s | FileCheck %s
// RUN: %clang_cc1 -triple spir64-unknown-unknown-intelfpga -O0 -cl-std=CL1.2 -fsyntax-only -verify -ast-dump -pedantic %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown-intelfpga -O0 -cl-std=CL2.0 -fsyntax-only -verify -ast-dump -pedantic %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown-intelfpga -O0 -cl-std=CL1.2 -fsyntax-only -verify -ast-dump -pedantic %s | FileCheck %s

// This test file is a copy of SemaIntelHLS/loop_fuse.cpp adopted for OpenCL C

#define N 1024
//CHECK: FunctionDecl{{.*}}foo_loop_fuse
__kernel void foo_loop_fuse(__global float* A, __global float* B, __global float* C) {
  //CHECK:  |-AttributedStmt
  //CHECK-NEXT:  | |-LoopFuseAttr{{.*}}0
  //CHECK-NEXT:  | `-CompoundStmt
  #pragma loop_fuse
  {
    for (int i = 0; i < N; ++i)
      C[i] += A[i];
    for (int j = 0; j < N; ++j)
      C[j] += B[j];
  }
  //CHECK:  |-AttributedStmt
  //CHECK-NEXT:  | |-LoopFuseAttr{{.*}}2
  //CHECK-NEXT:  | `-CompoundStmt
  #pragma loop_fuse depth(2)
  {
    for (int i = 0; i < N; ++i)
      C[i] += A[i];
    for (int j = 0; j < N; ++j)
      C[j] += B[j];
  }
  //CHECK:  |-AttributedStmt
  //CHECK-NEXT:  | |-LoopFuseAttr{{.*}}0 Independent
  //CHECK-NEXT:  | `-CompoundStmt
  #pragma loop_fuse independent
  {
    for (int i = 0; i < N; ++i)
      C[i] += A[i];
    for (int j = 0; j < N; ++j)
      C[j] += B[j];
  }
  //CHECK:  |-AttributedStmt
  //CHECK-NEXT:  | |-LoopFuseAttr{{.*}}2 Independent
  //CHECK-NEXT:  | `-CompoundStmt
  #pragma loop_fuse independent depth(2)
  {
    for (int i = 0; i < N; ++i)
      C[i] += A[i];
    for (int j = 0; j < N; ++j)
      C[j] += B[j];
  }
  //CHECK:  |-AttributedStmt
  //CHECK-NEXT:  | |-LoopFuseAttr{{.*}}2 Independent
  //CHECK-NEXT:  | `-CompoundStmt
  #pragma loop_fuse depth(2) independent
  {
    for (int i = 0; i < N; ++i)
      C[i] += A[i];
    for (int j = 0; j < N; ++j)
      C[j] += B[j];
  }
  //expected-warning@+1{{'depth' cannot appear multiple times in '#pragma loop_fuse' - ignored}}
  #pragma loop_fuse independent depth(2) depth(2)
  {
    for (int i = 0; i < N; ++i)
      C[i] += A[i];
    for (int j = 0; j < N; ++j)
      C[j] += B[j];
  }
  //expected-warning@+1{{'independent' cannot appear multiple times in '#pragma loop_fuse' - ignored}}
  #pragma loop_fuse independent depth(2) independent independent
  {
    for (int i = 0; i < N; ++i)
      C[i] += A[i];
    for (int j = 0; j < N; ++j)
      C[j] += B[j];
  }
  //expected-error@+1{{invalid argument of type '__global float *__private'; expected an integer type}}
  #pragma loop_fuse depth(A)
  {
    for (int i = 0; i < N; ++i)
      C[i] += A[i];
    for (int j = 0; j < N; ++j)
      C[j] += B[j];
  }
  //expected-warning@+1{{Invalid clause. Expected independent or depth value with '#pragma loop_fuse' - ignored}}
  #pragma loop_fuse safelen(11)
  {
    for (int i = 0; i < N; ++i)
      C[i] += A[i];
    for (int j = 0; j < N; ++j)
      C[j] += B[j];
  }
  #pragma loop_fuse
  #pragma loop_fuse //expected-error{{expected '{'}}
  {
    for (int i = 0; i < N; ++i)
      C[i] += A[i];
    for (int j = 0; j < N; ++j)
      C[j] += B[j];
  }
}
