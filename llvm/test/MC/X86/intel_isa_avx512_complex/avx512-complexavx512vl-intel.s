// REQUIRES: intel_feature_isa_avx512_complex
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vaddsubpd ymm2, ymm3, ymm4
// CHECK: encoding: [0xc5,0xe5,0xd0,0xd4]
          vaddsubpd ymm2, ymm3, ymm4

// CHECK: vaddsubpd ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0xe5,0x2f,0xd0,0xd4]
          vaddsubpd ymm2 {k7}, ymm3, ymm4

// CHECK: vaddsubpd ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0xe5,0xaf,0xd0,0xd4]
          vaddsubpd ymm2 {k7} {z}, ymm3, ymm4

// CHECK: vaddsubpd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc5,0xe1,0xd0,0xd4]
          vaddsubpd xmm2, xmm3, xmm4

// CHECK: vaddsubpd xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0xe5,0x0f,0xd0,0xd4]
          vaddsubpd xmm2 {k7}, xmm3, xmm4

// CHECK: vaddsubpd xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0xe5,0x8f,0xd0,0xd4]
          vaddsubpd xmm2 {k7} {z}, xmm3, xmm4

// CHECK: vaddsubpd ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc5,0xe5,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubpd ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vaddsubpd ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0xe5,0x2f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubpd ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vaddsubpd ymm2, ymm3, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf6,0xe5,0x38,0xd0,0x10]
          vaddsubpd ymm2, ymm3, qword ptr [eax]{1to4}

// CHECK: vaddsubpd ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc5,0xe5,0xd0,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vaddsubpd ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vaddsubpd ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf6,0xe5,0xaf,0xd0,0x51,0x7f]
          vaddsubpd ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vaddsubpd ymm2 {k7} {z}, ymm3, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf6,0xe5,0xbf,0xd0,0x52,0x80]
          vaddsubpd ymm2 {k7} {z}, ymm3, qword ptr [edx - 1024]{1to4}

// CHECK: vaddsubpd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc5,0xe1,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubpd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vaddsubpd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0xe5,0x0f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubpd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vaddsubpd xmm2, xmm3, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf6,0xe5,0x18,0xd0,0x10]
          vaddsubpd xmm2, xmm3, qword ptr [eax]{1to2}

// CHECK: vaddsubpd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc5,0xe1,0xd0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vaddsubpd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vaddsubpd xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf6,0xe5,0x8f,0xd0,0x51,0x7f]
          vaddsubpd xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vaddsubpd xmm2 {k7} {z}, xmm3, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf6,0xe5,0x9f,0xd0,0x52,0x80]
          vaddsubpd xmm2 {k7} {z}, xmm3, qword ptr [edx - 1024]{1to2}

// CHECK: vaddsubph ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xd0,0xd4]
          vaddsubph ymm2, ymm3, ymm4

// CHECK: vaddsubph ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xd0,0xd4]
          vaddsubph ymm2 {k7}, ymm3, ymm4

// CHECK: vaddsubph ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xd0,0xd4]
          vaddsubph ymm2 {k7} {z}, ymm3, ymm4

// CHECK: vaddsubph xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xd0,0xd4]
          vaddsubph xmm2, xmm3, xmm4

// CHECK: vaddsubph xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xd0,0xd4]
          vaddsubph xmm2 {k7}, xmm3, xmm4

// CHECK: vaddsubph xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xd0,0xd4]
          vaddsubph xmm2 {k7} {z}, xmm3, xmm4

// CHECK: vaddsubph ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubph ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vaddsubph ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubph ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vaddsubph ymm2, ymm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0xd0,0x10]
          vaddsubph ymm2, ymm3, word ptr [eax]{1to16}

// CHECK: vaddsubph ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xd0,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vaddsubph ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vaddsubph ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xd0,0x51,0x7f]
          vaddsubph ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vaddsubph ymm2 {k7} {z}, ymm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0xd0,0x52,0x80]
          vaddsubph ymm2 {k7} {z}, ymm3, word ptr [edx - 256]{1to16}

// CHECK: vaddsubph xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubph xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vaddsubph xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubph xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vaddsubph xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xd0,0x10]
          vaddsubph xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vaddsubph xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xd0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vaddsubph xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vaddsubph xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xd0,0x51,0x7f]
          vaddsubph xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vaddsubph xmm2 {k7} {z}, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0xd0,0x52,0x80]
          vaddsubph xmm2 {k7} {z}, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vaddsubps ymm2, ymm3, ymm4
// CHECK: encoding: [0xc5,0xe7,0xd0,0xd4]
          vaddsubps ymm2, ymm3, ymm4

// CHECK: vaddsubps ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0x65,0x2f,0xd0,0xd4]
          vaddsubps ymm2 {k7}, ymm3, ymm4

// CHECK: vaddsubps ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0x65,0xaf,0xd0,0xd4]
          vaddsubps ymm2 {k7} {z}, ymm3, ymm4

// CHECK: vaddsubps xmm2, xmm3, xmm4
// CHECK: encoding: [0xc5,0xe3,0xd0,0xd4]
          vaddsubps xmm2, xmm3, xmm4

// CHECK: vaddsubps xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0x65,0x0f,0xd0,0xd4]
          vaddsubps xmm2 {k7}, xmm3, xmm4

// CHECK: vaddsubps xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0x65,0x8f,0xd0,0xd4]
          vaddsubps xmm2 {k7} {z}, xmm3, xmm4

// CHECK: vaddsubps ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc5,0xe7,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubps ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vaddsubps ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0x65,0x2f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubps ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vaddsubps ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf6,0x65,0x38,0xd0,0x10]
          vaddsubps ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK: vaddsubps ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc5,0xe7,0xd0,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vaddsubps ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vaddsubps ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf6,0x65,0xaf,0xd0,0x51,0x7f]
          vaddsubps ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vaddsubps ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf6,0x65,0xbf,0xd0,0x52,0x80]
          vaddsubps ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK: vaddsubps xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc5,0xe3,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubps xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vaddsubps xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0x65,0x0f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubps xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vaddsubps xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf6,0x65,0x18,0xd0,0x10]
          vaddsubps xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK: vaddsubps xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc5,0xe3,0xd0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vaddsubps xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vaddsubps xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf6,0x65,0x8f,0xd0,0x51,0x7f]
          vaddsubps xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vaddsubps xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf6,0x65,0x9f,0xd0,0x52,0x80]
          vaddsubps xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK: vmovdhdup xmm2, xmm3
// CHECK: encoding: [0x62,0xf1,0xff,0x08,0x16,0xd3]
          vmovdhdup xmm2, xmm3

// CHECK: vmovdhdup xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf1,0xff,0x0f,0x16,0xd3]
          vmovdhdup xmm2 {k7}, xmm3

// CHECK: vmovdhdup xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf1,0xff,0x8f,0x16,0xd3]
          vmovdhdup xmm2 {k7} {z}, xmm3

// CHECK: vmovdhdup ymm2, ymm3
// CHECK: encoding: [0x62,0xf1,0xff,0x28,0x16,0xd3]
          vmovdhdup ymm2, ymm3

// CHECK: vmovdhdup ymm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf1,0xff,0x2f,0x16,0xd3]
          vmovdhdup ymm2 {k7}, ymm3

// CHECK: vmovdhdup ymm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf1,0xff,0xaf,0x16,0xd3]
          vmovdhdup ymm2 {k7} {z}, ymm3

// CHECK: vmovdhdup xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf1,0xff,0x08,0x16,0x94,0xf4,0x00,0x00,0x00,0x10]
          vmovdhdup xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vmovdhdup xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf1,0xff,0x0f,0x16,0x94,0x87,0x23,0x01,0x00,0x00]
          vmovdhdup xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vmovdhdup xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf1,0xff,0x08,0x16,0x10]
          vmovdhdup xmm2, xmmword ptr [eax]

// CHECK: vmovdhdup xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf1,0xff,0x08,0x16,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vmovdhdup xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vmovdhdup xmm2 {k7} {z}, xmmword ptr [ecx + 1016]
// CHECK: encoding: [0x62,0xf1,0xff,0x8f,0x16,0x51,0x7f]
          vmovdhdup xmm2 {k7} {z}, xmmword ptr [ecx + 1016]

// CHECK: vmovdhdup xmm2 {k7} {z}, xmmword ptr [edx - 1024]
// CHECK: encoding: [0x62,0xf1,0xff,0x8f,0x16,0x52,0x80]
          vmovdhdup xmm2 {k7} {z}, xmmword ptr [edx - 1024]

// CHECK: vmovdhdup ymm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf1,0xff,0x28,0x16,0x94,0xf4,0x00,0x00,0x00,0x10]
          vmovdhdup ymm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vmovdhdup ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf1,0xff,0x2f,0x16,0x94,0x87,0x23,0x01,0x00,0x00]
          vmovdhdup ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]

// CHECK: vmovdhdup ymm2, ymmword ptr [eax]
// CHECK: encoding: [0x62,0xf1,0xff,0x28,0x16,0x10]
          vmovdhdup ymm2, ymmword ptr [eax]

// CHECK: vmovdhdup ymm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf1,0xff,0x28,0x16,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vmovdhdup ymm2, ymmword ptr [2*ebp - 1024]

// CHECK: vmovdhdup ymm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf1,0xff,0xaf,0x16,0x51,0x7f]
          vmovdhdup ymm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK: vmovdhdup ymm2 {k7} {z}, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf1,0xff,0xaf,0x16,0x52,0x80]
          vmovdhdup ymm2 {k7} {z}, ymmword ptr [edx - 4096]

// CHECK: vsubaddpd ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0xe5,0x28,0xd1,0xd4]
          vsubaddpd ymm2, ymm3, ymm4

// CHECK: vsubaddpd ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0xe5,0x2f,0xd1,0xd4]
          vsubaddpd ymm2 {k7}, ymm3, ymm4

// CHECK: vsubaddpd ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0xe5,0xaf,0xd1,0xd4]
          vsubaddpd ymm2 {k7} {z}, ymm3, ymm4

// CHECK: vsubaddpd xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0xe5,0x08,0xd1,0xd4]
          vsubaddpd xmm2, xmm3, xmm4

// CHECK: vsubaddpd xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0xe5,0x0f,0xd1,0xd4]
          vsubaddpd xmm2 {k7}, xmm3, xmm4

// CHECK: vsubaddpd xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0xe5,0x8f,0xd1,0xd4]
          vsubaddpd xmm2 {k7} {z}, xmm3, xmm4

// CHECK: vsubaddpd ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0xe5,0x28,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddpd ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vsubaddpd ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0xe5,0x2f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddpd ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vsubaddpd ymm2, ymm3, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf6,0xe5,0x38,0xd1,0x10]
          vsubaddpd ymm2, ymm3, qword ptr [eax]{1to4}

// CHECK: vsubaddpd ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf6,0xe5,0x28,0xd1,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vsubaddpd ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vsubaddpd ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf6,0xe5,0xaf,0xd1,0x51,0x7f]
          vsubaddpd ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vsubaddpd ymm2 {k7} {z}, ymm3, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf6,0xe5,0xbf,0xd1,0x52,0x80]
          vsubaddpd ymm2 {k7} {z}, ymm3, qword ptr [edx - 1024]{1to4}

// CHECK: vsubaddpd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0xe5,0x08,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddpd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vsubaddpd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0xe5,0x0f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddpd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vsubaddpd xmm2, xmm3, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf6,0xe5,0x18,0xd1,0x10]
          vsubaddpd xmm2, xmm3, qword ptr [eax]{1to2}

// CHECK: vsubaddpd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf6,0xe5,0x08,0xd1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vsubaddpd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vsubaddpd xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf6,0xe5,0x8f,0xd1,0x51,0x7f]
          vsubaddpd xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vsubaddpd xmm2 {k7} {z}, xmm3, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf6,0xe5,0x9f,0xd1,0x52,0x80]
          vsubaddpd xmm2 {k7} {z}, xmm3, qword ptr [edx - 1024]{1to2}

// CHECK: vsubaddph ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xd1,0xd4]
          vsubaddph ymm2, ymm3, ymm4

// CHECK: vsubaddph ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xd1,0xd4]
          vsubaddph ymm2 {k7}, ymm3, ymm4

// CHECK: vsubaddph ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xd1,0xd4]
          vsubaddph ymm2 {k7} {z}, ymm3, ymm4

// CHECK: vsubaddph xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xd1,0xd4]
          vsubaddph xmm2, xmm3, xmm4

// CHECK: vsubaddph xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xd1,0xd4]
          vsubaddph xmm2 {k7}, xmm3, xmm4

// CHECK: vsubaddph xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xd1,0xd4]
          vsubaddph xmm2 {k7} {z}, xmm3, xmm4

// CHECK: vsubaddph ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddph ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vsubaddph ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddph ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vsubaddph ymm2, ymm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0xd1,0x10]
          vsubaddph ymm2, ymm3, word ptr [eax]{1to16}

// CHECK: vsubaddph ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xd1,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vsubaddph ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vsubaddph ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xd1,0x51,0x7f]
          vsubaddph ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vsubaddph ymm2 {k7} {z}, ymm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0xd1,0x52,0x80]
          vsubaddph ymm2 {k7} {z}, ymm3, word ptr [edx - 256]{1to16}

// CHECK: vsubaddph xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddph xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vsubaddph xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddph xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vsubaddph xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xd1,0x10]
          vsubaddph xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vsubaddph xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xd1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vsubaddph xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vsubaddph xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xd1,0x51,0x7f]
          vsubaddph xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vsubaddph xmm2 {k7} {z}, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0xd1,0x52,0x80]
          vsubaddph xmm2 {k7} {z}, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vsubaddps ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0x65,0x28,0xd1,0xd4]
          vsubaddps ymm2, ymm3, ymm4

// CHECK: vsubaddps ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0x65,0x2f,0xd1,0xd4]
          vsubaddps ymm2 {k7}, ymm3, ymm4

// CHECK: vsubaddps ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0x65,0xaf,0xd1,0xd4]
          vsubaddps ymm2 {k7} {z}, ymm3, ymm4

// CHECK: vsubaddps xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0x65,0x08,0xd1,0xd4]
          vsubaddps xmm2, xmm3, xmm4

// CHECK: vsubaddps xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0x65,0x0f,0xd1,0xd4]
          vsubaddps xmm2 {k7}, xmm3, xmm4

// CHECK: vsubaddps xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0x65,0x8f,0xd1,0xd4]
          vsubaddps xmm2 {k7} {z}, xmm3, xmm4

// CHECK: vsubaddps ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0x65,0x28,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddps ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vsubaddps ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0x65,0x2f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddps ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vsubaddps ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf6,0x65,0x38,0xd1,0x10]
          vsubaddps ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK: vsubaddps ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf6,0x65,0x28,0xd1,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vsubaddps ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vsubaddps ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf6,0x65,0xaf,0xd1,0x51,0x7f]
          vsubaddps ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vsubaddps ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf6,0x65,0xbf,0xd1,0x52,0x80]
          vsubaddps ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK: vsubaddps xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0x65,0x08,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddps xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vsubaddps xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0x65,0x0f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddps xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vsubaddps xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf6,0x65,0x18,0xd1,0x10]
          vsubaddps xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK: vsubaddps xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf6,0x65,0x08,0xd1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vsubaddps xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vsubaddps xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf6,0x65,0x8f,0xd1,0x51,0x7f]
          vsubaddps xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vsubaddps xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf6,0x65,0x9f,0xd1,0x52,0x80]
          vsubaddps xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

