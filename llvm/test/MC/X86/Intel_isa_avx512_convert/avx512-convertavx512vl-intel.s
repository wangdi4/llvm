// REQUIRES: intel_feature_isa_avx512_convert
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avx512vl,+avx512convert -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:  vcvt2ps2ph ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x67,0xd4]
     {evex} vcvt2ps2ph ymm2, ymm3, ymm4

// CHECK:  vcvt2ps2ph ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x65,0x2f,0x67,0xd4]
     {evex} vcvt2ps2ph ymm2 {k7}, ymm3, ymm4

// CHECK:  vcvt2ps2ph ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x65,0xaf,0x67,0xd4]
     {evex} vcvt2ps2ph ymm2 {k7} {z}, ymm3, ymm4

// CHECK:  vcvt2ps2ph xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x67,0xd4]
     {evex} vcvt2ps2ph xmm2, xmm3, xmm4

// CHECK:  vcvt2ps2ph xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x0f,0x67,0xd4]
     {evex} vcvt2ps2ph xmm2 {k7}, xmm3, xmm4

// CHECK:  vcvt2ps2ph xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x8f,0x67,0xd4]
     {evex} vcvt2ps2ph xmm2 {k7} {z}, xmm3, xmm4

// CHECK:  vcvt2ps2ph ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vcvt2ps2ph ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:  vcvt2ps2ph ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x65,0x2f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vcvt2ps2ph ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK:  vcvt2ps2ph ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x65,0x38,0x67,0x10]
     {evex} vcvt2ps2ph ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK:  vcvt2ps2ph ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x67,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vcvt2ps2ph ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK:  vcvt2ps2ph ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x65,0xaf,0x67,0x51,0x7f]
     {evex} vcvt2ps2ph ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK:  vcvt2ps2ph ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x65,0xbf,0x67,0x52,0x80]
     {evex} vcvt2ps2ph ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK:  vcvt2ps2ph xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vcvt2ps2ph xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:  vcvt2ps2ph xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x65,0x0f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vcvt2ps2ph xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:  vcvt2ps2ph xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0x67,0x10]
     {evex} vcvt2ps2ph xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK:  vcvt2ps2ph xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x67,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vcvt2ps2ph xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:  vcvt2ps2ph xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x65,0x8f,0x67,0x51,0x7f]
     {evex} vcvt2ps2ph xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK:  vcvt2ps2ph xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x65,0x9f,0x67,0x52,0x80]
     {evex} vcvt2ps2ph xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK:  vcvtbf162ph xmm2, xmm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x67,0xd3]
     {evex} vcvtbf162ph xmm2, xmm3

// CHECK:  vcvtbf162ph xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x67,0xd3]
     {evex} vcvtbf162ph xmm2 {k7}, xmm3

// CHECK:  vcvtbf162ph xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x8f,0x67,0xd3]
     {evex} vcvtbf162ph xmm2 {k7} {z}, xmm3

// CHECK:  vcvtbf162ph ymm2, ymm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0x67,0xd3]
     {evex} vcvtbf162ph ymm2, ymm3

// CHECK:  vcvtbf162ph ymm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0x67,0xd3]
     {evex} vcvtbf162ph ymm2 {k7}, ymm3

// CHECK:  vcvtbf162ph ymm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf2,0x7e,0xaf,0x67,0xd3]
     {evex} vcvtbf162ph ymm2 {k7} {z}, ymm3

// CHECK:  vcvtbf162ph xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vcvtbf162ph xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:  vcvtbf162ph xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vcvtbf162ph xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK:  vcvtbf162ph xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x7e,0x18,0x67,0x10]
     {evex} vcvtbf162ph xmm2, word ptr [eax]{1to8}

// CHECK:  vcvtbf162ph xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x67,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vcvtbf162ph xmm2, xmmword ptr [2*ebp - 512]

// CHECK:  vcvtbf162ph xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x7e,0x8f,0x67,0x51,0x7f]
     {evex} vcvtbf162ph xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK:  vcvtbf162ph xmm2 {k7} {z}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf2,0x7e,0x9f,0x67,0x52,0x80]
     {evex} vcvtbf162ph xmm2 {k7} {z}, word ptr [edx - 256]{1to8}

// CHECK:  vcvtbf162ph ymm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vcvtbf162ph ymm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:  vcvtbf162ph ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vcvtbf162ph ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]

// CHECK:  vcvtbf162ph ymm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x7e,0x38,0x67,0x10]
     {evex} vcvtbf162ph ymm2, word ptr [eax]{1to16}

// CHECK:  vcvtbf162ph ymm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0x67,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vcvtbf162ph ymm2, ymmword ptr [2*ebp - 1024]

// CHECK:  vcvtbf162ph ymm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x7e,0xaf,0x67,0x51,0x7f]
     {evex} vcvtbf162ph ymm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK:  vcvtbf162ph ymm2 {k7} {z}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf2,0x7e,0xbf,0x67,0x52,0x80]
     {evex} vcvtbf162ph ymm2 {k7} {z}, word ptr [edx - 256]{1to16}

// CHECK:  vcvtneph2bf16 xmm2, xmm3
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0x67,0xd3]
     {evex} vcvtneph2bf16 xmm2, xmm3

// CHECK:  vcvtneph2bf16 xmm2 {k7}, xmm3
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0x67,0xd3]
     {evex} vcvtneph2bf16 xmm2 {k7}, xmm3

// CHECK:  vcvtneph2bf16 xmm2 {k7} {z}, xmm3
// CHECK: encoding: [0x62,0xf2,0x7f,0x8f,0x67,0xd3]
     {evex} vcvtneph2bf16 xmm2 {k7} {z}, xmm3

// CHECK:  vcvtneph2bf16 ymm2, ymm3
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0x67,0xd3]
     {evex} vcvtneph2bf16 ymm2, ymm3

// CHECK:  vcvtneph2bf16 ymm2 {k7}, ymm3
// CHECK: encoding: [0x62,0xf2,0x7f,0x2f,0x67,0xd3]
     {evex} vcvtneph2bf16 ymm2 {k7}, ymm3

// CHECK:  vcvtneph2bf16 ymm2 {k7} {z}, ymm3
// CHECK: encoding: [0x62,0xf2,0x7f,0xaf,0x67,0xd3]
     {evex} vcvtneph2bf16 ymm2 {k7} {z}, ymm3

// CHECK:  vcvtneph2bf16 xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vcvtneph2bf16 xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:  vcvtneph2bf16 xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vcvtneph2bf16 xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK:  vcvtneph2bf16 xmm2, word ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x7f,0x18,0x67,0x10]
     {evex} vcvtneph2bf16 xmm2, word ptr [eax]{1to8}

// CHECK:  vcvtneph2bf16 xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0x67,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vcvtneph2bf16 xmm2, xmmword ptr [2*ebp - 512]

// CHECK:  vcvtneph2bf16 xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x7f,0x8f,0x67,0x51,0x7f]
     {evex} vcvtneph2bf16 xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK:  vcvtneph2bf16 xmm2 {k7} {z}, word ptr [edx - 256]{1to8}
// CHECK: encoding: [0x62,0xf2,0x7f,0x9f,0x67,0x52,0x80]
     {evex} vcvtneph2bf16 xmm2 {k7} {z}, word ptr [edx - 256]{1to8}

// CHECK:  vcvtneph2bf16 ymm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vcvtneph2bf16 ymm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:  vcvtneph2bf16 ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7f,0x2f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vcvtneph2bf16 ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]

// CHECK:  vcvtneph2bf16 ymm2, word ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x7f,0x38,0x67,0x10]
     {evex} vcvtneph2bf16 ymm2, word ptr [eax]{1to16}

// CHECK:  vcvtneph2bf16 ymm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0x67,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vcvtneph2bf16 ymm2, ymmword ptr [2*ebp - 1024]

// CHECK:  vcvtneph2bf16 ymm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x7f,0xaf,0x67,0x51,0x7f]
     {evex} vcvtneph2bf16 ymm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK:  vcvtneph2bf16 ymm2 {k7} {z}, word ptr [edx - 256]{1to16}
// CHECK: encoding: [0x62,0xf2,0x7f,0xbf,0x67,0x52,0x80]
     {evex} vcvtneph2bf16 ymm2 {k7} {z}, word ptr [edx - 256]{1to16}

