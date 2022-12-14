// REQUIRES: intel_feature_isa_avx512_ne_convert_fp8
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vcvtbias2ph2bf8 zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x74,0xd4]
          vcvtbias2ph2bf8 zmm2, zmm3, zmm4

// CHECK: vcvtbias2ph2bf8 zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8 zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbias2ph2bf8 zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8 zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbias2ph2bf8 zmm2, zmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf2,0x65,0x58,0x74,0x10]
          vcvtbias2ph2bf8 zmm2, zmm3, word ptr [eax]{1to32}

// CHECK: vcvtbias2ph2bf8 zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2bf8 zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtbias2ph2bf8 zmm2, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x74,0x51,0x7f]
          vcvtbias2ph2bf8 zmm2, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vcvtbias2ph2bf8 zmm2, zmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf2,0x65,0x58,0x74,0x52,0x80]
          vcvtbias2ph2bf8 zmm2, zmm3, word ptr [edx - 256]{1to32}

// CHECK: vcvtbias2ph2bf8s zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x74,0xd4]
          vcvtbias2ph2bf8s zmm2, zmm3, zmm4

// CHECK: vcvtbias2ph2bf8s zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8s zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbias2ph2bf8s zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8s zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbias2ph2bf8s zmm2, zmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x74,0x10]
          vcvtbias2ph2bf8s zmm2, zmm3, word ptr [eax]{1to32}

// CHECK: vcvtbias2ph2bf8s zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2bf8s zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtbias2ph2bf8s zmm2, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x74,0x51,0x7f]
          vcvtbias2ph2bf8s zmm2, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vcvtbias2ph2bf8s zmm2, zmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x74,0x52,0x80]
          vcvtbias2ph2bf8s zmm2, zmm3, word ptr [edx - 256]{1to32}

// CHECK: vcvtbias2ph2hf8 zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x18,0xd4]
          vcvtbias2ph2hf8 zmm2, zmm3, zmm4

// CHECK: vcvtbias2ph2hf8 zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8 zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbias2ph2hf8 zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8 zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbias2ph2hf8 zmm2, zmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x18,0x10]
          vcvtbias2ph2hf8 zmm2, zmm3, word ptr [eax]{1to32}

// CHECK: vcvtbias2ph2hf8 zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x18,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2hf8 zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtbias2ph2hf8 zmm2, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x18,0x51,0x7f]
          vcvtbias2ph2hf8 zmm2, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vcvtbias2ph2hf8 zmm2, zmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x18,0x52,0x80]
          vcvtbias2ph2hf8 zmm2, zmm3, word ptr [edx - 256]{1to32}

// CHECK: vcvtbias2ph2hf8s zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x1b,0xd4]
          vcvtbias2ph2hf8s zmm2, zmm3, zmm4

// CHECK: vcvtbias2ph2hf8s zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8s zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbias2ph2hf8s zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8s zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbias2ph2hf8s zmm2, zmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x1b,0x10]
          vcvtbias2ph2hf8s zmm2, zmm3, word ptr [eax]{1to32}

// CHECK: vcvtbias2ph2hf8s zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x1b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2hf8s zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtbias2ph2hf8s zmm2, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x1b,0x51,0x7f]
          vcvtbias2ph2hf8s zmm2, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vcvtbias2ph2hf8s zmm2, zmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x1b,0x52,0x80]
          vcvtbias2ph2hf8s zmm2, zmm3, word ptr [edx - 256]{1to32}

// CHECK: vcvtbiasph2bf8 ymm2, zmm3
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0x74,0xd3]
          vcvtbiasph2bf8 ymm2, zmm3

// CHECK: vcvtbiasph2bf8 ymm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0x74,0xd3]
          vcvtbiasph2bf8 ymm2 {k7}, zmm3

// CHECK: vcvtbiasph2bf8 ymm2 {k7} {z}, zmm3
// CHECK: encoding: [0x62,0xf2,0x7c,0xcf,0x74,0xd3]
          vcvtbiasph2bf8 ymm2 {k7} {z}, zmm3

// CHECK: vcvtbiasph2bf8 ymm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8 ymm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbiasph2bf8 ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8 ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbiasph2bf8 ymm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf2,0x7c,0x58,0x74,0x10]
          vcvtbiasph2bf8 ymm2, word ptr [eax]{1to32}

// CHECK: vcvtbiasph2bf8 ymm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2bf8 ymm2, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtbiasph2bf8 ymm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x7c,0xcf,0x74,0x51,0x7f]
          vcvtbiasph2bf8 ymm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK: vcvtbiasph2bf8 ymm2 {k7} {z}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf2,0x7c,0xdf,0x74,0x52,0x80]
          vcvtbiasph2bf8 ymm2 {k7} {z}, word ptr [edx - 256]{1to32}

// CHECK: vcvtbiasph2bf8s ymm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x74,0xd3]
          vcvtbiasph2bf8s ymm2, zmm3

// CHECK: vcvtbiasph2bf8s ymm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x74,0xd3]
          vcvtbiasph2bf8s ymm2 {k7}, zmm3

// CHECK: vcvtbiasph2bf8s ymm2 {k7} {z}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0xcf,0x74,0xd3]
          vcvtbiasph2bf8s ymm2 {k7} {z}, zmm3

// CHECK: vcvtbiasph2bf8s ymm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8s ymm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbiasph2bf8s ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8s ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbiasph2bf8s ymm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0x7c,0x58,0x74,0x10]
          vcvtbiasph2bf8s ymm2, word ptr [eax]{1to32}

// CHECK: vcvtbiasph2bf8s ymm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2bf8s ymm2, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtbiasph2bf8s ymm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7c,0xcf,0x74,0x51,0x7f]
          vcvtbiasph2bf8s ymm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK: vcvtbiasph2bf8s ymm2 {k7} {z}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0x7c,0xdf,0x74,0x52,0x80]
          vcvtbiasph2bf8s ymm2 {k7} {z}, word ptr [edx - 256]{1to32}

// CHECK: vcvtbiasph2hf8 ymm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x18,0xd3]
          vcvtbiasph2hf8 ymm2, zmm3

// CHECK: vcvtbiasph2hf8 ymm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x18,0xd3]
          vcvtbiasph2hf8 ymm2 {k7}, zmm3

// CHECK: vcvtbiasph2hf8 ymm2 {k7} {z}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0xcf,0x18,0xd3]
          vcvtbiasph2hf8 ymm2 {k7} {z}, zmm3

// CHECK: vcvtbiasph2hf8 ymm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8 ymm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbiasph2hf8 ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8 ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbiasph2hf8 ymm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0x7c,0x58,0x18,0x10]
          vcvtbiasph2hf8 ymm2, word ptr [eax]{1to32}

// CHECK: vcvtbiasph2hf8 ymm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x18,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2hf8 ymm2, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtbiasph2hf8 ymm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7c,0xcf,0x18,0x51,0x7f]
          vcvtbiasph2hf8 ymm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK: vcvtbiasph2hf8 ymm2 {k7} {z}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0x7c,0xdf,0x18,0x52,0x80]
          vcvtbiasph2hf8 ymm2 {k7} {z}, word ptr [edx - 256]{1to32}

// CHECK: vcvtbiasph2hf8s ymm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x1b,0xd3]
          vcvtbiasph2hf8s ymm2, zmm3

// CHECK: vcvtbiasph2hf8s ymm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x1b,0xd3]
          vcvtbiasph2hf8s ymm2 {k7}, zmm3

// CHECK: vcvtbiasph2hf8s ymm2 {k7} {z}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0xcf,0x1b,0xd3]
          vcvtbiasph2hf8s ymm2 {k7} {z}, zmm3

// CHECK: vcvtbiasph2hf8s ymm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8s ymm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbiasph2hf8s ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8s ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbiasph2hf8s ymm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0x7c,0x58,0x1b,0x10]
          vcvtbiasph2hf8s ymm2, word ptr [eax]{1to32}

// CHECK: vcvtbiasph2hf8s ymm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x1b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2hf8s ymm2, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtbiasph2hf8s ymm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7c,0xcf,0x1b,0x51,0x7f]
          vcvtbiasph2hf8s ymm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK: vcvtbiasph2hf8s ymm2 {k7} {z}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0x7c,0xdf,0x1b,0x52,0x80]
          vcvtbiasph2hf8s ymm2 {k7} {z}, word ptr [edx - 256]{1to32}

// CHECK: vcvtne2ph2bf8 zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x74,0xd4]
          vcvtne2ph2bf8 zmm2, zmm3, zmm4

// CHECK: vcvtne2ph2bf8 zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8 zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtne2ph2bf8 zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8 zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtne2ph2bf8 zmm2, zmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf2,0x67,0x58,0x74,0x10]
          vcvtne2ph2bf8 zmm2, zmm3, word ptr [eax]{1to32}

// CHECK: vcvtne2ph2bf8 zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2bf8 zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtne2ph2bf8 zmm2, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x74,0x51,0x7f]
          vcvtne2ph2bf8 zmm2, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vcvtne2ph2bf8 zmm2, zmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf2,0x67,0x58,0x74,0x52,0x80]
          vcvtne2ph2bf8 zmm2, zmm3, word ptr [edx - 256]{1to32}

// CHECK: vcvtne2ph2bf8s zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x74,0xd4]
          vcvtne2ph2bf8s zmm2, zmm3, zmm4

// CHECK: vcvtne2ph2bf8s zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8s zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtne2ph2bf8s zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8s zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtne2ph2bf8s zmm2, zmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x74,0x10]
          vcvtne2ph2bf8s zmm2, zmm3, word ptr [eax]{1to32}

// CHECK: vcvtne2ph2bf8s zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2bf8s zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtne2ph2bf8s zmm2, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x74,0x51,0x7f]
          vcvtne2ph2bf8s zmm2, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vcvtne2ph2bf8s zmm2, zmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x74,0x52,0x80]
          vcvtne2ph2bf8s zmm2, zmm3, word ptr [edx - 256]{1to32}

// CHECK: vcvtne2ph2hf8 zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x18,0xd4]
          vcvtne2ph2hf8 zmm2, zmm3, zmm4

// CHECK: vcvtne2ph2hf8 zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8 zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtne2ph2hf8 zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8 zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtne2ph2hf8 zmm2, zmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x18,0x10]
          vcvtne2ph2hf8 zmm2, zmm3, word ptr [eax]{1to32}

// CHECK: vcvtne2ph2hf8 zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x18,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2hf8 zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtne2ph2hf8 zmm2, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x18,0x51,0x7f]
          vcvtne2ph2hf8 zmm2, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vcvtne2ph2hf8 zmm2, zmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x18,0x52,0x80]
          vcvtne2ph2hf8 zmm2, zmm3, word ptr [edx - 256]{1to32}

// CHECK: vcvtne2ph2hf8s zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x1b,0xd4]
          vcvtne2ph2hf8s zmm2, zmm3, zmm4

// CHECK: vcvtne2ph2hf8s zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8s zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtne2ph2hf8s zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8s zmm2, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtne2ph2hf8s zmm2, zmm3, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x1b,0x10]
          vcvtne2ph2hf8s zmm2, zmm3, word ptr [eax]{1to32}

// CHECK: vcvtne2ph2hf8s zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x1b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2hf8s zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtne2ph2hf8s zmm2, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x1b,0x51,0x7f]
          vcvtne2ph2hf8s zmm2, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vcvtne2ph2hf8s zmm2, zmm3, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x1b,0x52,0x80]
          vcvtne2ph2hf8s zmm2, zmm3, word ptr [edx - 256]{1to32}

// CHECK: vcvtnebf82ph zmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1e,0xd3]
          vcvtnebf82ph zmm2, ymm3

// CHECK: vcvtnebf82ph zmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x1e,0xd3]
          vcvtnebf82ph zmm2 {k7}, ymm3

// CHECK: vcvtnebf82ph zmm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x1e,0xd3]
          vcvtnebf82ph zmm2 {k7} {z}, ymm3

// CHECK: vcvtnebf82ph zmm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnebf82ph zmm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtnebf82ph zmm2 {k7}, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x1e,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnebf82ph zmm2 {k7}, ymmword ptr [edi + 4*eax + 291]

// CHECK: vcvtnebf82ph zmm2, ymmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1e,0x10]
          vcvtnebf82ph zmm2, ymmword ptr [eax]

// CHECK: vcvtnebf82ph zmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1e,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtnebf82ph zmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtnebf82ph zmm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x1e,0x51,0x7f]
          vcvtnebf82ph zmm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK: vcvtnebf82ph zmm2 {k7} {z}, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x1e,0x52,0x80]
          vcvtnebf82ph zmm2 {k7} {z}, ymmword ptr [edx - 4096]

// CHECK: vcvtnehf82ph zmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7f,0x48,0x1e,0xd3]
          vcvtnehf82ph zmm2, ymm3

// CHECK: vcvtnehf82ph zmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7f,0x4f,0x1e,0xd3]
          vcvtnehf82ph zmm2 {k7}, ymm3

// CHECK: vcvtnehf82ph zmm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7f,0xcf,0x1e,0xd3]
          vcvtnehf82ph zmm2 {k7} {z}, ymm3

// CHECK: vcvtnehf82ph zmm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7f,0x48,0x1e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnehf82ph zmm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtnehf82ph zmm2 {k7}, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7f,0x4f,0x1e,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnehf82ph zmm2 {k7}, ymmword ptr [edi + 4*eax + 291]

// CHECK: vcvtnehf82ph zmm2, ymmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7f,0x48,0x1e,0x10]
          vcvtnehf82ph zmm2, ymmword ptr [eax]

// CHECK: vcvtnehf82ph zmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7f,0x48,0x1e,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtnehf82ph zmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtnehf82ph zmm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7f,0xcf,0x1e,0x51,0x7f]
          vcvtnehf82ph zmm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK: vcvtnehf82ph zmm2 {k7} {z}, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf5,0x7f,0xcf,0x1e,0x52,0x80]
          vcvtnehf82ph zmm2 {k7} {z}, ymmword ptr [edx - 4096]

// CHECK: vcvtneph2bf8 ymm2, zmm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0x74,0xd3]
          vcvtneph2bf8 ymm2, zmm3

// CHECK: vcvtneph2bf8 ymm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0x74,0xd3]
          vcvtneph2bf8 ymm2 {k7}, zmm3

// CHECK: vcvtneph2bf8 ymm2 {k7} {z}, zmm3
// CHECK: encoding: [0x62,0xf2,0x7e,0xcf,0x74,0xd3]
          vcvtneph2bf8 ymm2 {k7} {z}, zmm3

// CHECK: vcvtneph2bf8 ymm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2bf8 ymm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtneph2bf8 ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2bf8 ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtneph2bf8 ymm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf2,0x7e,0x58,0x74,0x10]
          vcvtneph2bf8 ymm2, word ptr [eax]{1to32}

// CHECK: vcvtneph2bf8 ymm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2bf8 ymm2, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtneph2bf8 ymm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x7e,0xcf,0x74,0x51,0x7f]
          vcvtneph2bf8 ymm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK: vcvtneph2bf8 ymm2 {k7} {z}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf2,0x7e,0xdf,0x74,0x52,0x80]
          vcvtneph2bf8 ymm2 {k7} {z}, word ptr [edx - 256]{1to32}

// CHECK: vcvtneph2bf8s ymm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x74,0xd3]
          vcvtneph2bf8s ymm2, zmm3

// CHECK: vcvtneph2bf8s ymm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x74,0xd3]
          vcvtneph2bf8s ymm2 {k7}, zmm3

// CHECK: vcvtneph2bf8s ymm2 {k7} {z}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x74,0xd3]
          vcvtneph2bf8s ymm2 {k7} {z}, zmm3

// CHECK: vcvtneph2bf8s ymm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2bf8s ymm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtneph2bf8s ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2bf8s ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtneph2bf8s ymm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0x7e,0x58,0x74,0x10]
          vcvtneph2bf8s ymm2, word ptr [eax]{1to32}

// CHECK: vcvtneph2bf8s ymm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2bf8s ymm2, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtneph2bf8s ymm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x74,0x51,0x7f]
          vcvtneph2bf8s ymm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK: vcvtneph2bf8s ymm2 {k7} {z}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0x7e,0xdf,0x74,0x52,0x80]
          vcvtneph2bf8s ymm2 {k7} {z}, word ptr [edx - 256]{1to32}

// CHECK: vcvtneph2hf8 ymm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x18,0xd3]
          vcvtneph2hf8 ymm2, zmm3

// CHECK: vcvtneph2hf8 ymm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x18,0xd3]
          vcvtneph2hf8 ymm2 {k7}, zmm3

// CHECK: vcvtneph2hf8 ymm2 {k7} {z}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x18,0xd3]
          vcvtneph2hf8 ymm2 {k7} {z}, zmm3

// CHECK: vcvtneph2hf8 ymm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2hf8 ymm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtneph2hf8 ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2hf8 ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtneph2hf8 ymm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0x7e,0x58,0x18,0x10]
          vcvtneph2hf8 ymm2, word ptr [eax]{1to32}

// CHECK: vcvtneph2hf8 ymm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x18,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2hf8 ymm2, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtneph2hf8 ymm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x18,0x51,0x7f]
          vcvtneph2hf8 ymm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK: vcvtneph2hf8 ymm2 {k7} {z}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0x7e,0xdf,0x18,0x52,0x80]
          vcvtneph2hf8 ymm2 {k7} {z}, word ptr [edx - 256]{1to32}

// CHECK: vcvtneph2hf8s ymm2, zmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1b,0xd3]
          vcvtneph2hf8s ymm2, zmm3

// CHECK: vcvtneph2hf8s ymm2 {k7}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x1b,0xd3]
          vcvtneph2hf8s ymm2 {k7}, zmm3

// CHECK: vcvtneph2hf8s ymm2 {k7} {z}, zmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x1b,0xd3]
          vcvtneph2hf8s ymm2 {k7} {z}, zmm3

// CHECK: vcvtneph2hf8s ymm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2hf8s ymm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtneph2hf8s ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2hf8s ymm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtneph2hf8s ymm2, word ptr [eax]{1to32}
// CHECK: encoding: [0x62,0xf5,0x7e,0x58,0x1b,0x10]
          vcvtneph2hf8s ymm2, word ptr [eax]{1to32}

// CHECK: vcvtneph2hf8s ymm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2hf8s ymm2, zmmword ptr [2*ebp - 2048]

// CHECK: vcvtneph2hf8s ymm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x1b,0x51,0x7f]
          vcvtneph2hf8s ymm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK: vcvtneph2hf8s ymm2 {k7} {z}, word ptr [edx - 256]{1to32}
// CHECK: encoding: [0x62,0xf5,0x7e,0xdf,0x1b,0x52,0x80]
          vcvtneph2hf8s ymm2 {k7} {z}, word ptr [edx - 256]{1to32}

