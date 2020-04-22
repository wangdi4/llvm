// REQUIRES: intel_feature_isa_avx_bf16
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avxbf16 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: {vex} vcvtne2ps2bf16 ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x67,0x72,0xd4]
     {vex} vcvtne2ps2bf16 ymm2, ymm3, ymm4

// CHECK: {vex} vcvtne2ps2bf16 xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x63,0x72,0xd4]
     {vex} vcvtne2ps2bf16 xmm2, xmm3, xmm4

// CHECK: {vex} vcvtne2ps2bf16 ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x67,0x72,0x94,0xf4,0x00,0x00,0x00,0x10]
     {vex} vcvtne2ps2bf16 ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: {vex} vcvtne2ps2bf16 ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x67,0x72,0x94,0x87,0x23,0x01,0x00,0x00]
     {vex} vcvtne2ps2bf16 ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: {vex} vcvtne2ps2bf16 ymm2, ymm3, ymmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x67,0x72,0x10]
     {vex} vcvtne2ps2bf16 ymm2, ymm3, ymmword ptr [eax]

// CHECK: {vex} vcvtne2ps2bf16 ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x67,0x72,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {vex} vcvtne2ps2bf16 ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: {vex} vcvtne2ps2bf16 xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x63,0x72,0x94,0xf4,0x00,0x00,0x00,0x10]
     {vex} vcvtne2ps2bf16 xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: {vex} vcvtne2ps2bf16 xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x63,0x72,0x94,0x87,0x23,0x01,0x00,0x00]
     {vex} vcvtne2ps2bf16 xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: {vex} vcvtne2ps2bf16 xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x63,0x72,0x10]
     {vex} vcvtne2ps2bf16 xmm2, xmm3, xmmword ptr [eax]

// CHECK: {vex} vcvtne2ps2bf16 xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x63,0x72,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {vex} vcvtne2ps2bf16 xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: {vex} vcvtneps2bf16 xmm2, xmm3
// CHECK: encoding: [0xc4,0xe2,0x7a,0x72,0xd3]
     {vex} vcvtneps2bf16 xmm2, xmm3

// CHECK: {vex} vcvtneps2bf16 xmm2, ymm3
// CHECK: encoding: [0xc4,0xe2,0x7e,0x72,0xd3]
     {vex} vcvtneps2bf16 xmm2, ymm3

// CHECK: {vex} vcvtneps2bf16 xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x7a,0x72,0x94,0xf4,0x00,0x00,0x00,0x10]
     {vex} vcvtneps2bf16 xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: {vex} vcvtneps2bf16 xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x7a,0x72,0x94,0x87,0x23,0x01,0x00,0x00]
     {vex} vcvtneps2bf16 xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: {vex} vcvtneps2bf16 xmm2, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x7a,0x72,0x10]
     {vex} vcvtneps2bf16 xmm2, xmmword ptr [eax]

// CHECK: {vex} vcvtneps2bf16 xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x7a,0x72,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {vex} vcvtneps2bf16 xmm2, xmmword ptr [2*ebp - 512]

// CHECK: {vex} vcvtneps2bf16 xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x7e,0x72,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {vex} vcvtneps2bf16 xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: {vex} vdpbf16ps ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x66,0x52,0xd4]
     {vex} vdpbf16ps ymm2, ymm3, ymm4

// CHECK: {vex} vdpbf16ps xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x62,0x52,0xd4]
     {vex} vdpbf16ps xmm2, xmm3, xmm4

// CHECK: {vex} vdpbf16ps ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x66,0x52,0x94,0xf4,0x00,0x00,0x00,0x10]
     {vex} vdpbf16ps ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: {vex} vdpbf16ps ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x66,0x52,0x94,0x87,0x23,0x01,0x00,0x00]
     {vex} vdpbf16ps ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: {vex} vdpbf16ps ymm2, ymm3, ymmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x66,0x52,0x10]
     {vex} vdpbf16ps ymm2, ymm3, ymmword ptr [eax]

// CHECK: {vex} vdpbf16ps ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x66,0x52,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {vex} vdpbf16ps ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: {vex} vdpbf16ps xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x62,0x52,0x94,0xf4,0x00,0x00,0x00,0x10]
     {vex} vdpbf16ps xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: {vex} vdpbf16ps xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x62,0x52,0x94,0x87,0x23,0x01,0x00,0x00]
     {vex} vdpbf16ps xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: {vex} vdpbf16ps xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x62,0x52,0x10]
     {vex} vdpbf16ps xmm2, xmm3, xmmword ptr [eax]

// CHECK: {vex} vdpbf16ps xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x62,0x52,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {vex} vdpbf16ps xmm2, xmm3, xmmword ptr [2*ebp - 512]

