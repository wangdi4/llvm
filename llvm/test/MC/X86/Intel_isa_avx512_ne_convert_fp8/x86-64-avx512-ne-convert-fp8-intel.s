// REQUIRES: intel_feature_isa_avx512_ne_convert_fp8
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vcvtbias2ph2bf8 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x45,0x40,0x74,0xf0]
          vcvtbias2ph2bf8 zmm22, zmm23, zmm24

// CHECK: vcvtbias2ph2bf8 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x40,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbias2ph2bf8 zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x45,0x40,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8 zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbias2ph2bf8 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe2,0x45,0x50,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2bf8 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vcvtbias2ph2bf8 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x45,0x40,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2bf8 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtbias2ph2bf8 zmm22, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x45,0x40,0x74,0x71,0x7f]
          vcvtbias2ph2bf8 zmm22, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vcvtbias2ph2bf8 zmm22, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe2,0x45,0x50,0x74,0x72,0x80]
          vcvtbias2ph2bf8 zmm22, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vcvtbias2ph2bf8s zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x40,0x74,0xf0]
          vcvtbias2ph2bf8s zmm22, zmm23, zmm24

// CHECK: vcvtbias2ph2bf8s zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x40,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8s zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbias2ph2bf8s zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x40,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8s zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbias2ph2bf8s zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2bf8s zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vcvtbias2ph2bf8s zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2bf8s zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtbias2ph2bf8s zmm22, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x74,0x71,0x7f]
          vcvtbias2ph2bf8s zmm22, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vcvtbias2ph2bf8s zmm22, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x74,0x72,0x80]
          vcvtbias2ph2bf8s zmm22, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vcvtbias2ph2hf8 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x40,0x18,0xf0]
          vcvtbias2ph2hf8 zmm22, zmm23, zmm24

// CHECK: vcvtbias2ph2hf8 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x40,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbias2ph2hf8 zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x40,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8 zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbias2ph2hf8 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2hf8 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vcvtbias2ph2hf8 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x18,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2hf8 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtbias2ph2hf8 zmm22, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x18,0x71,0x7f]
          vcvtbias2ph2hf8 zmm22, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vcvtbias2ph2hf8 zmm22, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x18,0x72,0x80]
          vcvtbias2ph2hf8 zmm22, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vcvtbias2ph2hf8s zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x40,0x1b,0xf0]
          vcvtbias2ph2hf8s zmm22, zmm23, zmm24

// CHECK: vcvtbias2ph2hf8s zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x40,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8s zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbias2ph2hf8s zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x40,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8s zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbias2ph2hf8s zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2hf8s zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vcvtbias2ph2hf8s zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x1b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2hf8s zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtbias2ph2hf8s zmm22, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x1b,0x71,0x7f]
          vcvtbias2ph2hf8s zmm22, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vcvtbias2ph2hf8s zmm22, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x1b,0x72,0x80]
          vcvtbias2ph2hf8s zmm22, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vcvtbiasph2bf8 ymm22, zmm23
// CHECK: encoding: [0x62,0xa2,0x7c,0x48,0x74,0xf7]
          vcvtbiasph2bf8 ymm22, zmm23

// CHECK: vcvtbiasph2bf8 ymm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa2,0x7c,0x4f,0x74,0xf7]
          vcvtbiasph2bf8 ymm22 {k7}, zmm23

// CHECK: vcvtbiasph2bf8 ymm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa2,0x7c,0xcf,0x74,0xf7]
          vcvtbiasph2bf8 ymm22 {k7} {z}, zmm23

// CHECK: vcvtbiasph2bf8 ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7c,0x48,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8 ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbiasph2bf8 ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7c,0x4f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8 ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbiasph2bf8 ymm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe2,0x7c,0x58,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2bf8 ymm22, word ptr [rip]{1to32}

// CHECK: vcvtbiasph2bf8 ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x7c,0x48,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2bf8 ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtbiasph2bf8 ymm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x7c,0xcf,0x74,0x71,0x7f]
          vcvtbiasph2bf8 ymm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtbiasph2bf8 ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe2,0x7c,0xdf,0x74,0x72,0x80]
          vcvtbiasph2bf8 ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtbiasph2bf8s ymm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x74,0xf7]
          vcvtbiasph2bf8s ymm22, zmm23

// CHECK: vcvtbiasph2bf8s ymm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x74,0xf7]
          vcvtbiasph2bf8s ymm22 {k7}, zmm23

// CHECK: vcvtbiasph2bf8s ymm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xcf,0x74,0xf7]
          vcvtbiasph2bf8s ymm22 {k7} {z}, zmm23

// CHECK: vcvtbiasph2bf8s ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8s ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbiasph2bf8s ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8s ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbiasph2bf8s ymm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2bf8s ymm22, word ptr [rip]{1to32}

// CHECK: vcvtbiasph2bf8s ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2bf8s ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtbiasph2bf8s ymm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x74,0x71,0x7f]
          vcvtbiasph2bf8s ymm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtbiasph2bf8s ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x74,0x72,0x80]
          vcvtbiasph2bf8s ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtbiasph2hf8 ymm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x18,0xf7]
          vcvtbiasph2hf8 ymm22, zmm23

// CHECK: vcvtbiasph2hf8 ymm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x18,0xf7]
          vcvtbiasph2hf8 ymm22 {k7}, zmm23

// CHECK: vcvtbiasph2hf8 ymm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xcf,0x18,0xf7]
          vcvtbiasph2hf8 ymm22 {k7} {z}, zmm23

// CHECK: vcvtbiasph2hf8 ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8 ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbiasph2hf8 ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8 ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbiasph2hf8 ymm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2hf8 ymm22, word ptr [rip]{1to32}

// CHECK: vcvtbiasph2hf8 ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x18,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2hf8 ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtbiasph2hf8 ymm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x18,0x71,0x7f]
          vcvtbiasph2hf8 ymm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtbiasph2hf8 ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x18,0x72,0x80]
          vcvtbiasph2hf8 ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtbiasph2hf8s ymm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x1b,0xf7]
          vcvtbiasph2hf8s ymm22, zmm23

// CHECK: vcvtbiasph2hf8s ymm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x1b,0xf7]
          vcvtbiasph2hf8s ymm22 {k7}, zmm23

// CHECK: vcvtbiasph2hf8s ymm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0xcf,0x1b,0xf7]
          vcvtbiasph2hf8s ymm22 {k7} {z}, zmm23

// CHECK: vcvtbiasph2hf8s ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8s ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbiasph2hf8s ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8s ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbiasph2hf8s ymm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2hf8s ymm22, word ptr [rip]{1to32}

// CHECK: vcvtbiasph2hf8s ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x1b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2hf8s ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtbiasph2hf8s ymm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x1b,0x71,0x7f]
          vcvtbiasph2hf8s ymm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtbiasph2hf8s ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x1b,0x72,0x80]
          vcvtbiasph2hf8s ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtne2ph2bf8 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x47,0x40,0x74,0xf0]
          vcvtne2ph2bf8 zmm22, zmm23, zmm24

// CHECK: vcvtne2ph2bf8 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x47,0x40,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtne2ph2bf8 zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x47,0x40,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8 zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtne2ph2bf8 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe2,0x47,0x50,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2bf8 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vcvtne2ph2bf8 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x47,0x40,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2bf8 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtne2ph2bf8 zmm22, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x47,0x40,0x74,0x71,0x7f]
          vcvtne2ph2bf8 zmm22, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vcvtne2ph2bf8 zmm22, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe2,0x47,0x50,0x74,0x72,0x80]
          vcvtne2ph2bf8 zmm22, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vcvtne2ph2bf8s zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x74,0xf0]
          vcvtne2ph2bf8s zmm22, zmm23, zmm24

// CHECK: vcvtne2ph2bf8s zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8s zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtne2ph2bf8s zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x40,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8s zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtne2ph2bf8s zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2bf8s zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vcvtne2ph2bf8s zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2bf8s zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtne2ph2bf8s zmm22, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x74,0x71,0x7f]
          vcvtne2ph2bf8s zmm22, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vcvtne2ph2bf8s zmm22, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x74,0x72,0x80]
          vcvtne2ph2bf8s zmm22, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vcvtne2ph2hf8 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x18,0xf0]
          vcvtne2ph2hf8 zmm22, zmm23, zmm24

// CHECK: vcvtne2ph2hf8 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtne2ph2hf8 zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x40,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8 zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtne2ph2hf8 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2hf8 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vcvtne2ph2hf8 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x18,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2hf8 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtne2ph2hf8 zmm22, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x18,0x71,0x7f]
          vcvtne2ph2hf8 zmm22, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vcvtne2ph2hf8 zmm22, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x18,0x72,0x80]
          vcvtne2ph2hf8 zmm22, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vcvtne2ph2hf8s zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x1b,0xf0]
          vcvtne2ph2hf8s zmm22, zmm23, zmm24

// CHECK: vcvtne2ph2hf8s zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8s zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtne2ph2hf8s zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x40,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8s zmm22, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtne2ph2hf8s zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2hf8s zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vcvtne2ph2hf8s zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x1b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2hf8s zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtne2ph2hf8s zmm22, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x1b,0x71,0x7f]
          vcvtne2ph2hf8s zmm22, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vcvtne2ph2hf8s zmm22, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x1b,0x72,0x80]
          vcvtne2ph2hf8s zmm22, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vcvtnebf82ph zmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x1e,0xf7]
          vcvtnebf82ph zmm22, ymm23

// CHECK: vcvtnebf82ph zmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x4f,0x1e,0xf7]
          vcvtnebf82ph zmm22 {k7}, ymm23

// CHECK: vcvtnebf82ph zmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7e,0xcf,0x1e,0xf7]
          vcvtnebf82ph zmm22 {k7} {z}, ymm23

// CHECK: vcvtnebf82ph zmm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x1e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnebf82ph zmm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtnebf82ph zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x1e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnebf82ph zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtnebf82ph zmm22, ymmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x1e,0x35,0x00,0x00,0x00,0x00]
          vcvtnebf82ph zmm22, ymmword ptr [rip]

// CHECK: vcvtnebf82ph zmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x1e,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtnebf82ph zmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtnebf82ph zmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x1e,0x71,0x7f]
          vcvtnebf82ph zmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtnebf82ph zmm22 {k7} {z}, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x1e,0x72,0x80]
          vcvtnebf82ph zmm22 {k7} {z}, ymmword ptr [rdx - 4096]

// CHECK: vcvtnehf82ph zmm22, ymm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x1e,0xf7]
          vcvtnehf82ph zmm22, ymm23

// CHECK: vcvtnehf82ph zmm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x4f,0x1e,0xf7]
          vcvtnehf82ph zmm22 {k7}, ymm23

// CHECK: vcvtnehf82ph zmm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa5,0x7f,0xcf,0x1e,0xf7]
          vcvtnehf82ph zmm22 {k7} {z}, ymm23

// CHECK: vcvtnehf82ph zmm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x1e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnehf82ph zmm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtnehf82ph zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x4f,0x1e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnehf82ph zmm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtnehf82ph zmm22, ymmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x1e,0x35,0x00,0x00,0x00,0x00]
          vcvtnehf82ph zmm22, ymmword ptr [rip]

// CHECK: vcvtnehf82ph zmm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x1e,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtnehf82ph zmm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtnehf82ph zmm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x1e,0x71,0x7f]
          vcvtnehf82ph zmm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtnehf82ph zmm22 {k7} {z}, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x1e,0x72,0x80]
          vcvtnehf82ph zmm22 {k7} {z}, ymmword ptr [rdx - 4096]

// CHECK: vcvtneph2bf8 ymm22, zmm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0x74,0xf7]
          vcvtneph2bf8 ymm22, zmm23

// CHECK: vcvtneph2bf8 ymm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x4f,0x74,0xf7]
          vcvtneph2bf8 ymm22 {k7}, zmm23

// CHECK: vcvtneph2bf8 ymm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa2,0x7e,0xcf,0x74,0xf7]
          vcvtneph2bf8 ymm22 {k7} {z}, zmm23

// CHECK: vcvtneph2bf8 ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2bf8 ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtneph2bf8 ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x4f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2bf8 ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtneph2bf8 ymm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe2,0x7e,0x58,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2bf8 ymm22, word ptr [rip]{1to32}

// CHECK: vcvtneph2bf8 ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2bf8 ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtneph2bf8 ymm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x7e,0xcf,0x74,0x71,0x7f]
          vcvtneph2bf8 ymm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtneph2bf8 ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe2,0x7e,0xdf,0x74,0x72,0x80]
          vcvtneph2bf8 ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtneph2bf8s ymm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x74,0xf7]
          vcvtneph2bf8s ymm22, zmm23

// CHECK: vcvtneph2bf8s ymm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x4f,0x74,0xf7]
          vcvtneph2bf8s ymm22 {k7}, zmm23

// CHECK: vcvtneph2bf8s ymm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0xcf,0x74,0xf7]
          vcvtneph2bf8s ymm22 {k7} {z}, zmm23

// CHECK: vcvtneph2bf8s ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2bf8s ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtneph2bf8s ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2bf8s ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtneph2bf8s ymm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7e,0x58,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2bf8s ymm22, word ptr [rip]{1to32}

// CHECK: vcvtneph2bf8s ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2bf8s ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtneph2bf8s ymm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x74,0x71,0x7f]
          vcvtneph2bf8s ymm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtneph2bf8s ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7e,0xdf,0x74,0x72,0x80]
          vcvtneph2bf8s ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtneph2hf8 ymm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x18,0xf7]
          vcvtneph2hf8 ymm22, zmm23

// CHECK: vcvtneph2hf8 ymm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x4f,0x18,0xf7]
          vcvtneph2hf8 ymm22 {k7}, zmm23

// CHECK: vcvtneph2hf8 ymm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0xcf,0x18,0xf7]
          vcvtneph2hf8 ymm22 {k7} {z}, zmm23

// CHECK: vcvtneph2hf8 ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2hf8 ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtneph2hf8 ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2hf8 ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtneph2hf8 ymm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7e,0x58,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2hf8 ymm22, word ptr [rip]{1to32}

// CHECK: vcvtneph2hf8 ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x18,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2hf8 ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtneph2hf8 ymm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x18,0x71,0x7f]
          vcvtneph2hf8 ymm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtneph2hf8 ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7e,0xdf,0x18,0x72,0x80]
          vcvtneph2hf8 ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtneph2hf8s ymm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x1b,0xf7]
          vcvtneph2hf8s ymm22, zmm23

// CHECK: vcvtneph2hf8s ymm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0x4f,0x1b,0xf7]
          vcvtneph2hf8s ymm22 {k7}, zmm23

// CHECK: vcvtneph2hf8s ymm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7e,0xcf,0x1b,0xf7]
          vcvtneph2hf8s ymm22 {k7} {z}, zmm23

// CHECK: vcvtneph2hf8s ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2hf8s ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtneph2hf8s ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2hf8s ymm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtneph2hf8s ymm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7e,0x58,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2hf8s ymm22, word ptr [rip]{1to32}

// CHECK: vcvtneph2hf8s ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x1b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2hf8s ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtneph2hf8s ymm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x1b,0x71,0x7f]
          vcvtneph2hf8s ymm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtneph2hf8s ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7e,0xdf,0x1b,0x72,0x80]
          vcvtneph2hf8s ymm22 {k7} {z}, word ptr [rdx - 256]{1to32}

