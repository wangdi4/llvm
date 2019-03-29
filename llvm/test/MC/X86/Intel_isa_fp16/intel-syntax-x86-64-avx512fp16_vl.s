// REQUIRES: intel_feature_isa_fp16
// RUN: llvm-mc -triple x86_64-unknown-unknown -mcpu=knl -mattr=+avx512fp16 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vcmpph k5, ymm29, ymm28, 123
// CHECK: encoding: [0x62,0x93,0x14,0x20,0xc2,0xec,0x7b]
          vcmpph k5, ymm29, ymm28, 123

// CHECK: vcmpph k5 {k7}, ymm29, ymm28, 123
// CHECK: encoding: [0x62,0x93,0x14,0x27,0xc2,0xec,0x7b]
          vcmpph k5 {k7}, ymm29, ymm28, 123

// CHECK: vcmpph k5, xmm29, xmm28, 123
// CHECK: encoding: [0x62,0x93,0x14,0x00,0xc2,0xec,0x7b]
          vcmpph k5, xmm29, xmm28, 123

// CHECK: vcmpph k5 {k7}, xmm29, xmm28, 123
// CHECK: encoding: [0x62,0x93,0x14,0x07,0xc2,0xec,0x7b]
          vcmpph k5 {k7}, xmm29, xmm28, 123

// CHECK: vcmpph k5, xmm29, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xb3,0x14,0x00,0xc2,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcmpph k5, xmm29, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcmpph k5 {k7}, xmm29, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xd3,0x14,0x07,0xc2,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcmpph k5 {k7}, xmm29, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcmpph k5, xmm29, word ptr [rip]{1to8}, 123
// CHECK: encoding: [0x62,0xf3,0x14,0x10,0xc2,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vcmpph k5, xmm29, word ptr [rip]{1to8}, 123

// CHECK: vcmpph k5, xmm29, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0x62,0xf3,0x14,0x00,0xc2,0x2c,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vcmpph k5, xmm29, xmmword ptr [2*rbp - 512], 123

// CHECK: vcmpph k5 {k7}, xmm29, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0x62,0xf3,0x14,0x07,0xc2,0x69,0x7f,0x7b]
          vcmpph k5 {k7}, xmm29, xmmword ptr [rcx + 2032], 123

// CHECK: vcmpph k5 {k7}, xmm29, word ptr [rdx - 256]{1to8}, 123
// CHECK: encoding: [0x62,0xf3,0x14,0x17,0xc2,0x6a,0x80,0x7b]
          vcmpph k5 {k7}, xmm29, word ptr [rdx - 256]{1to8}, 123

// CHECK: vcmpph k5, ymm29, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xb3,0x14,0x20,0xc2,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcmpph k5, ymm29, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcmpph k5 {k7}, ymm29, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xd3,0x14,0x27,0xc2,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcmpph k5 {k7}, ymm29, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcmpph k5, ymm29, word ptr [rip]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x14,0x30,0xc2,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vcmpph k5, ymm29, word ptr [rip]{1to16}, 123

// CHECK: vcmpph k5, ymm29, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0x62,0xf3,0x14,0x20,0xc2,0x2c,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vcmpph k5, ymm29, ymmword ptr [2*rbp - 1024], 123

// CHECK: vcmpph k5 {k7}, ymm29, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0x62,0xf3,0x14,0x27,0xc2,0x69,0x7f,0x7b]
          vcmpph k5 {k7}, ymm29, ymmword ptr [rcx + 4064], 123

// CHECK: vcmpph k5 {k7}, ymm29, word ptr [rdx - 256]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x14,0x37,0xc2,0x6a,0x80,0x7b]
          vcmpph k5 {k7}, ymm29, word ptr [rdx - 256]{1to16}, 123

// CHECK: vdivph ymm30, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0x20,0x5e,0xf4]
          vdivph ymm30, ymm29, ymm28

// CHECK: vdivph ymm30 {k7}, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0x27,0x5e,0xf4]
          vdivph ymm30 {k7}, ymm29, ymm28

// CHECK: vdivph ymm30 {k7} {z}, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0xa7,0x5e,0xf4]
          vdivph ymm30 {k7} {z}, ymm29, ymm28

// CHECK: vdivph xmm30, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x00,0x5e,0xf4]
          vdivph xmm30, xmm29, xmm28

// CHECK: vdivph xmm30 {k7}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x07,0x5e,0xf4]
          vdivph xmm30 {k7}, xmm29, xmm28

// CHECK: vdivph xmm30 {k7} {z}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x87,0x5e,0xf4]
          vdivph xmm30 {k7} {z}, xmm29, xmm28

// CHECK: vdivph ymm30, ymm29, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x20,0x5e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdivph ymm30, ymm29, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdivph ymm30 {k7}, ymm29, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x27,0x5e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdivph ymm30 {k7}, ymm29, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vdivph ymm30, ymm29, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0x65,0x14,0x30,0x5e,0x35,0x00,0x00,0x00,0x00]
          vdivph ymm30, ymm29, word ptr [rip]{1to16}

// CHECK: vdivph ymm30, ymm29, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0x65,0x14,0x20,0x5e,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vdivph ymm30, ymm29, ymmword ptr [2*rbp - 1024]

// CHECK: vdivph ymm30 {k7} {z}, ymm29, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0x65,0x14,0xa7,0x5e,0x71,0x7f]
          vdivph ymm30 {k7} {z}, ymm29, ymmword ptr [rcx + 4064]

// CHECK: vdivph ymm30 {k7} {z}, ymm29, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0x65,0x14,0xb7,0x5e,0x72,0x80]
          vdivph ymm30 {k7} {z}, ymm29, word ptr [rdx - 256]{1to16}

// CHECK: vdivph xmm30, xmm29, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x00,0x5e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdivph xmm30, xmm29, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdivph xmm30 {k7}, xmm29, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x07,0x5e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdivph xmm30 {k7}, xmm29, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vdivph xmm30, xmm29, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0x65,0x14,0x10,0x5e,0x35,0x00,0x00,0x00,0x00]
          vdivph xmm30, xmm29, word ptr [rip]{1to8}

// CHECK: vdivph xmm30, xmm29, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0x65,0x14,0x00,0x5e,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vdivph xmm30, xmm29, xmmword ptr [2*rbp - 512]

// CHECK: vdivph xmm30 {k7} {z}, xmm29, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0x65,0x14,0x87,0x5e,0x71,0x7f]
          vdivph xmm30 {k7} {z}, xmm29, xmmword ptr [rcx + 2032]

// CHECK: vdivph xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0x65,0x14,0x97,0x5e,0x72,0x80]
          vdivph xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]{1to8}

// CHECK: vmaxph ymm30, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0x20,0x5f,0xf4]
          vmaxph ymm30, ymm29, ymm28

// CHECK: vmaxph ymm30 {k7}, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0x27,0x5f,0xf4]
          vmaxph ymm30 {k7}, ymm29, ymm28

// CHECK: vmaxph ymm30 {k7} {z}, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0xa7,0x5f,0xf4]
          vmaxph ymm30 {k7} {z}, ymm29, ymm28

// CHECK: vmaxph xmm30, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x00,0x5f,0xf4]
          vmaxph xmm30, xmm29, xmm28

// CHECK: vmaxph xmm30 {k7}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x07,0x5f,0xf4]
          vmaxph xmm30 {k7}, xmm29, xmm28

// CHECK: vmaxph xmm30 {k7} {z}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x87,0x5f,0xf4]
          vmaxph xmm30 {k7} {z}, xmm29, xmm28

// CHECK: vmaxph ymm30, ymm29, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x20,0x5f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmaxph ymm30, ymm29, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmaxph ymm30 {k7}, ymm29, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x27,0x5f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmaxph ymm30 {k7}, ymm29, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vmaxph ymm30, ymm29, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0x65,0x14,0x30,0x5f,0x35,0x00,0x00,0x00,0x00]
          vmaxph ymm30, ymm29, word ptr [rip]{1to16}

// CHECK: vmaxph ymm30, ymm29, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0x65,0x14,0x20,0x5f,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vmaxph ymm30, ymm29, ymmword ptr [2*rbp - 1024]

// CHECK: vmaxph ymm30 {k7} {z}, ymm29, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0x65,0x14,0xa7,0x5f,0x71,0x7f]
          vmaxph ymm30 {k7} {z}, ymm29, ymmword ptr [rcx + 4064]

// CHECK: vmaxph ymm30 {k7} {z}, ymm29, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0x65,0x14,0xb7,0x5f,0x72,0x80]
          vmaxph ymm30 {k7} {z}, ymm29, word ptr [rdx - 256]{1to16}

// CHECK: vmaxph xmm30, xmm29, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x00,0x5f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmaxph xmm30, xmm29, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmaxph xmm30 {k7}, xmm29, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x07,0x5f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmaxph xmm30 {k7}, xmm29, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vmaxph xmm30, xmm29, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0x65,0x14,0x10,0x5f,0x35,0x00,0x00,0x00,0x00]
          vmaxph xmm30, xmm29, word ptr [rip]{1to8}

// CHECK: vmaxph xmm30, xmm29, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0x65,0x14,0x00,0x5f,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vmaxph xmm30, xmm29, xmmword ptr [2*rbp - 512]

// CHECK: vmaxph xmm30 {k7} {z}, xmm29, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0x65,0x14,0x87,0x5f,0x71,0x7f]
          vmaxph xmm30 {k7} {z}, xmm29, xmmword ptr [rcx + 2032]

// CHECK: vmaxph xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0x65,0x14,0x97,0x5f,0x72,0x80]
          vmaxph xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]{1to8}

// CHECK: vminph ymm30, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0x20,0x5d,0xf4]
          vminph ymm30, ymm29, ymm28

// CHECK: vminph ymm30 {k7}, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0x27,0x5d,0xf4]
          vminph ymm30 {k7}, ymm29, ymm28

// CHECK: vminph ymm30 {k7} {z}, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0xa7,0x5d,0xf4]
          vminph ymm30 {k7} {z}, ymm29, ymm28

// CHECK: vminph xmm30, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x00,0x5d,0xf4]
          vminph xmm30, xmm29, xmm28

// CHECK: vminph xmm30 {k7}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x07,0x5d,0xf4]
          vminph xmm30 {k7}, xmm29, xmm28

// CHECK: vminph xmm30 {k7} {z}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x87,0x5d,0xf4]
          vminph xmm30 {k7} {z}, xmm29, xmm28

// CHECK: vminph ymm30, ymm29, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x20,0x5d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vminph ymm30, ymm29, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vminph ymm30 {k7}, ymm29, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x27,0x5d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vminph ymm30 {k7}, ymm29, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vminph ymm30, ymm29, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0x65,0x14,0x30,0x5d,0x35,0x00,0x00,0x00,0x00]
          vminph ymm30, ymm29, word ptr [rip]{1to16}

// CHECK: vminph ymm30, ymm29, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0x65,0x14,0x20,0x5d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vminph ymm30, ymm29, ymmword ptr [2*rbp - 1024]

// CHECK: vminph ymm30 {k7} {z}, ymm29, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0x65,0x14,0xa7,0x5d,0x71,0x7f]
          vminph ymm30 {k7} {z}, ymm29, ymmword ptr [rcx + 4064]

// CHECK: vminph ymm30 {k7} {z}, ymm29, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0x65,0x14,0xb7,0x5d,0x72,0x80]
          vminph ymm30 {k7} {z}, ymm29, word ptr [rdx - 256]{1to16}

// CHECK: vminph xmm30, xmm29, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x00,0x5d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vminph xmm30, xmm29, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vminph xmm30 {k7}, xmm29, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x07,0x5d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vminph xmm30 {k7}, xmm29, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vminph xmm30, xmm29, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0x65,0x14,0x10,0x5d,0x35,0x00,0x00,0x00,0x00]
          vminph xmm30, xmm29, word ptr [rip]{1to8}

// CHECK: vminph xmm30, xmm29, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0x65,0x14,0x00,0x5d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vminph xmm30, xmm29, xmmword ptr [2*rbp - 512]

// CHECK: vminph xmm30 {k7} {z}, xmm29, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0x65,0x14,0x87,0x5d,0x71,0x7f]
          vminph xmm30 {k7} {z}, xmm29, xmmword ptr [rcx + 2032]

// CHECK: vminph xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0x65,0x14,0x97,0x5d,0x72,0x80]
          vminph xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]{1to8}

// CHECK: vmulph ymm30, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0x20,0x59,0xf4]
          vmulph ymm30, ymm29, ymm28

// CHECK: vmulph ymm30 {k7}, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0x27,0x59,0xf4]
          vmulph ymm30 {k7}, ymm29, ymm28

// CHECK: vmulph ymm30 {k7} {z}, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0xa7,0x59,0xf4]
          vmulph ymm30 {k7} {z}, ymm29, ymm28

// CHECK: vmulph xmm30, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x00,0x59,0xf4]
          vmulph xmm30, xmm29, xmm28

// CHECK: vmulph xmm30 {k7}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x07,0x59,0xf4]
          vmulph xmm30 {k7}, xmm29, xmm28

// CHECK: vmulph xmm30 {k7} {z}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x87,0x59,0xf4]
          vmulph xmm30 {k7} {z}, xmm29, xmm28

// CHECK: vmulph ymm30, ymm29, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x20,0x59,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmulph ymm30, ymm29, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmulph ymm30 {k7}, ymm29, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x27,0x59,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmulph ymm30 {k7}, ymm29, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vmulph ymm30, ymm29, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0x65,0x14,0x30,0x59,0x35,0x00,0x00,0x00,0x00]
          vmulph ymm30, ymm29, word ptr [rip]{1to16}

// CHECK: vmulph ymm30, ymm29, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0x65,0x14,0x20,0x59,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vmulph ymm30, ymm29, ymmword ptr [2*rbp - 1024]

// CHECK: vmulph ymm30 {k7} {z}, ymm29, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0x65,0x14,0xa7,0x59,0x71,0x7f]
          vmulph ymm30 {k7} {z}, ymm29, ymmword ptr [rcx + 4064]

// CHECK: vmulph ymm30 {k7} {z}, ymm29, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0x65,0x14,0xb7,0x59,0x72,0x80]
          vmulph ymm30 {k7} {z}, ymm29, word ptr [rdx - 256]{1to16}

// CHECK: vmulph xmm30, xmm29, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x00,0x59,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmulph xmm30, xmm29, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmulph xmm30 {k7}, xmm29, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x07,0x59,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmulph xmm30 {k7}, xmm29, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vmulph xmm30, xmm29, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0x65,0x14,0x10,0x59,0x35,0x00,0x00,0x00,0x00]
          vmulph xmm30, xmm29, word ptr [rip]{1to8}

// CHECK: vmulph xmm30, xmm29, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0x65,0x14,0x00,0x59,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vmulph xmm30, xmm29, xmmword ptr [2*rbp - 512]

// CHECK: vmulph xmm30 {k7} {z}, xmm29, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0x65,0x14,0x87,0x59,0x71,0x7f]
          vmulph xmm30 {k7} {z}, xmm29, xmmword ptr [rcx + 2032]

// CHECK: vmulph xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0x65,0x14,0x97,0x59,0x72,0x80]
          vmulph xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]{1to8}

// CHECK: vsubph ymm30, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0x20,0x5c,0xf4]
          vsubph ymm30, ymm29, ymm28

// CHECK: vsubph ymm30 {k7}, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0x27,0x5c,0xf4]
          vsubph ymm30 {k7}, ymm29, ymm28

// CHECK: vsubph ymm30 {k7} {z}, ymm29, ymm28
// CHECK: encoding: [0x62,0x05,0x14,0xa7,0x5c,0xf4]
          vsubph ymm30 {k7} {z}, ymm29, ymm28

// CHECK: vsubph xmm30, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x00,0x5c,0xf4]
          vsubph xmm30, xmm29, xmm28

// CHECK: vsubph xmm30 {k7}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x07,0x5c,0xf4]
          vsubph xmm30 {k7}, xmm29, xmm28

// CHECK: vsubph xmm30 {k7} {z}, xmm29, xmm28
// CHECK: encoding: [0x62,0x05,0x14,0x87,0x5c,0xf4]
          vsubph xmm30 {k7} {z}, xmm29, xmm28

// CHECK: vsubph ymm30, ymm29, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x20,0x5c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubph ymm30, ymm29, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubph ymm30 {k7}, ymm29, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x27,0x5c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubph ymm30 {k7}, ymm29, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vsubph ymm30, ymm29, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0x65,0x14,0x30,0x5c,0x35,0x00,0x00,0x00,0x00]
          vsubph ymm30, ymm29, word ptr [rip]{1to16}

// CHECK: vsubph ymm30, ymm29, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0x65,0x14,0x20,0x5c,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vsubph ymm30, ymm29, ymmword ptr [2*rbp - 1024]

// CHECK: vsubph ymm30 {k7} {z}, ymm29, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0x65,0x14,0xa7,0x5c,0x71,0x7f]
          vsubph ymm30 {k7} {z}, ymm29, ymmword ptr [rcx + 4064]

// CHECK: vsubph ymm30 {k7} {z}, ymm29, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0x65,0x14,0xb7,0x5c,0x72,0x80]
          vsubph ymm30 {k7} {z}, ymm29, word ptr [rdx - 256]{1to16}

// CHECK: vsubph xmm30, xmm29, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0x25,0x14,0x00,0x5c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubph xmm30, xmm29, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubph xmm30 {k7}, xmm29, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0x45,0x14,0x07,0x5c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubph xmm30 {k7}, xmm29, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vsubph xmm30, xmm29, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0x65,0x14,0x10,0x5c,0x35,0x00,0x00,0x00,0x00]
          vsubph xmm30, xmm29, word ptr [rip]{1to8}

// CHECK: vsubph xmm30, xmm29, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0x65,0x14,0x00,0x5c,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vsubph xmm30, xmm29, xmmword ptr [2*rbp - 512]

// CHECK: vsubph xmm30 {k7} {z}, xmm29, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0x65,0x14,0x87,0x5c,0x71,0x7f]
          vsubph xmm30 {k7} {z}, xmm29, xmmword ptr [rcx + 2032]

// CHECK: vsubph xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0x65,0x14,0x97,0x5c,0x72,0x80]
          vsubph xmm30 {k7} {z}, xmm29, word ptr [rdx - 256]{1to8}

