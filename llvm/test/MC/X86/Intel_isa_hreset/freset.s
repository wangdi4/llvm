// REQUIRES: intel_feature_isa_hreset
// RUN: llvm-mc --show-encoding -triple i386 %s | FileCheck %s
// RUN: llvm-mc --show-encoding -triple x86_64 %s | FileCheck %s

// CHECK: hreset
// CHECK: encoding: [0xf3,0x0f,0x3a,0xf0,0xc0,0x01]
hreset $1
