// RUN: %clang_cc1 -fintel-compatibility -triple=x86_64-unknown-linux-gnu %s -emit-llvm -verify -o - | FileCheck %s
// expected-no-diagnostics

void foo() {
  __memory_barrier();
  // CHECK: fence syncscope("singlethread") seq_cst
}

