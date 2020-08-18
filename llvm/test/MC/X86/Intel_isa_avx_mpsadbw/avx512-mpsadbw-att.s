// REQUIRES: intel_feature_isa_avx_mpsadbw
// RUN: llvm-mc -triple i386-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      vmpsadbw $123, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x66,0x48,0x42,0xd4,0x7b]
               vmpsadbw $123, %zmm4, %zmm3, %zmm2

// CHECK:      vmpsadbw $123, %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x66,0x4f,0x42,0xd4,0x7b]
               vmpsadbw $123, %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vmpsadbw $123, %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x66,0xcf,0x42,0xd4,0x7b]
               vmpsadbw $123, %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vmpsadbw  $123, 268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x66,0x48,0x42,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
               vmpsadbw  $123, 268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vmpsadbw  $123, 291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x66,0x4f,0x42,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
               vmpsadbw  $123, 291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vmpsadbw  $123, (%eax), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x66,0x48,0x42,0x10,0x7b]
               vmpsadbw  $123, (%eax), %zmm3, %zmm2

// CHECK:      vmpsadbw  $123, -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x66,0x48,0x42,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
               vmpsadbw  $123, -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vmpsadbw  $123, 8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x66,0xcf,0x42,0x51,0x7f,0x7b]
               vmpsadbw  $123, 8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vmpsadbw  $123, -8192(%edx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x66,0xcf,0x42,0x52,0x80,0x7b]
               vmpsadbw  $123, -8192(%edx), %zmm3, %zmm2 {%k7} {z}
