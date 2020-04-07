// REQUIRES: intel_feature_isa_avx_bf16
// RUN: llvm-mc -triple x86_64-unknown-unknown -mattr=+avxbf16 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: {vex} vcvtne2ps2bf16 ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x17,0x72,0xe6]
     {vex} vcvtne2ps2bf16 ymm12, ymm13, ymm14

// CHECK: {vex} vcvtne2ps2bf16 xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x13,0x72,0xe6]
     {vex} vcvtne2ps2bf16 xmm12, xmm13, xmm14

// CHECK: {vex} vcvtne2ps2bf16 ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x17,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10]
     {vex} vcvtne2ps2bf16 ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: {vex} vcvtne2ps2bf16 ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x17,0x72,0xa4,0x80,0x23,0x01,0x00,0x00]
     {vex} vcvtne2ps2bf16 ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK: {vex} vcvtne2ps2bf16 ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x17,0x72,0x25,0x00,0x00,0x00,0x00]
     {vex} vcvtne2ps2bf16 ymm12, ymm13, ymmword ptr [rip]

// CHECK: {vex} vcvtne2ps2bf16 ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x17,0x72,0x24,0x6d,0x00,0xfc,0xff,0xff]
     {vex} vcvtne2ps2bf16 ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK: {vex} vcvtne2ps2bf16 xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x13,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10]
     {vex} vcvtne2ps2bf16 xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: {vex} vcvtne2ps2bf16 xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x13,0x72,0xa4,0x80,0x23,0x01,0x00,0x00]
     {vex} vcvtne2ps2bf16 xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK: {vex} vcvtne2ps2bf16 xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x13,0x72,0x25,0x00,0x00,0x00,0x00]
     {vex} vcvtne2ps2bf16 xmm12, xmm13, xmmword ptr [rip]

// CHECK: {vex} vcvtne2ps2bf16 xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x13,0x72,0x24,0x6d,0x00,0xfe,0xff,0xff]
     {vex} vcvtne2ps2bf16 xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK: {vex} vcvtneps2bf16 xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0x7a,0x72,0xe5]
     {vex} vcvtneps2bf16 xmm12, xmm13

// CHECK: {vex} vcvtneps2bf16 xmm12, ymm13
// CHECK: encoding: [0xc4,0x42,0x7e,0x72,0xe5]
     {vex} vcvtneps2bf16 xmm12, ymm13

// CHECK: {vex} vcvtneps2bf16 xmm12, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x7a,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10]
     {vex} vcvtneps2bf16 xmm12, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: {vex} vcvtneps2bf16 xmm12, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x7a,0x72,0xa4,0x80,0x23,0x01,0x00,0x00]
     {vex} vcvtneps2bf16 xmm12, xmmword ptr [r8 + 4*rax + 291]

// CHECK: {vex} vcvtneps2bf16 xmm12, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x7a,0x72,0x25,0x00,0x00,0x00,0x00]
     {vex} vcvtneps2bf16 xmm12, xmmword ptr [rip]

// CHECK: {vex} vcvtneps2bf16 xmm12, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x7a,0x72,0x24,0x6d,0x00,0xfe,0xff,0xff]
     {vex} vcvtneps2bf16 xmm12, xmmword ptr [2*rbp - 512]

// CHECK: {vex} vcvtneps2bf16 xmm12, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x7e,0x72,0x24,0x6d,0x00,0xfc,0xff,0xff]
     {vex} vcvtneps2bf16 xmm12, ymmword ptr [2*rbp - 1024]

// CHECK: {vex} vdpbf16ps ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x16,0x52,0xe6]
     {vex} vdpbf16ps ymm12, ymm13, ymm14

// CHECK: {vex} vdpbf16ps xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x12,0x52,0xe6]
     {vex} vdpbf16ps xmm12, xmm13, xmm14

// CHECK: {vex} vdpbf16ps ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x16,0x52,0xa4,0xf5,0x00,0x00,0x00,0x10]
     {vex} vdpbf16ps ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: {vex} vdpbf16ps ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x16,0x52,0xa4,0x80,0x23,0x01,0x00,0x00]
     {vex} vdpbf16ps ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK: {vex} vdpbf16ps ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x16,0x52,0x25,0x00,0x00,0x00,0x00]
     {vex} vdpbf16ps ymm12, ymm13, ymmword ptr [rip]

// CHECK: {vex} vdpbf16ps ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x16,0x52,0x24,0x6d,0x00,0xfc,0xff,0xff]
     {vex} vdpbf16ps ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK: {vex} vdpbf16ps xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x12,0x52,0xa4,0xf5,0x00,0x00,0x00,0x10]
     {vex} vdpbf16ps xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: {vex} vdpbf16ps xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x12,0x52,0xa4,0x80,0x23,0x01,0x00,0x00]
     {vex} vdpbf16ps xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK: {vex} vdpbf16ps xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x12,0x52,0x25,0x00,0x00,0x00,0x00]
     {vex} vdpbf16ps xmm12, xmm13, xmmword ptr [rip]

// CHECK: {vex} vdpbf16ps xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x12,0x52,0x24,0x6d,0x00,0xfe,0xff,0xff]
     {vex} vdpbf16ps xmm12, xmm13, xmmword ptr [2*rbp - 512]

