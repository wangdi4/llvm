// REQUIRES: intel_feature_isa_amx_fp16
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      tdpfp16ps       %tmm5, %tmm4, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x52,0x5d,0xdc]
               tdpfp16ps       %tmm5, %tmm4, %tmm3
