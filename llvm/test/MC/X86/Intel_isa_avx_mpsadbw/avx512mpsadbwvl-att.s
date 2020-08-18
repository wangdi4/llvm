// REQUIRES: intel_feature_isa_avx_mpsadbw
// RUN: llvm-mc -triple i386-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      vmpsadbw $123, %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe3,0x61,0x42,0xd4,0x7b]
               vmpsadbw $123, %xmm4, %xmm3, %xmm2

// CHECK:      vmpsadbw $123, %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf3,0x66,0x08,0x42,0xd4,0x7b]
               {evex} vmpsadbw $123, %xmm4, %xmm3, %xmm2

// CHECK:      vmpsadbw $123, %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x66,0x0f,0x42,0xd4,0x7b]
               vmpsadbw $123, %xmm4, %xmm3, %xmm2 {%k7}

// CHECK:      vmpsadbw $123, %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x66,0x8f,0x42,0xd4,0x7b]
               vmpsadbw $123, %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK:      vmpsadbw $123, %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe3,0x65,0x42,0xd4,0x7b]
               vmpsadbw $123, %ymm4, %ymm3, %ymm2

// CHECK:      vmpsadbw $123, %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf3,0x66,0x28,0x42,0xd4,0x7b]
               {evex} vmpsadbw $123, %ymm4, %ymm3, %ymm2

// CHECK:      vmpsadbw $123, %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x66,0x2f,0x42,0xd4,0x7b]
               vmpsadbw $123, %ymm4, %ymm3, %ymm2 {%k7}

// CHECK:      vmpsadbw $123, %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x66,0xaf,0x42,0xd4,0x7b]
               vmpsadbw $123, %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vmpsadbw  $123, 268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe3,0x65,0x42,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
               vmpsadbw  $123, 268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK:      vmpsadbw  $123, 291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x66,0x2f,0x42,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
               vmpsadbw  $123, 291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK:      vmpsadbw  $123, (%eax), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe3,0x65,0x42,0x10,0x7b]
               vmpsadbw  $123, (%eax), %ymm3, %ymm2

// CHECK:      vmpsadbw  $123, -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe3,0x65,0x42,0x14,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               vmpsadbw  $123, -1024(,%ebp,2), %ymm3, %ymm2

// CHECK:      vmpsadbw  $123, 4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x66,0xaf,0x42,0x51,0x7f,0x7b]
               vmpsadbw  $123, 4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK:      vmpsadbw  $123, -4096(%edx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x66,0xaf,0x42,0x52,0x80,0x7b]
               vmpsadbw  $123, -4096(%edx), %ymm3, %ymm2 {%k7} {z}

// CHECK:      vmpsadbw  $123, 268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe3,0x61,0x42,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
               vmpsadbw  $123, 268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      vmpsadbw  $123, 291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x66,0x0f,0x42,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
               vmpsadbw  $123, 291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK:      vmpsadbw  $123, (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe3,0x61,0x42,0x10,0x7b]
               vmpsadbw  $123, (%eax), %xmm3, %xmm2

// CHECK:      vmpsadbw  $123, -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe3,0x61,0x42,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               vmpsadbw  $123, -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      vmpsadbw  $123, 2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x66,0x8f,0x42,0x51,0x7f,0x7b]
               vmpsadbw  $123, 2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK:      vmpsadbw  $123, -2048(%edx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x66,0x8f,0x42,0x52,0x80,0x7b]
               vmpsadbw  $123, -2048(%edx), %xmm3, %xmm2 {%k7} {z}
