// REQUIRES: intel_feature_isa_avx512_movget
// RUN: llvm-mc -triple i686-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      vmovget  268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xc5,0x94,0xf4,0x00,0x00,0x00,0x10]
               vmovget  268435456(%esp,%esi,8), %zmm2

// CHECK:      vmovget  291(%edi,%eax,4), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xc5,0x94,0x87,0x23,0x01,0x00,0x00]
               vmovget  291(%edi,%eax,4), %zmm2

// CHECK:      vmovget  (%eax), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xc5,0x10]
               vmovget  (%eax), %zmm2

// CHECK:      vmovget  -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xc5,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vmovget  -2048(,%ebp,2), %zmm2

// CHECK:      vmovget  8128(%ecx), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xc5,0x51,0x7f]
               vmovget  8128(%ecx), %zmm2

// CHECK:      vmovget  -8192(%edx), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xc5,0x52,0x80]
               vmovget  -8192(%edx), %zmm2

