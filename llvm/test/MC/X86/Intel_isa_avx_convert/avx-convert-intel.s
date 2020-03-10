// REQUIRES: intel_feature_isa_avx_convert
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avxconvert -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:  vcvt2ps2ph ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x65,0x67,0xd4]
      vcvt2ps2ph ymm2, ymm3, ymm4

// CHECK:  vcvt2ps2ph xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x61,0x67,0xd4]
      vcvt2ps2ph xmm2, xmm3, xmm4

// CHECK:  vcvt2ps2ph ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x65,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
      vcvt2ps2ph ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:  vcvt2ps2ph ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x65,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
      vcvt2ps2ph ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK:  vcvt2ps2ph ymm2, ymm3, ymmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x65,0x67,0x10]
      vcvt2ps2ph ymm2, ymm3, ymmword ptr [eax]

// CHECK:  vcvt2ps2ph ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x65,0x67,0x14,0x6d,0x00,0xfc,0xff,0xff]
      vcvt2ps2ph ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK:  vcvt2ps2ph xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x61,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
      vcvt2ps2ph xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:  vcvt2ps2ph xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x61,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
      vcvt2ps2ph xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:  vcvt2ps2ph xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x61,0x67,0x10]
      vcvt2ps2ph xmm2, xmm3, xmmword ptr [eax]

// CHECK:  vcvt2ps2ph xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x61,0x67,0x14,0x6d,0x00,0xfe,0xff,0xff]
      vcvt2ps2ph xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:  vcvtbf162ph xmm2, xmm3
// CHECK: encoding: [0xc4,0xe2,0x7a,0x67,0xd3]
      vcvtbf162ph xmm2, xmm3

// CHECK:  vcvtbf162ph ymm2, ymm3
// CHECK: encoding: [0xc4,0xe2,0x7e,0x67,0xd3]
      vcvtbf162ph ymm2, ymm3

// CHECK:  vcvtbf162ph xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x7a,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
      vcvtbf162ph xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:  vcvtbf162ph xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x7a,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
      vcvtbf162ph xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK:  vcvtbf162ph xmm2, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x7a,0x67,0x10]
      vcvtbf162ph xmm2, xmmword ptr [eax]

// CHECK:  vcvtbf162ph xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x7a,0x67,0x14,0x6d,0x00,0xfe,0xff,0xff]
      vcvtbf162ph xmm2, xmmword ptr [2*ebp - 512]

// CHECK:  vcvtbf162ph ymm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x7e,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
      vcvtbf162ph ymm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:  vcvtbf162ph ymm2, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x7e,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
      vcvtbf162ph ymm2, ymmword ptr [edi + 4*eax + 291]

// CHECK:  vcvtbf162ph ymm2, ymmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x7e,0x67,0x10]
      vcvtbf162ph ymm2, ymmword ptr [eax]

// CHECK:  vcvtbf162ph ymm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x7e,0x67,0x14,0x6d,0x00,0xfc,0xff,0xff]
      vcvtbf162ph ymm2, ymmword ptr [2*ebp - 1024]

// CHECK:  vcvtneph2bf16 xmm2, xmm3
// CHECK: encoding: [0xc4,0xe2,0x7b,0x67,0xd3]
      vcvtneph2bf16 xmm2, xmm3

// CHECK:  vcvtneph2bf16 ymm2, ymm3
// CHECK: encoding: [0xc4,0xe2,0x7f,0x67,0xd3]
      vcvtneph2bf16 ymm2, ymm3

// CHECK:  vcvtneph2bf16 xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x7b,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
      vcvtneph2bf16 xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:  vcvtneph2bf16 xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x7b,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
      vcvtneph2bf16 xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK:  vcvtneph2bf16 xmm2, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x7b,0x67,0x10]
      vcvtneph2bf16 xmm2, xmmword ptr [eax]

// CHECK:  vcvtneph2bf16 xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x7b,0x67,0x14,0x6d,0x00,0xfe,0xff,0xff]
      vcvtneph2bf16 xmm2, xmmword ptr [2*ebp - 512]

// CHECK:  vcvtneph2bf16 ymm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x7f,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
      vcvtneph2bf16 ymm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:  vcvtneph2bf16 ymm2, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x7f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
      vcvtneph2bf16 ymm2, ymmword ptr [edi + 4*eax + 291]

// CHECK:  vcvtneph2bf16 ymm2, ymmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x7f,0x67,0x10]
      vcvtneph2bf16 ymm2, ymmword ptr [eax]

// CHECK:  vcvtneph2bf16 ymm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x7f,0x67,0x14,0x6d,0x00,0xfc,0xff,0xff]
      vcvtneph2bf16 ymm2, ymmword ptr [2*ebp - 1024]

