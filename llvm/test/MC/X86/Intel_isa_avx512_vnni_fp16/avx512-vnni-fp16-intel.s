// REQUIRES: intel_feature_isa_avx512_vnni_fp16
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avx512vnnifp16 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vdpphps zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x52,0xd4]
               vdpphps zmm2, zmm3, zmm4

// CHECK:      vdpphps zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0x52,0xd4]
               vdpphps zmm2 {k7}, zmm3, zmm4

// CHECK:      vdpphps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x52,0x94,0xf4,0x00,0x00,0x00,0x10]
               vdpphps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vdpphps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0x52,0x94,0x87,0x23,0x01,0x00,0x00]
               vdpphps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vdpphps zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf2,0x64,0x58,0x52,0x10]
               vdpphps zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK:      vdpphps zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x52,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vdpphps zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK:      vdpphps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0x52,0x51,0x7f]
               vdpphps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK:      vdpphps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf2,0x64,0xdf,0x52,0x52,0x80]
               vdpphps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

