// REQUIRES: intel_feature_isa_avx512_vnni_fp16
// RUN: llvm-mc -triple=x86_64-unknown-unknown -mattr=+avx512vnnifp16 --show-encoding < %s  | FileCheck %s

// CHECK:      vdpphps %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x44,0x40,0x52,0xf0]
               vdpphps %zmm24, %zmm23, %zmm22

// CHECK:      vdpphps %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x47,0x52,0xf0]
               vdpphps %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vdpphps  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x44,0x40,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vdpphps  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vdpphps  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x47,0x52,0xb4,0x80,0x23,0x01,0x00,0x00]
               vdpphps  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vdpphps  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x50,0x52,0x35,0x00,0x00,0x00,0x00]
               vdpphps  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vdpphps  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x40,0x52,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vdpphps  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vdpphps  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xc7,0x52,0x71,0x7f]
               vdpphps  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vdpphps  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xd7,0x52,0x72,0x80]
               vdpphps  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

