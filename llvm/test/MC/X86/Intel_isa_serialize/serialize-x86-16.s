// REQUIRES: intel_feature_isa_serialize
// RUN: llvm-mc -triple i386-unknown-unknown-code16 --show-encoding %s | FileCheck %s

// CHECK: serialize
// CHECK: encoding: [0x0f,0x01,0xe8]
serialize
