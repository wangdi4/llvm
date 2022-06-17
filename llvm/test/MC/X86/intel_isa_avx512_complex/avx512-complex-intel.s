// REQUIRES: intel_feature_isa_avx512_complex
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vaddsubpd zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0xe5,0x48,0xd0,0xd4]
          vaddsubpd zmm2, zmm3, zmm4

// CHECK: vaddsubpd zmm2, zmm3, zmm4, {rn-sae}
// CHECK: encoding: [0x62,0xf6,0xe5,0x18,0xd0,0xd4]
          vaddsubpd zmm2, zmm3, zmm4, {rn-sae}

// CHECK: vaddsubpd zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0xe5,0x4f,0xd0,0xd4]
          vaddsubpd zmm2 {k7}, zmm3, zmm4

// CHECK: vaddsubpd zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}
// CHECK: encoding: [0x62,0xf6,0xe5,0xff,0xd0,0xd4]
          vaddsubpd zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}

// CHECK: vaddsubpd zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0xe5,0x48,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubpd zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vaddsubpd zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0xe5,0x4f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubpd zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vaddsubpd zmm2, zmm3, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf6,0xe5,0x58,0xd0,0x10]
          vaddsubpd zmm2, zmm3, qword ptr [eax]{1to8}

// CHECK: vaddsubpd zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf6,0xe5,0x48,0xd0,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vaddsubpd zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vaddsubpd zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf6,0xe5,0xcf,0xd0,0x51,0x7f]
          vaddsubpd zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vaddsubpd zmm2 {k7} {z}, zmm3, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf6,0xe5,0xdf,0xd0,0x52,0x80]
          vaddsubpd zmm2 {k7} {z}, zmm3, qword ptr [edx - 1024]{1to8}

// CHECK: vaddsubph zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0x64,0x48,0xd0,0xd4]
          vaddsubph zmm2, zmm3, zmm4

// CHECK: vaddsubph zmm2, zmm3, zmm4, {rn-sae}
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xd0,0xd4]
          vaddsubph zmm2, zmm3, zmm4, {rn-sae}

// CHECK: vaddsubph zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0x64,0x4f,0xd0,0xd4]
          vaddsubph zmm2 {k7}, zmm3, zmm4

// CHECK: vaddsubph zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}
// CHECK: encoding: [0x62,0xf6,0x64,0xff,0xd0,0xd4]
          vaddsubph zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}

// CHECK: vaddsubph zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0x64,0x48,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubph zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vaddsubph zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0x64,0x4f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubph zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vaddsubph zmm2, zmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf6,0x64,0x58,0xd0,0x10]
          vaddsubph zmm2, zmm3, word ptr [eax]{1to32}

// CHECK: vaddsubph zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf6,0x64,0x48,0xd0,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vaddsubph zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vaddsubph zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf6,0x64,0xcf,0xd0,0x51,0x7f]
          vaddsubph zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vaddsubph zmm2 {k7} {z}, zmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf6,0x64,0xdf,0xd0,0x52,0x80]
          vaddsubph zmm2 {k7} {z}, zmm3, word ptr [edx - 256]{1to32}

// CHECK: vaddsubps zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0x65,0x48,0xd0,0xd4]
          vaddsubps zmm2, zmm3, zmm4

// CHECK: vaddsubps zmm2, zmm3, zmm4, {rn-sae}
// CHECK: encoding: [0x62,0xf6,0x65,0x18,0xd0,0xd4]
          vaddsubps zmm2, zmm3, zmm4, {rn-sae}

// CHECK: vaddsubps zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0x65,0x4f,0xd0,0xd4]
          vaddsubps zmm2 {k7}, zmm3, zmm4

// CHECK: vaddsubps zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}
// CHECK: encoding: [0x62,0xf6,0x65,0xff,0xd0,0xd4]
          vaddsubps zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}

// CHECK: vaddsubps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0x65,0x48,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vaddsubps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0x65,0x4f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vaddsubps zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf6,0x65,0x58,0xd0,0x10]
          vaddsubps zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK: vaddsubps zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf6,0x65,0x48,0xd0,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vaddsubps zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vaddsubps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf6,0x65,0xcf,0xd0,0x51,0x7f]
          vaddsubps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vaddsubps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf6,0x65,0xdf,0xd0,0x52,0x80]
          vaddsubps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK: vmovdhdup zmm2, zmm3
// CHECK: encoding: [0x62,0xf1,0xff,0x48,0x16,0xd3]
          vmovdhdup zmm2, zmm3

// CHECK: vmovdhdup zmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf1,0xff,0x4f,0x16,0xd3]
          vmovdhdup zmm2 {k7}, zmm3

// CHECK: vmovdhdup zmm2 {k7} {z}, zmm3
// CHECK: encoding: [0x62,0xf1,0xff,0xcf,0x16,0xd3]
          vmovdhdup zmm2 {k7} {z}, zmm3

// CHECK: vmovdhdup zmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf1,0xff,0x48,0x16,0x94,0xf4,0x00,0x00,0x00,0x10]
          vmovdhdup zmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vmovdhdup zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf1,0xff,0x4f,0x16,0x94,0x87,0x23,0x01,0x00,0x00]
          vmovdhdup zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vmovdhdup zmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf1,0xff,0x48,0x16,0x10]
          vmovdhdup zmm2, zmmword ptr [eax]

// CHECK: vmovdhdup zmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf1,0xff,0x48,0x16,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vmovdhdup zmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vmovdhdup zmm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf1,0xff,0xcf,0x16,0x51,0x7f]
          vmovdhdup zmm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK: vmovdhdup zmm2 {k7} {z}, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf1,0xff,0xcf,0x16,0x52,0x80]
          vmovdhdup zmm2 {k7} {z}, zmmword ptr [edx - 8192]

// CHECK: vsubaddpd zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0xe5,0x48,0xd1,0xd4]
          vsubaddpd zmm2, zmm3, zmm4

// CHECK: vsubaddpd zmm2, zmm3, zmm4, {rn-sae}
// CHECK: encoding: [0x62,0xf6,0xe5,0x18,0xd1,0xd4]
          vsubaddpd zmm2, zmm3, zmm4, {rn-sae}

// CHECK: vsubaddpd zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0xe5,0x4f,0xd1,0xd4]
          vsubaddpd zmm2 {k7}, zmm3, zmm4

// CHECK: vsubaddpd zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}
// CHECK: encoding: [0x62,0xf6,0xe5,0xff,0xd1,0xd4]
          vsubaddpd zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}

// CHECK: vsubaddpd zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0xe5,0x48,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddpd zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vsubaddpd zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0xe5,0x4f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddpd zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vsubaddpd zmm2, zmm3, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf6,0xe5,0x58,0xd1,0x10]
          vsubaddpd zmm2, zmm3, qword ptr [eax]{1to8}

// CHECK: vsubaddpd zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf6,0xe5,0x48,0xd1,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vsubaddpd zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vsubaddpd zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf6,0xe5,0xcf,0xd1,0x51,0x7f]
          vsubaddpd zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vsubaddpd zmm2 {k7} {z}, zmm3, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf6,0xe5,0xdf,0xd1,0x52,0x80]
          vsubaddpd zmm2 {k7} {z}, zmm3, qword ptr [edx - 1024]{1to8}

// CHECK: vsubaddph zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0x64,0x48,0xd1,0xd4]
          vsubaddph zmm2, zmm3, zmm4

// CHECK: vsubaddph zmm2, zmm3, zmm4, {rn-sae}
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xd1,0xd4]
          vsubaddph zmm2, zmm3, zmm4, {rn-sae}

// CHECK: vsubaddph zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0x64,0x4f,0xd1,0xd4]
          vsubaddph zmm2 {k7}, zmm3, zmm4

// CHECK: vsubaddph zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}
// CHECK: encoding: [0x62,0xf6,0x64,0xff,0xd1,0xd4]
          vsubaddph zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}

// CHECK: vsubaddph zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0x64,0x48,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddph zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vsubaddph zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0x64,0x4f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddph zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vsubaddph zmm2, zmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf6,0x64,0x58,0xd1,0x10]
          vsubaddph zmm2, zmm3, word ptr [eax]{1to32}

// CHECK: vsubaddph zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf6,0x64,0x48,0xd1,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vsubaddph zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vsubaddph zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf6,0x64,0xcf,0xd1,0x51,0x7f]
          vsubaddph zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vsubaddph zmm2 {k7} {z}, zmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf6,0x64,0xdf,0xd1,0x52,0x80]
          vsubaddph zmm2 {k7} {z}, zmm3, word ptr [edx - 256]{1to32}

// CHECK: vsubaddps zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0x65,0x48,0xd1,0xd4]
          vsubaddps zmm2, zmm3, zmm4

// CHECK: vsubaddps zmm2, zmm3, zmm4, {rn-sae}
// CHECK: encoding: [0x62,0xf6,0x65,0x18,0xd1,0xd4]
          vsubaddps zmm2, zmm3, zmm4, {rn-sae}

// CHECK: vsubaddps zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0x65,0x4f,0xd1,0xd4]
          vsubaddps zmm2 {k7}, zmm3, zmm4

// CHECK: vsubaddps zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}
// CHECK: encoding: [0x62,0xf6,0x65,0xff,0xd1,0xd4]
          vsubaddps zmm2 {k7} {z}, zmm3, zmm4, {rz-sae}

// CHECK: vsubaddps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf6,0x65,0x48,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vsubaddps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf6,0x65,0x4f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vsubaddps zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf6,0x65,0x58,0xd1,0x10]
          vsubaddps zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK: vsubaddps zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf6,0x65,0x48,0xd1,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vsubaddps zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vsubaddps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf6,0x65,0xcf,0xd1,0x51,0x7f]
          vsubaddps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vsubaddps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf6,0x65,0xdf,0xd1,0x52,0x80]
          vsubaddps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

