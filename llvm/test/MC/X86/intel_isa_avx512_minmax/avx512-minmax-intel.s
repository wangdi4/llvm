// REQUIRES: intel_feature_isa_avx512_minmax
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vminmaxnepbf16 zmm2, zmm3, zmm4, 123
// CHECK: encoding: [0x62,0xf3,0x67,0x48,0x52,0xd4,0x7b]
          vminmaxnepbf16 zmm2, zmm3, zmm4, 123

// CHECK: vminmaxnepbf16 zmm2 {k7}, zmm3, zmm4, 123
// CHECK: encoding: [0x62,0xf3,0x67,0x4f,0x52,0xd4,0x7b]
          vminmaxnepbf16 zmm2 {k7}, zmm3, zmm4, 123

// CHECK: vminmaxnepbf16 zmm2 {k7} {z}, zmm3, zmm4, 123
// CHECK: encoding: [0x62,0xf3,0x67,0xcf,0x52,0xd4,0x7b]
          vminmaxnepbf16 zmm2 {k7} {z}, zmm3, zmm4, 123

// CHECK: vminmaxnepbf16 zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x67,0x48,0x52,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vminmaxnepbf16 zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vminmaxnepbf16 zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x67,0x4f,0x52,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vminmaxnepbf16 zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vminmaxnepbf16 zmm2, zmm3, word ptr [eax]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x67,0x58,0x52,0x10,0x7b]
          vminmaxnepbf16 zmm2, zmm3, word ptr [eax]{1to32}, 123

// CHECK: vminmaxnepbf16 zmm2, zmm3, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x67,0x48,0x52,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxnepbf16 zmm2, zmm3, zmmword ptr [2*ebp - 2048], 123

// CHECK: vminmaxnepbf16 zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x67,0xcf,0x52,0x51,0x7f,0x7b]
          vminmaxnepbf16 zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128], 123

// CHECK: vminmaxnepbf16 zmm2 {k7} {z}, zmm3, word ptr [edx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x67,0xdf,0x52,0x52,0x80,0x7b]
          vminmaxnepbf16 zmm2 {k7} {z}, zmm3, word ptr [edx - 256]{1to32}, 123

// CHECK: vminmaxpd zmm2, zmm3, zmm4, 123
// CHECK: encoding: [0x62,0xf3,0xe5,0x48,0x52,0xd4,0x7b]
          vminmaxpd zmm2, zmm3, zmm4, 123

// CHECK: vminmaxpd zmm2, zmm3, zmm4, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0xe5,0x18,0x52,0xd4,0x7b]
          vminmaxpd zmm2, zmm3, zmm4, {sae}, 123

// CHECK: vminmaxpd zmm2 {k7}, zmm3, zmm4, 123
// CHECK: encoding: [0x62,0xf3,0xe5,0x4f,0x52,0xd4,0x7b]
          vminmaxpd zmm2 {k7}, zmm3, zmm4, 123

// CHECK: vminmaxpd zmm2 {k7} {z}, zmm3, zmm4, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0xe5,0x9f,0x52,0xd4,0x7b]
          vminmaxpd zmm2 {k7} {z}, zmm3, zmm4, {sae}, 123

// CHECK: vminmaxpd zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0xe5,0x48,0x52,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vminmaxpd zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vminmaxpd zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0xe5,0x4f,0x52,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vminmaxpd zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vminmaxpd zmm2, zmm3, qword ptr [eax]{1to8}, 123
// CHECK: encoding: [0x62,0xf3,0xe5,0x58,0x52,0x10,0x7b]
          vminmaxpd zmm2, zmm3, qword ptr [eax]{1to8}, 123

// CHECK: vminmaxpd zmm2, zmm3, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0xe5,0x48,0x52,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxpd zmm2, zmm3, zmmword ptr [2*ebp - 2048], 123

// CHECK: vminmaxpd zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0xe5,0xcf,0x52,0x51,0x7f,0x7b]
          vminmaxpd zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128], 123

// CHECK: vminmaxpd zmm2 {k7} {z}, zmm3, qword ptr [edx - 1024]{1to8}, 123
// CHECK: encoding: [0x62,0xf3,0xe5,0xdf,0x52,0x52,0x80,0x7b]
          vminmaxpd zmm2 {k7} {z}, zmm3, qword ptr [edx - 1024]{1to8}, 123

// CHECK: vminmaxph zmm2, zmm3, zmm4, 123
// CHECK: encoding: [0x62,0xf3,0x64,0x48,0x52,0xd4,0x7b]
          vminmaxph zmm2, zmm3, zmm4, 123

// CHECK: vminmaxph zmm2, zmm3, zmm4, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0x64,0x18,0x52,0xd4,0x7b]
          vminmaxph zmm2, zmm3, zmm4, {sae}, 123

// CHECK: vminmaxph zmm2 {k7}, zmm3, zmm4, 123
// CHECK: encoding: [0x62,0xf3,0x64,0x4f,0x52,0xd4,0x7b]
          vminmaxph zmm2 {k7}, zmm3, zmm4, 123

// CHECK: vminmaxph zmm2 {k7} {z}, zmm3, zmm4, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0x64,0x9f,0x52,0xd4,0x7b]
          vminmaxph zmm2 {k7} {z}, zmm3, zmm4, {sae}, 123

// CHECK: vminmaxph zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x64,0x48,0x52,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vminmaxph zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vminmaxph zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x64,0x4f,0x52,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vminmaxph zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vminmaxph zmm2, zmm3, word ptr [eax]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x64,0x58,0x52,0x10,0x7b]
          vminmaxph zmm2, zmm3, word ptr [eax]{1to32}, 123

// CHECK: vminmaxph zmm2, zmm3, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x64,0x48,0x52,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxph zmm2, zmm3, zmmword ptr [2*ebp - 2048], 123

// CHECK: vminmaxph zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x64,0xcf,0x52,0x51,0x7f,0x7b]
          vminmaxph zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128], 123

// CHECK: vminmaxph zmm2 {k7} {z}, zmm3, word ptr [edx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x64,0xdf,0x52,0x52,0x80,0x7b]
          vminmaxph zmm2 {k7} {z}, zmm3, word ptr [edx - 256]{1to32}, 123

// CHECK: vminmaxps zmm2, zmm3, zmm4, 123
// CHECK: encoding: [0x62,0xf3,0x65,0x48,0x52,0xd4,0x7b]
          vminmaxps zmm2, zmm3, zmm4, 123

// CHECK: vminmaxps zmm2, zmm3, zmm4, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0x65,0x18,0x52,0xd4,0x7b]
          vminmaxps zmm2, zmm3, zmm4, {sae}, 123

// CHECK: vminmaxps zmm2 {k7}, zmm3, zmm4, 123
// CHECK: encoding: [0x62,0xf3,0x65,0x4f,0x52,0xd4,0x7b]
          vminmaxps zmm2 {k7}, zmm3, zmm4, 123

// CHECK: vminmaxps zmm2 {k7} {z}, zmm3, zmm4, {sae}, 123
// CHECK: encoding: [0x62,0xf3,0x65,0x9f,0x52,0xd4,0x7b]
          vminmaxps zmm2 {k7} {z}, zmm3, zmm4, {sae}, 123

// CHECK: vminmaxps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0x62,0xf3,0x65,0x48,0x52,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vminmaxps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: vminmaxps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0x62,0xf3,0x65,0x4f,0x52,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vminmaxps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291], 123

// CHECK: vminmaxps zmm2, zmm3, dword ptr [eax]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x65,0x58,0x52,0x10,0x7b]
          vminmaxps zmm2, zmm3, dword ptr [eax]{1to16}, 123

// CHECK: vminmaxps zmm2, zmm3, zmmword ptr [2*ebp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x65,0x48,0x52,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxps zmm2, zmm3, zmmword ptr [2*ebp - 2048], 123

// CHECK: vminmaxps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x65,0xcf,0x52,0x51,0x7f,0x7b]
          vminmaxps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128], 123

// CHECK: vminmaxps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}, 123
// CHECK: encoding: [0x62,0xf3,0x65,0xdf,0x52,0x52,0x80,0x7b]
          vminmaxps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}, 123

