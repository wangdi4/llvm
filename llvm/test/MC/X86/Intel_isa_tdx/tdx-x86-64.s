// REQUIRES: intel_feature_isa_tdx
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK: seamcall
// CHECK: encoding: [0x66,0x0f,0x01,0xcf]
seamcall

// CHECK: seamret
// CHECK: encoding: [0x0f,0x01,0xcd]
seamret

// CHECK: seamops
// CHECK: encoding: [0x0f,0x01,0xce]
seamops

// CHECK: tdcall
// CHECK: encoding: [0x0f,0x01,0xcc]
tdcall
