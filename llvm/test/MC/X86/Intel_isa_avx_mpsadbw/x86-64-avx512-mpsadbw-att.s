// REQUIRES: intel_feature_isa_avx_mpsadbw
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      vmpsadbw $123, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x83,0x46,0x40,0x42,0xf0,0x7b]
               vmpsadbw $123, %zmm24, %zmm23, %zmm22

// CHECK:      vmpsadbw $123, %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x83,0x46,0x47,0x42,0xf0,0x7b]
               vmpsadbw $123, %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vmpsadbw $123, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x83,0x46,0xc7,0x42,0xf0,0x7b]
               vmpsadbw $123, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vmpsadbw  $123, 268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0x46,0x40,0x42,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmpsadbw  $123, 268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vmpsadbw  $123, 291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x46,0x47,0x42,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
               vmpsadbw  $123, 291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vmpsadbw  $123, (%rip), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe3,0x46,0x40,0x42,0x35,0x00,0x00,0x00,0x00,0x7b]
               vmpsadbw  $123, (%rip), %zmm23, %zmm22

// CHECK:      vmpsadbw  $123, -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe3,0x46,0x40,0x42,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
               vmpsadbw  $123, -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vmpsadbw  $123, 8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x46,0xc7,0x42,0x71,0x7f,0x7b]
               vmpsadbw  $123, 8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vmpsadbw  $123, -8192(%rdx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x46,0xc7,0x42,0x72,0x80,0x7b]
               vmpsadbw  $123, -8192(%rdx), %zmm23, %zmm22 {%k7} {z}
