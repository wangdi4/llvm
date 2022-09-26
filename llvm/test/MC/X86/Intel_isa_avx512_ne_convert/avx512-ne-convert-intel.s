// REQUIRES: intel_feature_isa_avx512_ne_convert
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      {evex} vcvtne2ps2ph zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x67,0xd4]
               {evex} vcvtne2ps2ph zmm2, zmm3, zmm4

// CHECK:      {evex} vcvtne2ps2ph zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtne2ps2ph zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtne2ps2ph zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x65,0x58,0x67,0x10]
               {evex} vcvtne2ps2ph zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      {evex} vcvtne2ps2ph zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x67,0x14,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtne2ps2ph zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      {evex} vcvtne2ps2ph zmm2, zmm3, zmm4, {rn-sae}
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0x67,0xd4]
               {evex} vcvtne2ps2ph zmm2, zmm3, zmm4, {rn-sae}

// CHECK:      {evex} vcvtne2ps2ph zmm2, zmm3, zmm4, {rz-sae}
// CHECK: encoding: [0x62,0xf2,0x65,0x78,0x67,0xd4]
               {evex} vcvtne2ps2ph zmm2, zmm3, zmm4, {rz-sae}

// CHECK:      {evex} vbcstnebf162ps zmm2, word ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xb1,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vbcstnebf162ps zmm2, word ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vbcstnebf162ps zmm2 {k7}, word ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0xb1,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vbcstnebf162ps zmm2 {k7}, word ptr [edi + 4*eax + 291]

// CHECK:      {evex} vbcstnebf162ps zmm2, word ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xb1,0x10]
               {evex} vbcstnebf162ps zmm2, word ptr [eax]

// CHECK:      {evex} vbcstnebf162ps zmm2, word ptr [2*ebp - 64]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xb1,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnebf162ps zmm2, word ptr [2*ebp - 64]

// CHECK:      {evex} vbcstnesh2ps zmm2, word ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xb1,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vbcstnesh2ps zmm2, word ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vbcstnesh2ps zmm2 {k7}, word ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7d,0x4f,0xb1,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vbcstnesh2ps zmm2 {k7}, word ptr [edi + 4*eax + 291]

// CHECK:      {evex} vbcstnesh2ps zmm2, word ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xb1,0x10]
               {evex} vbcstnesh2ps zmm2, word ptr [eax]

// CHECK:      {evex} vbcstnesh2ps zmm2, word ptr [2*ebp - 64]
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xb1,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnesh2ps zmm2, word ptr [2*ebp - 64]

// CHECK:      {evex} vcvtneebf162ps zmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneebf162ps zmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtneebf162ps zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneebf162ps zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK:      {evex} vcvtneebf162ps zmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xb0,0x10]
               {evex} vcvtneebf162ps zmm2, zmmword ptr [eax]

// CHECK:      {evex} vcvtneebf162ps zmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xb0,0x14,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneebf162ps zmm2, zmmword ptr [2*ebp - 2048]

// CHECK:      {evex} vcvtneebf162ps zmm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x7e,0xcf,0xb0,0x51,0x7f]
               {evex} vcvtneebf162ps zmm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK:      {evex} vcvtneebf162ps zmm2 {k7} {z}, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf2,0x7e,0xcf,0xb0,0x52,0x80]
               {evex} vcvtneebf162ps zmm2 {k7} {z}, zmmword ptr [edx - 8192]

// CHECK:      {evex} vcvtneeph2ps zmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneeph2ps zmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtneeph2ps zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7d,0x4f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneeph2ps zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK:      {evex} vcvtneeph2ps zmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xb0,0x10]
               {evex} vcvtneeph2ps zmm2, zmmword ptr [eax]

// CHECK:      {evex} vcvtneeph2ps zmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xb0,0x14,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneeph2ps zmm2, zmmword ptr [2*ebp - 2048]

// CHECK:      {evex} vcvtneeph2ps zmm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x7d,0xcf,0xb0,0x51,0x7f]
               {evex} vcvtneeph2ps zmm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK:      {evex} vcvtneeph2ps zmm2 {k7} {z}, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf2,0x7d,0xcf,0xb0,0x52,0x80]
               {evex} vcvtneeph2ps zmm2 {k7} {z}, zmmword ptr [edx - 8192]

// CHECK:      {evex} vcvtneobf162ps zmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneobf162ps zmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtneobf162ps zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7f,0x4f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneobf162ps zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK:      {evex} vcvtneobf162ps zmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0xb0,0x10]
               {evex} vcvtneobf162ps zmm2, zmmword ptr [eax]

// CHECK:      {evex} vcvtneobf162ps zmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0xb0,0x14,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneobf162ps zmm2, zmmword ptr [2*ebp - 2048]

// CHECK:      {evex} vcvtneobf162ps zmm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x7f,0xcf,0xb0,0x51,0x7f]
               {evex} vcvtneobf162ps zmm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK:      {evex} vcvtneobf162ps zmm2 {k7} {z}, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf2,0x7f,0xcf,0xb0,0x52,0x80]
               {evex} vcvtneobf162ps zmm2 {k7} {z}, zmmword ptr [edx - 8192]

// CHECK:      {evex} vcvtneoph2ps zmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneoph2ps zmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      {evex} vcvtneoph2ps zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneoph2ps zmm2 {k7}, zmmword ptr [edi + 4*eax + 291]

// CHECK:      {evex} vcvtneoph2ps zmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0xb0,0x10]
               {evex} vcvtneoph2ps zmm2, zmmword ptr [eax]

// CHECK:      {evex} vcvtneoph2ps zmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0xb0,0x14,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneoph2ps zmm2, zmmword ptr [2*ebp - 2048]

// CHECK:      {evex} vcvtneoph2ps zmm2 {k7} {z}, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x7c,0xcf,0xb0,0x51,0x7f]
               {evex} vcvtneoph2ps zmm2 {k7} {z}, zmmword ptr [ecx + 8128]

// CHECK:      {evex} vcvtneoph2ps zmm2 {k7} {z}, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf2,0x7c,0xcf,0xb0,0x52,0x80]
               {evex} vcvtneoph2ps zmm2 {k7} {z}, zmmword ptr [edx - 8192]

