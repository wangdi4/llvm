// REQUIRES: intel_feature_isa_avx512_vnni_int16
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vpdpwsud ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x66,0xd2,0xd4]
               vpdpwsud ymm2, ymm3, ymm4

// CHECK:      vpdpwsud ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0xd2,0xd4]
               vpdpwsud ymm2 {k7}, ymm3, ymm4

// CHECK:      vpdpwsud ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0xd2,0xd4]
               vpdpwsud ymm2 {k7} {z}, ymm3, ymm4

// CHECK:      vpdpwsud xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x62,0xd2,0xd4]
               vpdpwsud xmm2, xmm3, xmm4

// CHECK:      vpdpwsud xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0xd2,0xd4]
               vpdpwsud xmm2 {k7}, xmm3, xmm4

// CHECK:      vpdpwsud xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0xd2,0xd4]
               vpdpwsud xmm2 {k7} {z}, xmm3, xmm4

// CHECK:      vpdpwsud ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x66,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwsud ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwsud ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwsud ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwsud ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x66,0x38,0xd2,0x10]
               vpdpwsud ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK:      vpdpwsud ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x66,0xd2,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwsud ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK:      vpdpwsud ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0xd2,0x51,0x7f]
               vpdpwsud ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK:      vpdpwsud ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x66,0xbf,0xd2,0x52,0x80]
               vpdpwsud ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK:      vpdpwsud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x62,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwsud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwsud xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwsud xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwsud xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x66,0x18,0xd2,0x10]
               vpdpwsud xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK:      vpdpwsud xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x62,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwsud xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      vpdpwsud xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0xd2,0x51,0x7f]
               vpdpwsud xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      vpdpwsud xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x66,0x9f,0xd2,0x52,0x80]
               vpdpwsud xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK:      vpdpwsuds ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x66,0xd3,0xd4]
               vpdpwsuds ymm2, ymm3, ymm4

// CHECK:      vpdpwsuds ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0xd3,0xd4]
               vpdpwsuds ymm2 {k7}, ymm3, ymm4

// CHECK:      vpdpwsuds ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0xd3,0xd4]
               vpdpwsuds ymm2 {k7} {z}, ymm3, ymm4

// CHECK:      vpdpwsuds xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x62,0xd3,0xd4]
               vpdpwsuds xmm2, xmm3, xmm4

// CHECK:      vpdpwsuds xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0xd3,0xd4]
               vpdpwsuds xmm2 {k7}, xmm3, xmm4

// CHECK:      vpdpwsuds xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0xd3,0xd4]
               vpdpwsuds xmm2 {k7} {z}, xmm3, xmm4

// CHECK:      vpdpwsuds ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x66,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwsuds ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwsuds ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwsuds ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwsuds ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x66,0x38,0xd3,0x10]
               vpdpwsuds ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK:      vpdpwsuds ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x66,0xd3,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwsuds ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK:      vpdpwsuds ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0xd3,0x51,0x7f]
               vpdpwsuds ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK:      vpdpwsuds ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x66,0xbf,0xd3,0x52,0x80]
               vpdpwsuds ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK:      vpdpwsuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x62,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwsuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwsuds xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwsuds xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwsuds xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x66,0x18,0xd3,0x10]
               vpdpwsuds xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK:      vpdpwsuds xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x62,0xd3,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwsuds xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      vpdpwsuds xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0xd3,0x51,0x7f]
               vpdpwsuds xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      vpdpwsuds xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x66,0x9f,0xd3,0x52,0x80]
               vpdpwsuds xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK:      vpdpwusd ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x65,0xd2,0xd4]
               vpdpwusd ymm2, ymm3, ymm4

// CHECK:      vpdpwusd ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x65,0x2f,0xd2,0xd4]
               vpdpwusd ymm2 {k7}, ymm3, ymm4

// CHECK:      vpdpwusd ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x65,0xaf,0xd2,0xd4]
               vpdpwusd ymm2 {k7} {z}, ymm3, ymm4

// CHECK:      vpdpwusd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x61,0xd2,0xd4]
               vpdpwusd xmm2, xmm3, xmm4

// CHECK:      vpdpwusd xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x0f,0xd2,0xd4]
               vpdpwusd xmm2 {k7}, xmm3, xmm4

// CHECK:      vpdpwusd xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x8f,0xd2,0xd4]
               vpdpwusd xmm2 {k7} {z}, xmm3, xmm4

// CHECK:      vpdpwusd ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x65,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwusd ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwusd ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x65,0x2f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwusd ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwusd ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x65,0x38,0xd2,0x10]
               vpdpwusd ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK:      vpdpwusd ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x65,0xd2,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwusd ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK:      vpdpwusd ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x65,0xaf,0xd2,0x51,0x7f]
               vpdpwusd ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK:      vpdpwusd ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x65,0xbf,0xd2,0x52,0x80]
               vpdpwusd ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK:      vpdpwusd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x61,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwusd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwusd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x65,0x0f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwusd xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwusd xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0xd2,0x10]
               vpdpwusd xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK:      vpdpwusd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x61,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwusd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      vpdpwusd xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x65,0x8f,0xd2,0x51,0x7f]
               vpdpwusd xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      vpdpwusd xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x65,0x9f,0xd2,0x52,0x80]
               vpdpwusd xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK:      vpdpwusds ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x65,0xd3,0xd4]
               vpdpwusds ymm2, ymm3, ymm4

// CHECK:      vpdpwusds ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x65,0x2f,0xd3,0xd4]
               vpdpwusds ymm2 {k7}, ymm3, ymm4

// CHECK:      vpdpwusds ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x65,0xaf,0xd3,0xd4]
               vpdpwusds ymm2 {k7} {z}, ymm3, ymm4

// CHECK:      vpdpwusds xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x61,0xd3,0xd4]
               vpdpwusds xmm2, xmm3, xmm4

// CHECK:      vpdpwusds xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x0f,0xd3,0xd4]
               vpdpwusds xmm2 {k7}, xmm3, xmm4

// CHECK:      vpdpwusds xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x8f,0xd3,0xd4]
               vpdpwusds xmm2 {k7} {z}, xmm3, xmm4

// CHECK:      vpdpwusds ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x65,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwusds ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwusds ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x65,0x2f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwusds ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwusds ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x65,0x38,0xd3,0x10]
               vpdpwusds ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK:      vpdpwusds ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x65,0xd3,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwusds ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK:      vpdpwusds ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x65,0xaf,0xd3,0x51,0x7f]
               vpdpwusds ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK:      vpdpwusds ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x65,0xbf,0xd3,0x52,0x80]
               vpdpwusds ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK:      vpdpwusds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x61,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwusds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwusds xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x65,0x0f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwusds xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwusds xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0xd3,0x10]
               vpdpwusds xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK:      vpdpwusds xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x61,0xd3,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwusds xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      vpdpwusds xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x65,0x8f,0xd3,0x51,0x7f]
               vpdpwusds xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      vpdpwusds xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x65,0x9f,0xd3,0x52,0x80]
               vpdpwusds xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK:      vpdpwuud ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x64,0xd2,0xd4]
               vpdpwuud ymm2, ymm3, ymm4

// CHECK:      vpdpwuud ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0xd2,0xd4]
               vpdpwuud ymm2 {k7}, ymm3, ymm4

// CHECK:      vpdpwuud ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0xd2,0xd4]
               vpdpwuud ymm2 {k7} {z}, ymm3, ymm4

// CHECK:      vpdpwuud xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x60,0xd2,0xd4]
               vpdpwuud xmm2, xmm3, xmm4

// CHECK:      vpdpwuud xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0xd2,0xd4]
               vpdpwuud xmm2 {k7}, xmm3, xmm4

// CHECK:      vpdpwuud xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0xd2,0xd4]
               vpdpwuud xmm2 {k7} {z}, xmm3, xmm4

// CHECK:      vpdpwuud ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x64,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwuud ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwuud ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwuud ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwuud ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x64,0x38,0xd2,0x10]
               vpdpwuud ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK:      vpdpwuud ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x64,0xd2,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwuud ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK:      vpdpwuud ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0xd2,0x51,0x7f]
               vpdpwuud ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK:      vpdpwuud ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x64,0xbf,0xd2,0x52,0x80]
               vpdpwuud ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK:      vpdpwuud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x60,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwuud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwuud xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwuud xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwuud xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x64,0x18,0xd2,0x10]
               vpdpwuud xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK:      vpdpwuud xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x60,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwuud xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      vpdpwuud xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0xd2,0x51,0x7f]
               vpdpwuud xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      vpdpwuud xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x64,0x9f,0xd2,0x52,0x80]
               vpdpwuud xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

// CHECK:      vpdpwuuds ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x64,0xd3,0xd4]
               vpdpwuuds ymm2, ymm3, ymm4

// CHECK:      vpdpwuuds ymm2 {k7}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0xd3,0xd4]
               vpdpwuuds ymm2 {k7}, ymm3, ymm4

// CHECK:      vpdpwuuds ymm2 {k7} {z}, ymm3, ymm4
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0xd3,0xd4]
               vpdpwuuds ymm2 {k7} {z}, ymm3, ymm4

// CHECK:      vpdpwuuds xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x60,0xd3,0xd4]
               vpdpwuuds xmm2, xmm3, xmm4

// CHECK:      vpdpwuuds xmm2 {k7}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0xd3,0xd4]
               vpdpwuuds xmm2 {k7}, xmm3, xmm4

// CHECK:      vpdpwuuds xmm2 {k7} {z}, xmm3, xmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0xd3,0xd4]
               vpdpwuuds xmm2 {k7} {z}, xmm3, xmm4

// CHECK:      vpdpwuuds ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x64,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwuuds ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwuuds ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwuuds ymm2 {k7}, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwuuds ymm2, ymm3, dword ptr [eax]{1to8}
// CHECK: encoding: [0x62,0xf2,0x64,0x38,0xd3,0x10]
               vpdpwuuds ymm2, ymm3, dword ptr [eax]{1to8}

// CHECK:      vpdpwuuds ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x64,0xd3,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwuuds ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK:      vpdpwuuds ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0xd3,0x51,0x7f]
               vpdpwuuds ymm2 {k7} {z}, ymm3, ymmword ptr [ecx + 4064]

// CHECK:      vpdpwuuds ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}
// CHECK: encoding: [0x62,0xf2,0x64,0xbf,0xd3,0x52,0x80]
               vpdpwuuds ymm2 {k7} {z}, ymm3, dword ptr [edx - 512]{1to8}

// CHECK:      vpdpwuuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x60,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwuuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwuuds xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwuuds xmm2 {k7}, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwuuds xmm2, xmm3, dword ptr [eax]{1to4}
// CHECK: encoding: [0x62,0xf2,0x64,0x18,0xd3,0x10]
               vpdpwuuds xmm2, xmm3, dword ptr [eax]{1to4}

// CHECK:      vpdpwuuds xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x60,0xd3,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwuuds xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK:      vpdpwuuds xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0xd3,0x51,0x7f]
               vpdpwuuds xmm2 {k7} {z}, xmm3, xmmword ptr [ecx + 2032]

// CHECK:      vpdpwuuds xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}
// CHECK: encoding: [0x62,0xf2,0x64,0x9f,0xd3,0x52,0x80]
               vpdpwuuds xmm2 {k7} {z}, xmm3, dword ptr [edx - 512]{1to4}

