// REQUIRES: intel_feature_isa_avx512_vnni_int8
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avx512vl,+avx512vnniint8 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vpdpbssd ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x50,0xd4]
     {evex} vpdpbssd ymm2, ymm3, ymm4

// CHECK: vpdpbssd ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x67,0x2f,0x50,0xd4]
     {evex} vpdpbssd ymm2 {k7}, ymm3, ymm4

// CHECK: vpdpbssd ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x67,0xaf,0x50,0xd4]
     {evex} vpdpbssd ymm2 {k7} {z}, ymm3, ymm4

// CHECK: vpdpbssd xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x50,0xd4]
     {evex} vpdpbssd xmm2, xmm3, xmm4

// CHECK: vpdpbssd xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x67,0x0f,0x50,0xd4]
     {evex} vpdpbssd xmm2 {k7}, xmm3, xmm4

// CHECK: vpdpbssd xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x67,0x8f,0x50,0xd4]
     {evex} vpdpbssd xmm2 {k7} {z}, xmm3, xmm4

// CHECK: vpdpbssd ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbssd ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbssd ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x67,0x2f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbssd ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbssd ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x67,0x38,0x50,0x10]
     {evex} vpdpbssd ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK: vpdpbssd ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x50,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbssd ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vpdpbssd ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x67,0xaf,0x50,0x51,0x7f]
     {evex} vpdpbssd ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vpdpbssd ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x67,0xbf,0x50,0x52,0x80]
     {evex} vpdpbssd ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK: vpdpbssd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbssd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbssd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x67,0x0f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbssd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbssd xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x67,0x18,0x50,0x10]
     {evex} vpdpbssd xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK: vpdpbssd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x50,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbssd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vpdpbssd xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x67,0x8f,0x50,0x51,0x7f]
     {evex} vpdpbssd xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vpdpbssd xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x67,0x9f,0x50,0x52,0x80]
     {evex} vpdpbssd xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK: vpdpbssds ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x51,0xd4]
     {evex} vpdpbssds ymm2, ymm3, ymm4

// CHECK: vpdpbssds ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x67,0x2f,0x51,0xd4]
     {evex} vpdpbssds ymm2 {k7}, ymm3, ymm4

// CHECK: vpdpbssds ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x67,0xaf,0x51,0xd4]
     {evex} vpdpbssds ymm2 {k7} {z}, ymm3, ymm4

// CHECK: vpdpbssds xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x51,0xd4]
     {evex} vpdpbssds xmm2, xmm3, xmm4

// CHECK: vpdpbssds xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x67,0x0f,0x51,0xd4]
     {evex} vpdpbssds xmm2 {k7}, xmm3, xmm4

// CHECK: vpdpbssds xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x67,0x8f,0x51,0xd4]
     {evex} vpdpbssds xmm2 {k7} {z}, xmm3, xmm4

// CHECK: vpdpbssds ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbssds ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbssds ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x67,0x2f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbssds ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbssds ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x67,0x38,0x51,0x10]
     {evex} vpdpbssds ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK: vpdpbssds ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x51,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbssds ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vpdpbssds ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x67,0xaf,0x51,0x51,0x7f]
     {evex} vpdpbssds ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vpdpbssds ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x67,0xbf,0x51,0x52,0x80]
     {evex} vpdpbssds ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK: vpdpbssds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbssds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbssds xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x67,0x0f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbssds xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbssds xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x67,0x18,0x51,0x10]
     {evex} vpdpbssds xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK: vpdpbssds xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x51,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbssds xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vpdpbssds xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x67,0x8f,0x51,0x51,0x7f]
     {evex} vpdpbssds xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vpdpbssds xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x67,0x9f,0x51,0x52,0x80]
     {evex} vpdpbssds xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK: vpdpbsud ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x66,0x28,0x50,0xd4]
     {evex} vpdpbsud ymm2, ymm3, ymm4

// CHECK: vpdpbsud ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0x50,0xd4]
     {evex} vpdpbsud ymm2 {k7}, ymm3, ymm4

// CHECK: vpdpbsud ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0x50,0xd4]
     {evex} vpdpbsud ymm2 {k7} {z}, ymm3, ymm4

// CHECK: vpdpbsud xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x08,0x50,0xd4]
     {evex} vpdpbsud xmm2, xmm3, xmm4

// CHECK: vpdpbsud xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0x50,0xd4]
     {evex} vpdpbsud xmm2 {k7}, xmm3, xmm4

// CHECK: vpdpbsud xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0x50,0xd4]
     {evex} vpdpbsud xmm2 {k7} {z}, xmm3, xmm4

// CHECK: vpdpbsud ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x66,0x28,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbsud ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbsud ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbsud ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbsud ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x66,0x38,0x50,0x10]
     {evex} vpdpbsud ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK: vpdpbsud ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x66,0x28,0x50,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbsud ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vpdpbsud ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0x50,0x51,0x7f]
     {evex} vpdpbsud ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vpdpbsud ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x66,0xbf,0x50,0x52,0x80]
     {evex} vpdpbsud ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK: vpdpbsud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x66,0x08,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbsud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbsud xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbsud xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbsud xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x66,0x18,0x50,0x10]
     {evex} vpdpbsud xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK: vpdpbsud xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x66,0x08,0x50,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbsud xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vpdpbsud xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0x50,0x51,0x7f]
     {evex} vpdpbsud xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vpdpbsud xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x66,0x9f,0x50,0x52,0x80]
     {evex} vpdpbsud xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK: vpdpbsuds ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x66,0x28,0x51,0xd4]
     {evex} vpdpbsuds ymm2, ymm3, ymm4

// CHECK: vpdpbsuds ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0x51,0xd4]
     {evex} vpdpbsuds ymm2 {k7}, ymm3, ymm4

// CHECK: vpdpbsuds ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0x51,0xd4]
     {evex} vpdpbsuds ymm2 {k7} {z}, ymm3, ymm4

// CHECK: vpdpbsuds xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x08,0x51,0xd4]
     {evex} vpdpbsuds xmm2, xmm3, xmm4

// CHECK: vpdpbsuds xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0x51,0xd4]
     {evex} vpdpbsuds xmm2 {k7}, xmm3, xmm4

// CHECK: vpdpbsuds xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0x51,0xd4]
     {evex} vpdpbsuds xmm2 {k7} {z}, xmm3, xmm4

// CHECK: vpdpbsuds ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x66,0x28,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbsuds ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbsuds ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbsuds ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbsuds ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x66,0x38,0x51,0x10]
     {evex} vpdpbsuds ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK: vpdpbsuds ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x66,0x28,0x51,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbsuds ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vpdpbsuds ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0x51,0x51,0x7f]
     {evex} vpdpbsuds ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vpdpbsuds ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x66,0xbf,0x51,0x52,0x80]
     {evex} vpdpbsuds ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK: vpdpbsuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x66,0x08,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbsuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbsuds xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbsuds xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbsuds xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x66,0x18,0x51,0x10]
     {evex} vpdpbsuds xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK: vpdpbsuds xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x66,0x08,0x51,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbsuds xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vpdpbsuds xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0x51,0x51,0x7f]
     {evex} vpdpbsuds xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vpdpbsuds xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x66,0x9f,0x51,0x52,0x80]
     {evex} vpdpbsuds xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK: vpdpbuud ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x50,0xd4]
     {evex} vpdpbuud ymm2, ymm3, ymm4

// CHECK: vpdpbuud ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0x50,0xd4]
     {evex} vpdpbuud ymm2 {k7}, ymm3, ymm4

// CHECK: vpdpbuud ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0x50,0xd4]
     {evex} vpdpbuud ymm2 {k7} {z}, ymm3, ymm4

// CHECK: vpdpbuud xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x50,0xd4]
     {evex} vpdpbuud xmm2, xmm3, xmm4

// CHECK: vpdpbuud xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0x50,0xd4]
     {evex} vpdpbuud xmm2 {k7}, xmm3, xmm4

// CHECK: vpdpbuud xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0x50,0xd4]
     {evex} vpdpbuud xmm2 {k7} {z}, xmm3, xmm4

// CHECK: vpdpbuud ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbuud ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbuud ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbuud ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbuud ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x64,0x38,0x50,0x10]
     {evex} vpdpbuud ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK: vpdpbuud ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x50,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbuud ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vpdpbuud ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0x50,0x51,0x7f]
     {evex} vpdpbuud ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vpdpbuud ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x64,0xbf,0x50,0x52,0x80]
     {evex} vpdpbuud ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK: vpdpbuud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbuud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbuud xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbuud xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbuud xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x64,0x18,0x50,0x10]
     {evex} vpdpbuud xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK: vpdpbuud xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x50,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbuud xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vpdpbuud xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0x50,0x51,0x7f]
     {evex} vpdpbuud xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vpdpbuud xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x64,0x9f,0x50,0x52,0x80]
     {evex} vpdpbuud xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK: vpdpbuuds ymm2, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x51,0xd4]
     {evex} vpdpbuuds ymm2, ymm3, ymm4

// CHECK: vpdpbuuds ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0x51,0xd4]
     {evex} vpdpbuuds ymm2 {k7}, ymm3, ymm4

// CHECK: vpdpbuuds ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0x51,0xd4]
     {evex} vpdpbuuds ymm2 {k7} {z}, ymm3, ymm4

// CHECK: vpdpbuuds xmm2, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x51,0xd4]
     {evex} vpdpbuuds xmm2, xmm3, xmm4

// CHECK: vpdpbuuds xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0x51,0xd4]
     {evex} vpdpbuuds xmm2 {k7}, xmm3, xmm4

// CHECK: vpdpbuuds xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0x51,0xd4]
     {evex} vpdpbuuds xmm2 {k7} {z}, xmm3, xmm4

// CHECK: vpdpbuuds ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbuuds ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbuuds ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbuuds ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbuuds ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x64,0x38,0x51,0x10]
     {evex} vpdpbuuds ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK: vpdpbuuds ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x51,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbuuds ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vpdpbuuds ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0x51,0x51,0x7f]
     {evex} vpdpbuuds ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK: vpdpbuuds ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x64,0xbf,0x51,0x52,0x80]
     {evex} vpdpbuuds ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK: vpdpbuuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbuuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vpdpbuuds xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbuuds xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vpdpbuuds xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x64,0x18,0x51,0x10]
     {evex} vpdpbuuds xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK: vpdpbuuds xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x51,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbuuds xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: vpdpbuuds xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0x51,0x51,0x7f]
     {evex} vpdpbuuds xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK: vpdpbuuds xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x64,0x9f,0x51,0x52,0x80]
     {evex} vpdpbuuds xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

