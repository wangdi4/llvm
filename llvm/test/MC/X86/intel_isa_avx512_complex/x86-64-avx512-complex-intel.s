// REQUIRES: intel_feature_isa_avx512_complex
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vaddsubpd zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0xc5,0x40,0xd0,0xf0]
          vaddsubpd zmm22, zmm23, zmm24

// CHECK: vaddsubpd zmm22, zmm23, zmm24, {rn-sae}
// CHECK: encoding: [0x62,0x86,0xc5,0x10,0xd0,0xf0]
          vaddsubpd zmm22, zmm23, zmm24, {rn-sae}

// CHECK: vaddsubpd zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0xc5,0x47,0xd0,0xf0]
          vaddsubpd zmm22 {k7}, zmm23, zmm24

// CHECK: vaddsubpd zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}
// CHECK: encoding: [0x62,0x86,0xc5,0xf7,0xd0,0xf0]
          vaddsubpd zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}

// CHECK: vaddsubpd zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0xc5,0x40,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubpd zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vaddsubpd zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0xc5,0x47,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubpd zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vaddsubpd zmm22, zmm23, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0xc5,0x50,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubpd zmm22, zmm23, qword ptr [rip]{1to8}

// CHECK: vaddsubpd zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0xc5,0x40,0xd0,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vaddsubpd zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vaddsubpd zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0xc5,0xc7,0xd0,0x71,0x7f]
          vaddsubpd zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vaddsubpd zmm22 {k7} {z}, zmm23, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe6,0xc5,0xd7,0xd0,0x72,0x80]
          vaddsubpd zmm22 {k7} {z}, zmm23, qword ptr [rdx - 1024]{1to8}

// CHECK: vaddsubph zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0xd0,0xf0]
          vaddsubph zmm22, zmm23, zmm24

// CHECK: vaddsubph zmm22, zmm23, zmm24, {rn-sae}
// CHECK: encoding: [0x62,0x86,0x44,0x10,0xd0,0xf0]
          vaddsubph zmm22, zmm23, zmm24, {rn-sae}

// CHECK: vaddsubph zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0xd0,0xf0]
          vaddsubph zmm22 {k7}, zmm23, zmm24

// CHECK: vaddsubph zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}
// CHECK: encoding: [0x62,0x86,0x44,0xf7,0xd0,0xf0]
          vaddsubph zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}

// CHECK: vaddsubph zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubph zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vaddsubph zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubph zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vaddsubph zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubph zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vaddsubph zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0xd0,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vaddsubph zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vaddsubph zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0xd0,0x71,0x7f]
          vaddsubph zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vaddsubph zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0xd0,0x72,0x80]
          vaddsubph zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vaddsubps zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xd0,0xf0]
          vaddsubps zmm22, zmm23, zmm24

// CHECK: vaddsubps zmm22, zmm23, zmm24, {rn-sae}
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xd0,0xf0]
          vaddsubps zmm22, zmm23, zmm24, {rn-sae}

// CHECK: vaddsubps zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xd0,0xf0]
          vaddsubps zmm22 {k7}, zmm23, zmm24

// CHECK: vaddsubps zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xd0,0xf0]
          vaddsubps zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}

// CHECK: vaddsubps zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubps zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vaddsubps zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubps zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vaddsubps zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubps zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK: vaddsubps zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xd0,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vaddsubps zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vaddsubps zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xd0,0x71,0x7f]
          vaddsubps zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vaddsubps zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xd0,0x72,0x80]
          vaddsubps zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

// CHECK: vmovdhdup zmm22, zmm23
// CHECK: encoding: [0x62,0xa1,0xff,0x48,0x16,0xf7]
          vmovdhdup zmm22, zmm23

// CHECK: vmovdhdup zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa1,0xff,0x4f,0x16,0xf7]
          vmovdhdup zmm22 {k7}, zmm23

// CHECK: vmovdhdup zmm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa1,0xff,0xcf,0x16,0xf7]
          vmovdhdup zmm22 {k7} {z}, zmm23

// CHECK: vmovdhdup zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa1,0xff,0x48,0x16,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovdhdup zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovdhdup zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc1,0xff,0x4f,0x16,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovdhdup zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vmovdhdup zmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe1,0xff,0x48,0x16,0x35,0x00,0x00,0x00,0x00]
          vmovdhdup zmm22, zmmword ptr [rip]

// CHECK: vmovdhdup zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe1,0xff,0x48,0x16,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vmovdhdup zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vmovdhdup zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe1,0xff,0xcf,0x16,0x71,0x7f]
          vmovdhdup zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vmovdhdup zmm22 {k7} {z}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe1,0xff,0xcf,0x16,0x72,0x80]
          vmovdhdup zmm22 {k7} {z}, zmmword ptr [rdx - 8192]

// CHECK: vsubaddpd zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0xc5,0x40,0xd1,0xf0]
          vsubaddpd zmm22, zmm23, zmm24

// CHECK: vsubaddpd zmm22, zmm23, zmm24, {rn-sae}
// CHECK: encoding: [0x62,0x86,0xc5,0x10,0xd1,0xf0]
          vsubaddpd zmm22, zmm23, zmm24, {rn-sae}

// CHECK: vsubaddpd zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0xc5,0x47,0xd1,0xf0]
          vsubaddpd zmm22 {k7}, zmm23, zmm24

// CHECK: vsubaddpd zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}
// CHECK: encoding: [0x62,0x86,0xc5,0xf7,0xd1,0xf0]
          vsubaddpd zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}

// CHECK: vsubaddpd zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0xc5,0x40,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddpd zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubaddpd zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0xc5,0x47,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddpd zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vsubaddpd zmm22, zmm23, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe6,0xc5,0x50,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddpd zmm22, zmm23, qword ptr [rip]{1to8}

// CHECK: vsubaddpd zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0xc5,0x40,0xd1,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vsubaddpd zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vsubaddpd zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0xc5,0xc7,0xd1,0x71,0x7f]
          vsubaddpd zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vsubaddpd zmm22 {k7} {z}, zmm23, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe6,0xc5,0xd7,0xd1,0x72,0x80]
          vsubaddpd zmm22 {k7} {z}, zmm23, qword ptr [rdx - 1024]{1to8}

// CHECK: vsubaddph zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0xd1,0xf0]
          vsubaddph zmm22, zmm23, zmm24

// CHECK: vsubaddph zmm22, zmm23, zmm24, {rn-sae}
// CHECK: encoding: [0x62,0x86,0x44,0x10,0xd1,0xf0]
          vsubaddph zmm22, zmm23, zmm24, {rn-sae}

// CHECK: vsubaddph zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0xd1,0xf0]
          vsubaddph zmm22 {k7}, zmm23, zmm24

// CHECK: vsubaddph zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}
// CHECK: encoding: [0x62,0x86,0x44,0xf7,0xd1,0xf0]
          vsubaddph zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}

// CHECK: vsubaddph zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddph zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubaddph zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddph zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vsubaddph zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddph zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vsubaddph zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0xd1,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vsubaddph zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vsubaddph zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0xd1,0x71,0x7f]
          vsubaddph zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vsubaddph zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0xd1,0x72,0x80]
          vsubaddph zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vsubaddps zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xd1,0xf0]
          vsubaddps zmm22, zmm23, zmm24

// CHECK: vsubaddps zmm22, zmm23, zmm24, {rn-sae}
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xd1,0xf0]
          vsubaddps zmm22, zmm23, zmm24, {rn-sae}

// CHECK: vsubaddps zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xd1,0xf0]
          vsubaddps zmm22 {k7}, zmm23, zmm24

// CHECK: vsubaddps zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xd1,0xf0]
          vsubaddps zmm22 {k7} {z}, zmm23, zmm24, {rz-sae}

// CHECK: vsubaddps zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddps zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubaddps zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddps zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vsubaddps zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddps zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK: vsubaddps zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xd1,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vsubaddps zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vsubaddps zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xd1,0x71,0x7f]
          vsubaddps zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vsubaddps zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xd1,0x72,0x80]
          vsubaddps zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

