// REQUIRES: intel_feature_isa_tdx
// RUN: llvm-mc -triple i386-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK: tdcall
// CHECK: encoding: [0x0f,0x01,0xcc]
tdcall
