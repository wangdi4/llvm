// REQUIRES: intel_feature_isa_avx_convert
// RUN: llvm-mc -triple x86_64-unknown-unknown -mattr=+avxconvert -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:  vcvt2ps2ph ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x15,0x67,0xe6]
      vcvt2ps2ph ymm12, ymm13, ymm14

// CHECK:  vcvt2ps2ph xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x11,0x67,0xe6]
      vcvt2ps2ph xmm12, xmm13, xmm14

// CHECK:  vcvt2ps2ph ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x15,0x67,0xa4,0xf5,0x00,0x00,0x00,0x10]
      vcvt2ps2ph ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:  vcvt2ps2ph ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x15,0x67,0xa4,0x80,0x23,0x01,0x00,0x00]
      vcvt2ps2ph ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:  vcvt2ps2ph ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x15,0x67,0x25,0x00,0x00,0x00,0x00]
      vcvt2ps2ph ymm12, ymm13, ymmword ptr [rip]

// CHECK:  vcvt2ps2ph ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x15,0x67,0x24,0x6d,0x00,0xfc,0xff,0xff]
      vcvt2ps2ph ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:  vcvt2ps2ph xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x11,0x67,0xa4,0xf5,0x00,0x00,0x00,0x10]
      vcvt2ps2ph xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:  vcvt2ps2ph xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x11,0x67,0xa4,0x80,0x23,0x01,0x00,0x00]
      vcvt2ps2ph xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:  vcvt2ps2ph xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x11,0x67,0x25,0x00,0x00,0x00,0x00]
      vcvt2ps2ph xmm12, xmm13, xmmword ptr [rip]

// CHECK:  vcvt2ps2ph xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x11,0x67,0x24,0x6d,0x00,0xfe,0xff,0xff]
      vcvt2ps2ph xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:  vcvtbf162ph xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0x7a,0x67,0xe5]
      vcvtbf162ph xmm12, xmm13

// CHECK:  vcvtbf162ph ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0x7e,0x67,0xe5]
      vcvtbf162ph ymm12, ymm13

// CHECK:  vcvtbf162ph xmm12, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x7a,0x67,0xa4,0xf5,0x00,0x00,0x00,0x10]
      vcvtbf162ph xmm12, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:  vcvtbf162ph xmm12, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x7a,0x67,0xa4,0x80,0x23,0x01,0x00,0x00]
      vcvtbf162ph xmm12, xmmword ptr [r8 + 4*rax + 291]

// CHECK:  vcvtbf162ph xmm12, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x7a,0x67,0x25,0x00,0x00,0x00,0x00]
      vcvtbf162ph xmm12, xmmword ptr [rip]

// CHECK:  vcvtbf162ph xmm12, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x7a,0x67,0x24,0x6d,0x00,0xfe,0xff,0xff]
      vcvtbf162ph xmm12, xmmword ptr [2*rbp - 512]

// CHECK:  vcvtbf162ph ymm12, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x7e,0x67,0xa4,0xf5,0x00,0x00,0x00,0x10]
      vcvtbf162ph ymm12, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:  vcvtbf162ph ymm12, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x7e,0x67,0xa4,0x80,0x23,0x01,0x00,0x00]
      vcvtbf162ph ymm12, ymmword ptr [r8 + 4*rax + 291]

// CHECK:  vcvtbf162ph ymm12, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x7e,0x67,0x25,0x00,0x00,0x00,0x00]
      vcvtbf162ph ymm12, ymmword ptr [rip]

// CHECK:  vcvtbf162ph ymm12, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x7e,0x67,0x24,0x6d,0x00,0xfc,0xff,0xff]
      vcvtbf162ph ymm12, ymmword ptr [2*rbp - 1024]

// CHECK:  vcvtneph2bf16 xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0x7b,0x67,0xe5]
      vcvtneph2bf16 xmm12, xmm13

// CHECK:  vcvtneph2bf16 ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0x7f,0x67,0xe5]
      vcvtneph2bf16 ymm12, ymm13

// CHECK:  vcvtneph2bf16 xmm12, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x7b,0x67,0xa4,0xf5,0x00,0x00,0x00,0x10]
      vcvtneph2bf16 xmm12, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:  vcvtneph2bf16 xmm12, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x7b,0x67,0xa4,0x80,0x23,0x01,0x00,0x00]
      vcvtneph2bf16 xmm12, xmmword ptr [r8 + 4*rax + 291]

// CHECK:  vcvtneph2bf16 xmm12, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x7b,0x67,0x25,0x00,0x00,0x00,0x00]
      vcvtneph2bf16 xmm12, xmmword ptr [rip]

// CHECK:  vcvtneph2bf16 xmm12, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x7b,0x67,0x24,0x6d,0x00,0xfe,0xff,0xff]
      vcvtneph2bf16 xmm12, xmmword ptr [2*rbp - 512]

// CHECK:  vcvtneph2bf16 ymm12, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x7f,0x67,0xa4,0xf5,0x00,0x00,0x00,0x10]
      vcvtneph2bf16 ymm12, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:  vcvtneph2bf16 ymm12, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x7f,0x67,0xa4,0x80,0x23,0x01,0x00,0x00]
      vcvtneph2bf16 ymm12, ymmword ptr [r8 + 4*rax + 291]

// CHECK:  vcvtneph2bf16 ymm12, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x7f,0x67,0x25,0x00,0x00,0x00,0x00]
      vcvtneph2bf16 ymm12, ymmword ptr [rip]

// CHECK:  vcvtneph2bf16 ymm12, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x7f,0x67,0x24,0x6d,0x00,0xfc,0xff,0xff]
      vcvtneph2bf16 ymm12, ymmword ptr [2*rbp - 1024]

