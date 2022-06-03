// REQUIRES: intel_feature_isa_avx512_sat_cvt
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vcvtnebf162ibs zmm2, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x69,0xd3,0x7b]
          vcvtnebf162ibs zmm2, zmm3, 123

// CHECK: vcvtnebf162ibs zmm2 {k7}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x69,0xd3,0x7b]
          vcvtnebf162ibs zmm2 {k7}, zmm3, 123

// CHECK: vcvtnebf162ibs zmm2 {k7} {z}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x69,0xd3,0x7b]
          vcvtnebf162ibs zmm2 {k7} {z}, zmm3, 123

// CHECK: vcvtnebf162ibs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x69,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvtnebf162ibs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vcvtnebf162ibs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x69,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvtnebf162ibs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vcvtnebf162ibs zmm2, word ptr [eax]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x58,0x69,0x10,0x7b]
          vcvtnebf162ibs zmm2, word ptr [eax]{1to32}, 123

// CHECK: vcvtnebf162ibs zmm2, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x69,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtnebf162ibs zmm2, zmmword ptr [2*ebp - 2048], 123

// CHECK: vcvtnebf162ibs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x69,0x51,0x7f,0x7b]
          vcvtnebf162ibs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123

// CHECK: vcvtnebf162ibs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0xdf,0x69,0x52,0x80,0x7b]
          vcvtnebf162ibs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123

// CHECK: vcvtnebf162iubs zmm2, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x6b,0xd3,0x7b]
          vcvtnebf162iubs zmm2, zmm3, 123

// CHECK: vcvtnebf162iubs zmm2 {k7}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x6b,0xd3,0x7b]
          vcvtnebf162iubs zmm2 {k7}, zmm3, 123

// CHECK: vcvtnebf162iubs zmm2 {k7} {z}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x6b,0xd3,0x7b]
          vcvtnebf162iubs zmm2 {k7} {z}, zmm3, 123

// CHECK: vcvtnebf162iubs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x6b,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvtnebf162iubs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vcvtnebf162iubs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x6b,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvtnebf162iubs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vcvtnebf162iubs zmm2, word ptr [eax]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x58,0x6b,0x10,0x7b]
          vcvtnebf162iubs zmm2, word ptr [eax]{1to32}, 123

// CHECK: vcvtnebf162iubs zmm2, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x6b,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtnebf162iubs zmm2, zmmword ptr [2*ebp - 2048], 123

// CHECK: vcvtnebf162iubs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x6b,0x51,0x7f,0x7b]
          vcvtnebf162iubs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123

// CHECK: vcvtnebf162iubs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0xdf,0x6b,0x52,0x80,0x7b]
          vcvtnebf162iubs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123

// CHECK: vcvtph2ibs zmm2, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x69,0xd3,0x7b]
          vcvtph2ibs zmm2, zmm3, 123

// CHECK: vcvtph2ibs zmm2, zmm3, {rn-sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x18,0x69,0xd3,0x7b]
          vcvtph2ibs zmm2, zmm3, {rn-sae}, 123

// CHECK: vcvtph2ibs zmm2 {k7}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x69,0xd3,0x7b]
          vcvtph2ibs zmm2 {k7}, zmm3, 123

// CHECK: vcvtph2ibs zmm2 {k7} {z}, zmm3, {rz-sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0xff,0x69,0xd3,0x7b]
          vcvtph2ibs zmm2 {k7} {z}, zmm3, {rz-sae}, 123

// CHECK: vcvtph2ibs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x69,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvtph2ibs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vcvtph2ibs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x69,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvtph2ibs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vcvtph2ibs zmm2, word ptr [eax]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x58,0x69,0x10,0x7b]
          vcvtph2ibs zmm2, word ptr [eax]{1to32}, 123

// CHECK: vcvtph2ibs zmm2, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x69,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtph2ibs zmm2, zmmword ptr [2*ebp - 2048], 123

// CHECK: vcvtph2ibs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0xcf,0x69,0x51,0x7f,0x7b]
          vcvtph2ibs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123

// CHECK: vcvtph2ibs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0xdf,0x69,0x52,0x80,0x7b]
          vcvtph2ibs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123

// CHECK: vcvtph2iubs zmm2, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x6b,0xd3,0x7b]
          vcvtph2iubs zmm2, zmm3, 123

// CHECK: vcvtph2iubs zmm2, zmm3, {rn-sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x18,0x6b,0xd3,0x7b]
          vcvtph2iubs zmm2, zmm3, {rn-sae}, 123

// CHECK: vcvtph2iubs zmm2 {k7}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x6b,0xd3,0x7b]
          vcvtph2iubs zmm2 {k7}, zmm3, 123

// CHECK: vcvtph2iubs zmm2 {k7} {z}, zmm3, {rz-sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0xff,0x6b,0xd3,0x7b]
          vcvtph2iubs zmm2 {k7} {z}, zmm3, {rz-sae}, 123

// CHECK: vcvtph2iubs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x6b,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvtph2iubs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vcvtph2iubs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x6b,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvtph2iubs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vcvtph2iubs zmm2, word ptr [eax]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x58,0x6b,0x10,0x7b]
          vcvtph2iubs zmm2, word ptr [eax]{1to32}, 123

// CHECK: vcvtph2iubs zmm2, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x6b,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtph2iubs zmm2, zmmword ptr [2*ebp - 2048], 123

// CHECK: vcvtph2iubs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0xcf,0x6b,0x51,0x7f,0x7b]
          vcvtph2iubs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123

// CHECK: vcvtph2iubs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0xdf,0x6b,0x52,0x80,0x7b]
          vcvtph2iubs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123

// CHECK: vcvtps2ibs zmm2, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x69,0xd3,0x7b]
          vcvtps2ibs zmm2, zmm3, 123

// CHECK: vcvtps2ibs zmm2, zmm3, {rn-sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x18,0x69,0xd3,0x7b]
          vcvtps2ibs zmm2, zmm3, {rn-sae}, 123

// CHECK: vcvtps2ibs zmm2 {k7}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x69,0xd3,0x7b]
          vcvtps2ibs zmm2 {k7}, zmm3, 123

// CHECK: vcvtps2ibs zmm2 {k7} {z}, zmm3, {rz-sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0xff,0x69,0xd3,0x7b]
          vcvtps2ibs zmm2 {k7} {z}, zmm3, {rz-sae}, 123

// CHECK: vcvtps2ibs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x69,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvtps2ibs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vcvtps2ibs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x69,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvtps2ibs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vcvtps2ibs zmm2, dword ptr [eax]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x58,0x69,0x10,0x7b]
          vcvtps2ibs zmm2, dword ptr [eax]{1to16}, 123

// CHECK: vcvtps2ibs zmm2, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x69,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtps2ibs zmm2, zmmword ptr [2*ebp - 2048], 123

// CHECK: vcvtps2ibs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0xcf,0x69,0x51,0x7f,0x7b]
          vcvtps2ibs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123

// CHECK: vcvtps2ibs zmm2 {k7} {z}, dword ptr [edx - 512]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0xdf,0x69,0x52,0x80,0x7b]
          vcvtps2ibs zmm2 {k7} {z}, dword ptr [edx - 512]{1to16}, 123

// CHECK: vcvtps2iubs zmm2, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x6b,0xd3,0x7b]
          vcvtps2iubs zmm2, zmm3, 123

// CHECK: vcvtps2iubs zmm2, zmm3, {rn-sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x18,0x6b,0xd3,0x7b]
          vcvtps2iubs zmm2, zmm3, {rn-sae}, 123

// CHECK: vcvtps2iubs zmm2 {k7}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x6b,0xd3,0x7b]
          vcvtps2iubs zmm2 {k7}, zmm3, 123

// CHECK: vcvtps2iubs zmm2 {k7} {z}, zmm3, {rz-sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0xff,0x6b,0xd3,0x7b]
          vcvtps2iubs zmm2 {k7} {z}, zmm3, {rz-sae}, 123

// CHECK: vcvtps2iubs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x6b,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvtps2iubs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vcvtps2iubs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x6b,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvtps2iubs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vcvtps2iubs zmm2, dword ptr [eax]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x58,0x6b,0x10,0x7b]
          vcvtps2iubs zmm2, dword ptr [eax]{1to16}, 123

// CHECK: vcvtps2iubs zmm2, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x6b,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtps2iubs zmm2, zmmword ptr [2*ebp - 2048], 123

// CHECK: vcvtps2iubs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0xcf,0x6b,0x51,0x7f,0x7b]
          vcvtps2iubs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123

// CHECK: vcvtps2iubs zmm2 {k7} {z}, dword ptr [edx - 512]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0xdf,0x6b,0x52,0x80,0x7b]
          vcvtps2iubs zmm2 {k7} {z}, dword ptr [edx - 512]{1to16}, 123

// CHECK: vcvttnebf162ibs zmm2, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x68,0xd3,0x7b]
          vcvttnebf162ibs zmm2, zmm3, 123

// CHECK: vcvttnebf162ibs zmm2 {k7}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x68,0xd3,0x7b]
          vcvttnebf162ibs zmm2 {k7}, zmm3, 123

// CHECK: vcvttnebf162ibs zmm2 {k7} {z}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x68,0xd3,0x7b]
          vcvttnebf162ibs zmm2 {k7} {z}, zmm3, 123

// CHECK: vcvttnebf162ibs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x68,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvttnebf162ibs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vcvttnebf162ibs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x68,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvttnebf162ibs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vcvttnebf162ibs zmm2, word ptr [eax]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x58,0x68,0x10,0x7b]
          vcvttnebf162ibs zmm2, word ptr [eax]{1to32}, 123

// CHECK: vcvttnebf162ibs zmm2, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x68,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttnebf162ibs zmm2, zmmword ptr [2*ebp - 2048], 123

// CHECK: vcvttnebf162ibs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x68,0x51,0x7f,0x7b]
          vcvttnebf162ibs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123

// CHECK: vcvttnebf162ibs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0xdf,0x68,0x52,0x80,0x7b]
          vcvttnebf162ibs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123

// CHECK: vcvttnebf162iubs zmm2, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x6a,0xd3,0x7b]
          vcvttnebf162iubs zmm2, zmm3, 123

// CHECK: vcvttnebf162iubs zmm2 {k7}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x6a,0xd3,0x7b]
          vcvttnebf162iubs zmm2 {k7}, zmm3, 123

// CHECK: vcvttnebf162iubs zmm2 {k7} {z}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x6a,0xd3,0x7b]
          vcvttnebf162iubs zmm2 {k7} {z}, zmm3, 123

// CHECK: vcvttnebf162iubs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x6a,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvttnebf162iubs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vcvttnebf162iubs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x6a,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvttnebf162iubs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vcvttnebf162iubs zmm2, word ptr [eax]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x58,0x6a,0x10,0x7b]
          vcvttnebf162iubs zmm2, word ptr [eax]{1to32}, 123

// CHECK: vcvttnebf162iubs zmm2, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x6a,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttnebf162iubs zmm2, zmmword ptr [2*ebp - 2048], 123

// CHECK: vcvttnebf162iubs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x6a,0x51,0x7f,0x7b]
          vcvttnebf162iubs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123

// CHECK: vcvttnebf162iubs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0xdf,0x6a,0x52,0x80,0x7b]
          vcvttnebf162iubs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123

// CHECK: vcvttph2ibs zmm2, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x68,0xd3,0x7b]
          vcvttph2ibs zmm2, zmm3, 123

// CHECK: vcvttph2ibs zmm2, zmm3, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x18,0x68,0xd3,0x7b]
          vcvttph2ibs zmm2, zmm3, {sae}, 123

// CHECK: vcvttph2ibs zmm2 {k7}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x68,0xd3,0x7b]
          vcvttph2ibs zmm2 {k7}, zmm3, 123

// CHECK: vcvttph2ibs zmm2 {k7} {z}, zmm3, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x9f,0x68,0xd3,0x7b]
          vcvttph2ibs zmm2 {k7} {z}, zmm3, {sae}, 123

// CHECK: vcvttph2ibs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x68,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvttph2ibs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vcvttph2ibs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x68,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvttph2ibs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vcvttph2ibs zmm2, word ptr [eax]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x58,0x68,0x10,0x7b]
          vcvttph2ibs zmm2, word ptr [eax]{1to32}, 123

// CHECK: vcvttph2ibs zmm2, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x68,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttph2ibs zmm2, zmmword ptr [2*ebp - 2048], 123

// CHECK: vcvttph2ibs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0xcf,0x68,0x51,0x7f,0x7b]
          vcvttph2ibs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123

// CHECK: vcvttph2ibs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0xdf,0x68,0x52,0x80,0x7b]
          vcvttph2ibs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123

// CHECK: vcvttph2iubs zmm2, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x6a,0xd3,0x7b]
          vcvttph2iubs zmm2, zmm3, 123

// CHECK: vcvttph2iubs zmm2, zmm3, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x18,0x6a,0xd3,0x7b]
          vcvttph2iubs zmm2, zmm3, {sae}, 123

// CHECK: vcvttph2iubs zmm2 {k7}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x6a,0xd3,0x7b]
          vcvttph2iubs zmm2 {k7}, zmm3, 123

// CHECK: vcvttph2iubs zmm2 {k7} {z}, zmm3, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x9f,0x6a,0xd3,0x7b]
          vcvttph2iubs zmm2 {k7} {z}, zmm3, {sae}, 123

// CHECK: vcvttph2iubs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x6a,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvttph2iubs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vcvttph2iubs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x6a,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvttph2iubs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vcvttph2iubs zmm2, word ptr [eax]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x58,0x6a,0x10,0x7b]
          vcvttph2iubs zmm2, word ptr [eax]{1to32}, 123

// CHECK: vcvttph2iubs zmm2, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x6a,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttph2iubs zmm2, zmmword ptr [2*ebp - 2048], 123

// CHECK: vcvttph2iubs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7c,0xcf,0x6a,0x51,0x7f,0x7b]
          vcvttph2iubs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123

// CHECK: vcvttph2iubs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7c,0xdf,0x6a,0x52,0x80,0x7b]
          vcvttph2iubs zmm2 {k7} {z}, word ptr [edx - 256]{1to32}, 123

// CHECK: vcvttps2ibs zmm2, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x68,0xd3,0x7b]
          vcvttps2ibs zmm2, zmm3, 123

// CHECK: vcvttps2ibs zmm2, zmm3, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x18,0x68,0xd3,0x7b]
          vcvttps2ibs zmm2, zmm3, {sae}, 123

// CHECK: vcvttps2ibs zmm2 {k7}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x68,0xd3,0x7b]
          vcvttps2ibs zmm2 {k7}, zmm3, 123

// CHECK: vcvttps2ibs zmm2 {k7} {z}, zmm3, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x9f,0x68,0xd3,0x7b]
          vcvttps2ibs zmm2 {k7} {z}, zmm3, {sae}, 123

// CHECK: vcvttps2ibs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x68,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvttps2ibs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vcvttps2ibs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x68,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvttps2ibs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vcvttps2ibs zmm2, dword ptr [eax]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x58,0x68,0x10,0x7b]
          vcvttps2ibs zmm2, dword ptr [eax]{1to16}, 123

// CHECK: vcvttps2ibs zmm2, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x68,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttps2ibs zmm2, zmmword ptr [2*ebp - 2048], 123

// CHECK: vcvttps2ibs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0xcf,0x68,0x51,0x7f,0x7b]
          vcvttps2ibs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123

// CHECK: vcvttps2ibs zmm2 {k7} {z}, dword ptr [edx - 512]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0xdf,0x68,0x52,0x80,0x7b]
          vcvttps2ibs zmm2 {k7} {z}, dword ptr [edx - 512]{1to16}, 123

// CHECK: vcvttps2iubs zmm2, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x6a,0xd3,0x7b]
          vcvttps2iubs zmm2, zmm3, 123

// CHECK: vcvttps2iubs zmm2, zmm3, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x18,0x6a,0xd3,0x7b]
          vcvttps2iubs zmm2, zmm3, {sae}, 123

// CHECK: vcvttps2iubs zmm2 {k7}, zmm3, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x6a,0xd3,0x7b]
          vcvttps2iubs zmm2 {k7}, zmm3, 123

// CHECK: vcvttps2iubs zmm2 {k7} {z}, zmm3, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x9f,0x6a,0xd3,0x7b]
          vcvttps2iubs zmm2 {k7} {z}, zmm3, {sae}, 123

// CHECK: vcvttps2iubs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x6a,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvttps2iubs zmm2, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vcvttps2iubs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x6a,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvttps2iubs zmm2 {k7}, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vcvttps2iubs zmm2, dword ptr [eax]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x58,0x6a,0x10,0x7b]
          vcvttps2iubs zmm2, dword ptr [eax]{1to16}, 123

// CHECK: vcvttps2iubs zmm2, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x6a,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttps2iubs zmm2, zmmword ptr [2*ebp - 2048], 123

// CHECK: vcvttps2iubs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7d,0xcf,0x6a,0x51,0x7f,0x7b]
          vcvttps2iubs zmm2 {k7} {z}, zmmword ptr [ecx + 8128], 123

// CHECK: vcvttps2iubs zmm2 {k7} {z}, dword ptr [edx - 512]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x7d,0xdf,0x6a,0x52,0x80,0x7b]
          vcvttps2iubs zmm2 {k7} {z}, dword ptr [edx - 512]{1to16}, 123

