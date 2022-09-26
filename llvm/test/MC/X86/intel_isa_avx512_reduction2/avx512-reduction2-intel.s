// REQUIRES: intel_feature_isa_avx512_reduction2
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

