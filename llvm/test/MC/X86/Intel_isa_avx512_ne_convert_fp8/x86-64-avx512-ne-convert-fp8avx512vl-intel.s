// REQUIRES: intel_feature_isa_avx512_ne_convert_fp8
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vcvtbias2ph2bf8 ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x45,0x20,0x74,0xf0]
          vcvtbias2ph2bf8 ymm22, ymm23, ymm24

// CHECK: vcvtbias2ph2bf8 xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x45,0x00,0x74,0xf0]
          vcvtbias2ph2bf8 xmm22, xmm23, xmm24

// CHECK: vcvtbias2ph2bf8 ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x20,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8 ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbias2ph2bf8 ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x45,0x20,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8 ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbias2ph2bf8 ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x45,0x30,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2bf8 ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vcvtbias2ph2bf8 ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x45,0x20,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2bf8 ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtbias2ph2bf8 ymm22, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x45,0x20,0x74,0x71,0x7f]
          vcvtbias2ph2bf8 ymm22, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vcvtbias2ph2bf8 ymm22, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe2,0x45,0x30,0x74,0x72,0x80]
          vcvtbias2ph2bf8 ymm22, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vcvtbias2ph2bf8 xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x00,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8 xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbias2ph2bf8 xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x45,0x00,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8 xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbias2ph2bf8 xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x45,0x10,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2bf8 xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vcvtbias2ph2bf8 xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x45,0x00,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2bf8 xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vcvtbias2ph2bf8 xmm22, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x45,0x00,0x74,0x71,0x7f]
          vcvtbias2ph2bf8 xmm22, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vcvtbias2ph2bf8 xmm22, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe2,0x45,0x10,0x74,0x72,0x80]
          vcvtbias2ph2bf8 xmm22, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vcvtbias2ph2bf8s ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x45,0x20,0x74,0xf0]
          vcvtbias2ph2bf8s ymm22, ymm23, ymm24

// CHECK: vcvtbias2ph2bf8s xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x45,0x00,0x74,0xf0]
          vcvtbias2ph2bf8s xmm22, xmm23, xmm24

// CHECK: vcvtbias2ph2bf8s ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x20,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8s ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbias2ph2bf8s ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x20,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8s ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbias2ph2bf8s ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2bf8s ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vcvtbias2ph2bf8s ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2bf8s ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtbias2ph2bf8s ymm22, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x74,0x71,0x7f]
          vcvtbias2ph2bf8s ymm22, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vcvtbias2ph2bf8s ymm22, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x74,0x72,0x80]
          vcvtbias2ph2bf8s ymm22, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vcvtbias2ph2bf8s xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x00,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8s xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbias2ph2bf8s xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x00,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8s xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbias2ph2bf8s xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2bf8s xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vcvtbias2ph2bf8s xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2bf8s xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vcvtbias2ph2bf8s xmm22, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x74,0x71,0x7f]
          vcvtbias2ph2bf8s xmm22, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vcvtbias2ph2bf8s xmm22, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x74,0x72,0x80]
          vcvtbias2ph2bf8s xmm22, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vcvtbias2ph2hf8 ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x45,0x20,0x18,0xf0]
          vcvtbias2ph2hf8 ymm22, ymm23, ymm24

// CHECK: vcvtbias2ph2hf8 xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x45,0x00,0x18,0xf0]
          vcvtbias2ph2hf8 xmm22, xmm23, xmm24

// CHECK: vcvtbias2ph2hf8 ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x20,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8 ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbias2ph2hf8 ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x20,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8 ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbias2ph2hf8 ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2hf8 ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vcvtbias2ph2hf8 ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x18,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2hf8 ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtbias2ph2hf8 ymm22, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x18,0x71,0x7f]
          vcvtbias2ph2hf8 ymm22, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vcvtbias2ph2hf8 ymm22, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x18,0x72,0x80]
          vcvtbias2ph2hf8 ymm22, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vcvtbias2ph2hf8 xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x00,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8 xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbias2ph2hf8 xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x00,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8 xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbias2ph2hf8 xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2hf8 xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vcvtbias2ph2hf8 xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x18,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2hf8 xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vcvtbias2ph2hf8 xmm22, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x18,0x71,0x7f]
          vcvtbias2ph2hf8 xmm22, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vcvtbias2ph2hf8 xmm22, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x18,0x72,0x80]
          vcvtbias2ph2hf8 xmm22, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vcvtbias2ph2hf8s ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x45,0x20,0x1b,0xf0]
          vcvtbias2ph2hf8s ymm22, ymm23, ymm24

// CHECK: vcvtbias2ph2hf8s xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x45,0x00,0x1b,0xf0]
          vcvtbias2ph2hf8s xmm22, xmm23, xmm24

// CHECK: vcvtbias2ph2hf8s ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x20,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8s ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbias2ph2hf8s ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x20,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8s ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbias2ph2hf8s ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2hf8s ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vcvtbias2ph2hf8s ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x1b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2hf8s ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtbias2ph2hf8s ymm22, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x1b,0x71,0x7f]
          vcvtbias2ph2hf8s ymm22, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vcvtbias2ph2hf8s ymm22, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x1b,0x72,0x80]
          vcvtbias2ph2hf8s ymm22, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vcvtbias2ph2hf8s xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x00,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8s xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbias2ph2hf8s xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x00,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8s xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbias2ph2hf8s xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2hf8s xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vcvtbias2ph2hf8s xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x1b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2hf8s xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vcvtbias2ph2hf8s xmm22, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x1b,0x71,0x7f]
          vcvtbias2ph2hf8s xmm22, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vcvtbias2ph2hf8s xmm22, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x1b,0x72,0x80]
          vcvtbias2ph2hf8s xmm22, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vcvtbiasph2bf8 xmm22, xmm23
// CHECK: encoding: [0x62,0xa2,0x7c,0x08,0x74,0xf7]
          vcvtbiasph2bf8 xmm22, xmm23

// CHECK: vcvtbiasph2bf8 xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa2,0x7c,0x0f,0x74,0xf7]
          vcvtbiasph2bf8 xmm22 {k7}, xmm23

// CHECK: vcvtbiasph2bf8 xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa2,0x7c,0x8f,0x74,0xf7]
          vcvtbiasph2bf8 xmm22 {k7} {z}, xmm23

// CHECK: vcvtbiasph2bf8 xmm22, ymm23
// CHECK: encoding: [0x62,0xa2,0x7c,0x28,0x74,0xf7]
          vcvtbiasph2bf8 xmm22, ymm23

// CHECK: vcvtbiasph2bf8 xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa2,0x7c,0x2f,0x74,0xf7]
          vcvtbiasph2bf8 xmm22 {k7}, ymm23

// CHECK: vcvtbiasph2bf8 xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa2,0x7c,0xaf,0x74,0xf7]
          vcvtbiasph2bf8 xmm22 {k7} {z}, ymm23

// CHECK: vcvtbiasph2bf8 xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7c,0x08,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8 xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbiasph2bf8 xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7c,0x0f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8 xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbiasph2bf8 xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x7c,0x18,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2bf8 xmm22, word ptr [rip]{1to8}

// CHECK: vcvtbiasph2bf8 xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x7c,0x08,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2bf8 xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtbiasph2bf8 xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x7c,0x8f,0x74,0x71,0x7f]
          vcvtbiasph2bf8 xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtbiasph2bf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe2,0x7c,0x9f,0x74,0x72,0x80]
          vcvtbiasph2bf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtbiasph2bf8 xmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x7c,0x38,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2bf8 xmm22, word ptr [rip]{1to16}

// CHECK: vcvtbiasph2bf8 xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x7c,0x28,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2bf8 xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtbiasph2bf8 xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x7c,0xaf,0x74,0x71,0x7f]
          vcvtbiasph2bf8 xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtbiasph2bf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe2,0x7c,0xbf,0x74,0x72,0x80]
          vcvtbiasph2bf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtbiasph2bf8s xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x74,0xf7]
          vcvtbiasph2bf8s xmm22, xmm23

// CHECK: vcvtbiasph2bf8s xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x74,0xf7]
          vcvtbiasph2bf8s xmm22 {k7}, xmm23

// CHECK: vcvtbiasph2bf8s xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x74,0xf7]
          vcvtbiasph2bf8s xmm22 {k7} {z}, xmm23

// CHECK: vcvtbiasph2bf8s xmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x74,0xf7]
          vcvtbiasph2bf8s xmm22, ymm23

// CHECK: vcvtbiasph2bf8s xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x74,0xf7]
          vcvtbiasph2bf8s xmm22 {k7}, ymm23

// CHECK: vcvtbiasph2bf8s xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x74,0xf7]
          vcvtbiasph2bf8s xmm22 {k7} {z}, ymm23

// CHECK: vcvtbiasph2bf8s xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8s xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbiasph2bf8s xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8s xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbiasph2bf8s xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2bf8s xmm22, word ptr [rip]{1to8}

// CHECK: vcvtbiasph2bf8s xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2bf8s xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtbiasph2bf8s xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x74,0x71,0x7f]
          vcvtbiasph2bf8s xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtbiasph2bf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x74,0x72,0x80]
          vcvtbiasph2bf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtbiasph2bf8s xmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2bf8s xmm22, word ptr [rip]{1to16}

// CHECK: vcvtbiasph2bf8s xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2bf8s xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtbiasph2bf8s xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x74,0x71,0x7f]
          vcvtbiasph2bf8s xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtbiasph2bf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x74,0x72,0x80]
          vcvtbiasph2bf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtbiasph2hf8 xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x18,0xf7]
          vcvtbiasph2hf8 xmm22, xmm23

// CHECK: vcvtbiasph2hf8 xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x18,0xf7]
          vcvtbiasph2hf8 xmm22 {k7}, xmm23

// CHECK: vcvtbiasph2hf8 xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x18,0xf7]
          vcvtbiasph2hf8 xmm22 {k7} {z}, xmm23

// CHECK: vcvtbiasph2hf8 xmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x18,0xf7]
          vcvtbiasph2hf8 xmm22, ymm23

// CHECK: vcvtbiasph2hf8 xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x18,0xf7]
          vcvtbiasph2hf8 xmm22 {k7}, ymm23

// CHECK: vcvtbiasph2hf8 xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x18,0xf7]
          vcvtbiasph2hf8 xmm22 {k7} {z}, ymm23

// CHECK: vcvtbiasph2hf8 xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8 xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbiasph2hf8 xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8 xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbiasph2hf8 xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2hf8 xmm22, word ptr [rip]{1to8}

// CHECK: vcvtbiasph2hf8 xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x18,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2hf8 xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtbiasph2hf8 xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x18,0x71,0x7f]
          vcvtbiasph2hf8 xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtbiasph2hf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x18,0x72,0x80]
          vcvtbiasph2hf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtbiasph2hf8 xmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2hf8 xmm22, word ptr [rip]{1to16}

// CHECK: vcvtbiasph2hf8 xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x18,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2hf8 xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtbiasph2hf8 xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x18,0x71,0x7f]
          vcvtbiasph2hf8 xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtbiasph2hf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x18,0x72,0x80]
          vcvtbiasph2hf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtbiasph2hf8s xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x1b,0xf7]
          vcvtbiasph2hf8s xmm22, xmm23

// CHECK: vcvtbiasph2hf8s xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x1b,0xf7]
          vcvtbiasph2hf8s xmm22 {k7}, xmm23

// CHECK: vcvtbiasph2hf8s xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x1b,0xf7]
          vcvtbiasph2hf8s xmm22 {k7} {z}, xmm23

// CHECK: vcvtbiasph2hf8s xmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x1b,0xf7]
          vcvtbiasph2hf8s xmm22, ymm23

// CHECK: vcvtbiasph2hf8s xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x1b,0xf7]
          vcvtbiasph2hf8s xmm22 {k7}, ymm23

// CHECK: vcvtbiasph2hf8s xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x1b,0xf7]
          vcvtbiasph2hf8s xmm22 {k7} {z}, ymm23

// CHECK: vcvtbiasph2hf8s xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8s xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbiasph2hf8s xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8s xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbiasph2hf8s xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2hf8s xmm22, word ptr [rip]{1to8}

// CHECK: vcvtbiasph2hf8s xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x1b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2hf8s xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtbiasph2hf8s xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x1b,0x71,0x7f]
          vcvtbiasph2hf8s xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtbiasph2hf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x1b,0x72,0x80]
          vcvtbiasph2hf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtbiasph2hf8s xmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2hf8s xmm22, word ptr [rip]{1to16}

// CHECK: vcvtbiasph2hf8s xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x1b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2hf8s xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtbiasph2hf8s xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x1b,0x71,0x7f]
          vcvtbiasph2hf8s xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtbiasph2hf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x1b,0x72,0x80]
          vcvtbiasph2hf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtne2ph2bf8 ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x47,0x20,0x74,0xf0]
          vcvtne2ph2bf8 ymm22, ymm23, ymm24

// CHECK: vcvtne2ph2bf8 xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x47,0x00,0x74,0xf0]
          vcvtne2ph2bf8 xmm22, xmm23, xmm24

// CHECK: vcvtne2ph2bf8 ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x47,0x20,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8 ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtne2ph2bf8 ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x47,0x20,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8 ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtne2ph2bf8 ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x47,0x30,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2bf8 ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vcvtne2ph2bf8 ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x47,0x20,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2bf8 ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtne2ph2bf8 ymm22, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x47,0x20,0x74,0x71,0x7f]
          vcvtne2ph2bf8 ymm22, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vcvtne2ph2bf8 ymm22, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe2,0x47,0x30,0x74,0x72,0x80]
          vcvtne2ph2bf8 ymm22, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vcvtne2ph2bf8 xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x47,0x00,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8 xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtne2ph2bf8 xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x47,0x00,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8 xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtne2ph2bf8 xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x47,0x10,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2bf8 xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vcvtne2ph2bf8 xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x47,0x00,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2bf8 xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vcvtne2ph2bf8 xmm22, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x47,0x00,0x74,0x71,0x7f]
          vcvtne2ph2bf8 xmm22, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vcvtne2ph2bf8 xmm22, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe2,0x47,0x10,0x74,0x72,0x80]
          vcvtne2ph2bf8 xmm22, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vcvtne2ph2bf8s ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x74,0xf0]
          vcvtne2ph2bf8s ymm22, ymm23, ymm24

// CHECK: vcvtne2ph2bf8s xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x74,0xf0]
          vcvtne2ph2bf8s xmm22, xmm23, xmm24

// CHECK: vcvtne2ph2bf8s ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x20,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8s ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtne2ph2bf8s ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x20,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8s ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtne2ph2bf8s ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2bf8s ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vcvtne2ph2bf8s ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2bf8s ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtne2ph2bf8s ymm22, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x74,0x71,0x7f]
          vcvtne2ph2bf8s ymm22, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vcvtne2ph2bf8s ymm22, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x74,0x72,0x80]
          vcvtne2ph2bf8s ymm22, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vcvtne2ph2bf8s xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8s xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtne2ph2bf8s xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x00,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8s xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtne2ph2bf8s xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2bf8s xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vcvtne2ph2bf8s xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2bf8s xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vcvtne2ph2bf8s xmm22, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x74,0x71,0x7f]
          vcvtne2ph2bf8s xmm22, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vcvtne2ph2bf8s xmm22, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x74,0x72,0x80]
          vcvtne2ph2bf8s xmm22, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vcvtne2ph2hf8 ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x18,0xf0]
          vcvtne2ph2hf8 ymm22, ymm23, ymm24

// CHECK: vcvtne2ph2hf8 xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x18,0xf0]
          vcvtne2ph2hf8 xmm22, xmm23, xmm24

// CHECK: vcvtne2ph2hf8 ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x20,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8 ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtne2ph2hf8 ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x20,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8 ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtne2ph2hf8 ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2hf8 ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vcvtne2ph2hf8 ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x18,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2hf8 ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtne2ph2hf8 ymm22, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x18,0x71,0x7f]
          vcvtne2ph2hf8 ymm22, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vcvtne2ph2hf8 ymm22, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x18,0x72,0x80]
          vcvtne2ph2hf8 ymm22, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vcvtne2ph2hf8 xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8 xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtne2ph2hf8 xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x00,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8 xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtne2ph2hf8 xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2hf8 xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vcvtne2ph2hf8 xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x18,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2hf8 xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vcvtne2ph2hf8 xmm22, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x18,0x71,0x7f]
          vcvtne2ph2hf8 xmm22, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vcvtne2ph2hf8 xmm22, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x18,0x72,0x80]
          vcvtne2ph2hf8 xmm22, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vcvtne2ph2hf8s ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x1b,0xf0]
          vcvtne2ph2hf8s ymm22, ymm23, ymm24

// CHECK: vcvtne2ph2hf8s xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x1b,0xf0]
          vcvtne2ph2hf8s xmm22, xmm23, xmm24

// CHECK: vcvtne2ph2hf8s ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x20,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8s ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtne2ph2hf8s ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x20,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8s ymm22, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtne2ph2hf8s ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2hf8s ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vcvtne2ph2hf8s ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x1b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2hf8s ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtne2ph2hf8s ymm22, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x1b,0x71,0x7f]
          vcvtne2ph2hf8s ymm22, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vcvtne2ph2hf8s ymm22, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x1b,0x72,0x80]
          vcvtne2ph2hf8s ymm22, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vcvtne2ph2hf8s xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8s xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtne2ph2hf8s xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x00,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8s xmm22, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtne2ph2hf8s xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2hf8s xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vcvtne2ph2hf8s xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x1b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2hf8s xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vcvtne2ph2hf8s xmm22, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x1b,0x71,0x7f]
          vcvtne2ph2hf8s xmm22, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vcvtne2ph2hf8s xmm22, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x1b,0x72,0x80]
          vcvtne2ph2hf8s xmm22, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vcvtnebf82ph xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x1e,0xf7]
          vcvtnebf82ph xmm22, xmm23

// CHECK: vcvtnebf82ph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x0f,0x1e,0xf7]
          vcvtnebf82ph xmm22 {k7}, xmm23

// CHECK: vcvtnebf82ph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x8f,0x1e,0xf7]
          vcvtnebf82ph xmm22 {k7} {z}, xmm23

// CHECK: vcvtnebf82ph ymm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x1e,0xf7]
          vcvtnebf82ph ymm22, xmm23

// CHECK: vcvtnebf82ph ymm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x2f,0x1e,0xf7]
          vcvtnebf82ph ymm22 {k7}, xmm23

// CHECK: vcvtnebf82ph ymm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0xaf,0x1e,0xf7]
          vcvtnebf82ph ymm22 {k7} {z}, xmm23

// CHECK: vcvtnebf82ph xmm22, qword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x1e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnebf82ph xmm22, qword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtnebf82ph xmm22 {k7}, qword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x1e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnebf82ph xmm22 {k7}, qword ptr [r8 + 4*rax + 291]

// CHECK: vcvtnebf82ph xmm22, qword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x1e,0x35,0x00,0x00,0x00,0x00]
          vcvtnebf82ph xmm22, qword ptr [rip]

// CHECK: vcvtnebf82ph xmm22, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x1e,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtnebf82ph xmm22, qword ptr [2*rbp - 256]

// CHECK: vcvtnebf82ph xmm22 {k7} {z}, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x1e,0x71,0x7f]
          vcvtnebf82ph xmm22 {k7} {z}, qword ptr [rcx + 1016]

// CHECK: vcvtnebf82ph xmm22 {k7} {z}, qword ptr [rdx - 1024]
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x1e,0x72,0x80]
          vcvtnebf82ph xmm22 {k7} {z}, qword ptr [rdx - 1024]

// CHECK: vcvtnebf82ph ymm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x1e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnebf82ph ymm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtnebf82ph ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x2f,0x1e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnebf82ph ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtnebf82ph ymm22, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x1e,0x35,0x00,0x00,0x00,0x00]
          vcvtnebf82ph ymm22, xmmword ptr [rip]

// CHECK: vcvtnebf82ph ymm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x1e,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtnebf82ph ymm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtnebf82ph ymm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x1e,0x71,0x7f]
          vcvtnebf82ph ymm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtnebf82ph ymm22 {k7} {z}, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x1e,0x72,0x80]
          vcvtnebf82ph ymm22 {k7} {z}, xmmword ptr [rdx - 2048]

// CHECK: vcvtnehf82ph xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x1e,0xf7]
          vcvtnehf82ph xmm22, xmm23

// CHECK: vcvtnehf82ph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x0f,0x1e,0xf7]
          vcvtnehf82ph xmm22 {k7}, xmm23

// CHECK: vcvtnehf82ph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x8f,0x1e,0xf7]
          vcvtnehf82ph xmm22 {k7} {z}, xmm23

// CHECK: vcvtnehf82ph ymm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x28,0x1e,0xf7]
          vcvtnehf82ph ymm22, xmm23

// CHECK: vcvtnehf82ph ymm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x2f,0x1e,0xf7]
          vcvtnehf82ph ymm22 {k7}, xmm23

// CHECK: vcvtnehf82ph ymm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0xaf,0x1e,0xf7]
          vcvtnehf82ph ymm22 {k7} {z}, xmm23

// CHECK: vcvtnehf82ph xmm22, qword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x1e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnehf82ph xmm22, qword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtnehf82ph xmm22 {k7}, qword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x0f,0x1e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnehf82ph xmm22 {k7}, qword ptr [r8 + 4*rax + 291]

// CHECK: vcvtnehf82ph xmm22, qword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7f,0x08,0x1e,0x35,0x00,0x00,0x00,0x00]
          vcvtnehf82ph xmm22, qword ptr [rip]

// CHECK: vcvtnehf82ph xmm22, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0x7f,0x08,0x1e,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtnehf82ph xmm22, qword ptr [2*rbp - 256]

// CHECK: vcvtnehf82ph xmm22 {k7} {z}, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0x7f,0x8f,0x1e,0x71,0x7f]
          vcvtnehf82ph xmm22 {k7} {z}, qword ptr [rcx + 1016]

// CHECK: vcvtnehf82ph xmm22 {k7} {z}, qword ptr [rdx - 1024]
// CHECK: encoding: [0x62,0xe5,0x7f,0x8f,0x1e,0x72,0x80]
          vcvtnehf82ph xmm22 {k7} {z}, qword ptr [rdx - 1024]

// CHECK: vcvtnehf82ph ymm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x28,0x1e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnehf82ph ymm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtnehf82ph ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x2f,0x1e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnehf82ph ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtnehf82ph ymm22, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7f,0x28,0x1e,0x35,0x00,0x00,0x00,0x00]
          vcvtnehf82ph ymm22, xmmword ptr [rip]

// CHECK: vcvtnehf82ph ymm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7f,0x28,0x1e,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtnehf82ph ymm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtnehf82ph ymm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7f,0xaf,0x1e,0x71,0x7f]
          vcvtnehf82ph ymm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtnehf82ph ymm22 {k7} {z}, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe5,0x7f,0xaf,0x1e,0x72,0x80]
          vcvtnehf82ph ymm22 {k7} {z}, xmmword ptr [rdx - 2048]

// CHECK: vcvtneph2bf8 xmm22, xmm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0x74,0xf7]
          vcvtneph2bf8 xmm22, xmm23

// CHECK: vcvtneph2bf8 xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x0f,0x74,0xf7]
          vcvtneph2bf8 xmm22 {k7}, xmm23

// CHECK: vcvtneph2bf8 xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x8f,0x74,0xf7]
          vcvtneph2bf8 xmm22 {k7} {z}, xmm23

// CHECK: vcvtneph2bf8 xmm22, ymm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x28,0x74,0xf7]
          vcvtneph2bf8 xmm22, ymm23

// CHECK: vcvtneph2bf8 xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x2f,0x74,0xf7]
          vcvtneph2bf8 xmm22 {k7}, ymm23

// CHECK: vcvtneph2bf8 xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa2,0x7e,0xaf,0x74,0xf7]
          vcvtneph2bf8 xmm22 {k7} {z}, ymm23

// CHECK: vcvtneph2bf8 xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2bf8 xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtneph2bf8 xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x0f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2bf8 xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtneph2bf8 xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x7e,0x18,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2bf8 xmm22, word ptr [rip]{1to8}

// CHECK: vcvtneph2bf8 xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2bf8 xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtneph2bf8 xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x7e,0x8f,0x74,0x71,0x7f]
          vcvtneph2bf8 xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtneph2bf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe2,0x7e,0x9f,0x74,0x72,0x80]
          vcvtneph2bf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtneph2bf8 xmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x7e,0x38,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2bf8 xmm22, word ptr [rip]{1to16}

// CHECK: vcvtneph2bf8 xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2bf8 xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtneph2bf8 xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x7e,0xaf,0x74,0x71,0x7f]
          vcvtneph2bf8 xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtneph2bf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe2,0x7e,0xbf,0x74,0x72,0x80]
          vcvtneph2bf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtneph2bf8s xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x74,0xf7]
          vcvtneph2bf8s xmm22, xmm23

// CHECK: vcvtneph2bf8s xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x0f,0x74,0xf7]
          vcvtneph2bf8s xmm22 {k7}, xmm23

// CHECK: vcvtneph2bf8s xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x8f,0x74,0xf7]
          vcvtneph2bf8s xmm22 {k7} {z}, xmm23

// CHECK: vcvtneph2bf8s xmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x74,0xf7]
          vcvtneph2bf8s xmm22, ymm23

// CHECK: vcvtneph2bf8s xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x2f,0x74,0xf7]
          vcvtneph2bf8s xmm22 {k7}, ymm23

// CHECK: vcvtneph2bf8s xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0xaf,0x74,0xf7]
          vcvtneph2bf8s xmm22 {k7} {z}, ymm23

// CHECK: vcvtneph2bf8s xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2bf8s xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtneph2bf8s xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2bf8s xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtneph2bf8s xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7e,0x18,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2bf8s xmm22, word ptr [rip]{1to8}

// CHECK: vcvtneph2bf8s xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2bf8s xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtneph2bf8s xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x74,0x71,0x7f]
          vcvtneph2bf8s xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtneph2bf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7e,0x9f,0x74,0x72,0x80]
          vcvtneph2bf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtneph2bf8s xmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7e,0x38,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2bf8s xmm22, word ptr [rip]{1to16}

// CHECK: vcvtneph2bf8s xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2bf8s xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtneph2bf8s xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x74,0x71,0x7f]
          vcvtneph2bf8s xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtneph2bf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7e,0xbf,0x74,0x72,0x80]
          vcvtneph2bf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtneph2hf8 xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x18,0xf7]
          vcvtneph2hf8 xmm22, xmm23

// CHECK: vcvtneph2hf8 xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x0f,0x18,0xf7]
          vcvtneph2hf8 xmm22 {k7}, xmm23

// CHECK: vcvtneph2hf8 xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x8f,0x18,0xf7]
          vcvtneph2hf8 xmm22 {k7} {z}, xmm23

// CHECK: vcvtneph2hf8 xmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x18,0xf7]
          vcvtneph2hf8 xmm22, ymm23

// CHECK: vcvtneph2hf8 xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x2f,0x18,0xf7]
          vcvtneph2hf8 xmm22 {k7}, ymm23

// CHECK: vcvtneph2hf8 xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0xaf,0x18,0xf7]
          vcvtneph2hf8 xmm22 {k7} {z}, ymm23

// CHECK: vcvtneph2hf8 xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2hf8 xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtneph2hf8 xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2hf8 xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtneph2hf8 xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7e,0x18,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2hf8 xmm22, word ptr [rip]{1to8}

// CHECK: vcvtneph2hf8 xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x18,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2hf8 xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtneph2hf8 xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x18,0x71,0x7f]
          vcvtneph2hf8 xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtneph2hf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7e,0x9f,0x18,0x72,0x80]
          vcvtneph2hf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtneph2hf8 xmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7e,0x38,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2hf8 xmm22, word ptr [rip]{1to16}

// CHECK: vcvtneph2hf8 xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x18,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2hf8 xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtneph2hf8 xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x18,0x71,0x7f]
          vcvtneph2hf8 xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtneph2hf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7e,0xbf,0x18,0x72,0x80]
          vcvtneph2hf8 xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtneph2hf8s xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x1b,0xf7]
          vcvtneph2hf8s xmm22, xmm23

// CHECK: vcvtneph2hf8s xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x0f,0x1b,0xf7]
          vcvtneph2hf8s xmm22 {k7}, xmm23

// CHECK: vcvtneph2hf8s xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x8f,0x1b,0xf7]
          vcvtneph2hf8s xmm22 {k7} {z}, xmm23

// CHECK: vcvtneph2hf8s xmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x1b,0xf7]
          vcvtneph2hf8s xmm22, ymm23

// CHECK: vcvtneph2hf8s xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x2f,0x1b,0xf7]
          vcvtneph2hf8s xmm22 {k7}, ymm23

// CHECK: vcvtneph2hf8s xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0xaf,0x1b,0xf7]
          vcvtneph2hf8s xmm22 {k7} {z}, ymm23

// CHECK: vcvtneph2hf8s xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2hf8s xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtneph2hf8s xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2hf8s xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtneph2hf8s xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7e,0x18,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2hf8s xmm22, word ptr [rip]{1to8}

// CHECK: vcvtneph2hf8s xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x1b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2hf8s xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtneph2hf8s xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x1b,0x71,0x7f]
          vcvtneph2hf8s xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtneph2hf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7e,0x9f,0x1b,0x72,0x80]
          vcvtneph2hf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtneph2hf8s xmm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7e,0x38,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2hf8s xmm22, word ptr [rip]{1to16}

// CHECK: vcvtneph2hf8s xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x1b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2hf8s xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtneph2hf8s xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x1b,0x71,0x7f]
          vcvtneph2hf8s xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtneph2hf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7e,0xbf,0x1b,0x72,0x80]
          vcvtneph2hf8s xmm22 {k7} {z}, word ptr [rdx - 256]{1to16}

