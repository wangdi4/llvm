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

