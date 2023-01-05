// REQUIRES: intel_feature_isa_avx512_ne_convert_fp8
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vcvtbias2ph2bf8 ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x74,0xd4]
          vcvtbias2ph2bf8 ymm2, ymm3, ymm4

// CHECK: vcvtbias2ph2bf8 xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x74,0xd4]
          vcvtbias2ph2bf8 xmm2, xmm3, xmm4

// CHECK: vcvtbias2ph2bf8 ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8 ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbias2ph2bf8 ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8 ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbias2ph2bf8 ymm2, ymm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x65,0x38,0x74,0x10]
          vcvtbias2ph2bf8 ymm2, ymm3, word ptr [eax]{1to16}

// CHECK: vcvtbias2ph2bf8 ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2bf8 ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtbias2ph2bf8 ymm2, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x74,0x51,0x7f]
          vcvtbias2ph2bf8 ymm2, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vcvtbias2ph2bf8 ymm2, ymm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf2,0x65,0x38,0x74,0x52,0x80]
          vcvtbias2ph2bf8 ymm2, ymm3, word ptr [edx - 256]{1to16}

// CHECK: vcvtbias2ph2bf8 xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8 xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbias2ph2bf8 xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8 xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbias2ph2bf8 xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0x74,0x10]
          vcvtbias2ph2bf8 xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vcvtbias2ph2bf8 xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2bf8 xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vcvtbias2ph2bf8 xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x74,0x51,0x7f]
          vcvtbias2ph2bf8 xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vcvtbias2ph2bf8 xmm2, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0x74,0x52,0x80]
          vcvtbias2ph2bf8 xmm2, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vcvtbias2ph2bf8s ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x74,0xd4]
          vcvtbias2ph2bf8s ymm2, ymm3, ymm4

// CHECK: vcvtbias2ph2bf8s xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x74,0xd4]
          vcvtbias2ph2bf8s xmm2, xmm3, xmm4

// CHECK: vcvtbias2ph2bf8s ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8s ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbias2ph2bf8s ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8s ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbias2ph2bf8s ymm2, ymm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x74,0x10]
          vcvtbias2ph2bf8s ymm2, ymm3, word ptr [eax]{1to16}

// CHECK: vcvtbias2ph2bf8s ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2bf8s ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtbias2ph2bf8s ymm2, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x74,0x51,0x7f]
          vcvtbias2ph2bf8s ymm2, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vcvtbias2ph2bf8s ymm2, ymm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x74,0x52,0x80]
          vcvtbias2ph2bf8s ymm2, ymm3, word ptr [edx - 256]{1to16}

// CHECK: vcvtbias2ph2bf8s xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8s xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbias2ph2bf8s xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8s xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbias2ph2bf8s xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x74,0x10]
          vcvtbias2ph2bf8s xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vcvtbias2ph2bf8s xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2bf8s xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vcvtbias2ph2bf8s xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x74,0x51,0x7f]
          vcvtbias2ph2bf8s xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vcvtbias2ph2bf8s xmm2, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x74,0x52,0x80]
          vcvtbias2ph2bf8s xmm2, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vcvtbias2ph2hf8 ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x18,0xd4]
          vcvtbias2ph2hf8 ymm2, ymm3, ymm4

// CHECK: vcvtbias2ph2hf8 xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x18,0xd4]
          vcvtbias2ph2hf8 xmm2, xmm3, xmm4

// CHECK: vcvtbias2ph2hf8 ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8 ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbias2ph2hf8 ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8 ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbias2ph2hf8 ymm2, ymm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x18,0x10]
          vcvtbias2ph2hf8 ymm2, ymm3, word ptr [eax]{1to16}

// CHECK: vcvtbias2ph2hf8 ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x18,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2hf8 ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtbias2ph2hf8 ymm2, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x18,0x51,0x7f]
          vcvtbias2ph2hf8 ymm2, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vcvtbias2ph2hf8 ymm2, ymm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x18,0x52,0x80]
          vcvtbias2ph2hf8 ymm2, ymm3, word ptr [edx - 256]{1to16}

// CHECK: vcvtbias2ph2hf8 xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8 xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbias2ph2hf8 xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8 xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbias2ph2hf8 xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x18,0x10]
          vcvtbias2ph2hf8 xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vcvtbias2ph2hf8 xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x18,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2hf8 xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vcvtbias2ph2hf8 xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x18,0x51,0x7f]
          vcvtbias2ph2hf8 xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vcvtbias2ph2hf8 xmm2, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x18,0x52,0x80]
          vcvtbias2ph2hf8 xmm2, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vcvtbias2ph2hf8s ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x1b,0xd4]
          vcvtbias2ph2hf8s ymm2, ymm3, ymm4

// CHECK: vcvtbias2ph2hf8s xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x1b,0xd4]
          vcvtbias2ph2hf8s xmm2, xmm3, xmm4

// CHECK: vcvtbias2ph2hf8s ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8s ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbias2ph2hf8s ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8s ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbias2ph2hf8s ymm2, ymm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x1b,0x10]
          vcvtbias2ph2hf8s ymm2, ymm3, word ptr [eax]{1to16}

// CHECK: vcvtbias2ph2hf8s ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x1b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2hf8s ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtbias2ph2hf8s ymm2, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x1b,0x51,0x7f]
          vcvtbias2ph2hf8s ymm2, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vcvtbias2ph2hf8s ymm2, ymm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x1b,0x52,0x80]
          vcvtbias2ph2hf8s ymm2, ymm3, word ptr [edx - 256]{1to16}

// CHECK: vcvtbias2ph2hf8s xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8s xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbias2ph2hf8s xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8s xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbias2ph2hf8s xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x1b,0x10]
          vcvtbias2ph2hf8s xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vcvtbias2ph2hf8s xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x1b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2hf8s xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vcvtbias2ph2hf8s xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x1b,0x51,0x7f]
          vcvtbias2ph2hf8s xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vcvtbias2ph2hf8s xmm2, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x1b,0x52,0x80]
          vcvtbias2ph2hf8s xmm2, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vcvtbiasph2bf8 xmm2, xmm3
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x74,0xd3]
          vcvtbiasph2bf8 xmm2, xmm3

// CHECK: vcvtbiasph2bf8 xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x74,0xd3]
          vcvtbiasph2bf8 xmm2 {k7}, xmm3

// CHECK: vcvtbiasph2bf8 xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf2,0x7c,0x8f,0x74,0xd3]
          vcvtbiasph2bf8 xmm2 {k7} {z}, xmm3

// CHECK: vcvtbiasph2bf8 xmm2, ymm3
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x74,0xd3]
          vcvtbiasph2bf8 xmm2, ymm3

// CHECK: vcvtbiasph2bf8 xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x74,0xd3]
          vcvtbiasph2bf8 xmm2 {k7}, ymm3

// CHECK: vcvtbiasph2bf8 xmm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf2,0x7c,0xaf,0x74,0xd3]
          vcvtbiasph2bf8 xmm2 {k7} {z}, ymm3

// CHECK: vcvtbiasph2bf8 xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8 xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbiasph2bf8 xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8 xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbiasph2bf8 xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x7c,0x18,0x74,0x10]
          vcvtbiasph2bf8 xmm2, word ptr [eax]{1to8}

// CHECK: vcvtbiasph2bf8 xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2bf8 xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vcvtbiasph2bf8 xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x7c,0x8f,0x74,0x51,0x7f]
          vcvtbiasph2bf8 xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK: vcvtbiasph2bf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf2,0x7c,0x9f,0x74,0x52,0x80]
          vcvtbiasph2bf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to8}

// CHECK: vcvtbiasph2bf8 xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x7c,0x38,0x74,0x10]
          vcvtbiasph2bf8 xmm2, word ptr [eax]{1to16}

// CHECK: vcvtbiasph2bf8 xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2bf8 xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtbiasph2bf8 xmm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x7c,0xaf,0x74,0x51,0x7f]
          vcvtbiasph2bf8 xmm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK: vcvtbiasph2bf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf2,0x7c,0xbf,0x74,0x52,0x80]
          vcvtbiasph2bf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to16}

// CHECK: vcvtbiasph2bf8s xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x74,0xd3]
          vcvtbiasph2bf8s xmm2, xmm3

// CHECK: vcvtbiasph2bf8s xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x74,0xd3]
          vcvtbiasph2bf8s xmm2 {k7}, xmm3

// CHECK: vcvtbiasph2bf8s xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x74,0xd3]
          vcvtbiasph2bf8s xmm2 {k7} {z}, xmm3

// CHECK: vcvtbiasph2bf8s xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x74,0xd3]
          vcvtbiasph2bf8s xmm2, ymm3

// CHECK: vcvtbiasph2bf8s xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x74,0xd3]
          vcvtbiasph2bf8s xmm2 {k7}, ymm3

// CHECK: vcvtbiasph2bf8s xmm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x74,0xd3]
          vcvtbiasph2bf8s xmm2 {k7} {z}, ymm3

// CHECK: vcvtbiasph2bf8s xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8s xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbiasph2bf8s xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8s xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbiasph2bf8s xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7c,0x18,0x74,0x10]
          vcvtbiasph2bf8s xmm2, word ptr [eax]{1to8}

// CHECK: vcvtbiasph2bf8s xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2bf8s xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vcvtbiasph2bf8s xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x74,0x51,0x7f]
          vcvtbiasph2bf8s xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK: vcvtbiasph2bf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7c,0x9f,0x74,0x52,0x80]
          vcvtbiasph2bf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to8}

// CHECK: vcvtbiasph2bf8s xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7c,0x38,0x74,0x10]
          vcvtbiasph2bf8s xmm2, word ptr [eax]{1to16}

// CHECK: vcvtbiasph2bf8s xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2bf8s xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtbiasph2bf8s xmm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x74,0x51,0x7f]
          vcvtbiasph2bf8s xmm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK: vcvtbiasph2bf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7c,0xbf,0x74,0x52,0x80]
          vcvtbiasph2bf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to16}

// CHECK: vcvtbiasph2hf8 xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x18,0xd3]
          vcvtbiasph2hf8 xmm2, xmm3

// CHECK: vcvtbiasph2hf8 xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x18,0xd3]
          vcvtbiasph2hf8 xmm2 {k7}, xmm3

// CHECK: vcvtbiasph2hf8 xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x18,0xd3]
          vcvtbiasph2hf8 xmm2 {k7} {z}, xmm3

// CHECK: vcvtbiasph2hf8 xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x18,0xd3]
          vcvtbiasph2hf8 xmm2, ymm3

// CHECK: vcvtbiasph2hf8 xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x18,0xd3]
          vcvtbiasph2hf8 xmm2 {k7}, ymm3

// CHECK: vcvtbiasph2hf8 xmm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x18,0xd3]
          vcvtbiasph2hf8 xmm2 {k7} {z}, ymm3

// CHECK: vcvtbiasph2hf8 xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8 xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbiasph2hf8 xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8 xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbiasph2hf8 xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7c,0x18,0x18,0x10]
          vcvtbiasph2hf8 xmm2, word ptr [eax]{1to8}

// CHECK: vcvtbiasph2hf8 xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x18,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2hf8 xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vcvtbiasph2hf8 xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x18,0x51,0x7f]
          vcvtbiasph2hf8 xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK: vcvtbiasph2hf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7c,0x9f,0x18,0x52,0x80]
          vcvtbiasph2hf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to8}

// CHECK: vcvtbiasph2hf8 xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7c,0x38,0x18,0x10]
          vcvtbiasph2hf8 xmm2, word ptr [eax]{1to16}

// CHECK: vcvtbiasph2hf8 xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x18,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2hf8 xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtbiasph2hf8 xmm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x18,0x51,0x7f]
          vcvtbiasph2hf8 xmm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK: vcvtbiasph2hf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7c,0xbf,0x18,0x52,0x80]
          vcvtbiasph2hf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to16}

// CHECK: vcvtbiasph2hf8s xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x1b,0xd3]
          vcvtbiasph2hf8s xmm2, xmm3

// CHECK: vcvtbiasph2hf8s xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x1b,0xd3]
          vcvtbiasph2hf8s xmm2 {k7}, xmm3

// CHECK: vcvtbiasph2hf8s xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x1b,0xd3]
          vcvtbiasph2hf8s xmm2 {k7} {z}, xmm3

// CHECK: vcvtbiasph2hf8s xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x1b,0xd3]
          vcvtbiasph2hf8s xmm2, ymm3

// CHECK: vcvtbiasph2hf8s xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x1b,0xd3]
          vcvtbiasph2hf8s xmm2 {k7}, ymm3

// CHECK: vcvtbiasph2hf8s xmm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x1b,0xd3]
          vcvtbiasph2hf8s xmm2 {k7} {z}, ymm3

// CHECK: vcvtbiasph2hf8s xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8s xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtbiasph2hf8s xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8s xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtbiasph2hf8s xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7c,0x18,0x1b,0x10]
          vcvtbiasph2hf8s xmm2, word ptr [eax]{1to8}

// CHECK: vcvtbiasph2hf8s xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x1b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2hf8s xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vcvtbiasph2hf8s xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x1b,0x51,0x7f]
          vcvtbiasph2hf8s xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK: vcvtbiasph2hf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7c,0x9f,0x1b,0x52,0x80]
          vcvtbiasph2hf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to8}

// CHECK: vcvtbiasph2hf8s xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7c,0x38,0x1b,0x10]
          vcvtbiasph2hf8s xmm2, word ptr [eax]{1to16}

// CHECK: vcvtbiasph2hf8s xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x1b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2hf8s xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtbiasph2hf8s xmm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x1b,0x51,0x7f]
          vcvtbiasph2hf8s xmm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK: vcvtbiasph2hf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7c,0xbf,0x1b,0x52,0x80]
          vcvtbiasph2hf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to16}

// CHECK: vcvtne2ph2bf8 ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x74,0xd4]
          vcvtne2ph2bf8 ymm2, ymm3, ymm4

// CHECK: vcvtne2ph2bf8 xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x74,0xd4]
          vcvtne2ph2bf8 xmm2, xmm3, xmm4

// CHECK: vcvtne2ph2bf8 ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8 ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtne2ph2bf8 ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8 ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vcvtne2ph2bf8 ymm2, ymm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x67,0x38,0x74,0x10]
          vcvtne2ph2bf8 ymm2, ymm3, word ptr [eax]{1to16}

// CHECK: vcvtne2ph2bf8 ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2bf8 ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtne2ph2bf8 ymm2, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x74,0x51,0x7f]
          vcvtne2ph2bf8 ymm2, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vcvtne2ph2bf8 ymm2, ymm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf2,0x67,0x38,0x74,0x52,0x80]
          vcvtne2ph2bf8 ymm2, ymm3, word ptr [edx - 256]{1to16}

// CHECK: vcvtne2ph2bf8 xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8 xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtne2ph2bf8 xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8 xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtne2ph2bf8 xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x67,0x18,0x74,0x10]
          vcvtne2ph2bf8 xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vcvtne2ph2bf8 xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2bf8 xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vcvtne2ph2bf8 xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x74,0x51,0x7f]
          vcvtne2ph2bf8 xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vcvtne2ph2bf8 xmm2, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf2,0x67,0x18,0x74,0x52,0x80]
          vcvtne2ph2bf8 xmm2, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vcvtne2ph2bf8s ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x74,0xd4]
          vcvtne2ph2bf8s ymm2, ymm3, ymm4

// CHECK: vcvtne2ph2bf8s xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x74,0xd4]
          vcvtne2ph2bf8s xmm2, xmm3, xmm4

// CHECK: vcvtne2ph2bf8s ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8s ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtne2ph2bf8s ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8s ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vcvtne2ph2bf8s ymm2, ymm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x74,0x10]
          vcvtne2ph2bf8s ymm2, ymm3, word ptr [eax]{1to16}

// CHECK: vcvtne2ph2bf8s ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2bf8s ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtne2ph2bf8s ymm2, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x74,0x51,0x7f]
          vcvtne2ph2bf8s ymm2, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vcvtne2ph2bf8s ymm2, ymm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x74,0x52,0x80]
          vcvtne2ph2bf8s ymm2, ymm3, word ptr [edx - 256]{1to16}

// CHECK: vcvtne2ph2bf8s xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8s xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtne2ph2bf8s xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8s xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtne2ph2bf8s xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x74,0x10]
          vcvtne2ph2bf8s xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vcvtne2ph2bf8s xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2bf8s xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vcvtne2ph2bf8s xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x74,0x51,0x7f]
          vcvtne2ph2bf8s xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vcvtne2ph2bf8s xmm2, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x74,0x52,0x80]
          vcvtne2ph2bf8s xmm2, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vcvtne2ph2hf8 ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x18,0xd4]
          vcvtne2ph2hf8 ymm2, ymm3, ymm4

// CHECK: vcvtne2ph2hf8 xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x18,0xd4]
          vcvtne2ph2hf8 xmm2, xmm3, xmm4

// CHECK: vcvtne2ph2hf8 ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8 ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtne2ph2hf8 ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8 ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vcvtne2ph2hf8 ymm2, ymm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x18,0x10]
          vcvtne2ph2hf8 ymm2, ymm3, word ptr [eax]{1to16}

// CHECK: vcvtne2ph2hf8 ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x18,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2hf8 ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtne2ph2hf8 ymm2, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x18,0x51,0x7f]
          vcvtne2ph2hf8 ymm2, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vcvtne2ph2hf8 ymm2, ymm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x18,0x52,0x80]
          vcvtne2ph2hf8 ymm2, ymm3, word ptr [edx - 256]{1to16}

// CHECK: vcvtne2ph2hf8 xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8 xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtne2ph2hf8 xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8 xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtne2ph2hf8 xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x18,0x10]
          vcvtne2ph2hf8 xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vcvtne2ph2hf8 xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x18,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2hf8 xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vcvtne2ph2hf8 xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x18,0x51,0x7f]
          vcvtne2ph2hf8 xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vcvtne2ph2hf8 xmm2, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x18,0x52,0x80]
          vcvtne2ph2hf8 xmm2, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vcvtne2ph2hf8s ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x1b,0xd4]
          vcvtne2ph2hf8s ymm2, ymm3, ymm4

// CHECK: vcvtne2ph2hf8s xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x1b,0xd4]
          vcvtne2ph2hf8s xmm2, xmm3, xmm4

// CHECK: vcvtne2ph2hf8s ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8s ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtne2ph2hf8s ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8s ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vcvtne2ph2hf8s ymm2, ymm3, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x1b,0x10]
          vcvtne2ph2hf8s ymm2, ymm3, word ptr [eax]{1to16}

// CHECK: vcvtne2ph2hf8s ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x1b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2hf8s ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtne2ph2hf8s ymm2, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x1b,0x51,0x7f]
          vcvtne2ph2hf8s ymm2, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vcvtne2ph2hf8s ymm2, ymm3, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x1b,0x52,0x80]
          vcvtne2ph2hf8s ymm2, ymm3, word ptr [edx - 256]{1to16}

// CHECK: vcvtne2ph2hf8s xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8s xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtne2ph2hf8s xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8s xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtne2ph2hf8s xmm2, xmm3, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x1b,0x10]
          vcvtne2ph2hf8s xmm2, xmm3, word ptr [eax]{1to8}

// CHECK: vcvtne2ph2hf8s xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x1b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2hf8s xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vcvtne2ph2hf8s xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x1b,0x51,0x7f]
          vcvtne2ph2hf8s xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vcvtne2ph2hf8s xmm2, xmm3, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x1b,0x52,0x80]
          vcvtne2ph2hf8s xmm2, xmm3, word ptr [edx - 256]{1to8}

// CHECK: vcvtnebf82ph xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1e,0xd3]
          vcvtnebf82ph xmm2, xmm3

// CHECK: vcvtnebf82ph xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x1e,0xd3]
          vcvtnebf82ph xmm2 {k7}, xmm3

// CHECK: vcvtnebf82ph xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x1e,0xd3]
          vcvtnebf82ph xmm2 {k7} {z}, xmm3

// CHECK: vcvtnebf82ph ymm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x1e,0xd3]
          vcvtnebf82ph ymm2, xmm3

// CHECK: vcvtnebf82ph ymm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x2f,0x1e,0xd3]
          vcvtnebf82ph ymm2 {k7}, xmm3

// CHECK: vcvtnebf82ph ymm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x1e,0xd3]
          vcvtnebf82ph ymm2 {k7} {z}, xmm3

// CHECK: vcvtnebf82ph xmm2, qword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnebf82ph xmm2, qword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtnebf82ph xmm2 {k7}, qword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x1e,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnebf82ph xmm2 {k7}, qword ptr [edi + 4*eax + 291]

// CHECK: vcvtnebf82ph xmm2, qword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1e,0x10]
          vcvtnebf82ph xmm2, qword ptr [eax]

// CHECK: vcvtnebf82ph xmm2, qword ptr [2*ebp - 256]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1e,0x14,0x6d,0x00,0xff,0xff,0xff]
          vcvtnebf82ph xmm2, qword ptr [2*ebp - 256]

// CHECK: vcvtnebf82ph xmm2 {k7} {z}, qword ptr [ecx + 1016]
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x1e,0x51,0x7f]
          vcvtnebf82ph xmm2 {k7} {z}, qword ptr [ecx + 1016]

// CHECK: vcvtnebf82ph xmm2 {k7} {z}, qword ptr [edx - 1024]
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x1e,0x52,0x80]
          vcvtnebf82ph xmm2 {k7} {z}, qword ptr [edx - 1024]

// CHECK: vcvtnebf82ph ymm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x1e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnebf82ph ymm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtnebf82ph ymm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7e,0x2f,0x1e,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnebf82ph ymm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtnebf82ph ymm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x1e,0x10]
          vcvtnebf82ph ymm2, xmmword ptr [eax]

// CHECK: vcvtnebf82ph ymm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x1e,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtnebf82ph ymm2, xmmword ptr [2*ebp - 512]

// CHECK: vcvtnebf82ph ymm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x1e,0x51,0x7f]
          vcvtnebf82ph ymm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK: vcvtnebf82ph ymm2 {k7} {z}, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x1e,0x52,0x80]
          vcvtnebf82ph ymm2 {k7} {z}, xmmword ptr [edx - 2048]

// CHECK: vcvtnehf82ph xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x1e,0xd3]
          vcvtnehf82ph xmm2, xmm3

// CHECK: vcvtnehf82ph xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7f,0x0f,0x1e,0xd3]
          vcvtnehf82ph xmm2 {k7}, xmm3

// CHECK: vcvtnehf82ph xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x1e,0xd3]
          vcvtnehf82ph xmm2 {k7} {z}, xmm3

// CHECK: vcvtnehf82ph ymm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x1e,0xd3]
          vcvtnehf82ph ymm2, xmm3

// CHECK: vcvtnehf82ph ymm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7f,0x2f,0x1e,0xd3]
          vcvtnehf82ph ymm2 {k7}, xmm3

// CHECK: vcvtnehf82ph ymm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x1e,0xd3]
          vcvtnehf82ph ymm2 {k7} {z}, xmm3

// CHECK: vcvtnehf82ph xmm2, qword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x1e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnehf82ph xmm2, qword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtnehf82ph xmm2 {k7}, qword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7f,0x0f,0x1e,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnehf82ph xmm2 {k7}, qword ptr [edi + 4*eax + 291]

// CHECK: vcvtnehf82ph xmm2, qword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x1e,0x10]
          vcvtnehf82ph xmm2, qword ptr [eax]

// CHECK: vcvtnehf82ph xmm2, qword ptr [2*ebp - 256]
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x1e,0x14,0x6d,0x00,0xff,0xff,0xff]
          vcvtnehf82ph xmm2, qword ptr [2*ebp - 256]

// CHECK: vcvtnehf82ph xmm2 {k7} {z}, qword ptr [ecx + 1016]
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x1e,0x51,0x7f]
          vcvtnehf82ph xmm2 {k7} {z}, qword ptr [ecx + 1016]

// CHECK: vcvtnehf82ph xmm2 {k7} {z}, qword ptr [edx - 1024]
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x1e,0x52,0x80]
          vcvtnehf82ph xmm2 {k7} {z}, qword ptr [edx - 1024]

// CHECK: vcvtnehf82ph ymm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x1e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnehf82ph ymm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtnehf82ph ymm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7f,0x2f,0x1e,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnehf82ph ymm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtnehf82ph ymm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x1e,0x10]
          vcvtnehf82ph ymm2, xmmword ptr [eax]

// CHECK: vcvtnehf82ph ymm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x1e,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtnehf82ph ymm2, xmmword ptr [2*ebp - 512]

// CHECK: vcvtnehf82ph ymm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x1e,0x51,0x7f]
          vcvtnehf82ph ymm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK: vcvtnehf82ph ymm2 {k7} {z}, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x1e,0x52,0x80]
          vcvtnehf82ph ymm2 {k7} {z}, xmmword ptr [edx - 2048]

// CHECK: vcvtneph2bf8 xmm2, xmm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x74,0xd3]
          vcvtneph2bf8 xmm2, xmm3

// CHECK: vcvtneph2bf8 xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x74,0xd3]
          vcvtneph2bf8 xmm2 {k7}, xmm3

// CHECK: vcvtneph2bf8 xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x8f,0x74,0xd3]
          vcvtneph2bf8 xmm2 {k7} {z}, xmm3

// CHECK: vcvtneph2bf8 xmm2, ymm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0x74,0xd3]
          vcvtneph2bf8 xmm2, ymm3

// CHECK: vcvtneph2bf8 xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0x74,0xd3]
          vcvtneph2bf8 xmm2 {k7}, ymm3

// CHECK: vcvtneph2bf8 xmm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf2,0x7e,0xaf,0x74,0xd3]
          vcvtneph2bf8 xmm2 {k7} {z}, ymm3

// CHECK: vcvtneph2bf8 xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2bf8 xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtneph2bf8 xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2bf8 xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtneph2bf8 xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x7e,0x18,0x74,0x10]
          vcvtneph2bf8 xmm2, word ptr [eax]{1to8}

// CHECK: vcvtneph2bf8 xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2bf8 xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vcvtneph2bf8 xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x7e,0x8f,0x74,0x51,0x7f]
          vcvtneph2bf8 xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK: vcvtneph2bf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf2,0x7e,0x9f,0x74,0x52,0x80]
          vcvtneph2bf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to8}

// CHECK: vcvtneph2bf8 xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x7e,0x38,0x74,0x10]
          vcvtneph2bf8 xmm2, word ptr [eax]{1to16}

// CHECK: vcvtneph2bf8 xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2bf8 xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtneph2bf8 xmm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x7e,0xaf,0x74,0x51,0x7f]
          vcvtneph2bf8 xmm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK: vcvtneph2bf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf2,0x7e,0xbf,0x74,0x52,0x80]
          vcvtneph2bf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to16}

// CHECK: vcvtneph2bf8s xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x74,0xd3]
          vcvtneph2bf8s xmm2, xmm3

// CHECK: vcvtneph2bf8s xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x74,0xd3]
          vcvtneph2bf8s xmm2 {k7}, xmm3

// CHECK: vcvtneph2bf8s xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x74,0xd3]
          vcvtneph2bf8s xmm2 {k7} {z}, xmm3

// CHECK: vcvtneph2bf8s xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x74,0xd3]
          vcvtneph2bf8s xmm2, ymm3

// CHECK: vcvtneph2bf8s xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x2f,0x74,0xd3]
          vcvtneph2bf8s xmm2 {k7}, ymm3

// CHECK: vcvtneph2bf8s xmm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x74,0xd3]
          vcvtneph2bf8s xmm2 {k7} {z}, ymm3

// CHECK: vcvtneph2bf8s xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2bf8s xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtneph2bf8s xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2bf8s xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtneph2bf8s xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7e,0x18,0x74,0x10]
          vcvtneph2bf8s xmm2, word ptr [eax]{1to8}

// CHECK: vcvtneph2bf8s xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2bf8s xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vcvtneph2bf8s xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x74,0x51,0x7f]
          vcvtneph2bf8s xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK: vcvtneph2bf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7e,0x9f,0x74,0x52,0x80]
          vcvtneph2bf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to8}

// CHECK: vcvtneph2bf8s xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7e,0x38,0x74,0x10]
          vcvtneph2bf8s xmm2, word ptr [eax]{1to16}

// CHECK: vcvtneph2bf8s xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2bf8s xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtneph2bf8s xmm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x74,0x51,0x7f]
          vcvtneph2bf8s xmm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK: vcvtneph2bf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7e,0xbf,0x74,0x52,0x80]
          vcvtneph2bf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to16}

// CHECK: vcvtneph2hf8 xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x18,0xd3]
          vcvtneph2hf8 xmm2, xmm3

// CHECK: vcvtneph2hf8 xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x18,0xd3]
          vcvtneph2hf8 xmm2 {k7}, xmm3

// CHECK: vcvtneph2hf8 xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x18,0xd3]
          vcvtneph2hf8 xmm2 {k7} {z}, xmm3

// CHECK: vcvtneph2hf8 xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x18,0xd3]
          vcvtneph2hf8 xmm2, ymm3

// CHECK: vcvtneph2hf8 xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x2f,0x18,0xd3]
          vcvtneph2hf8 xmm2 {k7}, ymm3

// CHECK: vcvtneph2hf8 xmm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x18,0xd3]
          vcvtneph2hf8 xmm2 {k7} {z}, ymm3

// CHECK: vcvtneph2hf8 xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2hf8 xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtneph2hf8 xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2hf8 xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtneph2hf8 xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7e,0x18,0x18,0x10]
          vcvtneph2hf8 xmm2, word ptr [eax]{1to8}

// CHECK: vcvtneph2hf8 xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x18,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2hf8 xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vcvtneph2hf8 xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x18,0x51,0x7f]
          vcvtneph2hf8 xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK: vcvtneph2hf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7e,0x9f,0x18,0x52,0x80]
          vcvtneph2hf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to8}

// CHECK: vcvtneph2hf8 xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7e,0x38,0x18,0x10]
          vcvtneph2hf8 xmm2, word ptr [eax]{1to16}

// CHECK: vcvtneph2hf8 xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x18,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2hf8 xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtneph2hf8 xmm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x18,0x51,0x7f]
          vcvtneph2hf8 xmm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK: vcvtneph2hf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7e,0xbf,0x18,0x52,0x80]
          vcvtneph2hf8 xmm2 {k7} {z}, word ptr [edx - 256]{1to16}

// CHECK: vcvtneph2hf8s xmm2, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1b,0xd3]
          vcvtneph2hf8s xmm2, xmm3

// CHECK: vcvtneph2hf8s xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x1b,0xd3]
          vcvtneph2hf8s xmm2 {k7}, xmm3

// CHECK: vcvtneph2hf8s xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x1b,0xd3]
          vcvtneph2hf8s xmm2 {k7} {z}, xmm3

// CHECK: vcvtneph2hf8s xmm2, ymm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x1b,0xd3]
          vcvtneph2hf8s xmm2, ymm3

// CHECK: vcvtneph2hf8s xmm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7e,0x2f,0x1b,0xd3]
          vcvtneph2hf8s xmm2 {k7}, ymm3

// CHECK: vcvtneph2hf8s xmm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x1b,0xd3]
          vcvtneph2hf8s xmm2 {k7} {z}, ymm3

// CHECK: vcvtneph2hf8s xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2hf8s xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vcvtneph2hf8s xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2hf8s xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK: vcvtneph2hf8s xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7e,0x18,0x1b,0x10]
          vcvtneph2hf8s xmm2, word ptr [eax]{1to8}

// CHECK: vcvtneph2hf8s xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2hf8s xmm2, xmmword ptr [2*ebp - 512]

// CHECK: vcvtneph2hf8s xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x1b,0x51,0x7f]
          vcvtneph2hf8s xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK: vcvtneph2hf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf5,0x7e,0x9f,0x1b,0x52,0x80]
          vcvtneph2hf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to8}

// CHECK: vcvtneph2hf8s xmm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7e,0x38,0x1b,0x10]
          vcvtneph2hf8s xmm2, word ptr [eax]{1to16}

// CHECK: vcvtneph2hf8s xmm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x1b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2hf8s xmm2, ymmword ptr [2*ebp - 1024]

// CHECK: vcvtneph2hf8s xmm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x1b,0x51,0x7f]
          vcvtneph2hf8s xmm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK: vcvtneph2hf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf5,0x7e,0xbf,0x1b,0x52,0x80]
          vcvtneph2hf8s xmm2 {k7} {z}, word ptr [edx - 256]{1to16}

