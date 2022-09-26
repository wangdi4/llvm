// REQUIRES: intel_feature_isa_avx512_ne_convert
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      {evex} vbcstnebf162ps xmm2, word ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xb1,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vbcstnebf162ps xmm2, word ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vbcstnebf162ps xmm2 {k7}, word ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0xb1,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vbcstnebf162ps xmm2 {k7}, word ptr [edi + 4*eax + 291]

// CHECK:      {evex} vbcstnebf162ps xmm2, word ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xb1,0x10]
               {evex} vbcstnebf162ps xmm2, word ptr [eax]

// CHECK:      {evex} vbcstnebf162ps xmm2, word ptr [2*ebp - 64]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xb1,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnebf162ps xmm2, word ptr [2*ebp - 64]

// CHECK:      {evex} vbcstnebf162ps ymm2, word ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xb1,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vbcstnebf162ps ymm2, word ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vbcstnebf162ps ymm2 {k7}, word ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0xb1,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vbcstnebf162ps ymm2 {k7}, word ptr [edi + 4*eax + 291]

// CHECK:      {evex} vbcstnebf162ps ymm2, word ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xb1,0x10]
               {evex} vbcstnebf162ps ymm2, word ptr [eax]

// CHECK:      {evex} vbcstnebf162ps ymm2, word ptr [2*ebp - 64]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xb1,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnebf162ps ymm2, word ptr [2*ebp - 64]

// CHECK:      {evex} vbcstnesh2ps xmm2, word ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xb1,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vbcstnesh2ps xmm2, word ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vbcstnesh2ps xmm2 {k7}, word ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0xb1,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vbcstnesh2ps xmm2 {k7}, word ptr [edi + 4*eax + 291]

// CHECK:      {evex} vbcstnesh2ps xmm2, word ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xb1,0x10]
               {evex} vbcstnesh2ps xmm2, word ptr [eax]

// CHECK:      {evex} vbcstnesh2ps xmm2, word ptr [2*ebp - 64]
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xb1,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnesh2ps xmm2, word ptr [2*ebp - 64]

// CHECK:      {evex} vbcstnesh2ps ymm2, word ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xb1,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vbcstnesh2ps ymm2, word ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vbcstnesh2ps ymm2 {k7}, word ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0xb1,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vbcstnesh2ps ymm2 {k7}, word ptr [edi + 4*eax + 291]

// CHECK:      {evex} vbcstnesh2ps ymm2, word ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xb1,0x10]
               {evex} vbcstnesh2ps ymm2, word ptr [eax]

// CHECK:      {evex} vbcstnesh2ps ymm2, word ptr [2*ebp - 64]
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xb1,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnesh2ps ymm2, word ptr [2*ebp - 64]

// CHECK:      {evex} vcvtne2ps2ph ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x67,0xd4]
               {evex} vcvtne2ps2ph ymm2, ymm3, ymm4

// CHECK:      {evex} vcvtne2ps2ph xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x67,0xd4]
               {evex} vcvtne2ps2ph xmm2, xmm3, xmm4

// CHECK:      {evex} vcvtne2ps2ph ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtne2ps2ph ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtne2ps2ph ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x65,0x38,0x67,0x10]
               {evex} vcvtne2ps2ph ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK:      {evex} vcvtne2ps2ph ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x67,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtne2ps2ph ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK:      {evex} vcvtne2ps2ph xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtne2ps2ph xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtne2ps2ph xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0x67,0x10]
               {evex} vcvtne2ps2ph xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK:      {evex} vcvtne2ps2ph xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x67,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtne2ps2ph xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      {evex} vcvtneebf162ps xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneebf162ps xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtneebf162ps xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneebf162ps xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK:      {evex} vcvtneebf162ps xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xb0,0x10]
               {evex} vcvtneebf162ps xmm2, xmmword ptr [eax]

// CHECK:      {evex} vcvtneebf162ps xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xb0,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneebf162ps xmm2, xmmword ptr [2*ebp - 512]

// CHECK:      {evex} vcvtneebf162ps xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x7e,0x8f,0xb0,0x51,0x7f]
               {evex} vcvtneebf162ps xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK:      {evex} vcvtneebf162ps xmm2 {k7} {z}, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf2,0x7e,0x8f,0xb0,0x52,0x80]
               {evex} vcvtneebf162ps xmm2 {k7} {z}, xmmword ptr [edx - 2048]

// CHECK:      {evex} vcvtneebf162ps ymm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneebf162ps ymm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtneebf162ps ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneebf162ps ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]

// CHECK:      {evex} vcvtneebf162ps ymm2, ymmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xb0,0x10]
               {evex} vcvtneebf162ps ymm2, ymmword ptr [eax]

// CHECK:      {evex} vcvtneebf162ps ymm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xb0,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneebf162ps ymm2, ymmword ptr [2*ebp - 1024]

// CHECK:      {evex} vcvtneebf162ps ymm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x7e,0xaf,0xb0,0x51,0x7f]
               {evex} vcvtneebf162ps ymm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK:      {evex} vcvtneebf162ps ymm2 {k7} {z}, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf2,0x7e,0xaf,0xb0,0x52,0x80]
               {evex} vcvtneebf162ps ymm2 {k7} {z}, ymmword ptr [edx - 4096]

// CHECK:      {evex} vcvtneeph2ps xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneeph2ps xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtneeph2ps xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneeph2ps xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK:      {evex} vcvtneeph2ps xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xb0,0x10]
               {evex} vcvtneeph2ps xmm2, xmmword ptr [eax]

// CHECK:      {evex} vcvtneeph2ps xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xb0,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneeph2ps xmm2, xmmword ptr [2*ebp - 512]

// CHECK:      {evex} vcvtneeph2ps xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x7d,0x8f,0xb0,0x51,0x7f]
               {evex} vcvtneeph2ps xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK:      {evex} vcvtneeph2ps xmm2 {k7} {z}, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf2,0x7d,0x8f,0xb0,0x52,0x80]
               {evex} vcvtneeph2ps xmm2 {k7} {z}, xmmword ptr [edx - 2048]

// CHECK:      {evex} vcvtneeph2ps ymm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneeph2ps ymm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtneeph2ps ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneeph2ps ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]

// CHECK:      {evex} vcvtneeph2ps ymm2, ymmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xb0,0x10]
               {evex} vcvtneeph2ps ymm2, ymmword ptr [eax]

// CHECK:      {evex} vcvtneeph2ps ymm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xb0,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneeph2ps ymm2, ymmword ptr [2*ebp - 1024]

// CHECK:      {evex} vcvtneeph2ps ymm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x7d,0xaf,0xb0,0x51,0x7f]
               {evex} vcvtneeph2ps ymm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK:      {evex} vcvtneeph2ps ymm2 {k7} {z}, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf2,0x7d,0xaf,0xb0,0x52,0x80]
               {evex} vcvtneeph2ps ymm2 {k7} {z}, ymmword ptr [edx - 4096]

// CHECK:      {evex} vcvtneobf162ps xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneobf162ps xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtneobf162ps xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneobf162ps xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK:      {evex} vcvtneobf162ps xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0xb0,0x10]
               {evex} vcvtneobf162ps xmm2, xmmword ptr [eax]

// CHECK:      {evex} vcvtneobf162ps xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0xb0,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneobf162ps xmm2, xmmword ptr [2*ebp - 512]

// CHECK:      {evex} vcvtneobf162ps xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x7f,0x8f,0xb0,0x51,0x7f]
               {evex} vcvtneobf162ps xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK:      {evex} vcvtneobf162ps xmm2 {k7} {z}, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf2,0x7f,0x8f,0xb0,0x52,0x80]
               {evex} vcvtneobf162ps xmm2 {k7} {z}, xmmword ptr [edx - 2048]

// CHECK:      {evex} vcvtneobf162ps ymm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneobf162ps ymm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtneobf162ps ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7f,0x2f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneobf162ps ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]

// CHECK:      {evex} vcvtneobf162ps ymm2, ymmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0xb0,0x10]
               {evex} vcvtneobf162ps ymm2, ymmword ptr [eax]

// CHECK:      {evex} vcvtneobf162ps ymm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0xb0,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneobf162ps ymm2, ymmword ptr [2*ebp - 1024]

// CHECK:      {evex} vcvtneobf162ps ymm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x7f,0xaf,0xb0,0x51,0x7f]
               {evex} vcvtneobf162ps ymm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK:      {evex} vcvtneobf162ps ymm2 {k7} {z}, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf2,0x7f,0xaf,0xb0,0x52,0x80]
               {evex} vcvtneobf162ps ymm2 {k7} {z}, ymmword ptr [edx - 4096]

// CHECK:      {evex} vcvtneoph2ps xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneoph2ps xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtneoph2ps xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneoph2ps xmm2 {k7}, xmmword ptr [edi + 4*eax + 291]

// CHECK:      {evex} vcvtneoph2ps xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0xb0,0x10]
               {evex} vcvtneoph2ps xmm2, xmmword ptr [eax]

// CHECK:      {evex} vcvtneoph2ps xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0xb0,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneoph2ps xmm2, xmmword ptr [2*ebp - 512]

// CHECK:      {evex} vcvtneoph2ps xmm2 {k7} {z}, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x7c,0x8f,0xb0,0x51,0x7f]
               {evex} vcvtneoph2ps xmm2 {k7} {z}, xmmword ptr [ecx + 2032]

// CHECK:      {evex} vcvtneoph2ps xmm2 {k7} {z}, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf2,0x7c,0x8f,0xb0,0x52,0x80]
               {evex} vcvtneoph2ps xmm2 {k7} {z}, xmmword ptr [edx - 2048]

// CHECK:      {evex} vcvtneoph2ps ymm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneoph2ps ymm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtneoph2ps ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneoph2ps ymm2 {k7}, ymmword ptr [edi + 4*eax + 291]

// CHECK:      {evex} vcvtneoph2ps ymm2, ymmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0xb0,0x10]
               {evex} vcvtneoph2ps ymm2, ymmword ptr [eax]

// CHECK:      {evex} vcvtneoph2ps ymm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0xb0,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneoph2ps ymm2, ymmword ptr [2*ebp - 1024]

// CHECK:      {evex} vcvtneoph2ps ymm2 {k7} {z}, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x7c,0xaf,0xb0,0x51,0x7f]
               {evex} vcvtneoph2ps ymm2 {k7} {z}, ymmword ptr [ecx + 4064]

// CHECK:      {evex} vcvtneoph2ps ymm2 {k7} {z}, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf2,0x7c,0xaf,0xb0,0x52,0x80]
               {evex} vcvtneoph2ps ymm2 {k7} {z}, ymmword ptr [edx - 4096]

