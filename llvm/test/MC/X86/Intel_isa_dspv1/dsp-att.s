// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      dvpcr2bfrsw $123, %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0xd4,0x7b]
               dvpcr2bfrsw $123, %xmm4, %xmm3, %xmm2

// CHECK:      dvpcr2bfrsw  $123, 268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
               dvpcr2bfrsw  $123, 268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      dvpcr2bfrsw  $123, 291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
               dvpcr2bfrsw  $123, 291(%edi,%eax,4), %xmm3, %xmm2

// CHECK:      dvpcr2bfrsw  $123, (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x10,0x7b]
               dvpcr2bfrsw  $123, (%eax), %xmm3, %xmm2

// CHECK:      dvpcr2bfrsw  $123, -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               dvpcr2bfrsw  $123, -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      dvpcr2bfrsw  $123, 2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x91,0xf0,0x07,0x00,0x00,0x7b]
               dvpcr2bfrsw  $123, 2032(%ecx), %xmm3, %xmm2

// CHECK:      dvpcr2bfrsw  $123, -2048(%edx), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x92,0x00,0xf8,0xff,0xff,0x7b]
               dvpcr2bfrsw  $123, -2048(%edx), %xmm3, %xmm2

// CHECK:      dvplutsincosw $123, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0xd3,0x7b]
               dvplutsincosw $123, %xmm3, %xmm2

// CHECK:      dvplutsincosw  $123, 268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
               dvplutsincosw  $123, 268435456(%esp,%esi,8), %xmm2

// CHECK:      dvplutsincosw  $123, 291(%edi,%eax,4), %xmm2
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
               dvplutsincosw  $123, 291(%edi,%eax,4), %xmm2

// CHECK:      dvplutsincosw  $123, (%eax), %xmm2
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x10,0x7b]
               dvplutsincosw  $123, (%eax), %xmm2

// CHECK:      dvplutsincosw  $123, -512(,%ebp,2), %xmm2
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               dvplutsincosw  $123, -512(,%ebp,2), %xmm2

// CHECK:      dvplutsincosw  $123, 2032(%ecx), %xmm2
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x91,0xf0,0x07,0x00,0x00,0x7b]
               dvplutsincosw  $123, 2032(%ecx), %xmm2

// CHECK:      dvplutsincosw  $123, -2048(%edx), %xmm2
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x92,0x00,0xf8,0xff,0xff,0x7b]
               dvplutsincosw  $123, -2048(%edx), %xmm2

