// REQUIRES: intel_feature_isa_avx512_reduction
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vphraaddbd xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x43,0xd4]
          vphraaddbd xmm2, xmm3, zmm4

// CHECK: vphraaddbd xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x43,0xd4]
          vphraaddbd xmm2 {k7}, xmm3, zmm4

// CHECK: vphraaddbd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddbd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraaddbd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddbd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraaddbd xmm2, xmm3, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x43,0x10]
          vphraaddbd xmm2, xmm3, zmmword ptr [eax]

// CHECK: vphraaddbd xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x43,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddbd xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraaddbd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x43,0x51,0x7f]
          vphraaddbd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraaddbd xmm2 {k7}, xmm3, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x43,0x52,0x80]
          vphraaddbd xmm2 {k7}, xmm3, zmmword ptr [edx - 8192]

// CHECK: vphraaddsbd xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x44,0xd4]
          vphraaddsbd xmm2, xmm3, zmm4

// CHECK: vphraaddsbd xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x44,0xd4]
          vphraaddsbd xmm2 {k7}, xmm3, zmm4

// CHECK: vphraaddsbd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddsbd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraaddsbd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddsbd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraaddsbd xmm2, xmm3, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x44,0x10]
          vphraaddsbd xmm2, xmm3, zmmword ptr [eax]

// CHECK: vphraaddsbd xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x44,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddsbd xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraaddsbd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x44,0x51,0x7f]
          vphraaddsbd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraaddsbd xmm2 {k7}, xmm3, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x44,0x52,0x80]
          vphraaddsbd xmm2 {k7}, xmm3, zmmword ptr [edx - 8192]

// CHECK: vphraaddswd xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x44,0xd4]
          vphraaddswd xmm2, xmm3, zmm4

// CHECK: vphraaddswd xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x44,0xd4]
          vphraaddswd xmm2 {k7}, xmm3, zmm4

// CHECK: vphraaddswd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddswd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraaddswd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddswd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraaddswd xmm2, xmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xe6,0x58,0x44,0x10]
          vphraaddswd xmm2, xmm3, word ptr [eax]{1to32}

// CHECK: vphraaddswd xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x44,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddswd xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraaddswd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x44,0x51,0x7f]
          vphraaddswd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraaddswd xmm2 {k7}, xmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xe6,0x5f,0x44,0x52,0x80]
          vphraaddswd xmm2 {k7}, xmm3, word ptr [edx - 256]{1to32}

// CHECK: vphraaddwd xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x43,0xd4]
          vphraaddwd xmm2, xmm3, zmm4

// CHECK: vphraaddwd xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x43,0xd4]
          vphraaddwd xmm2 {k7}, xmm3, zmm4

// CHECK: vphraaddwd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddwd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraaddwd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddwd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraaddwd xmm2, xmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xe6,0x58,0x43,0x10]
          vphraaddwd xmm2, xmm3, word ptr [eax]{1to32}

// CHECK: vphraaddwd xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x43,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddwd xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraaddwd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x43,0x51,0x7f]
          vphraaddwd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraaddwd xmm2 {k7}, xmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xe6,0x5f,0x43,0x52,0x80]
          vphraaddwd xmm2 {k7}, xmm3, word ptr [edx - 256]{1to32}

// CHECK: vphraandb xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4d,0xd4]
          vphraandb xmm2, xmm3, zmm4

// CHECK: vphraandb xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4d,0xd4]
          vphraandb xmm2 {k7}, xmm3, zmm4

// CHECK: vphraandb xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandb xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraandb xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandb xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraandb xmm2, xmm3, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4d,0x10]
          vphraandb xmm2, xmm3, zmmword ptr [eax]

// CHECK: vphraandb xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4d,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraandb xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraandb xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4d,0x51,0x7f]
          vphraandb xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraandb xmm2 {k7}, xmm3, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4d,0x52,0x80]
          vphraandb xmm2 {k7}, xmm3, zmmword ptr [edx - 8192]

// CHECK: vphraandd xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x4d,0xd4]
          vphraandd xmm2, xmm3, zmm4

// CHECK: vphraandd xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x4d,0xd4]
          vphraandd xmm2 {k7}, xmm3, zmm4

// CHECK: vphraandd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraandd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraandd xmm2, xmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x4d,0x10]
          vphraandd xmm2, xmm3, dword ptr [eax]{1to16}

// CHECK: vphraandd xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x4d,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraandd xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraandd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x4d,0x51,0x7f]
          vphraandd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraandd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x5f,0x4d,0x52,0x80]
          vphraandd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to16}

// CHECK: vphraandq xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x4d,0xd4]
          vphraandq xmm2, xmm3, zmm4

// CHECK: vphraandq xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x4d,0xd4]
          vphraandq xmm2 {k7}, xmm3, zmm4

// CHECK: vphraandq xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandq xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraandq xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandq xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraandq xmm2, xmm3, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe7,0x58,0x4d,0x10]
          vphraandq xmm2, xmm3, qword ptr [eax]{1to8}

// CHECK: vphraandq xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x4d,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraandq xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraandq xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x4d,0x51,0x7f]
          vphraandq xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraandq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe7,0x5f,0x4d,0x52,0x80]
          vphraandq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to8}

// CHECK: vphraandw xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x4d,0xd4]
          vphraandw xmm2, xmm3, zmm4

// CHECK: vphraandw xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x4d,0xd4]
          vphraandw xmm2 {k7}, xmm3, zmm4

// CHECK: vphraandw xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandw xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraandw xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandw xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraandw xmm2, xmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xe6,0x58,0x4d,0x10]
          vphraandw xmm2, xmm3, word ptr [eax]{1to32}

// CHECK: vphraandw xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x4d,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraandw xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraandw xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x4d,0x51,0x7f]
          vphraandw xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraandw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xe6,0x5f,0x4d,0x52,0x80]
          vphraandw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to32}

// CHECK: vphraddbd xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x43,0xd3]
          vphraddbd xmm2, zmm3

// CHECK: vphraddbd xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x43,0xd3]
          vphraddbd xmm2 {k7}, zmm3

// CHECK: vphraddbd xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddbd xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddbd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddbd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddbd xmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x43,0x10]
          vphraddbd xmm2, zmmword ptr [eax]

// CHECK: vphraddbd xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x43,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraddbd xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphraddbd xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x43,0x51,0x7f]
          vphraddbd xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphraddbd xmm2 {k7}, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x43,0x52,0x80]
          vphraddbd xmm2 {k7}, zmmword ptr [edx - 8192]

// CHECK: vphraddd xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x43,0xd3]
          vphraddd xmm2, zmm3

// CHECK: vphraddd xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x43,0xd3]
          vphraddd xmm2 {k7}, zmm3

// CHECK: vphraddd xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddd xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddd xmm2, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x43,0x10]
          vphraddd xmm2, dword ptr [eax]{1to16}

// CHECK: vphraddd xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x43,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraddd xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphraddd xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x43,0x51,0x7f]
          vphraddd xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphraddd xmm2 {k7}, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x5f,0x43,0x52,0x80]
          vphraddd xmm2 {k7}, dword ptr [edx - 512]{1to16}

// CHECK: vphraddq xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x43,0xd3]
          vphraddq xmm2, zmm3

// CHECK: vphraddq xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x43,0xd3]
          vphraddq xmm2 {k7}, zmm3

// CHECK: vphraddq xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddq xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddq xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddq xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddq xmm2, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x43,0x10]
          vphraddq xmm2, qword ptr [eax]{1to8}

// CHECK: vphraddq xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x43,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraddq xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphraddq xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x43,0x51,0x7f]
          vphraddq xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphraddq xmm2 {k7}, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x5f,0x43,0x52,0x80]
          vphraddq xmm2 {k7}, qword ptr [edx - 1024]{1to8}

// CHECK: vphraddsbd xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x44,0xd3]
          vphraddsbd xmm2, zmm3

// CHECK: vphraddsbd xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x44,0xd3]
          vphraddsbd xmm2 {k7}, zmm3

// CHECK: vphraddsbd xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddsbd xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddsbd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddsbd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddsbd xmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x44,0x10]
          vphraddsbd xmm2, zmmword ptr [eax]

// CHECK: vphraddsbd xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x44,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraddsbd xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphraddsbd xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x44,0x51,0x7f]
          vphraddsbd xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphraddsbd xmm2 {k7}, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x44,0x52,0x80]
          vphraddsbd xmm2 {k7}, zmmword ptr [edx - 8192]

// CHECK: vphraddsd xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x44,0xd3]
          vphraddsd xmm2, zmm3

// CHECK: vphraddsd xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x44,0xd3]
          vphraddsd xmm2 {k7}, zmm3

// CHECK: vphraddsd xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddsd xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddsd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddsd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddsd xmm2, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x44,0x10]
          vphraddsd xmm2, dword ptr [eax]{1to16}

// CHECK: vphraddsd xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x44,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraddsd xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphraddsd xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x44,0x51,0x7f]
          vphraddsd xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphraddsd xmm2 {k7}, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x5f,0x44,0x52,0x80]
          vphraddsd xmm2 {k7}, dword ptr [edx - 512]{1to16}

// CHECK: vphraddsq xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x44,0xd3]
          vphraddsq xmm2, zmm3

// CHECK: vphraddsq xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x44,0xd3]
          vphraddsq xmm2 {k7}, zmm3

// CHECK: vphraddsq xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddsq xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddsq xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddsq xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddsq xmm2, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x44,0x10]
          vphraddsq xmm2, qword ptr [eax]{1to8}

// CHECK: vphraddsq xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x44,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraddsq xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphraddsq xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x44,0x51,0x7f]
          vphraddsq xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphraddsq xmm2 {k7}, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x5f,0x44,0x52,0x80]
          vphraddsq xmm2 {k7}, qword ptr [edx - 1024]{1to8}

// CHECK: vphraddswd xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x44,0xd3]
          vphraddswd xmm2, zmm3

// CHECK: vphraddswd xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x44,0xd3]
          vphraddswd xmm2 {k7}, zmm3

// CHECK: vphraddswd xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddswd xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddswd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddswd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddswd xmm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x44,0x10]
          vphraddswd xmm2, word ptr [eax]{1to32}

// CHECK: vphraddswd xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x44,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraddswd xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphraddswd xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x44,0x51,0x7f]
          vphraddswd xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphraddswd xmm2 {k7}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x5f,0x44,0x52,0x80]
          vphraddswd xmm2 {k7}, word ptr [edx - 256]{1to32}

// CHECK: vphraddwd xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x43,0xd3]
          vphraddwd xmm2, zmm3

// CHECK: vphraddwd xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x43,0xd3]
          vphraddwd xmm2 {k7}, zmm3

// CHECK: vphraddwd xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddwd xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddwd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddwd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddwd xmm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x43,0x10]
          vphraddwd xmm2, word ptr [eax]{1to32}

// CHECK: vphraddwd xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x43,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraddwd xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphraddwd xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x43,0x51,0x7f]
          vphraddwd xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphraddwd xmm2 {k7}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x5f,0x43,0x52,0x80]
          vphraddwd xmm2 {k7}, word ptr [edx - 256]{1to32}

// CHECK: vphramaxsb xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4b,0xd4]
          vphramaxsb xmm2, xmm3, zmm4

// CHECK: vphramaxsb xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4b,0xd4]
          vphramaxsb xmm2 {k7}, xmm3, zmm4

// CHECK: vphramaxsb xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsb xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphramaxsb xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsb xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphramaxsb xmm2, xmm3, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4b,0x10]
          vphramaxsb xmm2, xmm3, zmmword ptr [eax]

// CHECK: vphramaxsb xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsb xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphramaxsb xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4b,0x51,0x7f]
          vphramaxsb xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphramaxsb xmm2 {k7}, xmm3, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4b,0x52,0x80]
          vphramaxsb xmm2 {k7}, xmm3, zmmword ptr [edx - 8192]

// CHECK: vphramaxsd xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x4b,0xd4]
          vphramaxsd xmm2, xmm3, zmm4

// CHECK: vphramaxsd xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x4b,0xd4]
          vphramaxsd xmm2 {k7}, xmm3, zmm4

// CHECK: vphramaxsd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphramaxsd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphramaxsd xmm2, xmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x4b,0x10]
          vphramaxsd xmm2, xmm3, dword ptr [eax]{1to16}

// CHECK: vphramaxsd xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x4b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsd xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphramaxsd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x4b,0x51,0x7f]
          vphramaxsd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphramaxsd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x5f,0x4b,0x52,0x80]
          vphramaxsd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to16}

// CHECK: vphramaxsq xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x4b,0xd4]
          vphramaxsq xmm2, xmm3, zmm4

// CHECK: vphramaxsq xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x4b,0xd4]
          vphramaxsq xmm2 {k7}, xmm3, zmm4

// CHECK: vphramaxsq xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsq xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphramaxsq xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsq xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphramaxsq xmm2, xmm3, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe7,0x58,0x4b,0x10]
          vphramaxsq xmm2, xmm3, qword ptr [eax]{1to8}

// CHECK: vphramaxsq xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x4b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsq xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphramaxsq xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x4b,0x51,0x7f]
          vphramaxsq xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphramaxsq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe7,0x5f,0x4b,0x52,0x80]
          vphramaxsq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to8}

// CHECK: vphramaxsw xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x4b,0xd4]
          vphramaxsw xmm2, xmm3, zmm4

// CHECK: vphramaxsw xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x4b,0xd4]
          vphramaxsw xmm2 {k7}, xmm3, zmm4

// CHECK: vphramaxsw xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsw xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphramaxsw xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsw xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphramaxsw xmm2, xmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xe6,0x58,0x4b,0x10]
          vphramaxsw xmm2, xmm3, word ptr [eax]{1to32}

// CHECK: vphramaxsw xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x4b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsw xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphramaxsw xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x4b,0x51,0x7f]
          vphramaxsw xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphramaxsw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xe6,0x5f,0x4b,0x52,0x80]
          vphramaxsw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to32}

// CHECK: vphraminb xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x48,0xd4]
          vphraminb xmm2, xmm3, zmm4

// CHECK: vphraminb xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x48,0xd4]
          vphraminb xmm2 {k7}, xmm3, zmm4

// CHECK: vphraminb xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminb xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminb xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminb xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminb xmm2, xmm3, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x48,0x10]
          vphraminb xmm2, xmm3, zmmword ptr [eax]

// CHECK: vphraminb xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x48,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminb xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraminb xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x48,0x51,0x7f]
          vphraminb xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraminb xmm2 {k7}, xmm3, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x48,0x52,0x80]
          vphraminb xmm2 {k7}, xmm3, zmmword ptr [edx - 8192]

// CHECK: vphramind xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x48,0xd4]
          vphramind xmm2, xmm3, zmm4

// CHECK: vphramind xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x48,0xd4]
          vphramind xmm2 {k7}, xmm3, zmm4

// CHECK: vphramind xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramind xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphramind xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramind xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphramind xmm2, xmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x48,0x10]
          vphramind xmm2, xmm3, dword ptr [eax]{1to16}

// CHECK: vphramind xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x48,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphramind xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphramind xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x48,0x51,0x7f]
          vphramind xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphramind xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x5f,0x48,0x52,0x80]
          vphramind xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to16}

// CHECK: vphraminq xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x48,0xd4]
          vphraminq xmm2, xmm3, zmm4

// CHECK: vphraminq xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x48,0xd4]
          vphraminq xmm2 {k7}, xmm3, zmm4

// CHECK: vphraminq xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminq xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminq xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminq xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminq xmm2, xmm3, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe7,0x58,0x48,0x10]
          vphraminq xmm2, xmm3, qword ptr [eax]{1to8}

// CHECK: vphraminq xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x48,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminq xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraminq xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x48,0x51,0x7f]
          vphraminq xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraminq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe7,0x5f,0x48,0x52,0x80]
          vphraminq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to8}

// CHECK: vphraminsb xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x49,0xd4]
          vphraminsb xmm2, xmm3, zmm4

// CHECK: vphraminsb xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x49,0xd4]
          vphraminsb xmm2 {k7}, xmm3, zmm4

// CHECK: vphraminsb xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsb xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminsb xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsb xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminsb xmm2, xmm3, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x49,0x10]
          vphraminsb xmm2, xmm3, zmmword ptr [eax]

// CHECK: vphraminsb xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x49,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsb xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraminsb xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x49,0x51,0x7f]
          vphraminsb xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraminsb xmm2 {k7}, xmm3, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x49,0x52,0x80]
          vphraminsb xmm2 {k7}, xmm3, zmmword ptr [edx - 8192]

// CHECK: vphraminsd xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x49,0xd4]
          vphraminsd xmm2, xmm3, zmm4

// CHECK: vphraminsd xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x49,0xd4]
          vphraminsd xmm2 {k7}, xmm3, zmm4

// CHECK: vphraminsd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsd xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminsd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsd xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminsd xmm2, xmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x49,0x10]
          vphraminsd xmm2, xmm3, dword ptr [eax]{1to16}

// CHECK: vphraminsd xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x49,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsd xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraminsd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x49,0x51,0x7f]
          vphraminsd xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraminsd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x5f,0x49,0x52,0x80]
          vphraminsd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to16}

// CHECK: vphraminsq xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x49,0xd4]
          vphraminsq xmm2, xmm3, zmm4

// CHECK: vphraminsq xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x49,0xd4]
          vphraminsq xmm2 {k7}, xmm3, zmm4

// CHECK: vphraminsq xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsq xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminsq xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsq xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminsq xmm2, xmm3, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe7,0x58,0x49,0x10]
          vphraminsq xmm2, xmm3, qword ptr [eax]{1to8}

// CHECK: vphraminsq xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x49,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsq xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraminsq xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x49,0x51,0x7f]
          vphraminsq xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraminsq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe7,0x5f,0x49,0x52,0x80]
          vphraminsq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to8}

// CHECK: vphraminsw xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x49,0xd4]
          vphraminsw xmm2, xmm3, zmm4

// CHECK: vphraminsw xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x49,0xd4]
          vphraminsw xmm2 {k7}, xmm3, zmm4

// CHECK: vphraminsw xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsw xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminsw xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsw xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminsw xmm2, xmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xe6,0x58,0x49,0x10]
          vphraminsw xmm2, xmm3, word ptr [eax]{1to32}

// CHECK: vphraminsw xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x49,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsw xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraminsw xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x49,0x51,0x7f]
          vphraminsw xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraminsw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xe6,0x5f,0x49,0x52,0x80]
          vphraminsw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to32}

// CHECK: vphraminw xmm2, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x48,0xd4]
          vphraminw xmm2, xmm3, zmm4

// CHECK: vphraminw xmm2 {k7}, xmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x48,0xd4]
          vphraminw xmm2 {k7}, xmm3, zmm4

// CHECK: vphraminw xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminw xmm2, xmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminw xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminw xmm2 {k7}, xmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminw xmm2, xmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xe6,0x58,0x48,0x10]
          vphraminw xmm2, xmm3, word ptr [eax]{1to32}

// CHECK: vphraminw xmm2, xmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x48,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminw xmm2, xmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vphraminw xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x48,0x51,0x7f]
          vphraminw xmm2 {k7}, xmm3, zmmword ptr [ecx + 8128]

// CHECK: vphraminw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xe6,0x5f,0x48,0x52,0x80]
          vphraminw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to32}

// CHECK: vphrandb xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4d,0xd3]
          vphrandb xmm2, zmm3

// CHECK: vphrandb xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrandb xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrandb xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrandb xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrandb xmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4d,0x10]
          vphrandb xmm2, zmmword ptr [eax]

// CHECK: vphrandb xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4d,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrandb xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrandb xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4d,0x51,0x7f]
          vphrandb xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrandb xmm2, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4d,0x52,0x80]
          vphrandb xmm2, zmmword ptr [edx - 8192]

// CHECK: vphrandd xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4d,0xd3]
          vphrandd xmm2, zmm3

// CHECK: vphrandd xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrandd xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrandd xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrandd xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrandd xmm2, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x4d,0x10]
          vphrandd xmm2, dword ptr [eax]{1to16}

// CHECK: vphrandd xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4d,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrandd xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrandd xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4d,0x51,0x7f]
          vphrandd xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrandd xmm2, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x4d,0x52,0x80]
          vphrandd xmm2, dword ptr [edx - 512]{1to16}

// CHECK: vphranddq xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x45,0xd3]
          vphranddq xmm2, zmm3

// CHECK: vphranddq xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x45,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphranddq xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphranddq xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x45,0x94,0x87,0x23,0x01,0x00,0x00]
          vphranddq xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphranddq xmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x45,0x10]
          vphranddq xmm2, zmmword ptr [eax]

// CHECK: vphranddq xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x45,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphranddq xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphranddq xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x45,0x51,0x7f]
          vphranddq xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphranddq xmm2, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x45,0x52,0x80]
          vphranddq xmm2, zmmword ptr [edx - 8192]

// CHECK: vphrandq xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4d,0xd3]
          vphrandq xmm2, zmm3

// CHECK: vphrandq xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrandq xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrandq xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrandq xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrandq xmm2, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x4d,0x10]
          vphrandq xmm2, qword ptr [eax]{1to8}

// CHECK: vphrandq xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4d,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrandq xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrandq xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4d,0x51,0x7f]
          vphrandq xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrandq xmm2, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x4d,0x52,0x80]
          vphrandq xmm2, qword ptr [edx - 1024]{1to8}

// CHECK: vphrandqq ymm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x45,0xd3]
          vphrandqq ymm2, zmm3

// CHECK: vphrandqq ymm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x45,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrandqq ymm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrandqq ymm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x45,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrandqq ymm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrandqq ymm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x45,0x10]
          vphrandqq ymm2, zmmword ptr [eax]

// CHECK: vphrandqq ymm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x45,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrandqq ymm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrandqq ymm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x45,0x51,0x7f]
          vphrandqq ymm2, zmmword ptr [ecx + 8128]

// CHECK: vphrandqq ymm2, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x45,0x52,0x80]
          vphrandqq ymm2, zmmword ptr [edx - 8192]

// CHECK: vphrandw xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4d,0xd3]
          vphrandw xmm2, zmm3

// CHECK: vphrandw xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrandw xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrandw xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrandw xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrandw xmm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x4d,0x10]
          vphrandw xmm2, word ptr [eax]{1to32}

// CHECK: vphrandw xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4d,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrandw xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrandw xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4d,0x51,0x7f]
          vphrandw xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrandw xmm2, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x4d,0x52,0x80]
          vphrandw xmm2, word ptr [edx - 256]{1to32}

// CHECK: vphrmaxb xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4a,0xd3]
          vphrmaxb xmm2, zmm3

// CHECK: vphrmaxb xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x4a,0xd3]
          vphrmaxb xmm2 {k7}, zmm3

// CHECK: vphrmaxb xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxb xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxb xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x4a,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxb xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxb xmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4a,0x10]
          vphrmaxb xmm2, zmmword ptr [eax]

// CHECK: vphrmaxb xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4a,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxb xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrmaxb xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x4a,0x51,0x7f]
          vphrmaxb xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphrmaxb xmm2 {k7}, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x4a,0x52,0x80]
          vphrmaxb xmm2 {k7}, zmmword ptr [edx - 8192]

// CHECK: vphrmaxd xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4a,0xd3]
          vphrmaxd xmm2, zmm3

// CHECK: vphrmaxd xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x4a,0xd3]
          vphrmaxd xmm2 {k7}, zmm3

// CHECK: vphrmaxd xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxd xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x4a,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxd xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxd xmm2, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x4a,0x10]
          vphrmaxd xmm2, dword ptr [eax]{1to16}

// CHECK: vphrmaxd xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4a,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxd xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrmaxd xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x4a,0x51,0x7f]
          vphrmaxd xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphrmaxd xmm2 {k7}, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x5f,0x4a,0x52,0x80]
          vphrmaxd xmm2 {k7}, dword ptr [edx - 512]{1to16}

// CHECK: vphrmaxq xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4a,0xd3]
          vphrmaxq xmm2, zmm3

// CHECK: vphrmaxq xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x4a,0xd3]
          vphrmaxq xmm2 {k7}, zmm3

// CHECK: vphrmaxq xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxq xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxq xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x4a,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxq xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxq xmm2, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x4a,0x10]
          vphrmaxq xmm2, qword ptr [eax]{1to8}

// CHECK: vphrmaxq xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4a,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxq xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrmaxq xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x4a,0x51,0x7f]
          vphrmaxq xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphrmaxq xmm2 {k7}, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x5f,0x4a,0x52,0x80]
          vphrmaxq xmm2 {k7}, qword ptr [edx - 1024]{1to8}

// CHECK: vphrmaxsb xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4b,0xd3]
          vphrmaxsb xmm2, zmm3

// CHECK: vphrmaxsb xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxsb xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxsb xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxsb xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxsb xmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4b,0x10]
          vphrmaxsb xmm2, zmmword ptr [eax]

// CHECK: vphrmaxsb xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxsb xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrmaxsb xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4b,0x51,0x7f]
          vphrmaxsb xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrmaxsb xmm2, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4b,0x52,0x80]
          vphrmaxsb xmm2, zmmword ptr [edx - 8192]

// CHECK: vphrmaxsd xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4b,0xd3]
          vphrmaxsd xmm2, zmm3

// CHECK: vphrmaxsd xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxsd xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxsd xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxsd xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxsd xmm2, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x4b,0x10]
          vphrmaxsd xmm2, dword ptr [eax]{1to16}

// CHECK: vphrmaxsd xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxsd xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrmaxsd xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4b,0x51,0x7f]
          vphrmaxsd xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrmaxsd xmm2, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x4b,0x52,0x80]
          vphrmaxsd xmm2, dword ptr [edx - 512]{1to16}

// CHECK: vphrmaxsq xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4b,0xd3]
          vphrmaxsq xmm2, zmm3

// CHECK: vphrmaxsq xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxsq xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxsq xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxsq xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxsq xmm2, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x4b,0x10]
          vphrmaxsq xmm2, qword ptr [eax]{1to8}

// CHECK: vphrmaxsq xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxsq xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrmaxsq xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4b,0x51,0x7f]
          vphrmaxsq xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrmaxsq xmm2, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x4b,0x52,0x80]
          vphrmaxsq xmm2, qword ptr [edx - 1024]{1to8}

// CHECK: vphrmaxsw xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4b,0xd3]
          vphrmaxsw xmm2, zmm3

// CHECK: vphrmaxsw xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxsw xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxsw xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxsw xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxsw xmm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x4b,0x10]
          vphrmaxsw xmm2, word ptr [eax]{1to32}

// CHECK: vphrmaxsw xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxsw xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrmaxsw xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4b,0x51,0x7f]
          vphrmaxsw xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrmaxsw xmm2, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x4b,0x52,0x80]
          vphrmaxsw xmm2, word ptr [edx - 256]{1to32}

// CHECK: vphrmaxw xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4a,0xd3]
          vphrmaxw xmm2, zmm3

// CHECK: vphrmaxw xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x4a,0xd3]
          vphrmaxw xmm2 {k7}, zmm3

// CHECK: vphrmaxw xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxw xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxw xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x4a,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxw xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxw xmm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x4a,0x10]
          vphrmaxw xmm2, word ptr [eax]{1to32}

// CHECK: vphrmaxw xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4a,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxw xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrmaxw xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x4a,0x51,0x7f]
          vphrmaxw xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphrmaxw xmm2 {k7}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x5f,0x4a,0x52,0x80]
          vphrmaxw xmm2 {k7}, word ptr [edx - 256]{1to32}

// CHECK: vphrminb xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x48,0xd3]
          vphrminb xmm2, zmm3

// CHECK: vphrminb xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminb xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminb xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminb xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminb xmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x48,0x10]
          vphrminb xmm2, zmmword ptr [eax]

// CHECK: vphrminb xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x48,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrminb xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrminb xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x48,0x51,0x7f]
          vphrminb xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrminb xmm2, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x48,0x52,0x80]
          vphrminb xmm2, zmmword ptr [edx - 8192]

// CHECK: vphrmind xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x48,0xd3]
          vphrmind xmm2, zmm3

// CHECK: vphrmind xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmind xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmind xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmind xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmind xmm2, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x48,0x10]
          vphrmind xmm2, dword ptr [eax]{1to16}

// CHECK: vphrmind xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x48,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrmind xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrmind xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x48,0x51,0x7f]
          vphrmind xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrmind xmm2, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x48,0x52,0x80]
          vphrmind xmm2, dword ptr [edx - 512]{1to16}

// CHECK: vphrminq xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x48,0xd3]
          vphrminq xmm2, zmm3

// CHECK: vphrminq xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminq xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminq xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminq xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminq xmm2, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x48,0x10]
          vphrminq xmm2, qword ptr [eax]{1to8}

// CHECK: vphrminq xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x48,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrminq xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrminq xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x48,0x51,0x7f]
          vphrminq xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrminq xmm2, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x48,0x52,0x80]
          vphrminq xmm2, qword ptr [edx - 1024]{1to8}

// CHECK: vphrminsb xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x49,0xd3]
          vphrminsb xmm2, zmm3

// CHECK: vphrminsb xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminsb xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminsb xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminsb xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminsb xmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x49,0x10]
          vphrminsb xmm2, zmmword ptr [eax]

// CHECK: vphrminsb xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x49,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrminsb xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrminsb xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x49,0x51,0x7f]
          vphrminsb xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrminsb xmm2, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x49,0x52,0x80]
          vphrminsb xmm2, zmmword ptr [edx - 8192]

// CHECK: vphrminsd xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x49,0xd3]
          vphrminsd xmm2, zmm3

// CHECK: vphrminsd xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminsd xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminsd xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminsd xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminsd xmm2, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x49,0x10]
          vphrminsd xmm2, dword ptr [eax]{1to16}

// CHECK: vphrminsd xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x49,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrminsd xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrminsd xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x49,0x51,0x7f]
          vphrminsd xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrminsd xmm2, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x49,0x52,0x80]
          vphrminsd xmm2, dword ptr [edx - 512]{1to16}

// CHECK: vphrminsq xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x49,0xd3]
          vphrminsq xmm2, zmm3

// CHECK: vphrminsq xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminsq xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminsq xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminsq xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminsq xmm2, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x49,0x10]
          vphrminsq xmm2, qword ptr [eax]{1to8}

// CHECK: vphrminsq xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x49,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrminsq xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrminsq xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x49,0x51,0x7f]
          vphrminsq xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrminsq xmm2, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x49,0x52,0x80]
          vphrminsq xmm2, qword ptr [edx - 1024]{1to8}

// CHECK: vphrminsw xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x49,0xd3]
          vphrminsw xmm2, zmm3

// CHECK: vphrminsw xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminsw xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminsw xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminsw xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminsw xmm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x49,0x10]
          vphrminsw xmm2, word ptr [eax]{1to32}

// CHECK: vphrminsw xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x49,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrminsw xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrminsw xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x49,0x51,0x7f]
          vphrminsw xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrminsw xmm2, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x49,0x52,0x80]
          vphrminsw xmm2, word ptr [edx - 256]{1to32}

// CHECK: vphrminw xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x48,0xd3]
          vphrminw xmm2, zmm3

// CHECK: vphrminw xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminw xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminw xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminw xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminw xmm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x48,0x10]
          vphrminw xmm2, word ptr [eax]{1to32}

// CHECK: vphrminw xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x48,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrminw xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrminw xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x48,0x51,0x7f]
          vphrminw xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrminw xmm2, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x48,0x52,0x80]
          vphrminw xmm2, word ptr [edx - 256]{1to32}

// CHECK: vphrorb xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4e,0xd3]
          vphrorb xmm2, zmm3

// CHECK: vphrorb xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x4e,0xd3]
          vphrorb xmm2 {k7}, zmm3

// CHECK: vphrorb xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrorb xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrorb xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x4e,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrorb xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrorb xmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4e,0x10]
          vphrorb xmm2, zmmword ptr [eax]

// CHECK: vphrorb xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4e,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrorb xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrorb xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x4e,0x51,0x7f]
          vphrorb xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphrorb xmm2 {k7}, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x4e,0x52,0x80]
          vphrorb xmm2 {k7}, zmmword ptr [edx - 8192]

// CHECK: vphrord xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4e,0xd3]
          vphrord xmm2, zmm3

// CHECK: vphrord xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x4e,0xd3]
          vphrord xmm2 {k7}, zmm3

// CHECK: vphrord xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrord xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrord xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x4e,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrord xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrord xmm2, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x4e,0x10]
          vphrord xmm2, dword ptr [eax]{1to16}

// CHECK: vphrord xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4e,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrord xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrord xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x4e,0x51,0x7f]
          vphrord xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphrord xmm2 {k7}, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x5f,0x4e,0x52,0x80]
          vphrord xmm2 {k7}, dword ptr [edx - 512]{1to16}

// CHECK: vphrordq xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x46,0xd3]
          vphrordq xmm2, zmm3

// CHECK: vphrordq xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x46,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrordq xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrordq xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x46,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrordq xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrordq xmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x46,0x10]
          vphrordq xmm2, zmmword ptr [eax]

// CHECK: vphrordq xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x46,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrordq xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrordq xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x46,0x51,0x7f]
          vphrordq xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrordq xmm2, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x46,0x52,0x80]
          vphrordq xmm2, zmmword ptr [edx - 8192]

// CHECK: vphrorq xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4e,0xd3]
          vphrorq xmm2, zmm3

// CHECK: vphrorq xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x4e,0xd3]
          vphrorq xmm2 {k7}, zmm3

// CHECK: vphrorq xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrorq xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrorq xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x4e,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrorq xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrorq xmm2, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x4e,0x10]
          vphrorq xmm2, qword ptr [eax]{1to8}

// CHECK: vphrorq xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4e,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrorq xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrorq xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x4e,0x51,0x7f]
          vphrorq xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphrorq xmm2 {k7}, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x5f,0x4e,0x52,0x80]
          vphrorq xmm2 {k7}, qword ptr [edx - 1024]{1to8}

// CHECK: vphrorqq ymm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x46,0xd3]
          vphrorqq ymm2, zmm3

// CHECK: vphrorqq ymm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x46,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrorqq ymm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrorqq ymm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x46,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrorqq ymm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrorqq ymm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x46,0x10]
          vphrorqq ymm2, zmmword ptr [eax]

// CHECK: vphrorqq ymm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x46,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrorqq ymm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrorqq ymm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x46,0x51,0x7f]
          vphrorqq ymm2, zmmword ptr [ecx + 8128]

// CHECK: vphrorqq ymm2, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x46,0x52,0x80]
          vphrorqq ymm2, zmmword ptr [edx - 8192]

// CHECK: vphrorw xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4e,0xd3]
          vphrorw xmm2, zmm3

// CHECK: vphrorw xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x4e,0xd3]
          vphrorw xmm2 {k7}, zmm3

// CHECK: vphrorw xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrorw xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrorw xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x4e,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrorw xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrorw xmm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x4e,0x10]
          vphrorw xmm2, word ptr [eax]{1to32}

// CHECK: vphrorw xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4e,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrorw xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrorw xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x4e,0x51,0x7f]
          vphrorw xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphrorw xmm2 {k7}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x5f,0x4e,0x52,0x80]
          vphrorw xmm2 {k7}, word ptr [edx - 256]{1to32}

// CHECK: vphrxorb xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4f,0xd3]
          vphrxorb xmm2, zmm3

// CHECK: vphrxorb xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x4f,0xd3]
          vphrxorb xmm2 {k7}, zmm3

// CHECK: vphrxorb xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4f,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrxorb xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrxorb xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x4f,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrxorb xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrxorb xmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4f,0x10]
          vphrxorb xmm2, zmmword ptr [eax]

// CHECK: vphrxorb xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x4f,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrxorb xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrxorb xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x4f,0x51,0x7f]
          vphrxorb xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphrxorb xmm2 {k7}, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x4f,0x52,0x80]
          vphrxorb xmm2 {k7}, zmmword ptr [edx - 8192]

// CHECK: vphrxord xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4f,0xd3]
          vphrxord xmm2, zmm3

// CHECK: vphrxord xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x4f,0xd3]
          vphrxord xmm2 {k7}, zmm3

// CHECK: vphrxord xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4f,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrxord xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrxord xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x4f,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrxord xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrxord xmm2, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x58,0x4f,0x10]
          vphrxord xmm2, dword ptr [eax]{1to16}

// CHECK: vphrxord xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x4f,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrxord xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrxord xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7d,0x4f,0x4f,0x51,0x7f]
          vphrxord xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphrxord xmm2 {k7}, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7d,0x5f,0x4f,0x52,0x80]
          vphrxord xmm2 {k7}, dword ptr [edx - 512]{1to16}

// CHECK: vphrxordq xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x47,0xd3]
          vphrxordq xmm2, zmm3

// CHECK: vphrxordq xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x47,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrxordq xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrxordq xmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x47,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrxordq xmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrxordq xmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x47,0x10]
          vphrxordq xmm2, zmmword ptr [eax]

// CHECK: vphrxordq xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x47,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrxordq xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrxordq xmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x47,0x51,0x7f]
          vphrxordq xmm2, zmmword ptr [ecx + 8128]

// CHECK: vphrxordq xmm2, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0x7d,0x48,0x47,0x52,0x80]
          vphrxordq xmm2, zmmword ptr [edx - 8192]

// CHECK: vphrxorq xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4f,0xd3]
          vphrxorq xmm2, zmm3

// CHECK: vphrxorq xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x4f,0xd3]
          vphrxorq xmm2 {k7}, zmm3

// CHECK: vphrxorq xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4f,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrxorq xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrxorq xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x4f,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrxorq xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrxorq xmm2, qword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x58,0x4f,0x10]
          vphrxorq xmm2, qword ptr [eax]{1to8}

// CHECK: vphrxorq xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x4f,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrxorq xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrxorq xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfd,0x4f,0x4f,0x51,0x7f]
          vphrxorq xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphrxorq xmm2 {k7}, qword ptr [edx - 1024]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfd,0x5f,0x4f,0x52,0x80]
          vphrxorq xmm2 {k7}, qword ptr [edx - 1024]{1to8}

// CHECK: vphrxorqq ymm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x47,0xd3]
          vphrxorqq ymm2, zmm3

// CHECK: vphrxorqq ymm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x47,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrxorqq ymm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrxorqq ymm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x47,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrxorqq ymm2, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrxorqq ymm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x47,0x10]
          vphrxorqq ymm2, zmmword ptr [eax]

// CHECK: vphrxorqq ymm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x47,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrxorqq ymm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrxorqq ymm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x47,0x51,0x7f]
          vphrxorqq ymm2, zmmword ptr [ecx + 8128]

// CHECK: vphrxorqq ymm2, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf5,0xfd,0x48,0x47,0x52,0x80]
          vphrxorqq ymm2, zmmword ptr [edx - 8192]

// CHECK: vphrxorw xmm2, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4f,0xd3]
          vphrxorw xmm2, zmm3

// CHECK: vphrxorw xmm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x4f,0xd3]
          vphrxorw xmm2 {k7}, zmm3

// CHECK: vphrxorw xmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4f,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrxorw xmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrxorw xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x4f,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrxorw xmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vphrxorw xmm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x58,0x4f,0x10]
          vphrxorw xmm2, word ptr [eax]{1to32}

// CHECK: vphrxorw xmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0xfc,0x48,0x4f,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphrxorw xmm2, zmmword ptr [2*ebp - 2048]

// CHECK: vphrxorw xmm2 {k7}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0xfc,0x4f,0x4f,0x51,0x7f]
          vphrxorw xmm2 {k7}, zmmword ptr [ecx + 8128]

// CHECK: vphrxorw xmm2 {k7}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0xfc,0x5f,0x4f,0x52,0x80]
          vphrxorw xmm2 {k7}, word ptr [edx - 256]{1to32}

