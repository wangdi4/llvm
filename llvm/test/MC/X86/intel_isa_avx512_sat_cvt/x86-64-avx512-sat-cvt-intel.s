// REQUIRES: intel_feature_isa_avx512_sat_cvt
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vcvtnebf162ibs zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x69,0xf7,0x7b]
          vcvtnebf162ibs zmm22, zmm23, 123

// CHECK: vcvtnebf162ibs zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x4f,0x69,0xf7,0x7b]
          vcvtnebf162ibs zmm22 {k7}, zmm23, 123

// CHECK: vcvtnebf162ibs zmm22 {k7} {z}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0xcf,0x69,0xf7,0x7b]
          vcvtnebf162ibs zmm22 {k7} {z}, zmm23, 123

// CHECK: vcvtnebf162ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x69,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcvtnebf162ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcvtnebf162ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7f,0x4f,0x69,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcvtnebf162ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcvtnebf162ibs zmm22, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x58,0x69,0x35,0x00,0x00,0x00,0x00,0x7b]
          vcvtnebf162ibs zmm22, word ptr [rip]{1to32}, 123

// CHECK: vcvtnebf162ibs zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x69,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtnebf162ibs zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcvtnebf162ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xcf,0x69,0x71,0x7f,0x7b]
          vcvtnebf162ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vcvtnebf162ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xdf,0x69,0x72,0x80,0x7b]
          vcvtnebf162ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123

// CHECK: vcvtnebf162iubs zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x6b,0xf7,0x7b]
          vcvtnebf162iubs zmm22, zmm23, 123

// CHECK: vcvtnebf162iubs zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x4f,0x6b,0xf7,0x7b]
          vcvtnebf162iubs zmm22 {k7}, zmm23, 123

// CHECK: vcvtnebf162iubs zmm22 {k7} {z}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0xcf,0x6b,0xf7,0x7b]
          vcvtnebf162iubs zmm22 {k7} {z}, zmm23, 123

// CHECK: vcvtnebf162iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x6b,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcvtnebf162iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcvtnebf162iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7f,0x4f,0x6b,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcvtnebf162iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcvtnebf162iubs zmm22, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x58,0x6b,0x35,0x00,0x00,0x00,0x00,0x7b]
          vcvtnebf162iubs zmm22, word ptr [rip]{1to32}, 123

// CHECK: vcvtnebf162iubs zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x6b,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtnebf162iubs zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcvtnebf162iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xcf,0x6b,0x71,0x7f,0x7b]
          vcvtnebf162iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vcvtnebf162iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xdf,0x6b,0x72,0x80,0x7b]
          vcvtnebf162iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123

// CHECK: vcvtph2ibs zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x69,0xf7,0x7b]
          vcvtph2ibs zmm22, zmm23, 123

// CHECK: vcvtph2ibs zmm22, zmm23, {rn-sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x18,0x69,0xf7,0x7b]
          vcvtph2ibs zmm22, zmm23, {rn-sae}, 123

// CHECK: vcvtph2ibs zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x4f,0x69,0xf7,0x7b]
          vcvtph2ibs zmm22 {k7}, zmm23, 123

// CHECK: vcvtph2ibs zmm22 {k7} {z}, zmm23, {rz-sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0xff,0x69,0xf7,0x7b]
          vcvtph2ibs zmm22 {k7} {z}, zmm23, {rz-sae}, 123

// CHECK: vcvtph2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x69,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcvtph2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcvtph2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7c,0x4f,0x69,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcvtph2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcvtph2ibs zmm22, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x58,0x69,0x35,0x00,0x00,0x00,0x00,0x7b]
          vcvtph2ibs zmm22, word ptr [rip]{1to32}, 123

// CHECK: vcvtph2ibs zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x48,0x69,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtph2ibs zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcvtph2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xcf,0x69,0x71,0x7f,0x7b]
          vcvtph2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vcvtph2ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xdf,0x69,0x72,0x80,0x7b]
          vcvtph2ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123

// CHECK: vcvtph2iubs zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x6b,0xf7,0x7b]
          vcvtph2iubs zmm22, zmm23, 123

// CHECK: vcvtph2iubs zmm22, zmm23, {rn-sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x18,0x6b,0xf7,0x7b]
          vcvtph2iubs zmm22, zmm23, {rn-sae}, 123

// CHECK: vcvtph2iubs zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x4f,0x6b,0xf7,0x7b]
          vcvtph2iubs zmm22 {k7}, zmm23, 123

// CHECK: vcvtph2iubs zmm22 {k7} {z}, zmm23, {rz-sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0xff,0x6b,0xf7,0x7b]
          vcvtph2iubs zmm22 {k7} {z}, zmm23, {rz-sae}, 123

// CHECK: vcvtph2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x6b,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcvtph2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcvtph2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7c,0x4f,0x6b,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcvtph2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcvtph2iubs zmm22, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x58,0x6b,0x35,0x00,0x00,0x00,0x00,0x7b]
          vcvtph2iubs zmm22, word ptr [rip]{1to32}, 123

// CHECK: vcvtph2iubs zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x48,0x6b,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtph2iubs zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcvtph2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xcf,0x6b,0x71,0x7f,0x7b]
          vcvtph2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vcvtph2iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xdf,0x6b,0x72,0x80,0x7b]
          vcvtph2iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123

// CHECK: vcvtps2ibs zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x48,0x69,0xf7,0x7b]
          vcvtps2ibs zmm22, zmm23, 123

// CHECK: vcvtps2ibs zmm22, zmm23, {rn-sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x18,0x69,0xf7,0x7b]
          vcvtps2ibs zmm22, zmm23, {rn-sae}, 123

// CHECK: vcvtps2ibs zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x4f,0x69,0xf7,0x7b]
          vcvtps2ibs zmm22 {k7}, zmm23, 123

// CHECK: vcvtps2ibs zmm22 {k7} {z}, zmm23, {rz-sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0xff,0x69,0xf7,0x7b]
          vcvtps2ibs zmm22 {k7} {z}, zmm23, {rz-sae}, 123

// CHECK: vcvtps2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x48,0x69,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcvtps2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcvtps2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7d,0x4f,0x69,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcvtps2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcvtps2ibs zmm22, dword ptr [rip]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7d,0x58,0x69,0x35,0x00,0x00,0x00,0x00,0x7b]
          vcvtps2ibs zmm22, dword ptr [rip]{1to16}, 123

// CHECK: vcvtps2ibs zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7d,0x48,0x69,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtps2ibs zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcvtps2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7d,0xcf,0x69,0x71,0x7f,0x7b]
          vcvtps2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vcvtps2ibs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7d,0xdf,0x69,0x72,0x80,0x7b]
          vcvtps2ibs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}, 123

// CHECK: vcvtps2iubs zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x48,0x6b,0xf7,0x7b]
          vcvtps2iubs zmm22, zmm23, 123

// CHECK: vcvtps2iubs zmm22, zmm23, {rn-sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x18,0x6b,0xf7,0x7b]
          vcvtps2iubs zmm22, zmm23, {rn-sae}, 123

// CHECK: vcvtps2iubs zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x4f,0x6b,0xf7,0x7b]
          vcvtps2iubs zmm22 {k7}, zmm23, 123

// CHECK: vcvtps2iubs zmm22 {k7} {z}, zmm23, {rz-sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0xff,0x6b,0xf7,0x7b]
          vcvtps2iubs zmm22 {k7} {z}, zmm23, {rz-sae}, 123

// CHECK: vcvtps2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x48,0x6b,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcvtps2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcvtps2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7d,0x4f,0x6b,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcvtps2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcvtps2iubs zmm22, dword ptr [rip]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7d,0x58,0x6b,0x35,0x00,0x00,0x00,0x00,0x7b]
          vcvtps2iubs zmm22, dword ptr [rip]{1to16}, 123

// CHECK: vcvtps2iubs zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7d,0x48,0x6b,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtps2iubs zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcvtps2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7d,0xcf,0x6b,0x71,0x7f,0x7b]
          vcvtps2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vcvtps2iubs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7d,0xdf,0x6b,0x72,0x80,0x7b]
          vcvtps2iubs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}, 123

// CHECK: vcvttnebf162ibs zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x68,0xf7,0x7b]
          vcvttnebf162ibs zmm22, zmm23, 123

// CHECK: vcvttnebf162ibs zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x4f,0x68,0xf7,0x7b]
          vcvttnebf162ibs zmm22 {k7}, zmm23, 123

// CHECK: vcvttnebf162ibs zmm22 {k7} {z}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0xcf,0x68,0xf7,0x7b]
          vcvttnebf162ibs zmm22 {k7} {z}, zmm23, 123

// CHECK: vcvttnebf162ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x68,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcvttnebf162ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcvttnebf162ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7f,0x4f,0x68,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcvttnebf162ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcvttnebf162ibs zmm22, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x58,0x68,0x35,0x00,0x00,0x00,0x00,0x7b]
          vcvttnebf162ibs zmm22, word ptr [rip]{1to32}, 123

// CHECK: vcvttnebf162ibs zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x68,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttnebf162ibs zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcvttnebf162ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xcf,0x68,0x71,0x7f,0x7b]
          vcvttnebf162ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vcvttnebf162ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xdf,0x68,0x72,0x80,0x7b]
          vcvttnebf162ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123

// CHECK: vcvttnebf162iubs zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x6a,0xf7,0x7b]
          vcvttnebf162iubs zmm22, zmm23, 123

// CHECK: vcvttnebf162iubs zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x4f,0x6a,0xf7,0x7b]
          vcvttnebf162iubs zmm22 {k7}, zmm23, 123

// CHECK: vcvttnebf162iubs zmm22 {k7} {z}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0xcf,0x6a,0xf7,0x7b]
          vcvttnebf162iubs zmm22 {k7} {z}, zmm23, 123

// CHECK: vcvttnebf162iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x6a,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcvttnebf162iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcvttnebf162iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7f,0x4f,0x6a,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcvttnebf162iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcvttnebf162iubs zmm22, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x58,0x6a,0x35,0x00,0x00,0x00,0x00,0x7b]
          vcvttnebf162iubs zmm22, word ptr [rip]{1to32}, 123

// CHECK: vcvttnebf162iubs zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x6a,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttnebf162iubs zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcvttnebf162iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xcf,0x6a,0x71,0x7f,0x7b]
          vcvttnebf162iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vcvttnebf162iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xdf,0x6a,0x72,0x80,0x7b]
          vcvttnebf162iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123

// CHECK: vcvttph2ibs zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x68,0xf7,0x7b]
          vcvttph2ibs zmm22, zmm23, 123

// CHECK: vcvttph2ibs zmm22, zmm23, {sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x18,0x68,0xf7,0x7b]
          vcvttph2ibs zmm22, zmm23, {sae}, 123

// CHECK: vcvttph2ibs zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x4f,0x68,0xf7,0x7b]
          vcvttph2ibs zmm22 {k7}, zmm23, 123

// CHECK: vcvttph2ibs zmm22 {k7} {z}, zmm23, {sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x9f,0x68,0xf7,0x7b]
          vcvttph2ibs zmm22 {k7} {z}, zmm23, {sae}, 123

// CHECK: vcvttph2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x68,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcvttph2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcvttph2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7c,0x4f,0x68,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcvttph2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcvttph2ibs zmm22, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x58,0x68,0x35,0x00,0x00,0x00,0x00,0x7b]
          vcvttph2ibs zmm22, word ptr [rip]{1to32}, 123

// CHECK: vcvttph2ibs zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x48,0x68,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttph2ibs zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcvttph2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xcf,0x68,0x71,0x7f,0x7b]
          vcvttph2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vcvttph2ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xdf,0x68,0x72,0x80,0x7b]
          vcvttph2ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123

// CHECK: vcvttph2iubs zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x6a,0xf7,0x7b]
          vcvttph2iubs zmm22, zmm23, 123

// CHECK: vcvttph2iubs zmm22, zmm23, {sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x18,0x6a,0xf7,0x7b]
          vcvttph2iubs zmm22, zmm23, {sae}, 123

// CHECK: vcvttph2iubs zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x4f,0x6a,0xf7,0x7b]
          vcvttph2iubs zmm22 {k7}, zmm23, 123

// CHECK: vcvttph2iubs zmm22 {k7} {z}, zmm23, {sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x9f,0x6a,0xf7,0x7b]
          vcvttph2iubs zmm22 {k7} {z}, zmm23, {sae}, 123

// CHECK: vcvttph2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x6a,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcvttph2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcvttph2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7c,0x4f,0x6a,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcvttph2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcvttph2iubs zmm22, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x58,0x6a,0x35,0x00,0x00,0x00,0x00,0x7b]
          vcvttph2iubs zmm22, word ptr [rip]{1to32}, 123

// CHECK: vcvttph2iubs zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x48,0x6a,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttph2iubs zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcvttph2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xcf,0x6a,0x71,0x7f,0x7b]
          vcvttph2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vcvttph2iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0xdf,0x6a,0x72,0x80,0x7b]
          vcvttph2iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123

// CHECK: vcvttps2ibs zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x48,0x68,0xf7,0x7b]
          vcvttps2ibs zmm22, zmm23, 123

// CHECK: vcvttps2ibs zmm22, zmm23, {sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x18,0x68,0xf7,0x7b]
          vcvttps2ibs zmm22, zmm23, {sae}, 123

// CHECK: vcvttps2ibs zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x4f,0x68,0xf7,0x7b]
          vcvttps2ibs zmm22 {k7}, zmm23, 123

// CHECK: vcvttps2ibs zmm22 {k7} {z}, zmm23, {sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x9f,0x68,0xf7,0x7b]
          vcvttps2ibs zmm22 {k7} {z}, zmm23, {sae}, 123

// CHECK: vcvttps2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x48,0x68,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcvttps2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcvttps2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7d,0x4f,0x68,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcvttps2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcvttps2ibs zmm22, dword ptr [rip]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7d,0x58,0x68,0x35,0x00,0x00,0x00,0x00,0x7b]
          vcvttps2ibs zmm22, dword ptr [rip]{1to16}, 123

// CHECK: vcvttps2ibs zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7d,0x48,0x68,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttps2ibs zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcvttps2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7d,0xcf,0x68,0x71,0x7f,0x7b]
          vcvttps2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vcvttps2ibs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7d,0xdf,0x68,0x72,0x80,0x7b]
          vcvttps2ibs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}, 123

// CHECK: vcvttps2iubs zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x48,0x6a,0xf7,0x7b]
          vcvttps2iubs zmm22, zmm23, 123

// CHECK: vcvttps2iubs zmm22, zmm23, {sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x18,0x6a,0xf7,0x7b]
          vcvttps2iubs zmm22, zmm23, {sae}, 123

// CHECK: vcvttps2iubs zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x4f,0x6a,0xf7,0x7b]
          vcvttps2iubs zmm22 {k7}, zmm23, 123

// CHECK: vcvttps2iubs zmm22 {k7} {z}, zmm23, {sae}, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x9f,0x6a,0xf7,0x7b]
          vcvttps2iubs zmm22 {k7} {z}, zmm23, {sae}, 123

// CHECK: vcvttps2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x48,0x6a,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcvttps2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcvttps2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7d,0x4f,0x6a,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcvttps2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcvttps2iubs zmm22, dword ptr [rip]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7d,0x58,0x6a,0x35,0x00,0x00,0x00,0x00,0x7b]
          vcvttps2iubs zmm22, dword ptr [rip]{1to16}, 123

// CHECK: vcvttps2iubs zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7d,0x48,0x6a,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttps2iubs zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcvttps2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7d,0xcf,0x6a,0x71,0x7f,0x7b]
          vcvttps2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vcvttps2iubs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}, 123
// CHECK: encoding: [0x62,0xe3,0x7d,0xdf,0x6a,0x72,0x80,0x7b]
          vcvttps2iubs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}, 123

