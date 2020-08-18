// REQUIRES: intel_feature_isa_avx_mpsadbw
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vmpsadbw xmm12, xmm13, xmm14, 123
// CHECK: encoding: [0xc4,0x43,0x11,0x42,0xe6,0x7b]
               vmpsadbw xmm12, xmm13, xmm14, 123

// CHECK:      vmpsadbw xmm12, xmm13, xmm14, 123
// CHECK: encoding: [0x62,0x53,0x16,0x08,0x42,0xe6,0x7b]
               {evex} vmpsadbw xmm12, xmm13, xmm14, 123

// CHECK:      vmpsadbw xmm22, xmm23, xmm24, 123
// CHECK: encoding: [0x62,0x83,0x46,0x00,0x42,0xf0,0x7b]
               vmpsadbw xmm22, xmm23, xmm24, 123

// CHECK:      vmpsadbw xmm22 {k7}, xmm23, xmm24, 123
// CHECK: encoding: [0x62,0x83,0x46,0x07,0x42,0xf0,0x7b]
               vmpsadbw xmm22 {k7}, xmm23, xmm24, 123

// CHECK:      vmpsadbw xmm22 {k7} {z}, xmm23, xmm24, 123
// CHECK: encoding: [0x62,0x83,0x46,0x87,0x42,0xf0,0x7b]
               vmpsadbw xmm22 {k7} {z}, xmm23, xmm24, 123

// CHECK:      vmpsadbw ymm12, ymm13, ymm14, 123
// CHECK: encoding: [0xc4,0x43,0x15,0x42,0xe6,0x7b]
               vmpsadbw ymm12, ymm13, ymm14, 123

// CHECK:      vmpsadbw ymm12, ymm13, ymm14, 123
// CHECK: encoding: [0x62,0x53,0x16,0x28,0x42,0xe6,0x7b]
               {evex} vmpsadbw ymm12, ymm13, ymm14, 123

// CHECK:      vmpsadbw ymm22, ymm23, ymm24, 123
// CHECK: encoding: [0x62,0x83,0x46,0x20,0x42,0xf0,0x7b]
               vmpsadbw ymm22, ymm23, ymm24, 123

// CHECK:      vmpsadbw ymm22 {k7}, ymm23, ymm24, 123
// CHECK: encoding: [0x62,0x83,0x46,0x27,0x42,0xf0,0x7b]
               vmpsadbw ymm22 {k7}, ymm23, ymm24, 123

// CHECK:      vmpsadbw ymm22 {k7} {z}, ymm23, ymm24, 123
// CHECK: encoding: [0x62,0x83,0x46,0xa7,0x42,0xf0,0x7b]
               vmpsadbw ymm22 {k7} {z}, ymm23, ymm24, 123

// CHECK:      vmpsadbw ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x46,0x20,0x42,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmpsadbw ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      vmpsadbw ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x46,0x27,0x42,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
               vmpsadbw ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK:      vmpsadbw ymm22, ymm23, ymmword ptr [rip], 123
// CHECK: encoding: [0x62,0xe3,0x46,0x20,0x42,0x35,0x00,0x00,0x00,0x00,0x7b]
               vmpsadbw ymm22, ymm23, ymmword ptr [rip], 123

// CHECK:      vmpsadbw ymm22, ymm23, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0x62,0xe3,0x46,0x20,0x42,0x34,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               vmpsadbw ymm22, ymm23, ymmword ptr [2*rbp - 1024], 123

// CHECK:      vmpsadbw ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0x62,0xe3,0x46,0xa7,0x42,0x71,0x7f,0x7b]
               vmpsadbw ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064], 123

// CHECK:      vmpsadbw ymm22 {k7} {z}, ymm23, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0x62,0xe3,0x46,0xa7,0x42,0x72,0x80,0x7b]
               vmpsadbw ymm22 {k7} {z}, ymm23, ymmword ptr [rdx - 4096], 123

// CHECK:      vmpsadbw xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x46,0x00,0x42,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmpsadbw xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      vmpsadbw xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x46,0x07,0x42,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
               vmpsadbw xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      vmpsadbw xmm22, xmm23, xmmword ptr [rip], 123
// CHECK: encoding: [0x62,0xe3,0x46,0x00,0x42,0x35,0x00,0x00,0x00,0x00,0x7b]
               vmpsadbw xmm22, xmm23, xmmword ptr [rip], 123

// CHECK:      vmpsadbw xmm22, xmm23, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0x62,0xe3,0x46,0x00,0x42,0x34,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               vmpsadbw xmm22, xmm23, xmmword ptr [2*rbp - 512], 123

// CHECK:      vmpsadbw xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0x62,0xe3,0x46,0x87,0x42,0x71,0x7f,0x7b]
               vmpsadbw xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032], 123

// CHECK:      vmpsadbw xmm22 {k7} {z}, xmm23, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x46,0x87,0x42,0x72,0x80,0x7b]
               vmpsadbw xmm22 {k7} {z}, xmm23, xmmword ptr [rdx - 2048], 123
