// REQUIRES: intel_feature_isa_avx512_vnni_int16
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vpdpwsud zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0xd2,0xd4]
               vpdpwsud zmm2, zmm3, zmm4

// CHECK:      vpdpwsud zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0xd2,0xd4]
               vpdpwsud zmm2 {k7}, zmm3, zmm4

// CHECK:      vpdpwsud zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0xd2,0xd4]
               vpdpwsud zmm2 {k7} {z}, zmm3, zmm4

// CHECK:      vpdpwsud zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwsud zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwsud zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwsud zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwsud zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x66,0x58,0xd2,0x10]
               vpdpwsud zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vpdpwsud zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0xd2,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwsud zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vpdpwsud zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0xd2,0x51,0x7f]
               vpdpwsud zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vpdpwsud zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x66,0xdf,0xd2,0x52,0x80]
               vpdpwsud zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK:      vpdpwsuds zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0xd3,0xd4]
               vpdpwsuds zmm2, zmm3, zmm4

// CHECK:      vpdpwsuds zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0xd3,0xd4]
               vpdpwsuds zmm2 {k7}, zmm3, zmm4

// CHECK:      vpdpwsuds zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0xd3,0xd4]
               vpdpwsuds zmm2 {k7} {z}, zmm3, zmm4

// CHECK:      vpdpwsuds zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwsuds zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwsuds zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwsuds zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwsuds zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x66,0x58,0xd3,0x10]
               vpdpwsuds zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vpdpwsuds zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0xd3,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwsuds zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vpdpwsuds zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0xd3,0x51,0x7f]
               vpdpwsuds zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vpdpwsuds zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x66,0xdf,0xd3,0x52,0x80]
               vpdpwsuds zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK:      vpdpwusd zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0xd2,0xd4]
               vpdpwusd zmm2, zmm3, zmm4

// CHECK:      vpdpwusd zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x4f,0xd2,0xd4]
               vpdpwusd zmm2 {k7}, zmm3, zmm4

// CHECK:      vpdpwusd zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x65,0xcf,0xd2,0xd4]
               vpdpwusd zmm2 {k7} {z}, zmm3, zmm4

// CHECK:      vpdpwusd zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwusd zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwusd zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x65,0x4f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwusd zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwusd zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x65,0x58,0xd2,0x10]
               vpdpwusd zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vpdpwusd zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0xd2,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwusd zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vpdpwusd zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x65,0xcf,0xd2,0x51,0x7f]
               vpdpwusd zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vpdpwusd zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x65,0xdf,0xd2,0x52,0x80]
               vpdpwusd zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK:      vpdpwusds zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0xd3,0xd4]
               vpdpwusds zmm2, zmm3, zmm4

// CHECK:      vpdpwusds zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x65,0x4f,0xd3,0xd4]
               vpdpwusds zmm2 {k7}, zmm3, zmm4

// CHECK:      vpdpwusds zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x65,0xcf,0xd3,0xd4]
               vpdpwusds zmm2 {k7} {z}, zmm3, zmm4

// CHECK:      vpdpwusds zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwusds zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwusds zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x65,0x4f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwusds zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwusds zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x65,0x58,0xd3,0x10]
               vpdpwusds zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vpdpwusds zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0xd3,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwusds zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vpdpwusds zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x65,0xcf,0xd3,0x51,0x7f]
               vpdpwusds zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vpdpwusds zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x65,0xdf,0xd3,0x52,0x80]
               vpdpwusds zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK:      vpdpwuud zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0xd2,0xd4]
               vpdpwuud zmm2, zmm3, zmm4

// CHECK:      vpdpwuud zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0xd2,0xd4]
               vpdpwuud zmm2 {k7}, zmm3, zmm4

// CHECK:      vpdpwuud zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0xd2,0xd4]
               vpdpwuud zmm2 {k7} {z}, zmm3, zmm4

// CHECK:      vpdpwuud zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwuud zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwuud zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwuud zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwuud zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x64,0x58,0xd2,0x10]
               vpdpwuud zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vpdpwuud zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0xd2,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwuud zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vpdpwuud zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0xd2,0x51,0x7f]
               vpdpwuud zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vpdpwuud zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x64,0xdf,0xd2,0x52,0x80]
               vpdpwuud zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK:      vpdpwuuds zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0xd3,0xd4]
               vpdpwuuds zmm2, zmm3, zmm4

// CHECK:      vpdpwuuds zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0xd3,0xd4]
               vpdpwuuds zmm2 {k7}, zmm3, zmm4

// CHECK:      vpdpwuuds zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0xd3,0xd4]
               vpdpwuuds zmm2 {k7} {z}, zmm3, zmm4

// CHECK:      vpdpwuuds zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwuuds zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpwuuds zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwuuds zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpwuuds zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x64,0x58,0xd3,0x10]
               vpdpwuuds zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vpdpwuuds zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0xd3,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwuuds zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vpdpwuuds zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0xd3,0x51,0x7f]
               vpdpwuuds zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vpdpwuuds zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x64,0xdf,0xd3,0x52,0x80]
               vpdpwuuds zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

