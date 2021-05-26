// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0xd4,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmm4, 123

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x10,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmmword ptr [eax], 123

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x91,0xf0,0x07,0x00,0x00,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x92,0x00,0xf8,0xff,0xff,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK:      dvplutsincosw xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0xd3,0x7b]
               dvplutsincosw xmm2, xmm3, 123

// CHECK:      dvplutsincosw xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
               dvplutsincosw xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK:      dvplutsincosw xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
               dvplutsincosw xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK:      dvplutsincosw xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x10,0x7b]
               dvplutsincosw xmm2, xmmword ptr [eax], 123

// CHECK:      dvplutsincosw xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               dvplutsincosw xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK:      dvplutsincosw xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x91,0xf0,0x07,0x00,0x00,0x7b]
               dvplutsincosw xmm2, xmmword ptr [ecx + 2032], 123

// CHECK:      dvplutsincosw xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x92,0x00,0xf8,0xff,0xff,0x7b]
               dvplutsincosw xmm2, xmmword ptr [edx - 2048], 123

