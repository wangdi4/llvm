// REQUIRES: intel_feature_isa_avx512_vnni_int8
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avx512vnniint8 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vpdpbssd zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x50,0xd4]
               vpdpbssd zmm2, zmm3, zmm4

// CHECK:      vpdpbssd zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x67,0x4f,0x50,0xd4]
               vpdpbssd zmm2 {k7}, zmm3, zmm4

// CHECK:      vpdpbssd zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x67,0xcf,0x50,0xd4]
               vpdpbssd zmm2 {k7} {z}, zmm3, zmm4

// CHECK:      vpdpbssd zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpbssd zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpbssd zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x67,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpbssd zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpbssd zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x67,0x58,0x50,0x10]
               vpdpbssd zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vpdpbssd zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbssd zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vpdpbssd zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x67,0xcf,0x50,0x51,0x7f]
               vpdpbssd zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vpdpbssd zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x67,0xdf,0x50,0x52,0x80]
               vpdpbssd zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK:      vpdpbssds zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x51,0xd4]
               vpdpbssds zmm2, zmm3, zmm4

// CHECK:      vpdpbssds zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x67,0x4f,0x51,0xd4]
               vpdpbssds zmm2 {k7}, zmm3, zmm4

// CHECK:      vpdpbssds zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x67,0xcf,0x51,0xd4]
               vpdpbssds zmm2 {k7} {z}, zmm3, zmm4

// CHECK:      vpdpbssds zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpbssds zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpbssds zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x67,0x4f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpbssds zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpbssds zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x67,0x58,0x51,0x10]
               vpdpbssds zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vpdpbssds zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x51,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbssds zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vpdpbssds zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x67,0xcf,0x51,0x51,0x7f]
               vpdpbssds zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vpdpbssds zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x67,0xdf,0x51,0x52,0x80]
               vpdpbssds zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK:      vpdpbsud zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0x50,0xd4]
               vpdpbsud zmm2, zmm3, zmm4

// CHECK:      vpdpbsud zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0x50,0xd4]
               vpdpbsud zmm2 {k7}, zmm3, zmm4

// CHECK:      vpdpbsud zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0x50,0xd4]
               vpdpbsud zmm2 {k7} {z}, zmm3, zmm4

// CHECK:      vpdpbsud zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpbsud zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpbsud zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpbsud zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpbsud zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x66,0x58,0x50,0x10]
               vpdpbsud zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vpdpbsud zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbsud zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vpdpbsud zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0x50,0x51,0x7f]
               vpdpbsud zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vpdpbsud zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x66,0xdf,0x50,0x52,0x80]
               vpdpbsud zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK:      vpdpbsuds zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0x51,0xd4]
               vpdpbsuds zmm2, zmm3, zmm4

// CHECK:      vpdpbsuds zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0x51,0xd4]
               vpdpbsuds zmm2 {k7}, zmm3, zmm4

// CHECK:      vpdpbsuds zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0x51,0xd4]
               vpdpbsuds zmm2 {k7} {z}, zmm3, zmm4

// CHECK:      vpdpbsuds zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpbsuds zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpbsuds zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpbsuds zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpbsuds zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x66,0x58,0x51,0x10]
               vpdpbsuds zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vpdpbsuds zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0x51,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbsuds zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vpdpbsuds zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0x51,0x51,0x7f]
               vpdpbsuds zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vpdpbsuds zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x66,0xdf,0x51,0x52,0x80]
               vpdpbsuds zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK:      vpdpbuud zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x50,0xd4]
               vpdpbuud zmm2, zmm3, zmm4

// CHECK:      vpdpbuud zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0x50,0xd4]
               vpdpbuud zmm2 {k7}, zmm3, zmm4

// CHECK:      vpdpbuud zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0x50,0xd4]
               vpdpbuud zmm2 {k7} {z}, zmm3, zmm4

// CHECK:      vpdpbuud zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpbuud zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpbuud zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpbuud zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpbuud zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x64,0x58,0x50,0x10]
               vpdpbuud zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vpdpbuud zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbuud zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vpdpbuud zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0x50,0x51,0x7f]
               vpdpbuud zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vpdpbuud zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x64,0xdf,0x50,0x52,0x80]
               vpdpbuud zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK:      vpdpbuuds zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x51,0xd4]
               vpdpbuuds zmm2, zmm3, zmm4

// CHECK:      vpdpbuuds zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0x51,0xd4]
               vpdpbuuds zmm2 {k7}, zmm3, zmm4

// CHECK:      vpdpbuuds zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0x51,0xd4]
               vpdpbuuds zmm2 {k7} {z}, zmm3, zmm4

// CHECK:      vpdpbuuds zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpbuuds zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vpdpbuuds zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpbuuds zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vpdpbuuds zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x64,0x58,0x51,0x10]
               vpdpbuuds zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vpdpbuuds zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x51,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbuuds zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vpdpbuuds zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0x51,0x51,0x7f]
               vpdpbuuds zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vpdpbuuds zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x64,0xdf,0x51,0x52,0x80]
               vpdpbuuds zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

