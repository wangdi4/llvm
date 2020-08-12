// REQUIRES: intel_feature_isa_avx_mpsadbw
// RUN: llvm-mc -triple i386-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vmpsadbw xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x61,0x42,0xd4,0x7b]
               vmpsadbw xmm2, xmm3, xmm4, 123

// CHECK:      vmpsadbw xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0x62,0xf3,0x66,0x08,0x42,0xd4,0x7b]
               {evex} vmpsadbw xmm2, xmm3, xmm4, 123

// CHECK:      vmpsadbw xmm2 {k7}, xmm3, xmm4, 123
// CHECK: encoding: [0x62,0xf3,0x66,0x0f,0x42,0xd4,0x7b]
               vmpsadbw xmm2 {k7}, xmm3, xmm4, 123

// CHECK:      vmpsadbw xmm2 {k7} {z}, xmm3, xmm4, 123
// CHECK: encoding: [0x62,0xf3,0x66,0x8f,0x42,0xd4,0x7b]
               vmpsadbw xmm2 {k7} {z}, xmm3, xmm4, 123

// CHECK:      vmpsadbw ymm2, ymm3, ymm4, 123
// CHECK: encoding: [0xc4,0xe3,0x65,0x42,0xd4,0x7b]
               vmpsadbw ymm2, ymm3, ymm4, 123

// CHECK:      vmpsadbw ymm2, ymm3, ymm4, 123
// CHECK: encoding: [0x62,0xf3,0x66,0x28,0x42,0xd4,0x7b]
               {evex} vmpsadbw ymm2, ymm3, ymm4, 123

// CHECK:      vmpsadbw ymm2 {k7}, ymm3, ymm4, 123
// CHECK: encoding: [0x62,0xf3,0x66,0x2f,0x42,0xd4,0x7b]
               vmpsadbw ymm2 {k7}, ymm3, ymm4, 123

// CHECK:      vmpsadbw ymm2 {k7} {z}, ymm3, ymm4, 123
// CHECK: encoding: [0x62,0xf3,0x66,0xaf,0x42,0xd4,0x7b]
               vmpsadbw ymm2 {k7} {z}, ymm3, ymm4, 123

// CHECK:      vmpsadbw ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x65,0x42,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
               vmpsadbw ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456], 123

// CHECK:      vmpsadbw ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x66,0x2f,0x42,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
               vmpsadbw ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291], 123

// CHECK:      vmpsadbw ymm2, ymm3, ymmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x65,0x42,0x10,0x7b]
               vmpsadbw ymm2, ymm3, ymmword ptr [eax], 123

// CHECK:      vmpsadbw ymm2, ymm3, ymmword ptr [2*ebp - 1024], 123
// CHECK: encoding: [0xc4,0xe3,0x65,0x42,0x14,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               vmpsadbw ymm2, ymm3, ymmword ptr [2*ebp - 1024], 123

// CHECK:      vmpsadbw ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064], 123
// CHECK: encoding: [0x62,0xf3,0x66,0xaf,0x42,0x51,0x7f,0x7b]
               vmpsadbw ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064], 123

// CHECK:      vmpsadbw ymm2 {k7} {z}, ymm3, ymmword ptr [edx - 4096], 123
// CHECK: encoding: [0x62,0xf3,0x66,0xaf,0x42,0x52,0x80,0x7b]
               vmpsadbw ymm2 {k7} {z}, ymm3, ymmword ptr [edx - 4096], 123

// CHECK:      vmpsadbw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0x42,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
               vmpsadbw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK:      vmpsadbw xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x66,0x0f,0x42,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
               vmpsadbw xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK:      vmpsadbw xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0x42,0x10,0x7b]
               vmpsadbw xmm2, xmm3, xmmword ptr [eax], 123

// CHECK:      vmpsadbw xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0x42,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               vmpsadbw xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK:      vmpsadbw xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0x62,0xf3,0x66,0x8f,0x42,0x51,0x7f,0x7b]
               vmpsadbw xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK:      vmpsadbw xmm2 {k7} {z}, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x66,0x8f,0x42,0x52,0x80,0x7b]
               vmpsadbw xmm2 {k7} {z}, xmm3, xmmword ptr [edx - 2048], 123
