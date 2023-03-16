// REQUIRES: intel_feature_isa_avx512_sat_cvt
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vcvtnebf162ibs zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x69,0xf7]
          vcvtnebf162ibs zmm22, zmm23

// CHECK: vcvtnebf162ibs zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x4f,0x69,0xf7]
          vcvtnebf162ibs zmm22 {k7}, zmm23

// CHECK: vcvtnebf162ibs zmm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0xcf,0x69,0xf7]
          vcvtnebf162ibs zmm22 {k7} {z}, zmm23

// CHECK: vcvtnebf162ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x69,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnebf162ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtnebf162ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x4f,0x69,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnebf162ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtnebf162ibs zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7f,0x58,0x69,0x35,0x00,0x00,0x00,0x00]
          vcvtnebf162ibs zmm22, word ptr [rip]{1to32}

// CHECK: vcvtnebf162ibs zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x69,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtnebf162ibs zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtnebf162ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x69,0x71,0x7f]
          vcvtnebf162ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtnebf162ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7f,0xdf,0x69,0x72,0x80]
          vcvtnebf162ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtnebf162iubs zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x6b,0xf7]
          vcvtnebf162iubs zmm22, zmm23

// CHECK: vcvtnebf162iubs zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x4f,0x6b,0xf7]
          vcvtnebf162iubs zmm22 {k7}, zmm23

// CHECK: vcvtnebf162iubs zmm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0xcf,0x6b,0xf7]
          vcvtnebf162iubs zmm22 {k7} {z}, zmm23

// CHECK: vcvtnebf162iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x6b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnebf162iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtnebf162iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x4f,0x6b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnebf162iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtnebf162iubs zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7f,0x58,0x6b,0x35,0x00,0x00,0x00,0x00]
          vcvtnebf162iubs zmm22, word ptr [rip]{1to32}

// CHECK: vcvtnebf162iubs zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x6b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtnebf162iubs zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtnebf162iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x6b,0x71,0x7f]
          vcvtnebf162iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtnebf162iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7f,0xdf,0x6b,0x72,0x80]
          vcvtnebf162iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtph2ibs zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x69,0xf7]
          vcvtph2ibs zmm22, zmm23

// CHECK: vcvtph2ibs zmm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x69,0xf7]
          vcvtph2ibs zmm22, zmm23, {rn-sae}

// CHECK: vcvtph2ibs zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x69,0xf7]
          vcvtph2ibs zmm22 {k7}, zmm23

// CHECK: vcvtph2ibs zmm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0xff,0x69,0xf7]
          vcvtph2ibs zmm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtph2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x69,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x69,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2ibs zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x69,0x35,0x00,0x00,0x00,0x00]
          vcvtph2ibs zmm22, word ptr [rip]{1to32}

// CHECK: vcvtph2ibs zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x69,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtph2ibs zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtph2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x69,0x71,0x7f]
          vcvtph2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtph2ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x69,0x72,0x80]
          vcvtph2ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtph2iubs zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x6b,0xf7]
          vcvtph2iubs zmm22, zmm23

// CHECK: vcvtph2iubs zmm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x6b,0xf7]
          vcvtph2iubs zmm22, zmm23, {rn-sae}

// CHECK: vcvtph2iubs zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x6b,0xf7]
          vcvtph2iubs zmm22 {k7}, zmm23

// CHECK: vcvtph2iubs zmm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0xff,0x6b,0xf7]
          vcvtph2iubs zmm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtph2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x6b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtph2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x6b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtph2iubs zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x6b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2iubs zmm22, word ptr [rip]{1to32}

// CHECK: vcvtph2iubs zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x6b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtph2iubs zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtph2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x6b,0x71,0x7f]
          vcvtph2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtph2iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x6b,0x72,0x80]
          vcvtph2iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvtps2ibs zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x69,0xf7]
          vcvtps2ibs zmm22, zmm23

// CHECK: vcvtps2ibs zmm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x69,0xf7]
          vcvtps2ibs zmm22, zmm23, {rn-sae}

// CHECK: vcvtps2ibs zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x69,0xf7]
          vcvtps2ibs zmm22 {k7}, zmm23

// CHECK: vcvtps2ibs zmm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0xff,0x69,0xf7]
          vcvtps2ibs zmm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtps2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x69,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtps2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtps2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x69,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtps2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtps2ibs zmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x69,0x35,0x00,0x00,0x00,0x00]
          vcvtps2ibs zmm22, dword ptr [rip]{1to16}

// CHECK: vcvtps2ibs zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x69,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtps2ibs zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtps2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x69,0x71,0x7f]
          vcvtps2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtps2ibs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x69,0x72,0x80]
          vcvtps2ibs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}

// CHECK: vcvtps2iubs zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x6b,0xf7]
          vcvtps2iubs zmm22, zmm23

// CHECK: vcvtps2iubs zmm22, zmm23, {rn-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x6b,0xf7]
          vcvtps2iubs zmm22, zmm23, {rn-sae}

// CHECK: vcvtps2iubs zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x6b,0xf7]
          vcvtps2iubs zmm22 {k7}, zmm23

// CHECK: vcvtps2iubs zmm22 {k7} {z}, zmm23, {rz-sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0xff,0x6b,0xf7]
          vcvtps2iubs zmm22 {k7} {z}, zmm23, {rz-sae}

// CHECK: vcvtps2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x6b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtps2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtps2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x6b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtps2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtps2iubs zmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x6b,0x35,0x00,0x00,0x00,0x00]
          vcvtps2iubs zmm22, dword ptr [rip]{1to16}

// CHECK: vcvtps2iubs zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x6b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtps2iubs zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvtps2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x6b,0x71,0x7f]
          vcvtps2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvtps2iubs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x6b,0x72,0x80]
          vcvtps2iubs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}

// CHECK: vcvttnebf162ibs zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x68,0xf7]
          vcvttnebf162ibs zmm22, zmm23

// CHECK: vcvttnebf162ibs zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x4f,0x68,0xf7]
          vcvttnebf162ibs zmm22 {k7}, zmm23

// CHECK: vcvttnebf162ibs zmm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0xcf,0x68,0xf7]
          vcvttnebf162ibs zmm22 {k7} {z}, zmm23

// CHECK: vcvttnebf162ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x68,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttnebf162ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttnebf162ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x4f,0x68,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttnebf162ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttnebf162ibs zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7f,0x58,0x68,0x35,0x00,0x00,0x00,0x00]
          vcvttnebf162ibs zmm22, word ptr [rip]{1to32}

// CHECK: vcvttnebf162ibs zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x68,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvttnebf162ibs zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvttnebf162ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x68,0x71,0x7f]
          vcvttnebf162ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvttnebf162ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7f,0xdf,0x68,0x72,0x80]
          vcvttnebf162ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvttnebf162iubs zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x6a,0xf7]
          vcvttnebf162iubs zmm22, zmm23

// CHECK: vcvttnebf162iubs zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0x4f,0x6a,0xf7]
          vcvttnebf162iubs zmm22 {k7}, zmm23

// CHECK: vcvttnebf162iubs zmm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7f,0xcf,0x6a,0xf7]
          vcvttnebf162iubs zmm22 {k7} {z}, zmm23

// CHECK: vcvttnebf162iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x6a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttnebf162iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttnebf162iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x4f,0x6a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttnebf162iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttnebf162iubs zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7f,0x58,0x6a,0x35,0x00,0x00,0x00,0x00]
          vcvttnebf162iubs zmm22, word ptr [rip]{1to32}

// CHECK: vcvttnebf162iubs zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x6a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvttnebf162iubs zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvttnebf162iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x6a,0x71,0x7f]
          vcvttnebf162iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvttnebf162iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7f,0xdf,0x6a,0x72,0x80]
          vcvttnebf162iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvttph2ibs zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x68,0xf7]
          vcvttph2ibs zmm22, zmm23

// CHECK: vcvttph2ibs zmm22, zmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x68,0xf7]
          vcvttph2ibs zmm22, zmm23, {sae}

// CHECK: vcvttph2ibs zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x68,0xf7]
          vcvttph2ibs zmm22 {k7}, zmm23

// CHECK: vcvttph2ibs zmm22 {k7} {z}, zmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x9f,0x68,0xf7]
          vcvttph2ibs zmm22 {k7} {z}, zmm23, {sae}

// CHECK: vcvttph2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x68,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x68,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2ibs zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x68,0x35,0x00,0x00,0x00,0x00]
          vcvttph2ibs zmm22, word ptr [rip]{1to32}

// CHECK: vcvttph2ibs zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x68,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvttph2ibs zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvttph2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x68,0x71,0x7f]
          vcvttph2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvttph2ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x68,0x72,0x80]
          vcvttph2ibs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvttph2iubs zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x6a,0xf7]
          vcvttph2iubs zmm22, zmm23

// CHECK: vcvttph2iubs zmm22, zmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x6a,0xf7]
          vcvttph2iubs zmm22, zmm23, {sae}

// CHECK: vcvttph2iubs zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x6a,0xf7]
          vcvttph2iubs zmm22 {k7}, zmm23

// CHECK: vcvttph2iubs zmm22 {k7} {z}, zmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7c,0x9f,0x6a,0xf7]
          vcvttph2iubs zmm22 {k7} {z}, zmm23, {sae}

// CHECK: vcvttph2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x6a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttph2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x6a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttph2iubs zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x6a,0x35,0x00,0x00,0x00,0x00]
          vcvttph2iubs zmm22, word ptr [rip]{1to32}

// CHECK: vcvttph2iubs zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x6a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvttph2iubs zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvttph2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x6a,0x71,0x7f]
          vcvttph2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvttph2iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x6a,0x72,0x80]
          vcvttph2iubs zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vcvttps2ibs zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x68,0xf7]
          vcvttps2ibs zmm22, zmm23

// CHECK: vcvttps2ibs zmm22, zmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x68,0xf7]
          vcvttps2ibs zmm22, zmm23, {sae}

// CHECK: vcvttps2ibs zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x68,0xf7]
          vcvttps2ibs zmm22 {k7}, zmm23

// CHECK: vcvttps2ibs zmm22 {k7} {z}, zmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x9f,0x68,0xf7]
          vcvttps2ibs zmm22 {k7} {z}, zmm23, {sae}

// CHECK: vcvttps2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x68,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttps2ibs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttps2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x68,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttps2ibs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttps2ibs zmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x68,0x35,0x00,0x00,0x00,0x00]
          vcvttps2ibs zmm22, dword ptr [rip]{1to16}

// CHECK: vcvttps2ibs zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x68,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvttps2ibs zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvttps2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x68,0x71,0x7f]
          vcvttps2ibs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvttps2ibs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x68,0x72,0x80]
          vcvttps2ibs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}

// CHECK: vcvttps2iubs zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x6a,0xf7]
          vcvttps2iubs zmm22, zmm23

// CHECK: vcvttps2iubs zmm22, zmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x6a,0xf7]
          vcvttps2iubs zmm22, zmm23, {sae}

// CHECK: vcvttps2iubs zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x6a,0xf7]
          vcvttps2iubs zmm22 {k7}, zmm23

// CHECK: vcvttps2iubs zmm22 {k7} {z}, zmm23, {sae}
// CHECK: encoding: [0x62,0xa5,0x7d,0x9f,0x6a,0xf7]
          vcvttps2iubs zmm22 {k7} {z}, zmm23, {sae}

// CHECK: vcvttps2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x6a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttps2iubs zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvttps2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x6a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttps2iubs zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvttps2iubs zmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x6a,0x35,0x00,0x00,0x00,0x00]
          vcvttps2iubs zmm22, dword ptr [rip]{1to16}

// CHECK: vcvttps2iubs zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x6a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvttps2iubs zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vcvttps2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x6a,0x71,0x7f]
          vcvttps2iubs zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vcvttps2iubs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x6a,0x72,0x80]
          vcvttps2iubs zmm22 {k7} {z}, dword ptr [rdx - 512]{1to16}

