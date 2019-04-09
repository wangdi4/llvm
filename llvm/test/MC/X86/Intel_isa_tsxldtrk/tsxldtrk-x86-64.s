// REQUIRES: intel_feature_isa_tsxldtrk
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK: encoding: [0xf2,0x0f,0x01,0xe8]
xsusldtrk

// CHECK: encoding: [0xf2,0x0f,0x01,0xe9]
xresldtrk
