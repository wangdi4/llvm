// REQUIRES: intel_feature_isa_avx_mpsadbw
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      vmpsadbw $123, %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x11,0x42,0xe6,0x7b]
               vmpsadbw $123, %xmm14, %xmm13, %xmm12

// CHECK:      vmpsadbw $123, %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0x62,0x53,0x16,0x08,0x42,0xe6,0x7b]
               {evex} vmpsadbw $123, %xmm14, %xmm13, %xmm12

// CHECK:      vmpsadbw $123, %xmm14, %xmm13, %xmm12 {%k7}
// CHECK: encoding: [0x62,0x53,0x16,0x0f,0x42,0xe6,0x7b]
               vmpsadbw $123, %xmm14, %xmm13, %xmm12 {%k7}

// CHECK:      vmpsadbw $123, %xmm14, %xmm13, %xmm12 {%k7} {z}
// CHECK: encoding: [0x62,0x53,0x16,0x8f,0x42,0xe6,0x7b]
               vmpsadbw $123, %xmm14, %xmm13, %xmm12 {%k7} {z}

// CHECK:      vmpsadbw $123, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x83,0x46,0x00,0x42,0xf0,0x7b]
               vmpsadbw $123, %xmm24, %xmm23, %xmm22

// CHECK:      vmpsadbw $123, %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x83,0x46,0x07,0x42,0xf0,0x7b]
               vmpsadbw $123, %xmm24, %xmm23, %xmm22 {%k7}

// CHECK:      vmpsadbw $123, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x83,0x46,0x87,0x42,0xf0,0x7b]
               vmpsadbw $123, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK:      vmpsadbw $123, %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x15,0x42,0xe6,0x7b]
               vmpsadbw $123, %ymm14, %ymm13, %ymm12

// CHECK:      vmpsadbw $123, %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0x62,0x53,0x16,0x28,0x42,0xe6,0x7b]
               {evex} vmpsadbw $123, %ymm14, %ymm13, %ymm12

// CHECK:      vmpsadbw $123, %ymm14, %ymm13, %ymm12 {%k7}
// CHECK: encoding: [0x62,0x53,0x16,0x2f,0x42,0xe6,0x7b]
               vmpsadbw $123, %ymm14, %ymm13, %ymm12 {%k7}

// CHECK:      vmpsadbw $123, %ymm14, %ymm13, %ymm12 {%k7} {z}
// CHECK: encoding: [0x62,0x53,0x16,0xaf,0x42,0xe6,0x7b]
               vmpsadbw $123, %ymm14, %ymm13, %ymm12 {%k7} {z}

// CHECK:      vmpsadbw $123, %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x83,0x46,0x20,0x42,0xf0,0x7b]
               vmpsadbw $123, %ymm24, %ymm23, %ymm22

// CHECK:      vmpsadbw $123, %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x83,0x46,0x27,0x42,0xf0,0x7b]
               vmpsadbw $123, %ymm24, %ymm23, %ymm22 {%k7}

// CHECK:      vmpsadbw $123, %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x83,0x46,0xa7,0x42,0xf0,0x7b]
               vmpsadbw $123, %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vmpsadbw  $123, 268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa3,0x46,0x20,0x42,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmpsadbw  $123, 268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK:      vmpsadbw  $123, 291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x46,0x27,0x42,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
               vmpsadbw  $123, 291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK:      vmpsadbw  $123, (%rip), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe3,0x46,0x20,0x42,0x35,0x00,0x00,0x00,0x00,0x7b]
               vmpsadbw  $123, (%rip), %ymm23, %ymm22

// CHECK:      vmpsadbw  $123, -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe3,0x46,0x20,0x42,0x34,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               vmpsadbw  $123, -1024(,%rbp,2), %ymm23, %ymm22

// CHECK:      vmpsadbw  $123, 4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x46,0xa7,0x42,0x71,0x7f,0x7b]
               vmpsadbw  $123, 4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK:      vmpsadbw  $123, -4096(%rdx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x46,0xa7,0x42,0x72,0x80,0x7b]
               vmpsadbw  $123, -4096(%rdx), %ymm23, %ymm22 {%k7} {z}

// CHECK:      vmpsadbw  $123, 268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa3,0x46,0x00,0x42,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmpsadbw  $123, 268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK:      vmpsadbw  $123, 291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x46,0x07,0x42,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
               vmpsadbw  $123, 291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK:      vmpsadbw  $123, (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe3,0x46,0x00,0x42,0x35,0x00,0x00,0x00,0x00,0x7b]
               vmpsadbw  $123, (%rip), %xmm23, %xmm22

// CHECK:      vmpsadbw  $123, -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe3,0x46,0x00,0x42,0x34,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               vmpsadbw  $123, -512(,%rbp,2), %xmm23, %xmm22

// CHECK:      vmpsadbw  $123, 2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x46,0x87,0x42,0x71,0x7f,0x7b]
               vmpsadbw  $123, 2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK:      vmpsadbw  $123, -2048(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x46,0x87,0x42,0x72,0x80,0x7b]
               vmpsadbw  $123, -2048(%rdx), %xmm23, %xmm22 {%k7} {z}
