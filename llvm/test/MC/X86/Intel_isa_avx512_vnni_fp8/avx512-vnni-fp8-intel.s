// REQUIRES: intel_feature_isa_avx512_vnni_fp8
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vdpbf8ps zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x64,0x48,0x50,0xd4]
          vdpbf8ps zmm2, zmm3, zmm4

// CHECK: vdpbf8ps zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x64,0x4f,0x50,0xd4]
          vdpbf8ps zmm2 {k7}, zmm3, zmm4

// CHECK: vdpbf8ps zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x64,0xcf,0x50,0xd4]
          vdpbf8ps zmm2 {k7} {z}, zmm3, zmm4

// CHECK: vdpbf8ps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x64,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
          vdpbf8ps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vdpbf8ps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x64,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
          vdpbf8ps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vdpbf8ps zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x64,0x58,0x50,0x10]
          vdpbf8ps zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK: vdpbf8ps zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x64,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vdpbf8ps zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vdpbf8ps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x64,0xcf,0x50,0x51,0x7f]
          vdpbf8ps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vdpbf8ps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x64,0xdf,0x50,0x52,0x80]
          vdpbf8ps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK: vdpbhf8ps zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x50,0xd4]
          vdpbhf8ps zmm2, zmm3, zmm4

// CHECK: vdpbhf8ps zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x50,0xd4]
          vdpbhf8ps zmm2 {k7}, zmm3, zmm4

// CHECK: vdpbhf8ps zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x67,0xcf,0x50,0xd4]
          vdpbhf8ps zmm2 {k7} {z}, zmm3, zmm4

// CHECK: vdpbhf8ps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
          vdpbhf8ps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vdpbhf8ps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
          vdpbhf8ps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vdpbhf8ps zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x50,0x10]
          vdpbhf8ps zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK: vdpbhf8ps zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vdpbhf8ps zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vdpbhf8ps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x67,0xcf,0x50,0x51,0x7f]
          vdpbhf8ps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vdpbhf8ps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x67,0xdf,0x50,0x52,0x80]
          vdpbhf8ps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK: vdphbf8ps zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x50,0xd4]
          vdphbf8ps zmm2, zmm3, zmm4

// CHECK: vdphbf8ps zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x50,0xd4]
          vdphbf8ps zmm2 {k7}, zmm3, zmm4

// CHECK: vdphbf8ps zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x66,0xcf,0x50,0xd4]
          vdphbf8ps zmm2 {k7} {z}, zmm3, zmm4

// CHECK: vdphbf8ps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
          vdphbf8ps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vdphbf8ps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
          vdphbf8ps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vdphbf8ps zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x66,0x58,0x50,0x10]
          vdphbf8ps zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK: vdphbf8ps zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vdphbf8ps zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vdphbf8ps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x66,0xcf,0x50,0x51,0x7f]
          vdphbf8ps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vdphbf8ps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x66,0xdf,0x50,0x52,0x80]
          vdphbf8ps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

// CHECK: vdphf8ps zmm2, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x50,0xd4]
          vdphf8ps zmm2, zmm3, zmm4

// CHECK: vdphf8ps zmm2 {k7}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x65,0x4f,0x50,0xd4]
          vdphf8ps zmm2 {k7}, zmm3, zmm4

// CHECK: vdphf8ps zmm2 {k7} {z}, zmm3, zmm4
// CHECK: encoding: [0x62,0xf5,0x65,0xcf,0x50,0xd4]
          vdphf8ps zmm2 {k7} {z}, zmm3, zmm4

// CHECK: vdphf8ps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
          vdphf8ps zmm2, zmm3, zmmword ptr [esp + 8*esi + 268435456]

// CHECK: vdphf8ps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf5,0x65,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
          vdphf8ps zmm2 {k7}, zmm3, zmmword ptr [edi + 4*eax + 291]

// CHECK: vdphf8ps zmm2, zmm3, dword ptr [eax]{1to16}
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x50,0x10]
          vdphf8ps zmm2, zmm3, dword ptr [eax]{1to16}

// CHECK: vdphf8ps zmm2, zmm3, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vdphf8ps zmm2, zmm3, zmmword ptr [2*ebp - 2048]

// CHECK: vdphf8ps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf5,0x65,0xcf,0x50,0x51,0x7f]
          vdphf8ps zmm2 {k7} {z}, zmm3, zmmword ptr [ecx + 8128]

// CHECK: vdphf8ps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}
// CHECK: encoding: [0x62,0xf5,0x65,0xdf,0x50,0x52,0x80]
          vdphf8ps zmm2 {k7} {z}, zmm3, dword ptr [edx - 512]{1to16}

