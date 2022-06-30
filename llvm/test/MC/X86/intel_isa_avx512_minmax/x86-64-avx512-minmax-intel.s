// REQUIRES: intel_feature_isa_avx512_minmax
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vminmaxnepbf16 zmm22, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x83,0x47,0x40,0x52,0xf0,0x7b]
          vminmaxnepbf16 zmm22, zmm23, zmm24, 123

// CHECK: vminmaxnepbf16 zmm22 {k7}, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x83,0x47,0x47,0x52,0xf0,0x7b]
          vminmaxnepbf16 zmm22 {k7}, zmm23, zmm24, 123

// CHECK: vminmaxnepbf16 zmm22 {k7} {z}, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x83,0x47,0xc7,0x52,0xf0,0x7b]
          vminmaxnepbf16 zmm22 {k7} {z}, zmm23, zmm24, 123

// CHECK: vminmaxnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x47,0x40,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vminmaxnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vminmaxnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x47,0x47,0x52,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vminmaxnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vminmaxnepbf16 zmm22, zmm23, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x47,0x50,0x52,0x35,0x00,0x00,0x00,0x00,0x7b]
          vminmaxnepbf16 zmm22, zmm23, word ptr [rip]{1to32}, 123

// CHECK: vminmaxnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x47,0x40,0x52,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048], 123

// CHECK: vminmaxnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x47,0xc7,0x52,0x71,0x7f,0x7b]
          vminmaxnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128], 123

// CHECK: vminmaxnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x47,0xd7,0x52,0x72,0x80,0x7b]
          vminmaxnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}, 123

// CHECK: vminmaxpd zmm22, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x83,0xc5,0x40,0x52,0xf0,0x7b]
          vminmaxpd zmm22, zmm23, zmm24, 123

// CHECK: vminmaxpd zmm22, zmm23, zmm24, {sae}, 123
// CHECK: encoding: [0x62,0x83,0xc5,0x10,0x52,0xf0,0x7b]
          vminmaxpd zmm22, zmm23, zmm24, {sae}, 123

// CHECK: vminmaxpd zmm22 {k7}, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x83,0xc5,0x47,0x52,0xf0,0x7b]
          vminmaxpd zmm22 {k7}, zmm23, zmm24, 123

// CHECK: vminmaxpd zmm22 {k7} {z}, zmm23, zmm24, {sae}, 123
// CHECK: encoding: [0x62,0x83,0xc5,0x97,0x52,0xf0,0x7b]
          vminmaxpd zmm22 {k7} {z}, zmm23, zmm24, {sae}, 123

// CHECK: vminmaxpd zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0xc5,0x40,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vminmaxpd zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vminmaxpd zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0xc5,0x47,0x52,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vminmaxpd zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vminmaxpd zmm22, zmm23, qword ptr [rip]{1to8}, 123
// CHECK: encoding: [0x62,0xe3,0xc5,0x50,0x52,0x35,0x00,0x00,0x00,0x00,0x7b]
          vminmaxpd zmm22, zmm23, qword ptr [rip]{1to8}, 123

// CHECK: vminmaxpd zmm22, zmm23, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0xc5,0x40,0x52,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxpd zmm22, zmm23, zmmword ptr [2*rbp - 2048], 123

// CHECK: vminmaxpd zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0xc5,0xc7,0x52,0x71,0x7f,0x7b]
          vminmaxpd zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128], 123

// CHECK: vminmaxpd zmm22 {k7} {z}, zmm23, qword ptr [rdx - 1024]{1to8}, 123
// CHECK: encoding: [0x62,0xe3,0xc5,0xd7,0x52,0x72,0x80,0x7b]
          vminmaxpd zmm22 {k7} {z}, zmm23, qword ptr [rdx - 1024]{1to8}, 123

// CHECK: vminmaxph zmm22, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x83,0x44,0x40,0x52,0xf0,0x7b]
          vminmaxph zmm22, zmm23, zmm24, 123

// CHECK: vminmaxph zmm22, zmm23, zmm24, {sae}, 123
// CHECK: encoding: [0x62,0x83,0x44,0x10,0x52,0xf0,0x7b]
          vminmaxph zmm22, zmm23, zmm24, {sae}, 123

// CHECK: vminmaxph zmm22 {k7}, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x83,0x44,0x47,0x52,0xf0,0x7b]
          vminmaxph zmm22 {k7}, zmm23, zmm24, 123

// CHECK: vminmaxph zmm22 {k7} {z}, zmm23, zmm24, {sae}, 123
// CHECK: encoding: [0x62,0x83,0x44,0x97,0x52,0xf0,0x7b]
          vminmaxph zmm22 {k7} {z}, zmm23, zmm24, {sae}, 123

// CHECK: vminmaxph zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x44,0x40,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vminmaxph zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vminmaxph zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x44,0x47,0x52,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vminmaxph zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vminmaxph zmm22, zmm23, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x44,0x50,0x52,0x35,0x00,0x00,0x00,0x00,0x7b]
          vminmaxph zmm22, zmm23, word ptr [rip]{1to32}, 123

// CHECK: vminmaxph zmm22, zmm23, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x44,0x40,0x52,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxph zmm22, zmm23, zmmword ptr [2*rbp - 2048], 123

// CHECK: vminmaxph zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x44,0xc7,0x52,0x71,0x7f,0x7b]
          vminmaxph zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128], 123

// CHECK: vminmaxph zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x44,0xd7,0x52,0x72,0x80,0x7b]
          vminmaxph zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}, 123

// CHECK: vminmaxps zmm22, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x83,0x45,0x40,0x52,0xf0,0x7b]
          vminmaxps zmm22, zmm23, zmm24, 123

// CHECK: vminmaxps zmm22, zmm23, zmm24, {sae}, 123
// CHECK: encoding: [0x62,0x83,0x45,0x10,0x52,0xf0,0x7b]
          vminmaxps zmm22, zmm23, zmm24, {sae}, 123

// CHECK: vminmaxps zmm22 {k7}, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x83,0x45,0x47,0x52,0xf0,0x7b]
          vminmaxps zmm22 {k7}, zmm23, zmm24, 123

// CHECK: vminmaxps zmm22 {k7} {z}, zmm23, zmm24, {sae}, 123
// CHECK: encoding: [0x62,0x83,0x45,0x97,0x52,0xf0,0x7b]
          vminmaxps zmm22 {k7} {z}, zmm23, zmm24, {sae}, 123

// CHECK: vminmaxps zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x45,0x40,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vminmaxps zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vminmaxps zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x45,0x47,0x52,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vminmaxps zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vminmaxps zmm22, zmm23, dword ptr [rip]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x45,0x50,0x52,0x35,0x00,0x00,0x00,0x00,0x7b]
          vminmaxps zmm22, zmm23, dword ptr [rip]{1to16}, 123

// CHECK: vminmaxps zmm22, zmm23, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x45,0x40,0x52,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxps zmm22, zmm23, zmmword ptr [2*rbp - 2048], 123

// CHECK: vminmaxps zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x45,0xc7,0x52,0x71,0x7f,0x7b]
          vminmaxps zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128], 123

// CHECK: vminmaxps zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x45,0xd7,0x52,0x72,0x80,0x7b]
          vminmaxps zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}, 123

