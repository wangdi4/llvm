// REQUIRES: intel_feature_isa_avx512_reduction
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vphraaddbd xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x43,0xd4]
          vphraaddbd xmm2, xmm3, xmm4

// CHECK: vphraaddbd xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x43,0xd4]
          vphraaddbd xmm2 {k7}, xmm3, xmm4

// CHECK: vphraaddbd xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x43,0xd4]
          vphraaddbd xmm2, xmm3, ymm4

// CHECK: vphraaddbd xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x43,0xd4]
          vphraaddbd xmm2 {k7}, xmm3, ymm4

// CHECK: vphraaddbd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddbd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraaddbd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddbd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraaddbd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x43,0x10]
          vphraaddbd xmm2, xmm3, xmmword ptr [eax]

// CHECK: vphraaddbd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x43,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddbd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraaddbd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x43,0x51,0x7f]
          vphraaddbd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraaddbd xmm2 {k7}, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x43,0x52,0x80]
          vphraaddbd xmm2 {k7}, xmm3, xmmword ptr [edx - 2048]

// CHECK: vphraaddbd xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x43,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddbd xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraaddbd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x43,0x51,0x7f]
          vphraaddbd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraaddbd xmm2 {k7}, xmm3, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x43,0x52,0x80]
          vphraaddbd xmm2 {k7}, xmm3, ymmword ptr [edx - 4096]

// CHECK: vphraaddsbd xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x44,0xd4]
          vphraaddsbd xmm2, xmm3, xmm4

// CHECK: vphraaddsbd xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x44,0xd4]
          vphraaddsbd xmm2 {k7}, xmm3, xmm4

// CHECK: vphraaddsbd xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x44,0xd4]
          vphraaddsbd xmm2, xmm3, ymm4

// CHECK: vphraaddsbd xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x44,0xd4]
          vphraaddsbd xmm2 {k7}, xmm3, ymm4

// CHECK: vphraaddsbd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddsbd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraaddsbd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddsbd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraaddsbd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x44,0x10]
          vphraaddsbd xmm2, xmm3, xmmword ptr [eax]

// CHECK: vphraaddsbd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x44,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddsbd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraaddsbd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x44,0x51,0x7f]
          vphraaddsbd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraaddsbd xmm2 {k7}, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x44,0x52,0x80]
          vphraaddsbd xmm2 {k7}, xmm3, xmmword ptr [edx - 2048]

// CHECK: vphraaddsbd xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x44,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddsbd xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraaddsbd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x44,0x51,0x7f]
          vphraaddsbd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraaddsbd xmm2 {k7}, xmm3, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x44,0x52,0x80]
          vphraaddsbd xmm2 {k7}, xmm3, ymmword ptr [edx - 4096]

// CHECK: vphraaddswd xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x44,0xd4]
          vphraaddswd xmm2, xmm3, xmm4

// CHECK: vphraaddswd xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x44,0xd4]
          vphraaddswd xmm2 {k7}, xmm3, xmm4

// CHECK: vphraaddswd xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x44,0xd4]
          vphraaddswd xmm2, xmm3, ymm4

// CHECK: vphraaddswd xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x44,0xd4]
          vphraaddswd xmm2 {k7}, xmm3, ymm4

// CHECK: vphraaddswd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddswd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraaddswd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddswd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraaddswd xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe6,0x18,0x44,0x10]
          vphraaddswd xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vphraaddswd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x44,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddswd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraaddswd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x44,0x51,0x7f]
          vphraaddswd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraaddswd xmm2 {k7}, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe6,0x1f,0x44,0x52,0x80]
          vphraaddswd xmm2 {k7}, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vphraaddswd xmm2, xmm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xe6,0x38,0x44,0x10]
          vphraaddswd xmm2, xmm3, word ptr [eax]{1to16}

// CHECK: vphraaddswd xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x44,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddswd xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraaddswd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x44,0x51,0x7f]
          vphraaddswd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraaddswd xmm2 {k7}, xmm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xe6,0x3f,0x44,0x52,0x80]
          vphraaddswd xmm2 {k7}, xmm3, word ptr [edx - 256]{1to16}

// CHECK: vphraaddwd xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x43,0xd4]
          vphraaddwd xmm2, xmm3, xmm4

// CHECK: vphraaddwd xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x43,0xd4]
          vphraaddwd xmm2 {k7}, xmm3, xmm4

// CHECK: vphraaddwd xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x43,0xd4]
          vphraaddwd xmm2, xmm3, ymm4

// CHECK: vphraaddwd xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x43,0xd4]
          vphraaddwd xmm2 {k7}, xmm3, ymm4

// CHECK: vphraaddwd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddwd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraaddwd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddwd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraaddwd xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe6,0x18,0x43,0x10]
          vphraaddwd xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vphraaddwd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x43,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddwd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraaddwd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x43,0x51,0x7f]
          vphraaddwd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraaddwd xmm2 {k7}, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe6,0x1f,0x43,0x52,0x80]
          vphraaddwd xmm2 {k7}, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vphraaddwd xmm2, xmm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xe6,0x38,0x43,0x10]
          vphraaddwd xmm2, xmm3, word ptr [eax]{1to16}

// CHECK: vphraaddwd xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x43,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddwd xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraaddwd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x43,0x51,0x7f]
          vphraaddwd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraaddwd xmm2 {k7}, xmm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xe6,0x3f,0x43,0x52,0x80]
          vphraaddwd xmm2 {k7}, xmm3, word ptr [edx - 256]{1to16}

// CHECK: vphraandb xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4d,0xd4]
          vphraandb xmm2, xmm3, xmm4

// CHECK: vphraandb xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4d,0xd4]
          vphraandb xmm2 {k7}, xmm3, xmm4

// CHECK: vphraandb xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x4d,0xd4]
          vphraandb xmm2, xmm3, ymm4

// CHECK: vphraandb xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x4d,0xd4]
          vphraandb xmm2 {k7}, xmm3, ymm4

// CHECK: vphraandb xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandb xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraandb xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandb xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraandb xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4d,0x10]
          vphraandb xmm2, xmm3, xmmword ptr [eax]

// CHECK: vphraandb xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraandb xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraandb xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4d,0x51,0x7f]
          vphraandb xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraandb xmm2 {k7}, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4d,0x52,0x80]
          vphraandb xmm2 {k7}, xmm3, xmmword ptr [edx - 2048]

// CHECK: vphraandb xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x4d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraandb xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraandb xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x4d,0x51,0x7f]
          vphraandb xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraandb xmm2 {k7}, xmm3, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x4d,0x52,0x80]
          vphraandb xmm2 {k7}, xmm3, ymmword ptr [edx - 4096]

// CHECK: vphraandd xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x4d,0xd4]
          vphraandd xmm2, xmm3, xmm4

// CHECK: vphraandd xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x4d,0xd4]
          vphraandd xmm2 {k7}, xmm3, xmm4

// CHECK: vphraandd xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x4d,0xd4]
          vphraandd xmm2, xmm3, ymm4

// CHECK: vphraandd xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x4d,0xd4]
          vphraandd xmm2 {k7}, xmm3, ymm4

// CHECK: vphraandd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraandd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraandd xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x4d,0x10]
          vphraandd xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK: vphraandd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x4d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraandd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraandd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x4d,0x51,0x7f]
          vphraandd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraandd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x67,0x1f,0x4d,0x52,0x80]
          vphraandd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK: vphraandd xmm2, xmm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x4d,0x10]
          vphraandd xmm2, xmm3, dword ptr [eax]{1to8}

// CHECK: vphraandd xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x4d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraandd xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraandd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x4d,0x51,0x7f]
          vphraandd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraandd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x3f,0x4d,0x52,0x80]
          vphraandd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to8}

// CHECK: vphraandq xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x4d,0xd4]
          vphraandq xmm2, xmm3, xmm4

// CHECK: vphraandq xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x4d,0xd4]
          vphraandq xmm2 {k7}, xmm3, xmm4

// CHECK: vphraandq xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x4d,0xd4]
          vphraandq xmm2, xmm3, ymm4

// CHECK: vphraandq xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x4d,0xd4]
          vphraandq xmm2 {k7}, xmm3, ymm4

// CHECK: vphraandq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraandq xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandq xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraandq xmm2, xmm3, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xe7,0x18,0x4d,0x10]
          vphraandq xmm2, xmm3, qword ptr [eax]{1to2}

// CHECK: vphraandq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x4d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraandq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraandq xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x4d,0x51,0x7f]
          vphraandq xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraandq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xe7,0x1f,0x4d,0x52,0x80]
          vphraandq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to2}

// CHECK: vphraandq xmm2, xmm3, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xe7,0x38,0x4d,0x10]
          vphraandq xmm2, xmm3, qword ptr [eax]{1to4}

// CHECK: vphraandq xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x4d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraandq xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraandq xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x4d,0x51,0x7f]
          vphraandq xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraandq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xe7,0x3f,0x4d,0x52,0x80]
          vphraandq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to4}

// CHECK: vphraandw xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x4d,0xd4]
          vphraandw xmm2, xmm3, xmm4

// CHECK: vphraandw xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x4d,0xd4]
          vphraandw xmm2 {k7}, xmm3, xmm4

// CHECK: vphraandw xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x4d,0xd4]
          vphraandw xmm2, xmm3, ymm4

// CHECK: vphraandw xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x4d,0xd4]
          vphraandw xmm2 {k7}, xmm3, ymm4

// CHECK: vphraandw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraandw xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandw xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraandw xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe6,0x18,0x4d,0x10]
          vphraandw xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vphraandw xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x4d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraandw xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraandw xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x4d,0x51,0x7f]
          vphraandw xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraandw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe6,0x1f,0x4d,0x52,0x80]
          vphraandw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vphraandw xmm2, xmm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xe6,0x38,0x4d,0x10]
          vphraandw xmm2, xmm3, word ptr [eax]{1to16}

// CHECK: vphraandw xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x4d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraandw xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraandw xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x4d,0x51,0x7f]
          vphraandw xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraandw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xe6,0x3f,0x4d,0x52,0x80]
          vphraandw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to16}

// CHECK: vphraddbd xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x43,0xd3]
          vphraddbd xmm2, xmm3

// CHECK: vphraddbd xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x43,0xd3]
          vphraddbd xmm2 {k7}, xmm3

// CHECK: vphraddbd xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x43,0xd3]
          vphraddbd xmm2, ymm3

// CHECK: vphraddbd xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x43,0xd3]
          vphraddbd xmm2 {k7}, ymm3

// CHECK: vphraddbd xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddbd xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddbd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddbd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddbd xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x43,0x10]
          vphraddbd xmm2, xmmword ptr [eax]

// CHECK: vphraddbd xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x43,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraddbd xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphraddbd xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x43,0x51,0x7f]
          vphraddbd xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphraddbd xmm2 {k7}, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x43,0x52,0x80]
          vphraddbd xmm2 {k7}, xmmword ptr [edx - 2048]

// CHECK: vphraddbd xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x43,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraddbd xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphraddbd xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x43,0x51,0x7f]
          vphraddbd xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphraddbd xmm2 {k7}, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x43,0x52,0x80]
          vphraddbd xmm2 {k7}, ymmword ptr [edx - 4096]

// CHECK: vphraddd xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x43,0xd3]
          vphraddd xmm2, xmm3

// CHECK: vphraddd xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x43,0xd3]
          vphraddd xmm2 {k7}, xmm3

// CHECK: vphraddd xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x43,0xd3]
          vphraddd xmm2, ymm3

// CHECK: vphraddd xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x43,0xd3]
          vphraddd xmm2 {k7}, ymm3

// CHECK: vphraddd xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddd xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddd xmm2, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x43,0x10]
          vphraddd xmm2, dword ptr [eax]{1to4}

// CHECK: vphraddd xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x43,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraddd xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphraddd xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x43,0x51,0x7f]
          vphraddd xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphraddd xmm2 {k7}, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x1f,0x43,0x52,0x80]
          vphraddd xmm2 {k7}, dword ptr [edx - 512]{1to4}

// CHECK: vphraddd xmm2, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x43,0x10]
          vphraddd xmm2, dword ptr [eax]{1to8}

// CHECK: vphraddd xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x43,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraddd xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphraddd xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x43,0x51,0x7f]
          vphraddd xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphraddd xmm2 {k7}, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x3f,0x43,0x52,0x80]
          vphraddd xmm2 {k7}, dword ptr [edx - 512]{1to8}

// CHECK: vphraddq xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x43,0xd3]
          vphraddq xmm2, xmm3

// CHECK: vphraddq xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x43,0xd3]
          vphraddq xmm2 {k7}, xmm3

// CHECK: vphraddq xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x43,0xd3]
          vphraddq xmm2, ymm3

// CHECK: vphraddq xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x2f,0x43,0xd3]
          vphraddq xmm2 {k7}, ymm3

// CHECK: vphraddq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddq xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddq xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddq xmm2, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x43,0x10]
          vphraddq xmm2, qword ptr [eax]{1to2}

// CHECK: vphraddq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x43,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraddq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphraddq xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x43,0x51,0x7f]
          vphraddq xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphraddq xmm2 {k7}, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x1f,0x43,0x52,0x80]
          vphraddq xmm2 {k7}, qword ptr [edx - 1024]{1to2}

// CHECK: vphraddq xmm2, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x43,0x10]
          vphraddq xmm2, qword ptr [eax]{1to4}

// CHECK: vphraddq xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x43,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraddq xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphraddq xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfd,0x2f,0x43,0x51,0x7f]
          vphraddq xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphraddq xmm2 {k7}, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x3f,0x43,0x52,0x80]
          vphraddq xmm2 {k7}, qword ptr [edx - 1024]{1to4}

// CHECK: vphraddsbd xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x44,0xd3]
          vphraddsbd xmm2, xmm3

// CHECK: vphraddsbd xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x44,0xd3]
          vphraddsbd xmm2 {k7}, xmm3

// CHECK: vphraddsbd xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x44,0xd3]
          vphraddsbd xmm2, ymm3

// CHECK: vphraddsbd xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x44,0xd3]
          vphraddsbd xmm2 {k7}, ymm3

// CHECK: vphraddsbd xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddsbd xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddsbd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddsbd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddsbd xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x44,0x10]
          vphraddsbd xmm2, xmmword ptr [eax]

// CHECK: vphraddsbd xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x44,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraddsbd xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphraddsbd xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x44,0x51,0x7f]
          vphraddsbd xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphraddsbd xmm2 {k7}, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x44,0x52,0x80]
          vphraddsbd xmm2 {k7}, xmmword ptr [edx - 2048]

// CHECK: vphraddsbd xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x44,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraddsbd xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphraddsbd xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x44,0x51,0x7f]
          vphraddsbd xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphraddsbd xmm2 {k7}, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x44,0x52,0x80]
          vphraddsbd xmm2 {k7}, ymmword ptr [edx - 4096]

// CHECK: vphraddsd xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x44,0xd3]
          vphraddsd xmm2, xmm3

// CHECK: vphraddsd xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x44,0xd3]
          vphraddsd xmm2 {k7}, xmm3

// CHECK: vphraddsd xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x44,0xd3]
          vphraddsd xmm2, ymm3

// CHECK: vphraddsd xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x44,0xd3]
          vphraddsd xmm2 {k7}, ymm3

// CHECK: vphraddsd xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddsd xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddsd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddsd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddsd xmm2, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x44,0x10]
          vphraddsd xmm2, dword ptr [eax]{1to4}

// CHECK: vphraddsd xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x44,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraddsd xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphraddsd xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x44,0x51,0x7f]
          vphraddsd xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphraddsd xmm2 {k7}, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x1f,0x44,0x52,0x80]
          vphraddsd xmm2 {k7}, dword ptr [edx - 512]{1to4}

// CHECK: vphraddsd xmm2, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x44,0x10]
          vphraddsd xmm2, dword ptr [eax]{1to8}

// CHECK: vphraddsd xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x44,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraddsd xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphraddsd xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x44,0x51,0x7f]
          vphraddsd xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphraddsd xmm2 {k7}, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x3f,0x44,0x52,0x80]
          vphraddsd xmm2 {k7}, dword ptr [edx - 512]{1to8}

// CHECK: vphraddsq xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x44,0xd3]
          vphraddsq xmm2, xmm3

// CHECK: vphraddsq xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x44,0xd3]
          vphraddsq xmm2 {k7}, xmm3

// CHECK: vphraddsq xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x44,0xd3]
          vphraddsq xmm2, ymm3

// CHECK: vphraddsq xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x2f,0x44,0xd3]
          vphraddsq xmm2 {k7}, ymm3

// CHECK: vphraddsq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddsq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddsq xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddsq xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddsq xmm2, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x44,0x10]
          vphraddsq xmm2, qword ptr [eax]{1to2}

// CHECK: vphraddsq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x44,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraddsq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphraddsq xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x44,0x51,0x7f]
          vphraddsq xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphraddsq xmm2 {k7}, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x1f,0x44,0x52,0x80]
          vphraddsq xmm2 {k7}, qword ptr [edx - 1024]{1to2}

// CHECK: vphraddsq xmm2, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x44,0x10]
          vphraddsq xmm2, qword ptr [eax]{1to4}

// CHECK: vphraddsq xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x44,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraddsq xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphraddsq xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfd,0x2f,0x44,0x51,0x7f]
          vphraddsq xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphraddsq xmm2 {k7}, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x3f,0x44,0x52,0x80]
          vphraddsq xmm2 {k7}, qword ptr [edx - 1024]{1to4}

// CHECK: vphraddswd xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x44,0xd3]
          vphraddswd xmm2, xmm3

// CHECK: vphraddswd xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x44,0xd3]
          vphraddswd xmm2 {k7}, xmm3

// CHECK: vphraddswd xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x44,0xd3]
          vphraddswd xmm2, ymm3

// CHECK: vphraddswd xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x2f,0x44,0xd3]
          vphraddswd xmm2 {k7}, ymm3

// CHECK: vphraddswd xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddswd xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddswd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddswd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddswd xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x44,0x10]
          vphraddswd xmm2, word ptr [eax]{1to8}

// CHECK: vphraddswd xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x44,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraddswd xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphraddswd xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x44,0x51,0x7f]
          vphraddswd xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphraddswd xmm2 {k7}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x1f,0x44,0x52,0x80]
          vphraddswd xmm2 {k7}, word ptr [edx - 256]{1to8}

// CHECK: vphraddswd xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x44,0x10]
          vphraddswd xmm2, word ptr [eax]{1to16}

// CHECK: vphraddswd xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x44,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraddswd xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphraddswd xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfc,0x2f,0x44,0x51,0x7f]
          vphraddswd xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphraddswd xmm2 {k7}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x3f,0x44,0x52,0x80]
          vphraddswd xmm2 {k7}, word ptr [edx - 256]{1to16}

// CHECK: vphraddwd xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x43,0xd3]
          vphraddwd xmm2, xmm3

// CHECK: vphraddwd xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x43,0xd3]
          vphraddwd xmm2 {k7}, xmm3

// CHECK: vphraddwd xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x43,0xd3]
          vphraddwd xmm2, ymm3

// CHECK: vphraddwd xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x2f,0x43,0xd3]
          vphraddwd xmm2 {k7}, ymm3

// CHECK: vphraddwd xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraddwd xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraddwd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraddwd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraddwd xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x43,0x10]
          vphraddwd xmm2, word ptr [eax]{1to8}

// CHECK: vphraddwd xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x43,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraddwd xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphraddwd xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x43,0x51,0x7f]
          vphraddwd xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphraddwd xmm2 {k7}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x1f,0x43,0x52,0x80]
          vphraddwd xmm2 {k7}, word ptr [edx - 256]{1to8}

// CHECK: vphraddwd xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x43,0x10]
          vphraddwd xmm2, word ptr [eax]{1to16}

// CHECK: vphraddwd xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x43,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraddwd xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphraddwd xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfc,0x2f,0x43,0x51,0x7f]
          vphraddwd xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphraddwd xmm2 {k7}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x3f,0x43,0x52,0x80]
          vphraddwd xmm2 {k7}, word ptr [edx - 256]{1to16}

// CHECK: vphramaxsb xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4b,0xd4]
          vphramaxsb xmm2, xmm3, xmm4

// CHECK: vphramaxsb xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4b,0xd4]
          vphramaxsb xmm2 {k7}, xmm3, xmm4

// CHECK: vphramaxsb xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x4b,0xd4]
          vphramaxsb xmm2, xmm3, ymm4

// CHECK: vphramaxsb xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x4b,0xd4]
          vphramaxsb xmm2 {k7}, xmm3, ymm4

// CHECK: vphramaxsb xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsb xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphramaxsb xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsb xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphramaxsb xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4b,0x10]
          vphramaxsb xmm2, xmm3, xmmword ptr [eax]

// CHECK: vphramaxsb xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsb xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphramaxsb xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4b,0x51,0x7f]
          vphramaxsb xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphramaxsb xmm2 {k7}, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4b,0x52,0x80]
          vphramaxsb xmm2 {k7}, xmm3, xmmword ptr [edx - 2048]

// CHECK: vphramaxsb xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x4b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsb xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphramaxsb xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x4b,0x51,0x7f]
          vphramaxsb xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphramaxsb xmm2 {k7}, xmm3, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x4b,0x52,0x80]
          vphramaxsb xmm2 {k7}, xmm3, ymmword ptr [edx - 4096]

// CHECK: vphramaxsd xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x4b,0xd4]
          vphramaxsd xmm2, xmm3, xmm4

// CHECK: vphramaxsd xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x4b,0xd4]
          vphramaxsd xmm2 {k7}, xmm3, xmm4

// CHECK: vphramaxsd xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x4b,0xd4]
          vphramaxsd xmm2, xmm3, ymm4

// CHECK: vphramaxsd xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x4b,0xd4]
          vphramaxsd xmm2 {k7}, xmm3, ymm4

// CHECK: vphramaxsd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphramaxsd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphramaxsd xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x4b,0x10]
          vphramaxsd xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK: vphramaxsd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x4b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphramaxsd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x4b,0x51,0x7f]
          vphramaxsd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphramaxsd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x67,0x1f,0x4b,0x52,0x80]
          vphramaxsd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK: vphramaxsd xmm2, xmm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x4b,0x10]
          vphramaxsd xmm2, xmm3, dword ptr [eax]{1to8}

// CHECK: vphramaxsd xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x4b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsd xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphramaxsd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x4b,0x51,0x7f]
          vphramaxsd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphramaxsd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x3f,0x4b,0x52,0x80]
          vphramaxsd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to8}

// CHECK: vphramaxsq xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x4b,0xd4]
          vphramaxsq xmm2, xmm3, xmm4

// CHECK: vphramaxsq xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x4b,0xd4]
          vphramaxsq xmm2 {k7}, xmm3, xmm4

// CHECK: vphramaxsq xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x4b,0xd4]
          vphramaxsq xmm2, xmm3, ymm4

// CHECK: vphramaxsq xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x4b,0xd4]
          vphramaxsq xmm2 {k7}, xmm3, ymm4

// CHECK: vphramaxsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphramaxsq xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsq xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphramaxsq xmm2, xmm3, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xe7,0x18,0x4b,0x10]
          vphramaxsq xmm2, xmm3, qword ptr [eax]{1to2}

// CHECK: vphramaxsq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x4b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphramaxsq xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x4b,0x51,0x7f]
          vphramaxsq xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphramaxsq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xe7,0x1f,0x4b,0x52,0x80]
          vphramaxsq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to2}

// CHECK: vphramaxsq xmm2, xmm3, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xe7,0x38,0x4b,0x10]
          vphramaxsq xmm2, xmm3, qword ptr [eax]{1to4}

// CHECK: vphramaxsq xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x4b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsq xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphramaxsq xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x4b,0x51,0x7f]
          vphramaxsq xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphramaxsq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xe7,0x3f,0x4b,0x52,0x80]
          vphramaxsq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to4}

// CHECK: vphramaxsw xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x4b,0xd4]
          vphramaxsw xmm2, xmm3, xmm4

// CHECK: vphramaxsw xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x4b,0xd4]
          vphramaxsw xmm2 {k7}, xmm3, xmm4

// CHECK: vphramaxsw xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x4b,0xd4]
          vphramaxsw xmm2, xmm3, ymm4

// CHECK: vphramaxsw xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x4b,0xd4]
          vphramaxsw xmm2 {k7}, xmm3, ymm4

// CHECK: vphramaxsw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphramaxsw xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsw xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphramaxsw xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe6,0x18,0x4b,0x10]
          vphramaxsw xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vphramaxsw xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x4b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsw xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphramaxsw xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x4b,0x51,0x7f]
          vphramaxsw xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphramaxsw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe6,0x1f,0x4b,0x52,0x80]
          vphramaxsw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vphramaxsw xmm2, xmm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xe6,0x38,0x4b,0x10]
          vphramaxsw xmm2, xmm3, word ptr [eax]{1to16}

// CHECK: vphramaxsw xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x4b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsw xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphramaxsw xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x4b,0x51,0x7f]
          vphramaxsw xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphramaxsw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xe6,0x3f,0x4b,0x52,0x80]
          vphramaxsw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to16}

// CHECK: vphraminb xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x48,0xd4]
          vphraminb xmm2, xmm3, xmm4

// CHECK: vphraminb xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x48,0xd4]
          vphraminb xmm2 {k7}, xmm3, xmm4

// CHECK: vphraminb xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x48,0xd4]
          vphraminb xmm2, xmm3, ymm4

// CHECK: vphraminb xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x48,0xd4]
          vphraminb xmm2 {k7}, xmm3, ymm4

// CHECK: vphraminb xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminb xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminb xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminb xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminb xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x48,0x10]
          vphraminb xmm2, xmm3, xmmword ptr [eax]

// CHECK: vphraminb xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x48,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminb xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraminb xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x48,0x51,0x7f]
          vphraminb xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraminb xmm2 {k7}, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x48,0x52,0x80]
          vphraminb xmm2 {k7}, xmm3, xmmword ptr [edx - 2048]

// CHECK: vphraminb xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x48,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminb xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraminb xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x48,0x51,0x7f]
          vphraminb xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraminb xmm2 {k7}, xmm3, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x48,0x52,0x80]
          vphraminb xmm2 {k7}, xmm3, ymmword ptr [edx - 4096]

// CHECK: vphramind xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x48,0xd4]
          vphramind xmm2, xmm3, xmm4

// CHECK: vphramind xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x48,0xd4]
          vphramind xmm2 {k7}, xmm3, xmm4

// CHECK: vphramind xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x48,0xd4]
          vphramind xmm2, xmm3, ymm4

// CHECK: vphramind xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x48,0xd4]
          vphramind xmm2 {k7}, xmm3, ymm4

// CHECK: vphramind xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramind xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphramind xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramind xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphramind xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x48,0x10]
          vphramind xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK: vphramind xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x48,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphramind xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphramind xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x48,0x51,0x7f]
          vphramind xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphramind xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x67,0x1f,0x48,0x52,0x80]
          vphramind xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK: vphramind xmm2, xmm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x48,0x10]
          vphramind xmm2, xmm3, dword ptr [eax]{1to8}

// CHECK: vphramind xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x48,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphramind xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphramind xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x48,0x51,0x7f]
          vphramind xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphramind xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x3f,0x48,0x52,0x80]
          vphramind xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to8}

// CHECK: vphraminq xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x48,0xd4]
          vphraminq xmm2, xmm3, xmm4

// CHECK: vphraminq xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x48,0xd4]
          vphraminq xmm2 {k7}, xmm3, xmm4

// CHECK: vphraminq xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x48,0xd4]
          vphraminq xmm2, xmm3, ymm4

// CHECK: vphraminq xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x48,0xd4]
          vphraminq xmm2 {k7}, xmm3, ymm4

// CHECK: vphraminq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminq xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminq xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminq xmm2, xmm3, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xe7,0x18,0x48,0x10]
          vphraminq xmm2, xmm3, qword ptr [eax]{1to2}

// CHECK: vphraminq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x48,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraminq xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x48,0x51,0x7f]
          vphraminq xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraminq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xe7,0x1f,0x48,0x52,0x80]
          vphraminq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to2}

// CHECK: vphraminq xmm2, xmm3, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xe7,0x38,0x48,0x10]
          vphraminq xmm2, xmm3, qword ptr [eax]{1to4}

// CHECK: vphraminq xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x48,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminq xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraminq xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x48,0x51,0x7f]
          vphraminq xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraminq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xe7,0x3f,0x48,0x52,0x80]
          vphraminq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to4}

// CHECK: vphraminsb xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x49,0xd4]
          vphraminsb xmm2, xmm3, xmm4

// CHECK: vphraminsb xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x49,0xd4]
          vphraminsb xmm2 {k7}, xmm3, xmm4

// CHECK: vphraminsb xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x49,0xd4]
          vphraminsb xmm2, xmm3, ymm4

// CHECK: vphraminsb xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x49,0xd4]
          vphraminsb xmm2 {k7}, xmm3, ymm4

// CHECK: vphraminsb xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsb xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminsb xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsb xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminsb xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x49,0x10]
          vphraminsb xmm2, xmm3, xmmword ptr [eax]

// CHECK: vphraminsb xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x49,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsb xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraminsb xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x49,0x51,0x7f]
          vphraminsb xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraminsb xmm2 {k7}, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x49,0x52,0x80]
          vphraminsb xmm2 {k7}, xmm3, xmmword ptr [edx - 2048]

// CHECK: vphraminsb xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x49,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsb xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraminsb xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x49,0x51,0x7f]
          vphraminsb xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraminsb xmm2 {k7}, xmm3, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x49,0x52,0x80]
          vphraminsb xmm2 {k7}, xmm3, ymmword ptr [edx - 4096]

// CHECK: vphraminsd xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x49,0xd4]
          vphraminsd xmm2, xmm3, xmm4

// CHECK: vphraminsd xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x49,0xd4]
          vphraminsd xmm2 {k7}, xmm3, xmm4

// CHECK: vphraminsd xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x49,0xd4]
          vphraminsd xmm2, xmm3, ymm4

// CHECK: vphraminsd xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x49,0xd4]
          vphraminsd xmm2 {k7}, xmm3, ymm4

// CHECK: vphraminsd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminsd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminsd xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x49,0x10]
          vphraminsd xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK: vphraminsd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x49,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraminsd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x49,0x51,0x7f]
          vphraminsd xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraminsd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x67,0x1f,0x49,0x52,0x80]
          vphraminsd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK: vphraminsd xmm2, xmm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x49,0x10]
          vphraminsd xmm2, xmm3, dword ptr [eax]{1to8}

// CHECK: vphraminsd xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x49,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsd xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraminsd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x49,0x51,0x7f]
          vphraminsd xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraminsd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x3f,0x49,0x52,0x80]
          vphraminsd xmm2 {k7}, xmm3, dword ptr [edx - 512]{1to8}

// CHECK: vphraminsq xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x49,0xd4]
          vphraminsq xmm2, xmm3, xmm4

// CHECK: vphraminsq xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x49,0xd4]
          vphraminsq xmm2 {k7}, xmm3, xmm4

// CHECK: vphraminsq xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x49,0xd4]
          vphraminsq xmm2, xmm3, ymm4

// CHECK: vphraminsq xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x49,0xd4]
          vphraminsq xmm2 {k7}, xmm3, ymm4

// CHECK: vphraminsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminsq xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsq xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminsq xmm2, xmm3, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xe7,0x18,0x49,0x10]
          vphraminsq xmm2, xmm3, qword ptr [eax]{1to2}

// CHECK: vphraminsq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x49,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraminsq xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x49,0x51,0x7f]
          vphraminsq xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraminsq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xe7,0x1f,0x49,0x52,0x80]
          vphraminsq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to2}

// CHECK: vphraminsq xmm2, xmm3, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xe7,0x38,0x49,0x10]
          vphraminsq xmm2, xmm3, qword ptr [eax]{1to4}

// CHECK: vphraminsq xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x49,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsq xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraminsq xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x49,0x51,0x7f]
          vphraminsq xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraminsq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xe7,0x3f,0x49,0x52,0x80]
          vphraminsq xmm2 {k7}, xmm3, qword ptr [edx - 1024]{1to4}

// CHECK: vphraminsw xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x49,0xd4]
          vphraminsw xmm2, xmm3, xmm4

// CHECK: vphraminsw xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x49,0xd4]
          vphraminsw xmm2 {k7}, xmm3, xmm4

// CHECK: vphraminsw xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x49,0xd4]
          vphraminsw xmm2, xmm3, ymm4

// CHECK: vphraminsw xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x49,0xd4]
          vphraminsw xmm2 {k7}, xmm3, ymm4

// CHECK: vphraminsw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminsw xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsw xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminsw xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe6,0x18,0x49,0x10]
          vphraminsw xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vphraminsw xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x49,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsw xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraminsw xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x49,0x51,0x7f]
          vphraminsw xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraminsw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe6,0x1f,0x49,0x52,0x80]
          vphraminsw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vphraminsw xmm2, xmm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xe6,0x38,0x49,0x10]
          vphraminsw xmm2, xmm3, word ptr [eax]{1to16}

// CHECK: vphraminsw xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x49,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsw xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraminsw xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x49,0x51,0x7f]
          vphraminsw xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraminsw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xe6,0x3f,0x49,0x52,0x80]
          vphraminsw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to16}

// CHECK: vphraminw xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x48,0xd4]
          vphraminw xmm2, xmm3, xmm4

// CHECK: vphraminw xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x48,0xd4]
          vphraminw xmm2 {k7}, xmm3, xmm4

// CHECK: vphraminw xmm2, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x48,0xd4]
          vphraminw xmm2, xmm3, ymm4

// CHECK: vphraminw xmm2 {k7}, xmm3, ymm4
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x48,0xd4]
          vphraminw xmm2 {k7}, xmm3, ymm4

// CHECK: vphraminw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphraminw xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminw xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphraminw xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe6,0x18,0x48,0x10]
          vphraminw xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vphraminw xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x48,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminw xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vphraminw xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x48,0x51,0x7f]
          vphraminw xmm2 {k7}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vphraminw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xe6,0x1f,0x48,0x52,0x80]
          vphraminw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vphraminw xmm2, xmm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xe6,0x38,0x48,0x10]
          vphraminw xmm2, xmm3, word ptr [eax]{1to16}

// CHECK: vphraminw xmm2, xmm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x48,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminw xmm2, xmm3, ymmword ptr [2*ebp - 1024]

// CHECK: vphraminw xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x48,0x51,0x7f]
          vphraminw xmm2 {k7}, xmm3, ymmword ptr [ecx + 4064]

// CHECK: vphraminw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xe6,0x3f,0x48,0x52,0x80]
          vphraminw xmm2 {k7}, xmm3, word ptr [edx - 256]{1to16}

// CHECK: vphrandb xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4d,0xd3]
          vphrandb xmm2, xmm3

// CHECK: vphrandb xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4d,0xd3]
          vphrandb xmm2, ymm3

// CHECK: vphrandb xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrandb xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrandb xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrandb xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrandb xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4d,0x10]
          vphrandb xmm2, xmmword ptr [eax]

// CHECK: vphrandb xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrandb xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrandb xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4d,0x51,0x7f]
          vphrandb xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrandb xmm2, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4d,0x52,0x80]
          vphrandb xmm2, xmmword ptr [edx - 2048]

// CHECK: vphrandb xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrandb xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrandb xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4d,0x51,0x7f]
          vphrandb xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrandb xmm2, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4d,0x52,0x80]
          vphrandb xmm2, ymmword ptr [edx - 4096]

// CHECK: vphrandd xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4d,0xd3]
          vphrandd xmm2, xmm3

// CHECK: vphrandd xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x4d,0xd3]
          vphrandd xmm2, ymm3

// CHECK: vphrandd xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrandd xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrandd xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrandd xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrandd xmm2, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x4d,0x10]
          vphrandd xmm2, dword ptr [eax]{1to4}

// CHECK: vphrandd xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrandd xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrandd xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4d,0x51,0x7f]
          vphrandd xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrandd xmm2, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x4d,0x52,0x80]
          vphrandd xmm2, dword ptr [edx - 512]{1to4}

// CHECK: vphrandd xmm2, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x4d,0x10]
          vphrandd xmm2, dword ptr [eax]{1to8}

// CHECK: vphrandd xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x4d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrandd xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrandd xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x4d,0x51,0x7f]
          vphrandd xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrandd xmm2, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x4d,0x52,0x80]
          vphrandd xmm2, dword ptr [edx - 512]{1to8}

// CHECK: vphranddq xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x45,0xd3]
          vphranddq xmm2, ymm3

// CHECK: vphranddq xmm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x45,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphranddq xmm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vphranddq xmm2, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x45,0x94,0x87,0x23,0x01,0x00,0x00]
          vphranddq xmm2, ymmword ptr [edi + 4*eax + 291]

// CHECK: vphranddq xmm2, ymmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x45,0x10]
          vphranddq xmm2, ymmword ptr [eax]

// CHECK: vphranddq xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x45,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphranddq xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphranddq xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x45,0x51,0x7f]
          vphranddq xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphranddq xmm2, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x45,0x52,0x80]
          vphranddq xmm2, ymmword ptr [edx - 4096]

// CHECK: vphrandq xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4d,0xd3]
          vphrandq xmm2, xmm3

// CHECK: vphrandq xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x4d,0xd3]
          vphrandq xmm2, ymm3

// CHECK: vphrandq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrandq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrandq xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrandq xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrandq xmm2, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x4d,0x10]
          vphrandq xmm2, qword ptr [eax]{1to2}

// CHECK: vphrandq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrandq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrandq xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4d,0x51,0x7f]
          vphrandq xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrandq xmm2, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x4d,0x52,0x80]
          vphrandq xmm2, qword ptr [edx - 1024]{1to2}

// CHECK: vphrandq xmm2, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x4d,0x10]
          vphrandq xmm2, qword ptr [eax]{1to4}

// CHECK: vphrandq xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x4d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrandq xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrandq xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x4d,0x51,0x7f]
          vphrandq xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrandq xmm2, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x4d,0x52,0x80]
          vphrandq xmm2, qword ptr [edx - 1024]{1to4}

// CHECK: vphrandw xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4d,0xd3]
          vphrandw xmm2, xmm3

// CHECK: vphrandw xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x4d,0xd3]
          vphrandw xmm2, ymm3

// CHECK: vphrandw xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrandw xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrandw xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrandw xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrandw xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x4d,0x10]
          vphrandw xmm2, word ptr [eax]{1to8}

// CHECK: vphrandw xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrandw xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrandw xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4d,0x51,0x7f]
          vphrandw xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrandw xmm2, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x4d,0x52,0x80]
          vphrandw xmm2, word ptr [edx - 256]{1to8}

// CHECK: vphrandw xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x4d,0x10]
          vphrandw xmm2, word ptr [eax]{1to16}

// CHECK: vphrandw xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x4d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrandw xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrandw xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x4d,0x51,0x7f]
          vphrandw xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrandw xmm2, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x4d,0x52,0x80]
          vphrandw xmm2, word ptr [edx - 256]{1to16}

// CHECK: vphrmaxb xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4a,0xd3]
          vphrmaxb xmm2, xmm3

// CHECK: vphrmaxb xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x4a,0xd3]
          vphrmaxb xmm2 {k7}, xmm3

// CHECK: vphrmaxb xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4a,0xd3]
          vphrmaxb xmm2, ymm3

// CHECK: vphrmaxb xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x4a,0xd3]
          vphrmaxb xmm2 {k7}, ymm3

// CHECK: vphrmaxb xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxb xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxb xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x4a,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxb xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxb xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4a,0x10]
          vphrmaxb xmm2, xmmword ptr [eax]

// CHECK: vphrmaxb xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4a,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrmaxb xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrmaxb xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x4a,0x51,0x7f]
          vphrmaxb xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphrmaxb xmm2 {k7}, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x4a,0x52,0x80]
          vphrmaxb xmm2 {k7}, xmmword ptr [edx - 2048]

// CHECK: vphrmaxb xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4a,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrmaxb xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrmaxb xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x4a,0x51,0x7f]
          vphrmaxb xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphrmaxb xmm2 {k7}, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x4a,0x52,0x80]
          vphrmaxb xmm2 {k7}, ymmword ptr [edx - 4096]

// CHECK: vphrmaxd xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4a,0xd3]
          vphrmaxd xmm2, xmm3

// CHECK: vphrmaxd xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x4a,0xd3]
          vphrmaxd xmm2 {k7}, xmm3

// CHECK: vphrmaxd xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x4a,0xd3]
          vphrmaxd xmm2, ymm3

// CHECK: vphrmaxd xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x4a,0xd3]
          vphrmaxd xmm2 {k7}, ymm3

// CHECK: vphrmaxd xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxd xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x4a,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxd xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxd xmm2, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x4a,0x10]
          vphrmaxd xmm2, dword ptr [eax]{1to4}

// CHECK: vphrmaxd xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4a,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrmaxd xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrmaxd xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x4a,0x51,0x7f]
          vphrmaxd xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphrmaxd xmm2 {k7}, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x1f,0x4a,0x52,0x80]
          vphrmaxd xmm2 {k7}, dword ptr [edx - 512]{1to4}

// CHECK: vphrmaxd xmm2, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x4a,0x10]
          vphrmaxd xmm2, dword ptr [eax]{1to8}

// CHECK: vphrmaxd xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x4a,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrmaxd xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrmaxd xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x4a,0x51,0x7f]
          vphrmaxd xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphrmaxd xmm2 {k7}, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x3f,0x4a,0x52,0x80]
          vphrmaxd xmm2 {k7}, dword ptr [edx - 512]{1to8}

// CHECK: vphrmaxq xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4a,0xd3]
          vphrmaxq xmm2, xmm3

// CHECK: vphrmaxq xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x4a,0xd3]
          vphrmaxq xmm2 {k7}, xmm3

// CHECK: vphrmaxq xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x4a,0xd3]
          vphrmaxq xmm2, ymm3

// CHECK: vphrmaxq xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x2f,0x4a,0xd3]
          vphrmaxq xmm2 {k7}, ymm3

// CHECK: vphrmaxq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxq xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x4a,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxq xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxq xmm2, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x4a,0x10]
          vphrmaxq xmm2, qword ptr [eax]{1to2}

// CHECK: vphrmaxq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4a,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrmaxq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrmaxq xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x4a,0x51,0x7f]
          vphrmaxq xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphrmaxq xmm2 {k7}, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x1f,0x4a,0x52,0x80]
          vphrmaxq xmm2 {k7}, qword ptr [edx - 1024]{1to2}

// CHECK: vphrmaxq xmm2, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x4a,0x10]
          vphrmaxq xmm2, qword ptr [eax]{1to4}

// CHECK: vphrmaxq xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x4a,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrmaxq xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrmaxq xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfd,0x2f,0x4a,0x51,0x7f]
          vphrmaxq xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphrmaxq xmm2 {k7}, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x3f,0x4a,0x52,0x80]
          vphrmaxq xmm2 {k7}, qword ptr [edx - 1024]{1to4}

// CHECK: vphrmaxsb xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4b,0xd3]
          vphrmaxsb xmm2, xmm3

// CHECK: vphrmaxsb xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4b,0xd3]
          vphrmaxsb xmm2, ymm3

// CHECK: vphrmaxsb xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxsb xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxsb xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxsb xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxsb xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4b,0x10]
          vphrmaxsb xmm2, xmmword ptr [eax]

// CHECK: vphrmaxsb xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrmaxsb xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrmaxsb xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4b,0x51,0x7f]
          vphrmaxsb xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrmaxsb xmm2, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4b,0x52,0x80]
          vphrmaxsb xmm2, xmmword ptr [edx - 2048]

// CHECK: vphrmaxsb xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrmaxsb xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrmaxsb xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4b,0x51,0x7f]
          vphrmaxsb xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrmaxsb xmm2, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4b,0x52,0x80]
          vphrmaxsb xmm2, ymmword ptr [edx - 4096]

// CHECK: vphrmaxsd xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4b,0xd3]
          vphrmaxsd xmm2, xmm3

// CHECK: vphrmaxsd xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x4b,0xd3]
          vphrmaxsd xmm2, ymm3

// CHECK: vphrmaxsd xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxsd xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxsd xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxsd xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxsd xmm2, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x4b,0x10]
          vphrmaxsd xmm2, dword ptr [eax]{1to4}

// CHECK: vphrmaxsd xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrmaxsd xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrmaxsd xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4b,0x51,0x7f]
          vphrmaxsd xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrmaxsd xmm2, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x4b,0x52,0x80]
          vphrmaxsd xmm2, dword ptr [edx - 512]{1to4}

// CHECK: vphrmaxsd xmm2, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x4b,0x10]
          vphrmaxsd xmm2, dword ptr [eax]{1to8}

// CHECK: vphrmaxsd xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x4b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrmaxsd xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrmaxsd xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x4b,0x51,0x7f]
          vphrmaxsd xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrmaxsd xmm2, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x4b,0x52,0x80]
          vphrmaxsd xmm2, dword ptr [edx - 512]{1to8}

// CHECK: vphrmaxsq xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4b,0xd3]
          vphrmaxsq xmm2, xmm3

// CHECK: vphrmaxsq xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x4b,0xd3]
          vphrmaxsq xmm2, ymm3

// CHECK: vphrmaxsq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxsq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxsq xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxsq xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxsq xmm2, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x4b,0x10]
          vphrmaxsq xmm2, qword ptr [eax]{1to2}

// CHECK: vphrmaxsq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrmaxsq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrmaxsq xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4b,0x51,0x7f]
          vphrmaxsq xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrmaxsq xmm2, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x4b,0x52,0x80]
          vphrmaxsq xmm2, qword ptr [edx - 1024]{1to2}

// CHECK: vphrmaxsq xmm2, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x4b,0x10]
          vphrmaxsq xmm2, qword ptr [eax]{1to4}

// CHECK: vphrmaxsq xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x4b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrmaxsq xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrmaxsq xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x4b,0x51,0x7f]
          vphrmaxsq xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrmaxsq xmm2, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x4b,0x52,0x80]
          vphrmaxsq xmm2, qword ptr [edx - 1024]{1to4}

// CHECK: vphrmaxsw xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4b,0xd3]
          vphrmaxsw xmm2, xmm3

// CHECK: vphrmaxsw xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x4b,0xd3]
          vphrmaxsw xmm2, ymm3

// CHECK: vphrmaxsw xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxsw xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxsw xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxsw xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxsw xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x4b,0x10]
          vphrmaxsw xmm2, word ptr [eax]{1to8}

// CHECK: vphrmaxsw xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrmaxsw xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrmaxsw xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4b,0x51,0x7f]
          vphrmaxsw xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrmaxsw xmm2, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x4b,0x52,0x80]
          vphrmaxsw xmm2, word ptr [edx - 256]{1to8}

// CHECK: vphrmaxsw xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x4b,0x10]
          vphrmaxsw xmm2, word ptr [eax]{1to16}

// CHECK: vphrmaxsw xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x4b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrmaxsw xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrmaxsw xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x4b,0x51,0x7f]
          vphrmaxsw xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrmaxsw xmm2, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x4b,0x52,0x80]
          vphrmaxsw xmm2, word ptr [edx - 256]{1to16}

// CHECK: vphrmaxw xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4a,0xd3]
          vphrmaxw xmm2, xmm3

// CHECK: vphrmaxw xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x4a,0xd3]
          vphrmaxw xmm2 {k7}, xmm3

// CHECK: vphrmaxw xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x4a,0xd3]
          vphrmaxw xmm2, ymm3

// CHECK: vphrmaxw xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x2f,0x4a,0xd3]
          vphrmaxw xmm2 {k7}, ymm3

// CHECK: vphrmaxw xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmaxw xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmaxw xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x4a,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmaxw xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmaxw xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x4a,0x10]
          vphrmaxw xmm2, word ptr [eax]{1to8}

// CHECK: vphrmaxw xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4a,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrmaxw xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrmaxw xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x4a,0x51,0x7f]
          vphrmaxw xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphrmaxw xmm2 {k7}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x1f,0x4a,0x52,0x80]
          vphrmaxw xmm2 {k7}, word ptr [edx - 256]{1to8}

// CHECK: vphrmaxw xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x4a,0x10]
          vphrmaxw xmm2, word ptr [eax]{1to16}

// CHECK: vphrmaxw xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x4a,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrmaxw xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrmaxw xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfc,0x2f,0x4a,0x51,0x7f]
          vphrmaxw xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphrmaxw xmm2 {k7}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x3f,0x4a,0x52,0x80]
          vphrmaxw xmm2 {k7}, word ptr [edx - 256]{1to16}

// CHECK: vphrminb xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x48,0xd3]
          vphrminb xmm2, xmm3

// CHECK: vphrminb xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x48,0xd3]
          vphrminb xmm2, ymm3

// CHECK: vphrminb xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminb xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminb xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminb xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminb xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x48,0x10]
          vphrminb xmm2, xmmword ptr [eax]

// CHECK: vphrminb xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x48,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrminb xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrminb xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x48,0x51,0x7f]
          vphrminb xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrminb xmm2, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x48,0x52,0x80]
          vphrminb xmm2, xmmword ptr [edx - 2048]

// CHECK: vphrminb xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x48,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrminb xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrminb xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x48,0x51,0x7f]
          vphrminb xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrminb xmm2, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x48,0x52,0x80]
          vphrminb xmm2, ymmword ptr [edx - 4096]

// CHECK: vphrmind xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x48,0xd3]
          vphrmind xmm2, xmm3

// CHECK: vphrmind xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x48,0xd3]
          vphrmind xmm2, ymm3

// CHECK: vphrmind xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrmind xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrmind xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrmind xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrmind xmm2, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x48,0x10]
          vphrmind xmm2, dword ptr [eax]{1to4}

// CHECK: vphrmind xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x48,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrmind xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrmind xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x48,0x51,0x7f]
          vphrmind xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrmind xmm2, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x48,0x52,0x80]
          vphrmind xmm2, dword ptr [edx - 512]{1to4}

// CHECK: vphrmind xmm2, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x48,0x10]
          vphrmind xmm2, dword ptr [eax]{1to8}

// CHECK: vphrmind xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x48,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrmind xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrmind xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x48,0x51,0x7f]
          vphrmind xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrmind xmm2, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x48,0x52,0x80]
          vphrmind xmm2, dword ptr [edx - 512]{1to8}

// CHECK: vphrminq xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x48,0xd3]
          vphrminq xmm2, xmm3

// CHECK: vphrminq xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x48,0xd3]
          vphrminq xmm2, ymm3

// CHECK: vphrminq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminq xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminq xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminq xmm2, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x48,0x10]
          vphrminq xmm2, qword ptr [eax]{1to2}

// CHECK: vphrminq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x48,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrminq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrminq xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x48,0x51,0x7f]
          vphrminq xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrminq xmm2, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x48,0x52,0x80]
          vphrminq xmm2, qword ptr [edx - 1024]{1to2}

// CHECK: vphrminq xmm2, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x48,0x10]
          vphrminq xmm2, qword ptr [eax]{1to4}

// CHECK: vphrminq xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x48,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrminq xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrminq xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x48,0x51,0x7f]
          vphrminq xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrminq xmm2, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x48,0x52,0x80]
          vphrminq xmm2, qword ptr [edx - 1024]{1to4}

// CHECK: vphrminsb xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x49,0xd3]
          vphrminsb xmm2, xmm3

// CHECK: vphrminsb xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x49,0xd3]
          vphrminsb xmm2, ymm3

// CHECK: vphrminsb xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminsb xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminsb xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminsb xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminsb xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x49,0x10]
          vphrminsb xmm2, xmmword ptr [eax]

// CHECK: vphrminsb xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x49,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrminsb xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrminsb xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x49,0x51,0x7f]
          vphrminsb xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrminsb xmm2, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x49,0x52,0x80]
          vphrminsb xmm2, xmmword ptr [edx - 2048]

// CHECK: vphrminsb xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x49,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrminsb xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrminsb xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x49,0x51,0x7f]
          vphrminsb xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrminsb xmm2, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x49,0x52,0x80]
          vphrminsb xmm2, ymmword ptr [edx - 4096]

// CHECK: vphrminsd xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x49,0xd3]
          vphrminsd xmm2, xmm3

// CHECK: vphrminsd xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x49,0xd3]
          vphrminsd xmm2, ymm3

// CHECK: vphrminsd xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminsd xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminsd xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminsd xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminsd xmm2, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x49,0x10]
          vphrminsd xmm2, dword ptr [eax]{1to4}

// CHECK: vphrminsd xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x49,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrminsd xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrminsd xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x49,0x51,0x7f]
          vphrminsd xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrminsd xmm2, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x49,0x52,0x80]
          vphrminsd xmm2, dword ptr [edx - 512]{1to4}

// CHECK: vphrminsd xmm2, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x49,0x10]
          vphrminsd xmm2, dword ptr [eax]{1to8}

// CHECK: vphrminsd xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x49,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrminsd xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrminsd xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x49,0x51,0x7f]
          vphrminsd xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrminsd xmm2, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x49,0x52,0x80]
          vphrminsd xmm2, dword ptr [edx - 512]{1to8}

// CHECK: vphrminsq xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x49,0xd3]
          vphrminsq xmm2, xmm3

// CHECK: vphrminsq xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x49,0xd3]
          vphrminsq xmm2, ymm3

// CHECK: vphrminsq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminsq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminsq xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminsq xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminsq xmm2, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x49,0x10]
          vphrminsq xmm2, qword ptr [eax]{1to2}

// CHECK: vphrminsq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x49,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrminsq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrminsq xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x49,0x51,0x7f]
          vphrminsq xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrminsq xmm2, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x49,0x52,0x80]
          vphrminsq xmm2, qword ptr [edx - 1024]{1to2}

// CHECK: vphrminsq xmm2, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x49,0x10]
          vphrminsq xmm2, qword ptr [eax]{1to4}

// CHECK: vphrminsq xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x49,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrminsq xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrminsq xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x49,0x51,0x7f]
          vphrminsq xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrminsq xmm2, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x49,0x52,0x80]
          vphrminsq xmm2, qword ptr [edx - 1024]{1to4}

// CHECK: vphrminsw xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x49,0xd3]
          vphrminsw xmm2, xmm3

// CHECK: vphrminsw xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x49,0xd3]
          vphrminsw xmm2, ymm3

// CHECK: vphrminsw xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminsw xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminsw xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminsw xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminsw xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x49,0x10]
          vphrminsw xmm2, word ptr [eax]{1to8}

// CHECK: vphrminsw xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x49,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrminsw xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrminsw xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x49,0x51,0x7f]
          vphrminsw xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrminsw xmm2, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x49,0x52,0x80]
          vphrminsw xmm2, word ptr [edx - 256]{1to8}

// CHECK: vphrminsw xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x49,0x10]
          vphrminsw xmm2, word ptr [eax]{1to16}

// CHECK: vphrminsw xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x49,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrminsw xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrminsw xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x49,0x51,0x7f]
          vphrminsw xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrminsw xmm2, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x49,0x52,0x80]
          vphrminsw xmm2, word ptr [edx - 256]{1to16}

// CHECK: vphrminw xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x48,0xd3]
          vphrminw xmm2, xmm3

// CHECK: vphrminw xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x48,0xd3]
          vphrminw xmm2, ymm3

// CHECK: vphrminw xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrminw xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrminw xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrminw xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrminw xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x48,0x10]
          vphrminw xmm2, word ptr [eax]{1to8}

// CHECK: vphrminw xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x48,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrminw xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrminw xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x48,0x51,0x7f]
          vphrminw xmm2, xmmword ptr [ecx + 2032]

// CHECK: vphrminw xmm2, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x48,0x52,0x80]
          vphrminw xmm2, word ptr [edx - 256]{1to8}

// CHECK: vphrminw xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x48,0x10]
          vphrminw xmm2, word ptr [eax]{1to16}

// CHECK: vphrminw xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x48,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrminw xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrminw xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x48,0x51,0x7f]
          vphrminw xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrminw xmm2, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x48,0x52,0x80]
          vphrminw xmm2, word ptr [edx - 256]{1to16}

// CHECK: vphrorb xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4e,0xd3]
          vphrorb xmm2, xmm3

// CHECK: vphrorb xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x4e,0xd3]
          vphrorb xmm2 {k7}, xmm3

// CHECK: vphrorb xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4e,0xd3]
          vphrorb xmm2, ymm3

// CHECK: vphrorb xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x4e,0xd3]
          vphrorb xmm2 {k7}, ymm3

// CHECK: vphrorb xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrorb xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrorb xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x4e,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrorb xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrorb xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4e,0x10]
          vphrorb xmm2, xmmword ptr [eax]

// CHECK: vphrorb xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4e,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrorb xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrorb xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x4e,0x51,0x7f]
          vphrorb xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphrorb xmm2 {k7}, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x4e,0x52,0x80]
          vphrorb xmm2 {k7}, xmmword ptr [edx - 2048]

// CHECK: vphrorb xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4e,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrorb xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrorb xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x4e,0x51,0x7f]
          vphrorb xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphrorb xmm2 {k7}, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x4e,0x52,0x80]
          vphrorb xmm2 {k7}, ymmword ptr [edx - 4096]

// CHECK: vphrord xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4e,0xd3]
          vphrord xmm2, xmm3

// CHECK: vphrord xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x4e,0xd3]
          vphrord xmm2 {k7}, xmm3

// CHECK: vphrord xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x4e,0xd3]
          vphrord xmm2, ymm3

// CHECK: vphrord xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x4e,0xd3]
          vphrord xmm2 {k7}, ymm3

// CHECK: vphrord xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrord xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrord xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x4e,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrord xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrord xmm2, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x4e,0x10]
          vphrord xmm2, dword ptr [eax]{1to4}

// CHECK: vphrord xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4e,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrord xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrord xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x4e,0x51,0x7f]
          vphrord xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphrord xmm2 {k7}, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x1f,0x4e,0x52,0x80]
          vphrord xmm2 {k7}, dword ptr [edx - 512]{1to4}

// CHECK: vphrord xmm2, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x4e,0x10]
          vphrord xmm2, dword ptr [eax]{1to8}

// CHECK: vphrord xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x4e,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrord xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrord xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x4e,0x51,0x7f]
          vphrord xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphrord xmm2 {k7}, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x3f,0x4e,0x52,0x80]
          vphrord xmm2 {k7}, dword ptr [edx - 512]{1to8}

// CHECK: vphrordq xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x46,0xd3]
          vphrordq xmm2, ymm3

// CHECK: vphrordq xmm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x46,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrordq xmm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrordq xmm2, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x46,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrordq xmm2, ymmword ptr [edi + 4*eax + 291]

// CHECK: vphrordq xmm2, ymmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x46,0x10]
          vphrordq xmm2, ymmword ptr [eax]

// CHECK: vphrordq xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x46,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrordq xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrordq xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x46,0x51,0x7f]
          vphrordq xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrordq xmm2, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x46,0x52,0x80]
          vphrordq xmm2, ymmword ptr [edx - 4096]

// CHECK: vphrorq xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4e,0xd3]
          vphrorq xmm2, xmm3

// CHECK: vphrorq xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x4e,0xd3]
          vphrorq xmm2 {k7}, xmm3

// CHECK: vphrorq xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x4e,0xd3]
          vphrorq xmm2, ymm3

// CHECK: vphrorq xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x2f,0x4e,0xd3]
          vphrorq xmm2 {k7}, ymm3

// CHECK: vphrorq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrorq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrorq xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x4e,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrorq xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrorq xmm2, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x4e,0x10]
          vphrorq xmm2, qword ptr [eax]{1to2}

// CHECK: vphrorq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4e,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrorq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrorq xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x4e,0x51,0x7f]
          vphrorq xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphrorq xmm2 {k7}, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x1f,0x4e,0x52,0x80]
          vphrorq xmm2 {k7}, qword ptr [edx - 1024]{1to2}

// CHECK: vphrorq xmm2, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x4e,0x10]
          vphrorq xmm2, qword ptr [eax]{1to4}

// CHECK: vphrorq xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x4e,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrorq xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrorq xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfd,0x2f,0x4e,0x51,0x7f]
          vphrorq xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphrorq xmm2 {k7}, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x3f,0x4e,0x52,0x80]
          vphrorq xmm2 {k7}, qword ptr [edx - 1024]{1to4}

// CHECK: vphrorw xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4e,0xd3]
          vphrorw xmm2, xmm3

// CHECK: vphrorw xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x4e,0xd3]
          vphrorw xmm2 {k7}, xmm3

// CHECK: vphrorw xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x4e,0xd3]
          vphrorw xmm2, ymm3

// CHECK: vphrorw xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x2f,0x4e,0xd3]
          vphrorw xmm2 {k7}, ymm3

// CHECK: vphrorw xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrorw xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrorw xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x4e,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrorw xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrorw xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x4e,0x10]
          vphrorw xmm2, word ptr [eax]{1to8}

// CHECK: vphrorw xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4e,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrorw xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrorw xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x4e,0x51,0x7f]
          vphrorw xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphrorw xmm2 {k7}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x1f,0x4e,0x52,0x80]
          vphrorw xmm2 {k7}, word ptr [edx - 256]{1to8}

// CHECK: vphrorw xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x4e,0x10]
          vphrorw xmm2, word ptr [eax]{1to16}

// CHECK: vphrorw xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x4e,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrorw xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrorw xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfc,0x2f,0x4e,0x51,0x7f]
          vphrorw xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphrorw xmm2 {k7}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x3f,0x4e,0x52,0x80]
          vphrorw xmm2 {k7}, word ptr [edx - 256]{1to16}

// CHECK: vphrxorb xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4f,0xd3]
          vphrxorb xmm2, xmm3

// CHECK: vphrxorb xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x4f,0xd3]
          vphrxorb xmm2 {k7}, xmm3

// CHECK: vphrxorb xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4f,0xd3]
          vphrxorb xmm2, ymm3

// CHECK: vphrxorb xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x4f,0xd3]
          vphrxorb xmm2 {k7}, ymm3

// CHECK: vphrxorb xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4f,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrxorb xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrxorb xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x4f,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrxorb xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrxorb xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4f,0x10]
          vphrxorb xmm2, xmmword ptr [eax]

// CHECK: vphrxorb xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x4f,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrxorb xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrxorb xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x4f,0x51,0x7f]
          vphrxorb xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphrxorb xmm2 {k7}, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x4f,0x52,0x80]
          vphrxorb xmm2 {k7}, xmmword ptr [edx - 2048]

// CHECK: vphrxorb xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x4f,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrxorb xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrxorb xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x4f,0x51,0x7f]
          vphrxorb xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphrxorb xmm2 {k7}, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x4f,0x52,0x80]
          vphrxorb xmm2 {k7}, ymmword ptr [edx - 4096]

// CHECK: vphrxord xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4f,0xd3]
          vphrxord xmm2, xmm3

// CHECK: vphrxord xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x4f,0xd3]
          vphrxord xmm2 {k7}, xmm3

// CHECK: vphrxord xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x4f,0xd3]
          vphrxord xmm2, ymm3

// CHECK: vphrxord xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x4f,0xd3]
          vphrxord xmm2 {k7}, ymm3

// CHECK: vphrxord xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4f,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrxord xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrxord xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x4f,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrxord xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrxord xmm2, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x4f,0x10]
          vphrxord xmm2, dword ptr [eax]{1to4}

// CHECK: vphrxord xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x4f,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrxord xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrxord xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x4f,0x51,0x7f]
          vphrxord xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphrxord xmm2 {k7}, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf5,0x7d,0x1f,0x4f,0x52,0x80]
          vphrxord xmm2 {k7}, dword ptr [edx - 512]{1to4}

// CHECK: vphrxord xmm2, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x4f,0x10]
          vphrxord xmm2, dword ptr [eax]{1to8}

// CHECK: vphrxord xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x4f,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrxord xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrxord xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x4f,0x51,0x7f]
          vphrxord xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphrxord xmm2 {k7}, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7d,0x3f,0x4f,0x52,0x80]
          vphrxord xmm2 {k7}, dword ptr [edx - 512]{1to8}

// CHECK: vphrxordq xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x47,0xd3]
          vphrxordq xmm2, ymm3

// CHECK: vphrxordq xmm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x47,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrxordq xmm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrxordq xmm2, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x47,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrxordq xmm2, ymmword ptr [edi + 4*eax + 291]

// CHECK: vphrxordq xmm2, ymmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x47,0x10]
          vphrxordq xmm2, ymmword ptr [eax]

// CHECK: vphrxordq xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x47,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrxordq xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrxordq xmm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x47,0x51,0x7f]
          vphrxordq xmm2, ymmword ptr [ecx + 4064]

// CHECK: vphrxordq xmm2, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x47,0x52,0x80]
          vphrxordq xmm2, ymmword ptr [edx - 4096]

// CHECK: vphrxorq xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4f,0xd3]
          vphrxorq xmm2, xmm3

// CHECK: vphrxorq xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x4f,0xd3]
          vphrxorq xmm2 {k7}, xmm3

// CHECK: vphrxorq xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x4f,0xd3]
          vphrxorq xmm2, ymm3

// CHECK: vphrxorq xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0xfd,0x2f,0x4f,0xd3]
          vphrxorq xmm2 {k7}, ymm3

// CHECK: vphrxorq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4f,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrxorq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrxorq xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x4f,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrxorq xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrxorq xmm2, qword ptr [eax]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x18,0x4f,0x10]
          vphrxorq xmm2, qword ptr [eax]{1to2}

// CHECK: vphrxorq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfd,0x08,0x4f,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrxorq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrxorq xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfd,0x0f,0x4f,0x51,0x7f]
          vphrxorq xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphrxorq xmm2 {k7}, qword ptr [edx - 1024]{1to2}
// CHECK: encoding: [0x62,0xf5,0xfd,0x1f,0x4f,0x52,0x80]
          vphrxorq xmm2 {k7}, qword ptr [edx - 1024]{1to2}

// CHECK: vphrxorq xmm2, qword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x38,0x4f,0x10]
          vphrxorq xmm2, qword ptr [eax]{1to4}

// CHECK: vphrxorq xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfd,0x28,0x4f,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrxorq xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrxorq xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfd,0x2f,0x4f,0x51,0x7f]
          vphrxorq xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphrxorq xmm2 {k7}, qword ptr [edx - 1024]{1to4}
// CHECK: encoding: [0x62,0xf5,0xfd,0x3f,0x4f,0x52,0x80]
          vphrxorq xmm2 {k7}, qword ptr [edx - 1024]{1to4}

// CHECK: vphrxorw xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4f,0xd3]
          vphrxorw xmm2, xmm3

// CHECK: vphrxorw xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x4f,0xd3]
          vphrxorw xmm2 {k7}, xmm3

// CHECK: vphrxorw xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x4f,0xd3]
          vphrxorw xmm2, ymm3

// CHECK: vphrxorw xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0xfc,0x2f,0x4f,0xd3]
          vphrxorw xmm2 {k7}, ymm3

// CHECK: vphrxorw xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4f,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphrxorw xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vphrxorw xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x4f,0x94,0x87,0x23,0x01,0x00,0x00]
          vphrxorw xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vphrxorw xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x18,0x4f,0x10]
          vphrxorw xmm2, word ptr [eax]{1to8}

// CHECK: vphrxorw xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0xfc,0x08,0x4f,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphrxorw xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vphrxorw xmm2 {k7}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0xfc,0x0f,0x4f,0x51,0x7f]
          vphrxorw xmm2 {k7}, xmmword ptr [ecx + 2032]

// CHECK: vphrxorw xmm2 {k7}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0xfc,0x1f,0x4f,0x52,0x80]
          vphrxorw xmm2 {k7}, word ptr [edx - 256]{1to8}

// CHECK: vphrxorw xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x38,0x4f,0x10]
          vphrxorw xmm2, word ptr [eax]{1to16}

// CHECK: vphrxorw xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0xfc,0x28,0x4f,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphrxorw xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vphrxorw xmm2 {k7}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0xfc,0x2f,0x4f,0x51,0x7f]
          vphrxorw xmm2 {k7}, ymmword ptr [ecx + 4064]

// CHECK: vphrxorw xmm2 {k7}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0xfc,0x3f,0x4f,0x52,0x80]
          vphrxorw xmm2 {k7}, word ptr [edx - 256]{1to16}

