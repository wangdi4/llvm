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

// CHECK: vcvtph2psx xmm22, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x13,0xf7]
          vcvtph2psx xmm22, xmm23

// CHECK: vcvtph2psx xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x0f,0x13,0xf7]
          vcvtph2psx xmm22 {k7}, xmm23

// CHECK: vcvtph2psx xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x8f,0x13,0xf7]
          vcvtph2psx xmm22 {k7} {z}, xmm23

// CHECK: vcvtph2psx ymm22, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x13,0xf7]
          vcvtph2psx ymm22, xmm23

// CHECK: vcvtph2psx ymm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x2f,0x13,0xf7]
          vcvtph2psx ymm22 {k7}, xmm23

// CHECK: vcvtph2psx ymm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0xaf,0x13,0xf7]
          vcvtph2psx ymm22 {k7} {z}, xmm23

// CHECK: vcvtph2psx xmm22, qword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x13,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2psx xmm22, qword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2psx xmm22 {k7}, qword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x7d,0x0f,0x13,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2psx xmm22 {k7}, qword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2psx xmm22, word ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe6,0x7d,0x18,0x13,0x35,0x00,0x00,0x00,0x00]
          vcvtph2psx xmm22, word ptr [rip]{1to4}

// CHECK: vcvtph2psx xmm22, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe6,0x7d,0x08,0x13,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtph2psx xmm22, qword ptr [2*rbp - 256]

// CHECK: vcvtph2psx xmm22 {k7} {z}, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe6,0x7d,0x8f,0x13,0x71,0x7f]
          vcvtph2psx xmm22 {k7} {z}, qword ptr [rcx + 1016]

// CHECK: vcvtph2psx xmm22 {k7} {z}, word ptr [rdx - 256]{1to4}
// CHECK: encoding: [0x62,0xe6,0x7d,0x9f,0x13,0x72,0x80]
          vcvtph2psx xmm22 {k7} {z}, word ptr [rdx - 256]{1to4}

// CHECK: vcvtph2psx ymm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x13,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2psx ymm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2psx ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x7d,0x2f,0x13,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2psx ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2psx ymm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x7d,0x38,0x13,0x35,0x00,0x00,0x00,0x00]
          vcvtph2psx ymm22, word ptr [rip]{1to8}

// CHECK: vcvtph2psx ymm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x7d,0x28,0x13,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2psx ymm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtph2psx ymm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x7d,0xaf,0x13,0x71,0x7f]
          vcvtph2psx ymm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtph2psx ymm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x7d,0xbf,0x13,0x72,0x80]
          vcvtph2psx ymm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtps2phx xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x1d,0xf7]
          vcvtps2phx xmm22, xmm23

// CHECK: vcvtps2phx xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x1d,0xf7]
          vcvtps2phx xmm22 {k7}, xmm23

// CHECK: vcvtps2phx xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x1d,0xf7]
          vcvtps2phx xmm22 {k7} {z}, xmm23

// CHECK: vcvtps2phx xmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x1d,0xf7]
          vcvtps2phx xmm22, ymm23

// CHECK: vcvtps2phx xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x1d,0xf7]
          vcvtps2phx xmm22 {k7}, ymm23

// CHECK: vcvtps2phx xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x1d,0xf7]
          vcvtps2phx xmm22 {k7} {z}, ymm23

// CHECK: vcvtps2phx xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x1d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtps2phx xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtps2phx xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x1d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtps2phx xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtps2phx xmm22, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x1d,0x35,0x00,0x00,0x00,0x00]
          vcvtps2phx xmm22, dword ptr [rip]{1to4}

// CHECK: vcvtps2phx xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x1d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtps2phx xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtps2phx xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x1d,0x71,0x7f]
          vcvtps2phx xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtps2phx xmm22 {k7} {z}, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x1d,0x72,0x80]
          vcvtps2phx xmm22 {k7} {z}, dword ptr [rdx - 512]{1to4}

// CHECK: vcvtps2phx xmm22, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x1d,0x35,0x00,0x00,0x00,0x00]
          vcvtps2phx xmm22, dword ptr [rip]{1to8}

// CHECK: vcvtps2phx xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x1d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtps2phx xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtps2phx xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x1d,0x71,0x7f]
          vcvtps2phx xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtps2phx xmm22 {k7} {z}, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x1d,0x72,0x80]
          vcvtps2phx xmm22 {k7} {z}, dword ptr [rdx - 512]{1to8}

// CHECK: vcvtpd2ph xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x08,0x5a,0xf7]
          vcvtpd2ph xmm22, xmm23

// CHECK: vcvtpd2ph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x0f,0x5a,0xf7]
          vcvtpd2ph xmm22 {k7}, xmm23

// CHECK: vcvtpd2ph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x8f,0x5a,0xf7]
          vcvtpd2ph xmm22 {k7} {z}, xmm23

// CHECK: vcvtpd2ph xmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x28,0x5a,0xf7]
          vcvtpd2ph xmm22, ymm23

// CHECK: vcvtpd2ph xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x2f,0x5a,0xf7]
          vcvtpd2ph xmm22 {k7}, ymm23

// CHECK: vcvtpd2ph xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0xfd,0xaf,0x5a,0xf7]
          vcvtpd2ph xmm22 {k7} {z}, ymm23

// CHECK: vcvtpd2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x08,0x5a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtpd2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtpd2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x0f,0x5a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtpd2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtpd2ph xmm22, qword ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe5,0xfd,0x18,0x5a,0x35,0x00,0x00,0x00,0x00]
          vcvtpd2ph xmm22, qword ptr [rip]{1to2}

// CHECK: vcvtpd2ph xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xfd,0x08,0x5a,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtpd2ph xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtpd2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xfd,0x8f,0x5a,0x71,0x7f]
          vcvtpd2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtpd2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to2}
// CHECK: encoding: [0x62,0xe5,0xfd,0x9f,0x5a,0x72,0x80]
          vcvtpd2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to2}

// CHECK: vcvtpd2ph xmm22, qword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0xfd,0x38,0x5a,0x35,0x00,0x00,0x00,0x00]
          vcvtpd2ph xmm22, qword ptr [rip]{1to4}

// CHECK: vcvtpd2ph xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xfd,0x28,0x5a,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtpd2ph xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtpd2ph xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xfd,0xaf,0x5a,0x71,0x7f]
          vcvtpd2ph xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtpd2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to4}
// CHECK: encoding: [0x62,0xe5,0xfd,0xbf,0x5a,0x72,0x80]
          vcvtpd2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to4}

// CHECK: vcvtph2pd xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x5a,0xf7]
          vcvtph2pd xmm22, xmm23

// CHECK: vcvtph2pd xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x5a,0xf7]
          vcvtph2pd xmm22 {k7}, xmm23

// CHECK: vcvtph2pd xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x5a,0xf7]
          vcvtph2pd xmm22 {k7} {z}, xmm23

// CHECK: vcvtph2pd ymm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x5a,0xf7]
          vcvtph2pd ymm22, xmm23

// CHECK: vcvtph2pd ymm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x5a,0xf7]
          vcvtph2pd ymm22 {k7}, xmm23

// CHECK: vcvtph2pd ymm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x5a,0xf7]
          vcvtph2pd ymm22 {k7} {z}, xmm23

// CHECK: vcvtph2pd xmm22, dword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x5a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2pd xmm22, dword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2pd xmm22 {k7}, dword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x5a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2pd xmm22 {k7}, dword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2pd xmm22, word ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x5a,0x35,0x00,0x00,0x00,0x00]
          vcvtph2pd xmm22, word ptr [rip]{1to2}

// CHECK: vcvtph2pd xmm22, dword ptr [2*rbp - 128]
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x5a,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvtph2pd xmm22, dword ptr [2*rbp - 128]

// CHECK: vcvtph2pd xmm22 {k7} {z}, dword ptr [rcx + 508]
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x5a,0x71,0x7f]
          vcvtph2pd xmm22 {k7} {z}, dword ptr [rcx + 508]

// CHECK: vcvtph2pd xmm22 {k7} {z}, word ptr [rdx - 256]{1to2}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x5a,0x72,0x80]
          vcvtph2pd xmm22 {k7} {z}, word ptr [rdx - 256]{1to2}

// CHECK: vcvtph2pd ymm22, qword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x5a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2pd ymm22, qword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2pd ymm22 {k7}, qword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x2f,0x5a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2pd ymm22 {k7}, qword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2pd ymm22, word ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x5a,0x35,0x00,0x00,0x00,0x00]
          vcvtph2pd ymm22, word ptr [rip]{1to4}

// CHECK: vcvtph2pd ymm22, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x5a,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtph2pd ymm22, qword ptr [2*rbp - 256]

// CHECK: vcvtph2pd ymm22 {k7} {z}, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x5a,0x71,0x7f]
          vcvtph2pd ymm22 {k7} {z}, qword ptr [rcx + 1016]

// CHECK: vcvtph2pd ymm22 {k7} {z}, word ptr [rdx - 256]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x5a,0x72,0x80]
          vcvtph2pd ymm22 {k7} {z}, word ptr [rdx - 256]{1to4}

// CHECK: vcvtph2uw xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x7d,0xf7]
          vcvtph2uw xmm22, xmm23

// CHECK: vcvtph2uw xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x7d,0xf7]
          vcvtph2uw xmm22 {k7}, xmm23

// CHECK: vcvtph2uw xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x7d,0xf7]
          vcvtph2uw xmm22 {k7} {z}, xmm23

// CHECK: vcvtph2uw ymm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x7d,0xf7]
          vcvtph2uw ymm22, ymm23

// CHECK: vcvtph2uw ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x7d,0xf7]
          vcvtph2uw ymm22 {k7}, ymm23

// CHECK: vcvtph2uw ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x7d,0xf7]
          vcvtph2uw ymm22 {k7} {z}, ymm23

// CHECK: vcvtph2uw xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2uw xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2uw xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2uw xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2uw xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtph2uw xmm22, word ptr [rip]{1to8}

// CHECK: vcvtph2uw xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x7d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2uw xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtph2uw xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x7d,0x71,0x7f]
          vcvtph2uw xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtph2uw xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x7d,0x72,0x80]
          vcvtph2uw xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtph2uw ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2uw ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2uw ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x2f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2uw ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2uw ymm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtph2uw ymm22, word ptr [rip]{1to16}

// CHECK: vcvtph2uw ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x7d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtph2uw ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtph2uw ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x7d,0x71,0x7f]
          vcvtph2uw ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtph2uw ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x7d,0x72,0x80]
          vcvtph2uw ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtph2w xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7d,0xf7]
          vcvtph2w xmm22, xmm23

// CHECK: vcvtph2w xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x7d,0xf7]
          vcvtph2w xmm22 {k7}, xmm23

// CHECK: vcvtph2w xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x7d,0xf7]
          vcvtph2w xmm22 {k7} {z}, xmm23

// CHECK: vcvtph2w ymm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7d,0xf7]
          vcvtph2w ymm22, ymm23

// CHECK: vcvtph2w ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x7d,0xf7]
          vcvtph2w ymm22 {k7}, ymm23

// CHECK: vcvtph2w ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x7d,0xf7]
          vcvtph2w ymm22 {k7} {z}, ymm23

// CHECK: vcvtph2w xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2w xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2w xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2w xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2w xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtph2w xmm22, word ptr [rip]{1to8}

// CHECK: vcvtph2w xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2w xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtph2w xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x7d,0x71,0x7f]
          vcvtph2w xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtph2w xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x7d,0x72,0x80]
          vcvtph2w xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtph2w ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2w ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2w ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2w ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2w ymm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtph2w ymm22, word ptr [rip]{1to16}

// CHECK: vcvtph2w ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x7d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtph2w ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtph2w ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x7d,0x71,0x7f]
          vcvtph2w ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtph2w ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x7d,0x72,0x80]
          vcvtph2w ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvttph2uw xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x7c,0xf7]
          vcvttph2uw xmm22, xmm23

// CHECK: vcvttph2uw xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x7c,0xf7]
          vcvttph2uw xmm22 {k7}, xmm23

// CHECK: vcvttph2uw xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x7c,0xf7]
          vcvttph2uw xmm22 {k7} {z}, xmm23

// CHECK: vcvttph2uw ymm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x7c,0xf7]
          vcvttph2uw ymm22, ymm23

// CHECK: vcvttph2uw ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x7c,0xf7]
          vcvttph2uw ymm22 {k7}, ymm23

// CHECK: vcvttph2uw ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x7c,0xf7]
          vcvttph2uw ymm22 {k7} {z}, ymm23

// CHECK: vcvttph2uw xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x7c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2uw xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2uw xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x7c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2uw xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2uw xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x7c,0x35,0x00,0x00,0x00,0x00]
          vcvttph2uw xmm22, word ptr [rip]{1to8}

// CHECK: vcvttph2uw xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x7c,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2uw xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvttph2uw xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x7c,0x71,0x7f]
          vcvttph2uw xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvttph2uw xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x7c,0x72,0x80]
          vcvttph2uw xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvttph2uw ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x7c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2uw ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2uw ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x2f,0x7c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2uw ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2uw ymm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x7c,0x35,0x00,0x00,0x00,0x00]
          vcvttph2uw ymm22, word ptr [rip]{1to16}

// CHECK: vcvttph2uw ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x7c,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvttph2uw ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvttph2uw ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x7c,0x71,0x7f]
          vcvttph2uw ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvttph2uw ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x7c,0x72,0x80]
          vcvttph2uw ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvttph2w xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7c,0xf7]
          vcvttph2w xmm22, xmm23

// CHECK: vcvttph2w xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x7c,0xf7]
          vcvttph2w xmm22 {k7}, xmm23

// CHECK: vcvttph2w xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x7c,0xf7]
          vcvttph2w xmm22 {k7} {z}, xmm23

// CHECK: vcvttph2w ymm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7c,0xf7]
          vcvttph2w ymm22, ymm23

// CHECK: vcvttph2w ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x7c,0xf7]
          vcvttph2w ymm22 {k7}, ymm23

// CHECK: vcvttph2w ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x7c,0xf7]
          vcvttph2w ymm22 {k7} {z}, ymm23

// CHECK: vcvttph2w xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2w xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2w xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x7c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2w xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2w xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x7c,0x35,0x00,0x00,0x00,0x00]
          vcvttph2w xmm22, word ptr [rip]{1to8}

// CHECK: vcvttph2w xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7c,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2w xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvttph2w xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x7c,0x71,0x7f]
          vcvttph2w xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvttph2w xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x7c,0x72,0x80]
          vcvttph2w xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvttph2w ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2w ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2w ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x7c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2w ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2w ymm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x7c,0x35,0x00,0x00,0x00,0x00]
          vcvttph2w ymm22, word ptr [rip]{1to16}

// CHECK: vcvttph2w ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x7c,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvttph2w ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvttph2w ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x7c,0x71,0x7f]
          vcvttph2w ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvttph2w ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x7c,0x72,0x80]
          vcvttph2w ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtuw2ph xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x7d,0xf7]
          vcvtuw2ph xmm22, xmm23

// CHECK: vcvtuw2ph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x0f,0x7d,0xf7]
          vcvtuw2ph xmm22 {k7}, xmm23

// CHECK: vcvtuw2ph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x8f,0x7d,0xf7]
          vcvtuw2ph xmm22 {k7} {z}, xmm23

// CHECK: vcvtuw2ph ymm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x28,0x7d,0xf7]
          vcvtuw2ph ymm22, ymm23

// CHECK: vcvtuw2ph ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x2f,0x7d,0xf7]
          vcvtuw2ph ymm22 {k7}, ymm23

// CHECK: vcvtuw2ph ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7f,0xaf,0x7d,0xf7]
          vcvtuw2ph ymm22 {k7} {z}, ymm23

// CHECK: vcvtuw2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtuw2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtuw2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x0f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtuw2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtuw2ph xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7f,0x18,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtuw2ph xmm22, word ptr [rip]{1to8}

// CHECK: vcvtuw2ph xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7f,0x08,0x7d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtuw2ph xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtuw2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7f,0x8f,0x7d,0x71,0x7f]
          vcvtuw2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtuw2ph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7f,0x9f,0x7d,0x72,0x80]
          vcvtuw2ph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtuw2ph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x28,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtuw2ph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtuw2ph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x2f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtuw2ph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtuw2ph ymm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7f,0x38,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtuw2ph ymm22, word ptr [rip]{1to16}

// CHECK: vcvtuw2ph ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7f,0x28,0x7d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtuw2ph ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtuw2ph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7f,0xaf,0x7d,0x71,0x7f]
          vcvtuw2ph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtuw2ph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7f,0xbf,0x7d,0x72,0x80]
          vcvtuw2ph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtw2ph xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x7d,0xf7]
          vcvtw2ph xmm22, xmm23

// CHECK: vcvtw2ph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x0f,0x7d,0xf7]
          vcvtw2ph xmm22 {k7}, xmm23

// CHECK: vcvtw2ph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x8f,0x7d,0xf7]
          vcvtw2ph xmm22 {k7} {z}, xmm23

// CHECK: vcvtw2ph ymm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x7d,0xf7]
          vcvtw2ph ymm22, ymm23

// CHECK: vcvtw2ph ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x2f,0x7d,0xf7]
          vcvtw2ph ymm22 {k7}, ymm23

// CHECK: vcvtw2ph ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0xaf,0x7d,0xf7]
          vcvtw2ph ymm22 {k7} {z}, ymm23

// CHECK: vcvtw2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtw2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtw2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtw2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtw2ph xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7e,0x18,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtw2ph xmm22, word ptr [rip]{1to8}

// CHECK: vcvtw2ph xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x7d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtw2ph xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtw2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x7d,0x71,0x7f]
          vcvtw2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtw2ph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7e,0x9f,0x7d,0x72,0x80]
          vcvtw2ph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtw2ph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtw2ph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtw2ph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x2f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtw2ph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtw2ph ymm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7e,0x38,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtw2ph ymm22, word ptr [rip]{1to16}

// CHECK: vcvtw2ph ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x7d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtw2ph ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtw2ph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x7d,0x71,0x7f]
          vcvtw2ph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtw2ph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7e,0xbf,0x7d,0x72,0x80]
          vcvtw2ph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtdq2ph xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x5b,0xf7]
          vcvtdq2ph xmm22, xmm23

// CHECK: vcvtdq2ph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x5b,0xf7]
          vcvtdq2ph xmm22 {k7}, xmm23

// CHECK: vcvtdq2ph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x5b,0xf7]
          vcvtdq2ph xmm22 {k7} {z}, xmm23

// CHECK: vcvtdq2ph xmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x5b,0xf7]
          vcvtdq2ph xmm22, ymm23

// CHECK: vcvtdq2ph xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x5b,0xf7]
          vcvtdq2ph xmm22 {k7}, ymm23

// CHECK: vcvtdq2ph xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x5b,0xf7]
          vcvtdq2ph xmm22 {k7} {z}, ymm23

// CHECK: vcvtdq2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtdq2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtdq2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtdq2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtdq2ph xmm22, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtdq2ph xmm22, dword ptr [rip]{1to4}

// CHECK: vcvtdq2ph xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x5b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtdq2ph xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtdq2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x5b,0x71,0x7f]
          vcvtdq2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtdq2ph xmm22 {k7} {z}, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x5b,0x72,0x80]
          vcvtdq2ph xmm22 {k7} {z}, dword ptr [rdx - 512]{1to4}

// CHECK: vcvtdq2ph xmm22, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtdq2ph xmm22, dword ptr [rip]{1to8}

// CHECK: vcvtdq2ph xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x5b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtdq2ph xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtdq2ph xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x5b,0x71,0x7f]
          vcvtdq2ph xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtdq2ph xmm22 {k7} {z}, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x5b,0x72,0x80]
          vcvtdq2ph xmm22 {k7} {z}, dword ptr [rdx - 512]{1to8}

// CHECK: vcvtph2dq xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x5b,0xf7]
          vcvtph2dq xmm22, xmm23

// CHECK: vcvtph2dq xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x5b,0xf7]
          vcvtph2dq xmm22 {k7}, xmm23

// CHECK: vcvtph2dq xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x5b,0xf7]
          vcvtph2dq xmm22 {k7} {z}, xmm23

// CHECK: vcvtph2dq ymm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x5b,0xf7]
          vcvtph2dq ymm22, xmm23

// CHECK: vcvtph2dq ymm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x5b,0xf7]
          vcvtph2dq ymm22 {k7}, xmm23

// CHECK: vcvtph2dq ymm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x5b,0xf7]
          vcvtph2dq ymm22 {k7} {z}, xmm23

// CHECK: vcvtph2dq xmm22, qword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2dq xmm22, qword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2dq xmm22 {k7}, qword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2dq xmm22 {k7}, qword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2dq xmm22, word ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2dq xmm22, word ptr [rip]{1to4}

// CHECK: vcvtph2dq xmm22, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x5b,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtph2dq xmm22, qword ptr [2*rbp - 256]

// CHECK: vcvtph2dq xmm22 {k7} {z}, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x5b,0x71,0x7f]
          vcvtph2dq xmm22 {k7} {z}, qword ptr [rcx + 1016]

// CHECK: vcvtph2dq xmm22 {k7} {z}, word ptr [rdx - 256]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x5b,0x72,0x80]
          vcvtph2dq xmm22 {k7} {z}, word ptr [rdx - 256]{1to4}

// CHECK: vcvtph2dq ymm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2dq ymm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2dq ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2dq ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2dq ymm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2dq ymm22, word ptr [rip]{1to8}

// CHECK: vcvtph2dq ymm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x5b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2dq ymm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtph2dq ymm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x5b,0x71,0x7f]
          vcvtph2dq ymm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtph2dq ymm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x5b,0x72,0x80]
          vcvtph2dq ymm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtph2udq xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x79,0xf7]
          vcvtph2udq xmm22, xmm23

// CHECK: vcvtph2udq xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x79,0xf7]
          vcvtph2udq xmm22 {k7}, xmm23

// CHECK: vcvtph2udq xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x79,0xf7]
          vcvtph2udq xmm22 {k7} {z}, xmm23

// CHECK: vcvtph2udq ymm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x79,0xf7]
          vcvtph2udq ymm22, xmm23

// CHECK: vcvtph2udq ymm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x79,0xf7]
          vcvtph2udq ymm22 {k7}, xmm23

// CHECK: vcvtph2udq ymm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x79,0xf7]
          vcvtph2udq ymm22 {k7} {z}, xmm23

// CHECK: vcvtph2udq xmm22, qword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x79,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2udq xmm22, qword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2udq xmm22 {k7}, qword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x79,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2udq xmm22 {k7}, qword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2udq xmm22, word ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x79,0x35,0x00,0x00,0x00,0x00]
          vcvtph2udq xmm22, word ptr [rip]{1to4}

// CHECK: vcvtph2udq xmm22, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x79,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtph2udq xmm22, qword ptr [2*rbp - 256]

// CHECK: vcvtph2udq xmm22 {k7} {z}, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x79,0x71,0x7f]
          vcvtph2udq xmm22 {k7} {z}, qword ptr [rcx + 1016]

// CHECK: vcvtph2udq xmm22 {k7} {z}, word ptr [rdx - 256]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x79,0x72,0x80]
          vcvtph2udq xmm22 {k7} {z}, word ptr [rdx - 256]{1to4}

// CHECK: vcvtph2udq ymm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x79,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2udq ymm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2udq ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x2f,0x79,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2udq ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2udq ymm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x79,0x35,0x00,0x00,0x00,0x00]
          vcvtph2udq ymm22, word ptr [rip]{1to8}

// CHECK: vcvtph2udq ymm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x79,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2udq ymm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtph2udq ymm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x79,0x71,0x7f]
          vcvtph2udq ymm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtph2udq ymm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x79,0x72,0x80]
          vcvtph2udq ymm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvttph2dq xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x5b,0xf7]
          vcvttph2dq xmm22, xmm23

// CHECK: vcvttph2dq xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x0f,0x5b,0xf7]
          vcvttph2dq xmm22 {k7}, xmm23

// CHECK: vcvttph2dq xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x8f,0x5b,0xf7]
          vcvttph2dq xmm22 {k7} {z}, xmm23

// CHECK: vcvttph2dq ymm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x5b,0xf7]
          vcvttph2dq ymm22, xmm23

// CHECK: vcvttph2dq ymm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x2f,0x5b,0xf7]
          vcvttph2dq ymm22 {k7}, xmm23

// CHECK: vcvttph2dq ymm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0xaf,0x5b,0xf7]
          vcvttph2dq ymm22 {k7} {z}, xmm23

// CHECK: vcvttph2dq xmm22, qword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2dq xmm22, qword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2dq xmm22 {k7}, qword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2dq xmm22 {k7}, qword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2dq xmm22, word ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7e,0x18,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvttph2dq xmm22, word ptr [rip]{1to4}

// CHECK: vcvttph2dq xmm22, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x5b,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvttph2dq xmm22, qword ptr [2*rbp - 256]

// CHECK: vcvttph2dq xmm22 {k7} {z}, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x5b,0x71,0x7f]
          vcvttph2dq xmm22 {k7} {z}, qword ptr [rcx + 1016]

// CHECK: vcvttph2dq xmm22 {k7} {z}, word ptr [rdx - 256]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7e,0x9f,0x5b,0x72,0x80]
          vcvttph2dq xmm22 {k7} {z}, word ptr [rdx - 256]{1to4}

// CHECK: vcvttph2dq ymm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2dq ymm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2dq ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x2f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2dq ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2dq ymm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7e,0x38,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvttph2dq ymm22, word ptr [rip]{1to8}

// CHECK: vcvttph2dq ymm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x5b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2dq ymm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvttph2dq ymm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x5b,0x71,0x7f]
          vcvttph2dq ymm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvttph2dq ymm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7e,0xbf,0x5b,0x72,0x80]
          vcvttph2dq ymm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvttph2udq xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x78,0xf7]
          vcvttph2udq xmm22, xmm23

// CHECK: vcvttph2udq xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x78,0xf7]
          vcvttph2udq xmm22 {k7}, xmm23

// CHECK: vcvttph2udq xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x78,0xf7]
          vcvttph2udq xmm22 {k7} {z}, xmm23

// CHECK: vcvttph2udq ymm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x78,0xf7]
          vcvttph2udq ymm22, xmm23

// CHECK: vcvttph2udq ymm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x78,0xf7]
          vcvttph2udq ymm22 {k7}, xmm23

// CHECK: vcvttph2udq ymm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x78,0xf7]
          vcvttph2udq ymm22 {k7} {z}, xmm23

// CHECK: vcvttph2udq xmm22, qword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x78,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2udq xmm22, qword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2udq xmm22 {k7}, qword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x78,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2udq xmm22 {k7}, qword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2udq xmm22, word ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x78,0x35,0x00,0x00,0x00,0x00]
          vcvttph2udq xmm22, word ptr [rip]{1to4}

// CHECK: vcvttph2udq xmm22, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x78,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvttph2udq xmm22, qword ptr [2*rbp - 256]

// CHECK: vcvttph2udq xmm22 {k7} {z}, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x78,0x71,0x7f]
          vcvttph2udq xmm22 {k7} {z}, qword ptr [rcx + 1016]

// CHECK: vcvttph2udq xmm22 {k7} {z}, word ptr [rdx - 256]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x78,0x72,0x80]
          vcvttph2udq xmm22 {k7} {z}, word ptr [rdx - 256]{1to4}

// CHECK: vcvttph2udq ymm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x78,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2udq ymm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2udq ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x2f,0x78,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2udq ymm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2udq ymm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x78,0x35,0x00,0x00,0x00,0x00]
          vcvttph2udq ymm22, word ptr [rip]{1to8}

// CHECK: vcvttph2udq ymm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x78,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2udq ymm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvttph2udq ymm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x78,0x71,0x7f]
          vcvttph2udq ymm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvttph2udq ymm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x78,0x72,0x80]
          vcvttph2udq ymm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtudq2ph xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x7a,0xf7]
          vcvtudq2ph xmm22, xmm23

// CHECK: vcvtudq2ph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x0f,0x7a,0xf7]
          vcvtudq2ph xmm22 {k7}, xmm23

// CHECK: vcvtudq2ph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x8f,0x7a,0xf7]
          vcvtudq2ph xmm22 {k7} {z}, xmm23

// CHECK: vcvtudq2ph xmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x28,0x7a,0xf7]
          vcvtudq2ph xmm22, ymm23

// CHECK: vcvtudq2ph xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x2f,0x7a,0xf7]
          vcvtudq2ph xmm22 {k7}, ymm23

// CHECK: vcvtudq2ph xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7f,0xaf,0x7a,0xf7]
          vcvtudq2ph xmm22 {k7} {z}, ymm23

// CHECK: vcvtudq2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtudq2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtudq2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x0f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtudq2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtudq2ph xmm22, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7f,0x18,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvtudq2ph xmm22, dword ptr [rip]{1to4}

// CHECK: vcvtudq2ph xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7f,0x08,0x7a,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtudq2ph xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtudq2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7f,0x8f,0x7a,0x71,0x7f]
          vcvtudq2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtudq2ph xmm22 {k7} {z}, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7f,0x9f,0x7a,0x72,0x80]
          vcvtudq2ph xmm22 {k7} {z}, dword ptr [rdx - 512]{1to4}

// CHECK: vcvtudq2ph xmm22, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7f,0x38,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvtudq2ph xmm22, dword ptr [rip]{1to8}

// CHECK: vcvtudq2ph xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7f,0x28,0x7a,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtudq2ph xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtudq2ph xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7f,0xaf,0x7a,0x71,0x7f]
          vcvtudq2ph xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtudq2ph xmm22 {k7} {z}, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7f,0xbf,0x7a,0x72,0x80]
          vcvtudq2ph xmm22 {k7} {z}, dword ptr [rdx - 512]{1to8}

// CHECK: vcvtph2qq xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7b,0xf7]
          vcvtph2qq xmm22, xmm23

// CHECK: vcvtph2qq xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x7b,0xf7]
          vcvtph2qq xmm22 {k7}, xmm23

// CHECK: vcvtph2qq xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x7b,0xf7]
          vcvtph2qq xmm22 {k7} {z}, xmm23

// CHECK: vcvtph2qq ymm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7b,0xf7]
          vcvtph2qq ymm22, xmm23

// CHECK: vcvtph2qq ymm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x7b,0xf7]
          vcvtph2qq ymm22 {k7}, xmm23

// CHECK: vcvtph2qq ymm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x7b,0xf7]
          vcvtph2qq ymm22 {k7} {z}, xmm23

// CHECK: vcvtph2qq xmm22, dword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2qq xmm22, dword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2qq xmm22 {k7}, dword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x7b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2qq xmm22 {k7}, dword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2qq xmm22, word ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x7b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2qq xmm22, word ptr [rip]{1to2}

// CHECK: vcvtph2qq xmm22, dword ptr [2*rbp - 128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7b,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvtph2qq xmm22, dword ptr [2*rbp - 128]

// CHECK: vcvtph2qq xmm22 {k7} {z}, dword ptr [rcx + 508]
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x7b,0x71,0x7f]
          vcvtph2qq xmm22 {k7} {z}, dword ptr [rcx + 508]

// CHECK: vcvtph2qq xmm22 {k7} {z}, word ptr [rdx - 256]{1to2}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x7b,0x72,0x80]
          vcvtph2qq xmm22 {k7} {z}, word ptr [rdx - 256]{1to2}

// CHECK: vcvtph2qq ymm22, qword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2qq ymm22, qword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2qq ymm22 {k7}, qword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x7b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2qq ymm22 {k7}, qword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2qq ymm22, word ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x7b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2qq ymm22, word ptr [rip]{1to4}

// CHECK: vcvtph2qq ymm22, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x7b,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtph2qq ymm22, qword ptr [2*rbp - 256]

// CHECK: vcvtph2qq ymm22 {k7} {z}, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x7b,0x71,0x7f]
          vcvtph2qq ymm22 {k7} {z}, qword ptr [rcx + 1016]

// CHECK: vcvtph2qq ymm22 {k7} {z}, word ptr [rdx - 256]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x7b,0x72,0x80]
          vcvtph2qq ymm22 {k7} {z}, word ptr [rdx - 256]{1to4}

// CHECK: vcvtph2uqq xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x79,0xf7]
          vcvtph2uqq xmm22, xmm23

// CHECK: vcvtph2uqq xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x79,0xf7]
          vcvtph2uqq xmm22 {k7}, xmm23

// CHECK: vcvtph2uqq xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x79,0xf7]
          vcvtph2uqq xmm22 {k7} {z}, xmm23

// CHECK: vcvtph2uqq ymm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x79,0xf7]
          vcvtph2uqq ymm22, xmm23

// CHECK: vcvtph2uqq ymm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x79,0xf7]
          vcvtph2uqq ymm22 {k7}, xmm23

// CHECK: vcvtph2uqq ymm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x79,0xf7]
          vcvtph2uqq ymm22 {k7} {z}, xmm23

// CHECK: vcvtph2uqq xmm22, dword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x79,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2uqq xmm22, dword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2uqq xmm22 {k7}, dword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x79,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2uqq xmm22 {k7}, dword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2uqq xmm22, word ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x79,0x35,0x00,0x00,0x00,0x00]
          vcvtph2uqq xmm22, word ptr [rip]{1to2}

// CHECK: vcvtph2uqq xmm22, dword ptr [2*rbp - 128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x79,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvtph2uqq xmm22, dword ptr [2*rbp - 128]

// CHECK: vcvtph2uqq xmm22 {k7} {z}, dword ptr [rcx + 508]
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x79,0x71,0x7f]
          vcvtph2uqq xmm22 {k7} {z}, dword ptr [rcx + 508]

// CHECK: vcvtph2uqq xmm22 {k7} {z}, word ptr [rdx - 256]{1to2}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x79,0x72,0x80]
          vcvtph2uqq xmm22 {k7} {z}, word ptr [rdx - 256]{1to2}

// CHECK: vcvtph2uqq ymm22, qword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x79,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2uqq ymm22, qword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2uqq ymm22 {k7}, qword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x79,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2uqq ymm22 {k7}, qword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2uqq ymm22, word ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x79,0x35,0x00,0x00,0x00,0x00]
          vcvtph2uqq ymm22, word ptr [rip]{1to4}

// CHECK: vcvtph2uqq ymm22, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x79,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtph2uqq ymm22, qword ptr [2*rbp - 256]

// CHECK: vcvtph2uqq ymm22 {k7} {z}, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x79,0x71,0x7f]
          vcvtph2uqq ymm22 {k7} {z}, qword ptr [rcx + 1016]

// CHECK: vcvtph2uqq ymm22 {k7} {z}, word ptr [rdx - 256]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x79,0x72,0x80]
          vcvtph2uqq ymm22 {k7} {z}, word ptr [rdx - 256]{1to4}

// CHECK: vcvtqq2ph xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x08,0x5b,0xf7]
          vcvtqq2ph xmm22, xmm23

// CHECK: vcvtqq2ph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x0f,0x5b,0xf7]
          vcvtqq2ph xmm22 {k7}, xmm23

// CHECK: vcvtqq2ph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x8f,0x5b,0xf7]
          vcvtqq2ph xmm22 {k7} {z}, xmm23

// CHECK: vcvtqq2ph xmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x28,0x5b,0xf7]
          vcvtqq2ph xmm22, ymm23

// CHECK: vcvtqq2ph xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x2f,0x5b,0xf7]
          vcvtqq2ph xmm22 {k7}, ymm23

// CHECK: vcvtqq2ph xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0xfc,0xaf,0x5b,0xf7]
          vcvtqq2ph xmm22 {k7} {z}, ymm23

// CHECK: vcvtqq2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfc,0x08,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtqq2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtqq2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfc,0x0f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtqq2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtqq2ph xmm22, qword ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe5,0xfc,0x18,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtqq2ph xmm22, qword ptr [rip]{1to2}

// CHECK: vcvtqq2ph xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xfc,0x08,0x5b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtqq2ph xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtqq2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xfc,0x8f,0x5b,0x71,0x7f]
          vcvtqq2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtqq2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to2}
// CHECK: encoding: [0x62,0xe5,0xfc,0x9f,0x5b,0x72,0x80]
          vcvtqq2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to2}

// CHECK: vcvtqq2ph xmm22, qword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0xfc,0x38,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtqq2ph xmm22, qword ptr [rip]{1to4}

// CHECK: vcvtqq2ph xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xfc,0x28,0x5b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtqq2ph xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtqq2ph xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xfc,0xaf,0x5b,0x71,0x7f]
          vcvtqq2ph xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtqq2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to4}
// CHECK: encoding: [0x62,0xe5,0xfc,0xbf,0x5b,0x72,0x80]
          vcvtqq2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to4}

// CHECK: vcvttph2qq xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7a,0xf7]
          vcvttph2qq xmm22, xmm23

// CHECK: vcvttph2qq xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x7a,0xf7]
          vcvttph2qq xmm22 {k7}, xmm23

// CHECK: vcvttph2qq xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x7a,0xf7]
          vcvttph2qq xmm22 {k7} {z}, xmm23

// CHECK: vcvttph2qq ymm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7a,0xf7]
          vcvttph2qq ymm22, xmm23

// CHECK: vcvttph2qq ymm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x7a,0xf7]
          vcvttph2qq ymm22 {k7}, xmm23

// CHECK: vcvttph2qq ymm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x7a,0xf7]
          vcvttph2qq ymm22 {k7} {z}, xmm23

// CHECK: vcvttph2qq xmm22, dword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2qq xmm22, dword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2qq xmm22 {k7}, dword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2qq xmm22 {k7}, dword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2qq xmm22, word ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvttph2qq xmm22, word ptr [rip]{1to2}

// CHECK: vcvttph2qq xmm22, dword ptr [2*rbp - 128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7a,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvttph2qq xmm22, dword ptr [2*rbp - 128]

// CHECK: vcvttph2qq xmm22 {k7} {z}, dword ptr [rcx + 508]
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x7a,0x71,0x7f]
          vcvttph2qq xmm22 {k7} {z}, dword ptr [rcx + 508]

// CHECK: vcvttph2qq xmm22 {k7} {z}, word ptr [rdx - 256]{1to2}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x7a,0x72,0x80]
          vcvttph2qq xmm22 {k7} {z}, word ptr [rdx - 256]{1to2}

// CHECK: vcvttph2qq ymm22, qword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2qq ymm22, qword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2qq ymm22 {k7}, qword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2qq ymm22 {k7}, qword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2qq ymm22, word ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvttph2qq ymm22, word ptr [rip]{1to4}

// CHECK: vcvttph2qq ymm22, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x7a,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvttph2qq ymm22, qword ptr [2*rbp - 256]

// CHECK: vcvttph2qq ymm22 {k7} {z}, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x7a,0x71,0x7f]
          vcvttph2qq ymm22 {k7} {z}, qword ptr [rcx + 1016]

// CHECK: vcvttph2qq ymm22 {k7} {z}, word ptr [rdx - 256]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x7a,0x72,0x80]
          vcvttph2qq ymm22 {k7} {z}, word ptr [rdx - 256]{1to4}

// CHECK: vcvttph2uqq xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x78,0xf7]
          vcvttph2uqq xmm22, xmm23

// CHECK: vcvttph2uqq xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x78,0xf7]
          vcvttph2uqq xmm22 {k7}, xmm23

// CHECK: vcvttph2uqq xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x78,0xf7]
          vcvttph2uqq xmm22 {k7} {z}, xmm23

// CHECK: vcvttph2uqq ymm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x78,0xf7]
          vcvttph2uqq ymm22, xmm23

// CHECK: vcvttph2uqq ymm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x78,0xf7]
          vcvttph2uqq ymm22 {k7}, xmm23

// CHECK: vcvttph2uqq ymm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x78,0xf7]
          vcvttph2uqq ymm22 {k7} {z}, xmm23

// CHECK: vcvttph2uqq xmm22, dword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x78,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2uqq xmm22, dword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2uqq xmm22 {k7}, dword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x78,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2uqq xmm22 {k7}, dword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2uqq xmm22, word ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x78,0x35,0x00,0x00,0x00,0x00]
          vcvttph2uqq xmm22, word ptr [rip]{1to2}

// CHECK: vcvttph2uqq xmm22, dword ptr [2*rbp - 128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x78,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvttph2uqq xmm22, dword ptr [2*rbp - 128]

// CHECK: vcvttph2uqq xmm22 {k7} {z}, dword ptr [rcx + 508]
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x78,0x71,0x7f]
          vcvttph2uqq xmm22 {k7} {z}, dword ptr [rcx + 508]

// CHECK: vcvttph2uqq xmm22 {k7} {z}, word ptr [rdx - 256]{1to2}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x78,0x72,0x80]
          vcvttph2uqq xmm22 {k7} {z}, word ptr [rdx - 256]{1to2}

// CHECK: vcvttph2uqq ymm22, qword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x78,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2uqq ymm22, qword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2uqq ymm22 {k7}, qword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x78,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2uqq ymm22 {k7}, qword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2uqq ymm22, word ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x78,0x35,0x00,0x00,0x00,0x00]
          vcvttph2uqq ymm22, word ptr [rip]{1to4}

// CHECK: vcvttph2uqq ymm22, qword ptr [2*rbp - 256]
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x78,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvttph2uqq ymm22, qword ptr [2*rbp - 256]

// CHECK: vcvttph2uqq ymm22 {k7} {z}, qword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x78,0x71,0x7f]
          vcvttph2uqq ymm22 {k7} {z}, qword ptr [rcx + 1016]

// CHECK: vcvttph2uqq ymm22 {k7} {z}, word ptr [rdx - 256]{1to4}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x78,0x72,0x80]
          vcvttph2uqq ymm22 {k7} {z}, word ptr [rdx - 256]{1to4}

// CHECK: vcvtuqq2ph xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0xff,0x08,0x7a,0xf7]
          vcvtuqq2ph xmm22, xmm23

// CHECK: vcvtuqq2ph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0xff,0x0f,0x7a,0xf7]
          vcvtuqq2ph xmm22 {k7}, xmm23

// CHECK: vcvtuqq2ph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0xff,0x8f,0x7a,0xf7]
          vcvtuqq2ph xmm22 {k7} {z}, xmm23

// CHECK: vcvtuqq2ph xmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0xff,0x28,0x7a,0xf7]
          vcvtuqq2ph xmm22, ymm23

// CHECK: vcvtuqq2ph xmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0xff,0x2f,0x7a,0xf7]
          vcvtuqq2ph xmm22 {k7}, ymm23

// CHECK: vcvtuqq2ph xmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0xff,0xaf,0x7a,0xf7]
          vcvtuqq2ph xmm22 {k7} {z}, ymm23

// CHECK: vcvtuqq2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xff,0x08,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtuqq2ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtuqq2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xff,0x0f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtuqq2ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtuqq2ph xmm22, qword ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe5,0xff,0x18,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvtuqq2ph xmm22, qword ptr [rip]{1to2}

// CHECK: vcvtuqq2ph xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xff,0x08,0x7a,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtuqq2ph xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtuqq2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xff,0x8f,0x7a,0x71,0x7f]
          vcvtuqq2ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtuqq2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to2}
// CHECK: encoding: [0x62,0xe5,0xff,0x9f,0x7a,0x72,0x80]
          vcvtuqq2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to2}

// CHECK: vcvtuqq2ph xmm22, qword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0xff,0x38,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvtuqq2ph xmm22, qword ptr [rip]{1to4}

// CHECK: vcvtuqq2ph xmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xff,0x28,0x7a,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtuqq2ph xmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtuqq2ph xmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xff,0xaf,0x7a,0x71,0x7f]
          vcvtuqq2ph xmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtuqq2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to4}
// CHECK: encoding: [0x62,0xe5,0xff,0xbf,0x7a,0x72,0x80]
          vcvtuqq2ph xmm22 {k7} {z}, qword ptr [rdx - 1024]{1to4}

// CHECK: vfpclassph k5, xmm22, 123
// CHECK: encoding: [0x62,0xb3,0x7c,0x08,0x66,0xee,0x7b]
          vfpclassph k5, xmm22, 123

// CHECK: vfpclassph k5 {k7}, xmm22, 123
// CHECK: encoding: [0x62,0xb3,0x7c,0x0f,0x66,0xee,0x7b]
          vfpclassph k5 {k7}, xmm22, 123

// CHECK: vfpclassph k5, ymm22, 123
// CHECK: encoding: [0x62,0xb3,0x7c,0x28,0x66,0xee,0x7b]
          vfpclassph k5, ymm22, 123

// CHECK: vfpclassph k5 {k7}, ymm22, 123
// CHECK: encoding: [0x62,0xb3,0x7c,0x2f,0x66,0xee,0x7b]
          vfpclassph k5 {k7}, ymm22, 123

// CHECK: vfpclassph k5, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xb3,0x7c,0x08,0x66,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vfpclassph k5, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vfpclassph k5 {k7}, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xd3,0x7c,0x0f,0x66,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vfpclassph k5 {k7}, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vfpclassph k5, word ptr [rip]{1to8}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x18,0x66,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vfpclassph k5, word ptr [rip]{1to8}, 123

// CHECK: vfpclassph k5, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x08,0x66,0x2c,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vfpclassph k5, xmmword ptr [2*rbp - 512], 123

// CHECK: vfpclassph k5 {k7}, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x0f,0x66,0x69,0x7f,0x7b]
          vfpclassph k5 {k7}, xmmword ptr [rcx + 2032], 123

// CHECK: vfpclassph k5 {k7}, word ptr [rdx - 256]{1to8}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x1f,0x66,0x6a,0x80,0x7b]
          vfpclassph k5 {k7}, word ptr [rdx - 256]{1to8}, 123

// CHECK: vfpclassph k5, word ptr [rip]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x38,0x66,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vfpclassph k5, word ptr [rip]{1to16}, 123

// CHECK: vfpclassph k5, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x28,0x66,0x2c,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vfpclassph k5, ymmword ptr [2*rbp - 1024], 123

// CHECK: vfpclassph k5 {k7}, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x2f,0x66,0x69,0x7f,0x7b]
          vfpclassph k5 {k7}, ymmword ptr [rcx + 4064], 123

// CHECK: vfpclassph k5 {k7}, word ptr [rdx - 256]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x3f,0x66,0x6a,0x80,0x7b]
          vfpclassph k5 {k7}, word ptr [rdx - 256]{1to16}, 123

// CHECK: vgetexpph xmm22, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x42,0xf7]
          vgetexpph xmm22, xmm23

// CHECK: vgetexpph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x0f,0x42,0xf7]
          vgetexpph xmm22 {k7}, xmm23

// CHECK: vgetexpph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x8f,0x42,0xf7]
          vgetexpph xmm22 {k7} {z}, xmm23

// CHECK: vgetexpph ymm22, ymm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x42,0xf7]
          vgetexpph ymm22, ymm23

// CHECK: vgetexpph ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x2f,0x42,0xf7]
          vgetexpph ymm22 {k7}, ymm23

// CHECK: vgetexpph ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa6,0x7d,0xaf,0x42,0xf7]
          vgetexpph ymm22 {k7} {z}, ymm23

// CHECK: vgetexpph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x42,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vgetexpph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vgetexpph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x7d,0x0f,0x42,0xb4,0x80,0x23,0x01,0x00,0x00]
          vgetexpph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vgetexpph xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x7d,0x18,0x42,0x35,0x00,0x00,0x00,0x00]
          vgetexpph xmm22, word ptr [rip]{1to8}

// CHECK: vgetexpph xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x7d,0x08,0x42,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vgetexpph xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vgetexpph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x7d,0x8f,0x42,0x71,0x7f]
          vgetexpph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vgetexpph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x7d,0x9f,0x42,0x72,0x80]
          vgetexpph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vgetexpph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x42,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vgetexpph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vgetexpph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x7d,0x2f,0x42,0xb4,0x80,0x23,0x01,0x00,0x00]
          vgetexpph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vgetexpph ymm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x7d,0x38,0x42,0x35,0x00,0x00,0x00,0x00]
          vgetexpph ymm22, word ptr [rip]{1to16}

// CHECK: vgetexpph ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x7d,0x28,0x42,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vgetexpph ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vgetexpph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x7d,0xaf,0x42,0x71,0x7f]
          vgetexpph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vgetexpph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x7d,0xbf,0x42,0x72,0x80]
          vgetexpph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vgetmantph ymm22, ymm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x28,0x26,0xf7,0x7b]
          vgetmantph ymm22, ymm23, 123

// CHECK: vgetmantph ymm22 {k7}, ymm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x2f,0x26,0xf7,0x7b]
          vgetmantph ymm22 {k7}, ymm23, 123

// CHECK: vgetmantph ymm22 {k7} {z}, ymm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0xaf,0x26,0xf7,0x7b]
          vgetmantph ymm22 {k7} {z}, ymm23, 123

// CHECK: vgetmantph xmm22, xmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x08,0x26,0xf7,0x7b]
          vgetmantph xmm22, xmm23, 123

// CHECK: vgetmantph xmm22 {k7}, xmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x0f,0x26,0xf7,0x7b]
          vgetmantph xmm22 {k7}, xmm23, 123

// CHECK: vgetmantph xmm22 {k7} {z}, xmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x8f,0x26,0xf7,0x7b]
          vgetmantph xmm22 {k7} {z}, xmm23, 123

// CHECK: vgetmantph xmm22, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x08,0x26,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vgetmantph xmm22, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vgetmantph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7c,0x0f,0x26,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vgetmantph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vgetmantph xmm22, word ptr [rip]{1to8}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x18,0x26,0x35,0x00,0x00,0x00,0x00,0x7b]
          vgetmantph xmm22, word ptr [rip]{1to8}, 123

// CHECK: vgetmantph xmm22, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x08,0x26,0x34,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vgetmantph xmm22, xmmword ptr [2*rbp - 512], 123

// CHECK: vgetmantph xmm22 {k7} {z}, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x8f,0x26,0x71,0x7f,0x7b]
          vgetmantph xmm22 {k7} {z}, xmmword ptr [rcx + 2032], 123

// CHECK: vgetmantph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x9f,0x26,0x72,0x80,0x7b]
          vgetmantph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}, 123

// CHECK: vgetmantph ymm22, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x28,0x26,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vgetmantph ymm22, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vgetmantph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7c,0x2f,0x26,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vgetmantph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK: vgetmantph ymm22, word ptr [rip]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x38,0x26,0x35,0x00,0x00,0x00,0x00,0x7b]
          vgetmantph ymm22, word ptr [rip]{1to16}, 123

// CHECK: vgetmantph ymm22, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x28,0x26,0x34,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vgetmantph ymm22, ymmword ptr [2*rbp - 1024], 123

// CHECK: vgetmantph ymm22 {k7} {z}, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xaf,0x26,0x71,0x7f,0x7b]
          vgetmantph ymm22 {k7} {z}, ymmword ptr [rcx + 4064], 123

// CHECK: vgetmantph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xbf,0x26,0x72,0x80,0x7b]
          vgetmantph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}, 123

// CHECK: vrcpph xmm22, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x4c,0xf7]
          vrcpph xmm22, xmm23

// CHECK: vrcpph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x0f,0x4c,0xf7]
          vrcpph xmm22 {k7}, xmm23

// CHECK: vrcpph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x8f,0x4c,0xf7]
          vrcpph xmm22 {k7} {z}, xmm23

// CHECK: vrcpph ymm22, ymm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x4c,0xf7]
          vrcpph ymm22, ymm23

// CHECK: vrcpph ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x2f,0x4c,0xf7]
          vrcpph ymm22 {k7}, ymm23

// CHECK: vrcpph ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa6,0x7d,0xaf,0x4c,0xf7]
          vrcpph ymm22 {k7} {z}, ymm23

// CHECK: vrcpph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x4c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrcpph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vrcpph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x7d,0x0f,0x4c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrcpph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vrcpph xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x7d,0x18,0x4c,0x35,0x00,0x00,0x00,0x00]
          vrcpph xmm22, word ptr [rip]{1to8}

// CHECK: vrcpph xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x7d,0x08,0x4c,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vrcpph xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vrcpph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x7d,0x8f,0x4c,0x71,0x7f]
          vrcpph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vrcpph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x7d,0x9f,0x4c,0x72,0x80]
          vrcpph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vrcpph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x4c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrcpph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vrcpph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x7d,0x2f,0x4c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrcpph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vrcpph ymm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x7d,0x38,0x4c,0x35,0x00,0x00,0x00,0x00]
          vrcpph ymm22, word ptr [rip]{1to16}

// CHECK: vrcpph ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x7d,0x28,0x4c,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vrcpph ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vrcpph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x7d,0xaf,0x4c,0x71,0x7f]
          vrcpph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vrcpph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x7d,0xbf,0x4c,0x72,0x80]
          vrcpph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vreduceph ymm22, ymm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x28,0x56,0xf7,0x7b]
          vreduceph ymm22, ymm23, 123

// CHECK: vreduceph ymm22 {k7}, ymm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x2f,0x56,0xf7,0x7b]
          vreduceph ymm22 {k7}, ymm23, 123

// CHECK: vreduceph ymm22 {k7} {z}, ymm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0xaf,0x56,0xf7,0x7b]
          vreduceph ymm22 {k7} {z}, ymm23, 123

// CHECK: vreduceph xmm22, xmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x08,0x56,0xf7,0x7b]
          vreduceph xmm22, xmm23, 123

// CHECK: vreduceph xmm22 {k7}, xmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x0f,0x56,0xf7,0x7b]
          vreduceph xmm22 {k7}, xmm23, 123

// CHECK: vreduceph xmm22 {k7} {z}, xmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x8f,0x56,0xf7,0x7b]
          vreduceph xmm22 {k7} {z}, xmm23, 123

// CHECK: vreduceph xmm22, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x08,0x56,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vreduceph xmm22, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vreduceph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7c,0x0f,0x56,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vreduceph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vreduceph xmm22, word ptr [rip]{1to8}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x18,0x56,0x35,0x00,0x00,0x00,0x00,0x7b]
          vreduceph xmm22, word ptr [rip]{1to8}, 123

// CHECK: vreduceph xmm22, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x08,0x56,0x34,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vreduceph xmm22, xmmword ptr [2*rbp - 512], 123

// CHECK: vreduceph xmm22 {k7} {z}, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x8f,0x56,0x71,0x7f,0x7b]
          vreduceph xmm22 {k7} {z}, xmmword ptr [rcx + 2032], 123

// CHECK: vreduceph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x9f,0x56,0x72,0x80,0x7b]
          vreduceph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}, 123

// CHECK: vreduceph ymm22, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x28,0x56,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vreduceph ymm22, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vreduceph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7c,0x2f,0x56,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vreduceph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK: vreduceph ymm22, word ptr [rip]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x38,0x56,0x35,0x00,0x00,0x00,0x00,0x7b]
          vreduceph ymm22, word ptr [rip]{1to16}, 123

// CHECK: vreduceph ymm22, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x28,0x56,0x34,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vreduceph ymm22, ymmword ptr [2*rbp - 1024], 123

// CHECK: vreduceph ymm22 {k7} {z}, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xaf,0x56,0x71,0x7f,0x7b]
          vreduceph ymm22 {k7} {z}, ymmword ptr [rcx + 4064], 123

// CHECK: vreduceph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xbf,0x56,0x72,0x80,0x7b]
          vreduceph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}, 123

// CHECK: vrndscaleph ymm22, ymm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x28,0x08,0xf7,0x7b]
          vrndscaleph ymm22, ymm23, 123

// CHECK: vrndscaleph ymm22 {k7}, ymm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x2f,0x08,0xf7,0x7b]
          vrndscaleph ymm22 {k7}, ymm23, 123

// CHECK: vrndscaleph ymm22 {k7} {z}, ymm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0xaf,0x08,0xf7,0x7b]
          vrndscaleph ymm22 {k7} {z}, ymm23, 123

// CHECK: vrndscaleph xmm22, xmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x08,0x08,0xf7,0x7b]
          vrndscaleph xmm22, xmm23, 123

// CHECK: vrndscaleph xmm22 {k7}, xmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x0f,0x08,0xf7,0x7b]
          vrndscaleph xmm22 {k7}, xmm23, 123

// CHECK: vrndscaleph xmm22 {k7} {z}, xmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x8f,0x08,0xf7,0x7b]
          vrndscaleph xmm22 {k7} {z}, xmm23, 123

// CHECK: vrndscaleph xmm22, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x08,0x08,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vrndscaleph xmm22, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vrndscaleph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7c,0x0f,0x08,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vrndscaleph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vrndscaleph xmm22, word ptr [rip]{1to8}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x18,0x08,0x35,0x00,0x00,0x00,0x00,0x7b]
          vrndscaleph xmm22, word ptr [rip]{1to8}, 123

// CHECK: vrndscaleph xmm22, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x08,0x08,0x34,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vrndscaleph xmm22, xmmword ptr [2*rbp - 512], 123

// CHECK: vrndscaleph xmm22 {k7} {z}, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x8f,0x08,0x71,0x7f,0x7b]
          vrndscaleph xmm22 {k7} {z}, xmmword ptr [rcx + 2032], 123

// CHECK: vrndscaleph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x9f,0x08,0x72,0x80,0x7b]
          vrndscaleph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}, 123

// CHECK: vrndscaleph ymm22, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x28,0x08,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vrndscaleph ymm22, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vrndscaleph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7c,0x2f,0x08,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vrndscaleph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK: vrndscaleph ymm22, word ptr [rip]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x38,0x08,0x35,0x00,0x00,0x00,0x00,0x7b]
          vrndscaleph ymm22, word ptr [rip]{1to16}, 123

// CHECK: vrndscaleph ymm22, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x28,0x08,0x34,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vrndscaleph ymm22, ymmword ptr [2*rbp - 1024], 123

// CHECK: vrndscaleph ymm22 {k7} {z}, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xaf,0x08,0x71,0x7f,0x7b]
          vrndscaleph ymm22 {k7} {z}, ymmword ptr [rcx + 4064], 123

// CHECK: vrndscaleph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xbf,0x08,0x72,0x80,0x7b]
          vrndscaleph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}, 123

// CHECK: vrsqrtph xmm22, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x4e,0xf7]
          vrsqrtph xmm22, xmm23

// CHECK: vrsqrtph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x0f,0x4e,0xf7]
          vrsqrtph xmm22 {k7}, xmm23

// CHECK: vrsqrtph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x8f,0x4e,0xf7]
          vrsqrtph xmm22 {k7} {z}, xmm23

// CHECK: vrsqrtph ymm22, ymm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x4e,0xf7]
          vrsqrtph ymm22, ymm23

// CHECK: vrsqrtph ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa6,0x7d,0x2f,0x4e,0xf7]
          vrsqrtph ymm22 {k7}, ymm23

// CHECK: vrsqrtph ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa6,0x7d,0xaf,0x4e,0xf7]
          vrsqrtph ymm22 {k7} {z}, ymm23

// CHECK: vrsqrtph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x4e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrsqrtph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vrsqrtph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x7d,0x0f,0x4e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrsqrtph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vrsqrtph xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x7d,0x18,0x4e,0x35,0x00,0x00,0x00,0x00]
          vrsqrtph xmm22, word ptr [rip]{1to8}

// CHECK: vrsqrtph xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x7d,0x08,0x4e,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vrsqrtph xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vrsqrtph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x7d,0x8f,0x4e,0x71,0x7f]
          vrsqrtph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vrsqrtph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x7d,0x9f,0x4e,0x72,0x80]
          vrsqrtph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vrsqrtph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x4e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrsqrtph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vrsqrtph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x7d,0x2f,0x4e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrsqrtph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vrsqrtph ymm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x7d,0x38,0x4e,0x35,0x00,0x00,0x00,0x00]
          vrsqrtph ymm22, word ptr [rip]{1to16}

// CHECK: vrsqrtph ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x7d,0x28,0x4e,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vrsqrtph ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vrsqrtph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x7d,0xaf,0x4e,0x71,0x7f]
          vrsqrtph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vrsqrtph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x7d,0xbf,0x4e,0x72,0x80]
          vrsqrtph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vscalefph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x2c,0xf0]
          vscalefph ymm22, ymm23, ymm24

// CHECK: vscalefph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x2c,0xf0]
          vscalefph ymm22 {k7}, ymm23, ymm24

// CHECK: vscalefph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x2c,0xf0]
          vscalefph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vscalefph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x2c,0xf0]
          vscalefph xmm22, xmm23, xmm24

// CHECK: vscalefph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x2c,0xf0]
          vscalefph xmm22 {k7}, xmm23, xmm24

// CHECK: vscalefph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x2c,0xf0]
          vscalefph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vscalefph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x2c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vscalefph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vscalefph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x2c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vscalefph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vscalefph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x2c,0x35,0x00,0x00,0x00,0x00]
          vscalefph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vscalefph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x2c,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vscalefph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vscalefph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x2c,0x71,0x7f]
          vscalefph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vscalefph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x2c,0x72,0x80]
          vscalefph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vscalefph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x2c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vscalefph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vscalefph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x2c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vscalefph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vscalefph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x2c,0x35,0x00,0x00,0x00,0x00]
          vscalefph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vscalefph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x2c,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vscalefph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vscalefph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x2c,0x71,0x7f]
          vscalefph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vscalefph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x2c,0x72,0x80]
          vscalefph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vsqrtph xmm22, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x51,0xf7]
          vsqrtph xmm22, xmm23

// CHECK: vsqrtph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x51,0xf7]
          vsqrtph xmm22 {k7}, xmm23

// CHECK: vsqrtph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x51,0xf7]
          vsqrtph xmm22 {k7} {z}, xmm23

// CHECK: vsqrtph ymm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x51,0xf7]
          vsqrtph ymm22, ymm23

// CHECK: vsqrtph ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x51,0xf7]
          vsqrtph ymm22 {k7}, ymm23

// CHECK: vsqrtph ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x51,0xf7]
          vsqrtph ymm22 {k7} {z}, ymm23

// CHECK: vsqrtph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsqrtph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsqrtph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsqrtph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vsqrtph xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x51,0x35,0x00,0x00,0x00,0x00]
          vsqrtph xmm22, word ptr [rip]{1to8}

// CHECK: vsqrtph xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x51,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vsqrtph xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vsqrtph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x51,0x71,0x7f]
          vsqrtph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vsqrtph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x51,0x72,0x80]
          vsqrtph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vsqrtph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsqrtph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsqrtph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x2f,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsqrtph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vsqrtph ymm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x51,0x35,0x00,0x00,0x00,0x00]
          vsqrtph ymm22, word ptr [rip]{1to16}

// CHECK: vsqrtph ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x51,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vsqrtph ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vsqrtph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x51,0x71,0x7f]
          vsqrtph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vsqrtph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x51,0x72,0x80]
          vsqrtph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vfmadd132ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x98,0xf0]
          vfmadd132ph ymm22, ymm23, ymm24

// CHECK: vfmadd132ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x98,0xf0]
          vfmadd132ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfmadd132ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x98,0xf0]
          vfmadd132ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfmadd132ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x98,0xf0]
          vfmadd132ph xmm22, xmm23, xmm24

// CHECK: vfmadd132ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x98,0xf0]
          vfmadd132ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfmadd132ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x98,0xf0]
          vfmadd132ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfmadd132ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x98,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd132ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmadd132ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x98,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd132ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfmadd132ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x98,0x35,0x00,0x00,0x00,0x00]
          vfmadd132ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfmadd132ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x98,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmadd132ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfmadd132ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x98,0x71,0x7f]
          vfmadd132ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfmadd132ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x98,0x72,0x80]
          vfmadd132ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfmadd132ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x98,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd132ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmadd132ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x98,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd132ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmadd132ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x98,0x35,0x00,0x00,0x00,0x00]
          vfmadd132ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfmadd132ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x98,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmadd132ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfmadd132ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x98,0x71,0x7f]
          vfmadd132ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfmadd132ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x98,0x72,0x80]
          vfmadd132ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfmadd213ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xa8,0xf0]
          vfmadd213ph ymm22, ymm23, ymm24

// CHECK: vfmadd213ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xa8,0xf0]
          vfmadd213ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfmadd213ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xa8,0xf0]
          vfmadd213ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfmadd213ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xa8,0xf0]
          vfmadd213ph xmm22, xmm23, xmm24

// CHECK: vfmadd213ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xa8,0xf0]
          vfmadd213ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfmadd213ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xa8,0xf0]
          vfmadd213ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfmadd213ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xa8,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd213ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmadd213ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xa8,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd213ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfmadd213ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xa8,0x35,0x00,0x00,0x00,0x00]
          vfmadd213ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfmadd213ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xa8,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmadd213ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfmadd213ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xa8,0x71,0x7f]
          vfmadd213ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfmadd213ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xa8,0x72,0x80]
          vfmadd213ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfmadd213ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xa8,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd213ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmadd213ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xa8,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd213ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmadd213ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xa8,0x35,0x00,0x00,0x00,0x00]
          vfmadd213ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfmadd213ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xa8,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmadd213ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfmadd213ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xa8,0x71,0x7f]
          vfmadd213ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfmadd213ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xa8,0x72,0x80]
          vfmadd213ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfmadd231ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xb8,0xf0]
          vfmadd231ph ymm22, ymm23, ymm24

// CHECK: vfmadd231ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xb8,0xf0]
          vfmadd231ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfmadd231ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xb8,0xf0]
          vfmadd231ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfmadd231ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xb8,0xf0]
          vfmadd231ph xmm22, xmm23, xmm24

// CHECK: vfmadd231ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xb8,0xf0]
          vfmadd231ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfmadd231ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xb8,0xf0]
          vfmadd231ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfmadd231ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xb8,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd231ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmadd231ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xb8,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd231ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfmadd231ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xb8,0x35,0x00,0x00,0x00,0x00]
          vfmadd231ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfmadd231ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xb8,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmadd231ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfmadd231ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xb8,0x71,0x7f]
          vfmadd231ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfmadd231ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xb8,0x72,0x80]
          vfmadd231ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfmadd231ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xb8,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd231ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmadd231ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xb8,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd231ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmadd231ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xb8,0x35,0x00,0x00,0x00,0x00]
          vfmadd231ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfmadd231ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xb8,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmadd231ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfmadd231ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xb8,0x71,0x7f]
          vfmadd231ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfmadd231ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xb8,0x72,0x80]
          vfmadd231ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfmaddsub132ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x96,0xf0]
          vfmaddsub132ph ymm22, ymm23, ymm24

// CHECK: vfmaddsub132ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x96,0xf0]
          vfmaddsub132ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfmaddsub132ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x96,0xf0]
          vfmaddsub132ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfmaddsub132ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x96,0xf0]
          vfmaddsub132ph xmm22, xmm23, xmm24

// CHECK: vfmaddsub132ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x96,0xf0]
          vfmaddsub132ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfmaddsub132ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x96,0xf0]
          vfmaddsub132ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfmaddsub132ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x96,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub132ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmaddsub132ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x96,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub132ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfmaddsub132ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x96,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub132ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfmaddsub132ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x96,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmaddsub132ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfmaddsub132ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x96,0x71,0x7f]
          vfmaddsub132ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfmaddsub132ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x96,0x72,0x80]
          vfmaddsub132ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfmaddsub132ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x96,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub132ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmaddsub132ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x96,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub132ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmaddsub132ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x96,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub132ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfmaddsub132ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x96,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmaddsub132ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfmaddsub132ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x96,0x71,0x7f]
          vfmaddsub132ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfmaddsub132ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x96,0x72,0x80]
          vfmaddsub132ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfmaddsub213ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xa6,0xf0]
          vfmaddsub213ph ymm22, ymm23, ymm24

// CHECK: vfmaddsub213ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xa6,0xf0]
          vfmaddsub213ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfmaddsub213ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xa6,0xf0]
          vfmaddsub213ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfmaddsub213ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xa6,0xf0]
          vfmaddsub213ph xmm22, xmm23, xmm24

// CHECK: vfmaddsub213ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xa6,0xf0]
          vfmaddsub213ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfmaddsub213ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xa6,0xf0]
          vfmaddsub213ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfmaddsub213ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xa6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub213ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmaddsub213ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xa6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub213ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfmaddsub213ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xa6,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub213ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfmaddsub213ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xa6,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmaddsub213ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfmaddsub213ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xa6,0x71,0x7f]
          vfmaddsub213ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfmaddsub213ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xa6,0x72,0x80]
          vfmaddsub213ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfmaddsub213ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xa6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub213ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmaddsub213ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xa6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub213ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmaddsub213ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xa6,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub213ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfmaddsub213ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xa6,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmaddsub213ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfmaddsub213ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xa6,0x71,0x7f]
          vfmaddsub213ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfmaddsub213ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xa6,0x72,0x80]
          vfmaddsub213ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfmaddsub231ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xb6,0xf0]
          vfmaddsub231ph ymm22, ymm23, ymm24

// CHECK: vfmaddsub231ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xb6,0xf0]
          vfmaddsub231ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfmaddsub231ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xb6,0xf0]
          vfmaddsub231ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfmaddsub231ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xb6,0xf0]
          vfmaddsub231ph xmm22, xmm23, xmm24

// CHECK: vfmaddsub231ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xb6,0xf0]
          vfmaddsub231ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfmaddsub231ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xb6,0xf0]
          vfmaddsub231ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfmaddsub231ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xb6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub231ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmaddsub231ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xb6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub231ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfmaddsub231ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xb6,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub231ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfmaddsub231ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xb6,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmaddsub231ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfmaddsub231ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xb6,0x71,0x7f]
          vfmaddsub231ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfmaddsub231ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xb6,0x72,0x80]
          vfmaddsub231ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfmaddsub231ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xb6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub231ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmaddsub231ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xb6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub231ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmaddsub231ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xb6,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub231ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfmaddsub231ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xb6,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmaddsub231ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfmaddsub231ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xb6,0x71,0x7f]
          vfmaddsub231ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfmaddsub231ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xb6,0x72,0x80]
          vfmaddsub231ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfmsub132ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x9a,0xf0]
          vfmsub132ph ymm22, ymm23, ymm24

// CHECK: vfmsub132ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x9a,0xf0]
          vfmsub132ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfmsub132ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x9a,0xf0]
          vfmsub132ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfmsub132ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x9a,0xf0]
          vfmsub132ph xmm22, xmm23, xmm24

// CHECK: vfmsub132ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x9a,0xf0]
          vfmsub132ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfmsub132ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x9a,0xf0]
          vfmsub132ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfmsub132ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x9a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub132ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsub132ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x9a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub132ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsub132ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x9a,0x35,0x00,0x00,0x00,0x00]
          vfmsub132ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfmsub132ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x9a,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmsub132ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfmsub132ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x9a,0x71,0x7f]
          vfmsub132ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfmsub132ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x9a,0x72,0x80]
          vfmsub132ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfmsub132ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x9a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub132ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsub132ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x9a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub132ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsub132ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x9a,0x35,0x00,0x00,0x00,0x00]
          vfmsub132ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfmsub132ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x9a,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmsub132ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfmsub132ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x9a,0x71,0x7f]
          vfmsub132ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfmsub132ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x9a,0x72,0x80]
          vfmsub132ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfmsub213ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xaa,0xf0]
          vfmsub213ph ymm22, ymm23, ymm24

// CHECK: vfmsub213ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xaa,0xf0]
          vfmsub213ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfmsub213ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xaa,0xf0]
          vfmsub213ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfmsub213ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xaa,0xf0]
          vfmsub213ph xmm22, xmm23, xmm24

// CHECK: vfmsub213ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xaa,0xf0]
          vfmsub213ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfmsub213ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xaa,0xf0]
          vfmsub213ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfmsub213ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xaa,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub213ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsub213ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xaa,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub213ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsub213ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xaa,0x35,0x00,0x00,0x00,0x00]
          vfmsub213ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfmsub213ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xaa,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmsub213ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfmsub213ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xaa,0x71,0x7f]
          vfmsub213ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfmsub213ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xaa,0x72,0x80]
          vfmsub213ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfmsub213ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xaa,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub213ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsub213ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xaa,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub213ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsub213ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xaa,0x35,0x00,0x00,0x00,0x00]
          vfmsub213ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfmsub213ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xaa,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmsub213ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfmsub213ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xaa,0x71,0x7f]
          vfmsub213ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfmsub213ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xaa,0x72,0x80]
          vfmsub213ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfmsub231ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xba,0xf0]
          vfmsub231ph ymm22, ymm23, ymm24

// CHECK: vfmsub231ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xba,0xf0]
          vfmsub231ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfmsub231ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xba,0xf0]
          vfmsub231ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfmsub231ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xba,0xf0]
          vfmsub231ph xmm22, xmm23, xmm24

// CHECK: vfmsub231ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xba,0xf0]
          vfmsub231ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfmsub231ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xba,0xf0]
          vfmsub231ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfmsub231ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xba,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub231ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsub231ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xba,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub231ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsub231ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xba,0x35,0x00,0x00,0x00,0x00]
          vfmsub231ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfmsub231ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xba,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmsub231ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfmsub231ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xba,0x71,0x7f]
          vfmsub231ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfmsub231ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xba,0x72,0x80]
          vfmsub231ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfmsub231ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xba,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub231ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsub231ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xba,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub231ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsub231ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xba,0x35,0x00,0x00,0x00,0x00]
          vfmsub231ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfmsub231ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xba,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmsub231ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfmsub231ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xba,0x71,0x7f]
          vfmsub231ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfmsub231ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xba,0x72,0x80]
          vfmsub231ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfmsubadd132ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x97,0xf0]
          vfmsubadd132ph ymm22, ymm23, ymm24

// CHECK: vfmsubadd132ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x97,0xf0]
          vfmsubadd132ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfmsubadd132ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x97,0xf0]
          vfmsubadd132ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfmsubadd132ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x97,0xf0]
          vfmsubadd132ph xmm22, xmm23, xmm24

// CHECK: vfmsubadd132ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x97,0xf0]
          vfmsubadd132ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfmsubadd132ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x97,0xf0]
          vfmsubadd132ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfmsubadd132ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x97,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd132ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsubadd132ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x97,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd132ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsubadd132ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x97,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd132ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfmsubadd132ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x97,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmsubadd132ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfmsubadd132ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x97,0x71,0x7f]
          vfmsubadd132ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfmsubadd132ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x97,0x72,0x80]
          vfmsubadd132ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfmsubadd132ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x97,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd132ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsubadd132ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x97,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd132ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsubadd132ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x97,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd132ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfmsubadd132ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x97,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmsubadd132ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfmsubadd132ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x97,0x71,0x7f]
          vfmsubadd132ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfmsubadd132ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x97,0x72,0x80]
          vfmsubadd132ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfmsubadd213ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xa7,0xf0]
          vfmsubadd213ph ymm22, ymm23, ymm24

// CHECK: vfmsubadd213ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xa7,0xf0]
          vfmsubadd213ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfmsubadd213ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xa7,0xf0]
          vfmsubadd213ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfmsubadd213ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xa7,0xf0]
          vfmsubadd213ph xmm22, xmm23, xmm24

// CHECK: vfmsubadd213ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xa7,0xf0]
          vfmsubadd213ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfmsubadd213ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xa7,0xf0]
          vfmsubadd213ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfmsubadd213ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xa7,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd213ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsubadd213ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xa7,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd213ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsubadd213ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xa7,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd213ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfmsubadd213ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xa7,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmsubadd213ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfmsubadd213ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xa7,0x71,0x7f]
          vfmsubadd213ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfmsubadd213ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xa7,0x72,0x80]
          vfmsubadd213ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfmsubadd213ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xa7,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd213ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsubadd213ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xa7,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd213ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsubadd213ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xa7,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd213ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfmsubadd213ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xa7,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmsubadd213ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfmsubadd213ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xa7,0x71,0x7f]
          vfmsubadd213ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfmsubadd213ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xa7,0x72,0x80]
          vfmsubadd213ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfmsubadd231ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xb7,0xf0]
          vfmsubadd231ph ymm22, ymm23, ymm24

// CHECK: vfmsubadd231ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xb7,0xf0]
          vfmsubadd231ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfmsubadd231ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xb7,0xf0]
          vfmsubadd231ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfmsubadd231ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xb7,0xf0]
          vfmsubadd231ph xmm22, xmm23, xmm24

// CHECK: vfmsubadd231ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xb7,0xf0]
          vfmsubadd231ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfmsubadd231ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xb7,0xf0]
          vfmsubadd231ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfmsubadd231ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xb7,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd231ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsubadd231ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xb7,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd231ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsubadd231ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xb7,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd231ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfmsubadd231ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xb7,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmsubadd231ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfmsubadd231ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xb7,0x71,0x7f]
          vfmsubadd231ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfmsubadd231ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xb7,0x72,0x80]
          vfmsubadd231ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfmsubadd231ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xb7,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd231ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsubadd231ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xb7,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd231ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsubadd231ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xb7,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd231ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfmsubadd231ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xb7,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmsubadd231ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfmsubadd231ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xb7,0x71,0x7f]
          vfmsubadd231ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfmsubadd231ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xb7,0x72,0x80]
          vfmsubadd231ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfnmadd132ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x9c,0xf0]
          vfnmadd132ph ymm22, ymm23, ymm24

// CHECK: vfnmadd132ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x9c,0xf0]
          vfnmadd132ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfnmadd132ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x9c,0xf0]
          vfnmadd132ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfnmadd132ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x9c,0xf0]
          vfnmadd132ph xmm22, xmm23, xmm24

// CHECK: vfnmadd132ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x9c,0xf0]
          vfnmadd132ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfnmadd132ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x9c,0xf0]
          vfnmadd132ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfnmadd132ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x9c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd132ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmadd132ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x9c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd132ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmadd132ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x9c,0x35,0x00,0x00,0x00,0x00]
          vfnmadd132ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfnmadd132ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x9c,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfnmadd132ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfnmadd132ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x9c,0x71,0x7f]
          vfnmadd132ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfnmadd132ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x9c,0x72,0x80]
          vfnmadd132ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfnmadd132ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x9c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd132ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmadd132ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x9c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd132ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmadd132ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x9c,0x35,0x00,0x00,0x00,0x00]
          vfnmadd132ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfnmadd132ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x9c,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfnmadd132ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfnmadd132ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x9c,0x71,0x7f]
          vfnmadd132ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfnmadd132ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x9c,0x72,0x80]
          vfnmadd132ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfnmadd213ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xac,0xf0]
          vfnmadd213ph ymm22, ymm23, ymm24

// CHECK: vfnmadd213ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xac,0xf0]
          vfnmadd213ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfnmadd213ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xac,0xf0]
          vfnmadd213ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfnmadd213ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xac,0xf0]
          vfnmadd213ph xmm22, xmm23, xmm24

// CHECK: vfnmadd213ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xac,0xf0]
          vfnmadd213ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfnmadd213ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xac,0xf0]
          vfnmadd213ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfnmadd213ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xac,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd213ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmadd213ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xac,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd213ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmadd213ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xac,0x35,0x00,0x00,0x00,0x00]
          vfnmadd213ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfnmadd213ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xac,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfnmadd213ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfnmadd213ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xac,0x71,0x7f]
          vfnmadd213ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfnmadd213ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xac,0x72,0x80]
          vfnmadd213ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfnmadd213ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xac,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd213ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmadd213ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xac,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd213ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmadd213ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xac,0x35,0x00,0x00,0x00,0x00]
          vfnmadd213ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfnmadd213ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xac,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfnmadd213ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfnmadd213ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xac,0x71,0x7f]
          vfnmadd213ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfnmadd213ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xac,0x72,0x80]
          vfnmadd213ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfnmadd231ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xbc,0xf0]
          vfnmadd231ph ymm22, ymm23, ymm24

// CHECK: vfnmadd231ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xbc,0xf0]
          vfnmadd231ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfnmadd231ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xbc,0xf0]
          vfnmadd231ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfnmadd231ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xbc,0xf0]
          vfnmadd231ph xmm22, xmm23, xmm24

// CHECK: vfnmadd231ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xbc,0xf0]
          vfnmadd231ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfnmadd231ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xbc,0xf0]
          vfnmadd231ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfnmadd231ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xbc,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd231ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmadd231ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xbc,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd231ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmadd231ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xbc,0x35,0x00,0x00,0x00,0x00]
          vfnmadd231ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfnmadd231ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xbc,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfnmadd231ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfnmadd231ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xbc,0x71,0x7f]
          vfnmadd231ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfnmadd231ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xbc,0x72,0x80]
          vfnmadd231ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfnmadd231ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xbc,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd231ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmadd231ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xbc,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd231ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmadd231ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xbc,0x35,0x00,0x00,0x00,0x00]
          vfnmadd231ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfnmadd231ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xbc,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfnmadd231ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfnmadd231ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xbc,0x71,0x7f]
          vfnmadd231ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfnmadd231ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xbc,0x72,0x80]
          vfnmadd231ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfnmsub132ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x9e,0xf0]
          vfnmsub132ph ymm22, ymm23, ymm24

// CHECK: vfnmsub132ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x9e,0xf0]
          vfnmsub132ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfnmsub132ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x9e,0xf0]
          vfnmsub132ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfnmsub132ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x9e,0xf0]
          vfnmsub132ph xmm22, xmm23, xmm24

// CHECK: vfnmsub132ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x9e,0xf0]
          vfnmsub132ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfnmsub132ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x9e,0xf0]
          vfnmsub132ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfnmsub132ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x9e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub132ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmsub132ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x9e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub132ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmsub132ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x9e,0x35,0x00,0x00,0x00,0x00]
          vfnmsub132ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfnmsub132ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x9e,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfnmsub132ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfnmsub132ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x9e,0x71,0x7f]
          vfnmsub132ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfnmsub132ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x9e,0x72,0x80]
          vfnmsub132ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfnmsub132ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x9e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub132ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmsub132ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x9e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub132ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmsub132ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x9e,0x35,0x00,0x00,0x00,0x00]
          vfnmsub132ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfnmsub132ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x9e,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfnmsub132ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfnmsub132ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x9e,0x71,0x7f]
          vfnmsub132ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfnmsub132ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x9e,0x72,0x80]
          vfnmsub132ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfnmsub213ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xae,0xf0]
          vfnmsub213ph ymm22, ymm23, ymm24

// CHECK: vfnmsub213ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xae,0xf0]
          vfnmsub213ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfnmsub213ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xae,0xf0]
          vfnmsub213ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfnmsub213ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xae,0xf0]
          vfnmsub213ph xmm22, xmm23, xmm24

// CHECK: vfnmsub213ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xae,0xf0]
          vfnmsub213ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfnmsub213ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xae,0xf0]
          vfnmsub213ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfnmsub213ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xae,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub213ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmsub213ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xae,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub213ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmsub213ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xae,0x35,0x00,0x00,0x00,0x00]
          vfnmsub213ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfnmsub213ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xae,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfnmsub213ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfnmsub213ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xae,0x71,0x7f]
          vfnmsub213ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfnmsub213ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xae,0x72,0x80]
          vfnmsub213ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfnmsub213ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xae,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub213ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmsub213ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xae,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub213ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmsub213ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xae,0x35,0x00,0x00,0x00,0x00]
          vfnmsub213ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfnmsub213ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xae,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfnmsub213ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfnmsub213ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xae,0x71,0x7f]
          vfnmsub213ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfnmsub213ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xae,0x72,0x80]
          vfnmsub213ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vfnmsub231ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xbe,0xf0]
          vfnmsub231ph ymm22, ymm23, ymm24

// CHECK: vfnmsub231ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xbe,0xf0]
          vfnmsub231ph ymm22 {k7}, ymm23, ymm24

// CHECK: vfnmsub231ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xbe,0xf0]
          vfnmsub231ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vfnmsub231ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xbe,0xf0]
          vfnmsub231ph xmm22, xmm23, xmm24

// CHECK: vfnmsub231ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xbe,0xf0]
          vfnmsub231ph xmm22 {k7}, xmm23, xmm24

// CHECK: vfnmsub231ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xbe,0xf0]
          vfnmsub231ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vfnmsub231ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xbe,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub231ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmsub231ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xbe,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub231ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmsub231ph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xbe,0x35,0x00,0x00,0x00,0x00]
          vfnmsub231ph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vfnmsub231ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xbe,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfnmsub231ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vfnmsub231ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xbe,0x71,0x7f]
          vfnmsub231ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vfnmsub231ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xbe,0x72,0x80]
          vfnmsub231ph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vfnmsub231ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xbe,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub231ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmsub231ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xbe,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub231ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmsub231ph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xbe,0x35,0x00,0x00,0x00,0x00]
          vfnmsub231ph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vfnmsub231ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xbe,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfnmsub231ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vfnmsub231ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xbe,0x71,0x7f]
          vfnmsub231ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vfnmsub231ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xbe,0x72,0x80]
          vfnmsub231ph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

