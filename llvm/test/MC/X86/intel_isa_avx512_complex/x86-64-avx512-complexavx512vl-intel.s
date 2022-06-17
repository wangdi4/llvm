// REQUIRES: intel_feature_isa_avx512_complex
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vaddsubpd ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0xc5,0x20,0xd0,0xf0]
          vaddsubpd ymm22, ymm23, ymm24

// CHECK: vaddsubpd ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0xc5,0x27,0xd0,0xf0]
          vaddsubpd ymm22 {k7}, ymm23, ymm24

// CHECK: vaddsubpd ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0xc5,0xa7,0xd0,0xf0]
          vaddsubpd ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vaddsubpd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0xc5,0x00,0xd0,0xf0]
          vaddsubpd xmm22, xmm23, xmm24

// CHECK: vaddsubpd xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0xc5,0x07,0xd0,0xf0]
          vaddsubpd xmm22 {k7}, xmm23, xmm24

// CHECK: vaddsubpd xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0xc5,0x87,0xd0,0xf0]
          vaddsubpd xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vaddsubpd ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0xc5,0x20,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubpd ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vaddsubpd ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0xc5,0x27,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubpd ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vaddsubpd ymm22, ymm23, qword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe6,0xc5,0x30,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubpd ymm22, ymm23, qword ptr [rip]{1to4}

// CHECK: vaddsubpd ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0xc5,0x20,0xd0,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vaddsubpd ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vaddsubpd ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0xc5,0xa7,0xd0,0x71,0x7f]
          vaddsubpd ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vaddsubpd ymm22 {k7} {z}, ymm23, qword ptr [rdx - 1024]{1to4}
// CHECK: encoding: [0x62,0xe6,0xc5,0xb7,0xd0,0x72,0x80]
          vaddsubpd ymm22 {k7} {z}, ymm23, qword ptr [rdx - 1024]{1to4}

// CHECK: vaddsubpd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0xc5,0x00,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubpd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vaddsubpd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0xc5,0x07,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubpd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vaddsubpd xmm22, xmm23, qword ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe6,0xc5,0x10,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubpd xmm22, xmm23, qword ptr [rip]{1to2}

// CHECK: vaddsubpd xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0xc5,0x00,0xd0,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vaddsubpd xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vaddsubpd xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0xc5,0x87,0xd0,0x71,0x7f]
          vaddsubpd xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vaddsubpd xmm22 {k7} {z}, xmm23, qword ptr [rdx - 1024]{1to2}
// CHECK: encoding: [0x62,0xe6,0xc5,0x97,0xd0,0x72,0x80]
          vaddsubpd xmm22 {k7} {z}, xmm23, qword ptr [rdx - 1024]{1to2}

// CHECK: vaddsubph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x44,0x20,0xd0,0xf0]
          vaddsubph ymm22, ymm23, ymm24

// CHECK: vaddsubph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x44,0x27,0xd0,0xf0]
          vaddsubph ymm22 {k7}, ymm23, ymm24

// CHECK: vaddsubph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x44,0xa7,0xd0,0xf0]
          vaddsubph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vaddsubph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x44,0x00,0xd0,0xf0]
          vaddsubph xmm22, xmm23, xmm24

// CHECK: vaddsubph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x44,0x07,0xd0,0xf0]
          vaddsubph xmm22 {k7}, xmm23, xmm24

// CHECK: vaddsubph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x44,0x87,0xd0,0xf0]
          vaddsubph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vaddsubph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x20,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vaddsubph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x27,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vaddsubph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x44,0x30,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vaddsubph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x44,0x20,0xd0,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vaddsubph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vaddsubph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x44,0xa7,0xd0,0x71,0x7f]
          vaddsubph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vaddsubph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x44,0xb7,0xd0,0x72,0x80]
          vaddsubph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vaddsubph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x00,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vaddsubph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x07,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vaddsubph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x44,0x10,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vaddsubph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x44,0x00,0xd0,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vaddsubph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vaddsubph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x44,0x87,0xd0,0x71,0x7f]
          vaddsubph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vaddsubph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x44,0x97,0xd0,0x72,0x80]
          vaddsubph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vaddsubps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xd0,0xf0]
          vaddsubps ymm22, ymm23, ymm24

// CHECK: vaddsubps ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xd0,0xf0]
          vaddsubps ymm22 {k7}, ymm23, ymm24

// CHECK: vaddsubps ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xd0,0xf0]
          vaddsubps ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vaddsubps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xd0,0xf0]
          vaddsubps xmm22, xmm23, xmm24

// CHECK: vaddsubps xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xd0,0xf0]
          vaddsubps xmm22 {k7}, xmm23, xmm24

// CHECK: vaddsubps xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xd0,0xf0]
          vaddsubps xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vaddsubps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vaddsubps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vaddsubps ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubps ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vaddsubps ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xd0,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vaddsubps ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vaddsubps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xd0,0x71,0x7f]
          vaddsubps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vaddsubps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xd0,0x72,0x80]
          vaddsubps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vaddsubps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vaddsubps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vaddsubps xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubps xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vaddsubps xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xd0,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vaddsubps xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vaddsubps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xd0,0x71,0x7f]
          vaddsubps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vaddsubps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xd0,0x72,0x80]
          vaddsubps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vmovdhdup xmm22, xmm23
// CHECK: encoding: [0x62,0xa1,0xff,0x08,0x16,0xf7]
          vmovdhdup xmm22, xmm23

// CHECK: vmovdhdup xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa1,0xff,0x0f,0x16,0xf7]
          vmovdhdup xmm22 {k7}, xmm23

// CHECK: vmovdhdup xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa1,0xff,0x8f,0x16,0xf7]
          vmovdhdup xmm22 {k7} {z}, xmm23

// CHECK: vmovdhdup ymm22, ymm23
// CHECK: encoding: [0x62,0xa1,0xff,0x28,0x16,0xf7]
          vmovdhdup ymm22, ymm23

// CHECK: vmovdhdup ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa1,0xff,0x2f,0x16,0xf7]
          vmovdhdup ymm22 {k7}, ymm23

// CHECK: vmovdhdup ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa1,0xff,0xaf,0x16,0xf7]
          vmovdhdup ymm22 {k7} {z}, ymm23

// CHECK: vmovdhdup xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa1,0xff,0x08,0x16,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovdhdup xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovdhdup xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc1,0xff,0x0f,0x16,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovdhdup xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vmovdhdup xmm22, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe1,0xff,0x08,0x16,0x35,0x00,0x00,0x00,0x00]
          vmovdhdup xmm22, xmmword ptr [rip]

// CHECK: vmovdhdup xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe1,0xff,0x08,0x16,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vmovdhdup xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vmovdhdup xmm22 {k7} {z}, xmmword ptr [rcx + 1016]
// CHECK: encoding: [0x62,0xe1,0xff,0x8f,0x16,0x71,0x7f]
          vmovdhdup xmm22 {k7} {z}, xmmword ptr [rcx + 1016]

// CHECK: vmovdhdup xmm22 {k7} {z}, xmmword ptr [rdx - 1024]
// CHECK: encoding: [0x62,0xe1,0xff,0x8f,0x16,0x72,0x80]
          vmovdhdup xmm22 {k7} {z}, xmmword ptr [rdx - 1024]

// CHECK: vmovdhdup ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa1,0xff,0x28,0x16,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovdhdup ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovdhdup ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc1,0xff,0x2f,0x16,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovdhdup ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vmovdhdup ymm22, ymmword ptr [rip]
// CHECK: encoding: [0x62,0xe1,0xff,0x28,0x16,0x35,0x00,0x00,0x00,0x00]
          vmovdhdup ymm22, ymmword ptr [rip]

// CHECK: vmovdhdup ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe1,0xff,0x28,0x16,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vmovdhdup ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vmovdhdup ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe1,0xff,0xaf,0x16,0x71,0x7f]
          vmovdhdup ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vmovdhdup ymm22 {k7} {z}, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe1,0xff,0xaf,0x16,0x72,0x80]
          vmovdhdup ymm22 {k7} {z}, ymmword ptr [rdx - 4096]

// CHECK: vsubaddpd ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0xc5,0x20,0xd1,0xf0]
          vsubaddpd ymm22, ymm23, ymm24

// CHECK: vsubaddpd ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0xc5,0x27,0xd1,0xf0]
          vsubaddpd ymm22 {k7}, ymm23, ymm24

// CHECK: vsubaddpd ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0xc5,0xa7,0xd1,0xf0]
          vsubaddpd ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vsubaddpd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0xc5,0x00,0xd1,0xf0]
          vsubaddpd xmm22, xmm23, xmm24

// CHECK: vsubaddpd xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0xc5,0x07,0xd1,0xf0]
          vsubaddpd xmm22 {k7}, xmm23, xmm24

// CHECK: vsubaddpd xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0xc5,0x87,0xd1,0xf0]
          vsubaddpd xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vsubaddpd ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0xc5,0x20,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddpd ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubaddpd ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0xc5,0x27,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddpd ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vsubaddpd ymm22, ymm23, qword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe6,0xc5,0x30,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddpd ymm22, ymm23, qword ptr [rip]{1to4}

// CHECK: vsubaddpd ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0xc5,0x20,0xd1,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vsubaddpd ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vsubaddpd ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0xc5,0xa7,0xd1,0x71,0x7f]
          vsubaddpd ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vsubaddpd ymm22 {k7} {z}, ymm23, qword ptr [rdx - 1024]{1to4}
// CHECK: encoding: [0x62,0xe6,0xc5,0xb7,0xd1,0x72,0x80]
          vsubaddpd ymm22 {k7} {z}, ymm23, qword ptr [rdx - 1024]{1to4}

// CHECK: vsubaddpd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0xc5,0x00,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddpd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubaddpd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0xc5,0x07,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddpd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vsubaddpd xmm22, xmm23, qword ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe6,0xc5,0x10,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddpd xmm22, xmm23, qword ptr [rip]{1to2}

// CHECK: vsubaddpd xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0xc5,0x00,0xd1,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vsubaddpd xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vsubaddpd xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0xc5,0x87,0xd1,0x71,0x7f]
          vsubaddpd xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vsubaddpd xmm22 {k7} {z}, xmm23, qword ptr [rdx - 1024]{1to2}
// CHECK: encoding: [0x62,0xe6,0xc5,0x97,0xd1,0x72,0x80]
          vsubaddpd xmm22 {k7} {z}, xmm23, qword ptr [rdx - 1024]{1to2}

// CHECK: vsubaddph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x44,0x20,0xd1,0xf0]
          vsubaddph ymm22, ymm23, ymm24

// CHECK: vsubaddph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x44,0x27,0xd1,0xf0]
          vsubaddph ymm22 {k7}, ymm23, ymm24

// CHECK: vsubaddph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x44,0xa7,0xd1,0xf0]
          vsubaddph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vsubaddph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x44,0x00,0xd1,0xf0]
          vsubaddph xmm22, xmm23, xmm24

// CHECK: vsubaddph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x44,0x07,0xd1,0xf0]
          vsubaddph xmm22 {k7}, xmm23, xmm24

// CHECK: vsubaddph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x44,0x87,0xd1,0xf0]
          vsubaddph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vsubaddph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x20,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubaddph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x27,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vsubaddph ymm22, ymm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x44,0x30,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddph ymm22, ymm23, word ptr [rip]{1to16}

// CHECK: vsubaddph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x44,0x20,0xd1,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vsubaddph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vsubaddph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x44,0xa7,0xd1,0x71,0x7f]
          vsubaddph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vsubaddph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe6,0x44,0xb7,0xd1,0x72,0x80]
          vsubaddph ymm22 {k7} {z}, ymm23, word ptr [rdx - 256]{1to16}

// CHECK: vsubaddph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x00,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubaddph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x07,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vsubaddph xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x44,0x10,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddph xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vsubaddph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x44,0x00,0xd1,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vsubaddph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vsubaddph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x44,0x87,0xd1,0x71,0x7f]
          vsubaddph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vsubaddph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe6,0x44,0x97,0xd1,0x72,0x80]
          vsubaddph xmm22 {k7} {z}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vsubaddps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xd1,0xf0]
          vsubaddps ymm22, ymm23, ymm24

// CHECK: vsubaddps ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xd1,0xf0]
          vsubaddps ymm22 {k7}, ymm23, ymm24

// CHECK: vsubaddps ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xd1,0xf0]
          vsubaddps ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vsubaddps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xd1,0xf0]
          vsubaddps xmm22, xmm23, xmm24

// CHECK: vsubaddps xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xd1,0xf0]
          vsubaddps xmm22 {k7}, xmm23, xmm24

// CHECK: vsubaddps xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xd1,0xf0]
          vsubaddps xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vsubaddps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubaddps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vsubaddps ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddps ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vsubaddps ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xd1,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vsubaddps ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vsubaddps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xd1,0x71,0x7f]
          vsubaddps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vsubaddps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xd1,0x72,0x80]
          vsubaddps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vsubaddps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubaddps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vsubaddps xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddps xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vsubaddps xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xd1,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vsubaddps xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vsubaddps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xd1,0x71,0x7f]
          vsubaddps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vsubaddps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xd1,0x72,0x80]
          vsubaddps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

