// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-unknown-linux-gnu -fhls %s -emit-llvm -o - | FileCheck %s

typedef unsigned int __attribute__((__ap_int(1))) apuint_1;
typedef unsigned int __attribute__((__ap_int(32))) apuint_32;

__kernel void CMPLRLLVM18270(int i, apuint_32 i32, apuint_1 i1) {
  bool b1 = i;
  // CHECK: %[[LOAD:.+]] = load i32, i32*
  // CHECK: %[[TO_BOOL:.+]] = icmp ne i32 %[[LOAD]], 0
  // CHECK: %[[FROM_BOOL:.+]] = zext i1 %[[TO_BOOL]] to i8
  // CHECK: store i8 %[[FROM_BOOL]], i8*
  bool b2 = i32;
  // CHECK: %[[LOAD:.+]] = load i32, i32*
  // CHECK: %[[TO_BOOL:.+]] = icmp ne i32 %[[LOAD]], 0
  // CHECK: %[[FROM_BOOL:.+]] = zext i1 %[[TO_BOOL]] to i8
  // CHECK: store i8 %[[FROM_BOOL]], i8*
  bool b3 = i1;
  // CHECK: %[[LOAD:.+]] = load i1, i1*
  // CHECK: %[[TO_BOOL:.+]] = icmp ne i1 %[[LOAD]], false
  // CHECK: %[[FROM_BOOL:.+]] = zext i1 %[[TO_BOOL]] to i8
  // CHECK: store i8 %[[FROM_BOOL]], i8*
}

