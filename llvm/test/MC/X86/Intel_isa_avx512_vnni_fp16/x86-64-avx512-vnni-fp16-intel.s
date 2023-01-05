// REQUIRES: intel_feature_isa_avx512_vnni_fp16
// RUN: llvm-mc -triple x86_64-unknown-unknown -mattr=+avx512vnnifp16 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vdpphps zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0x40,0x52,0xf0]
               vdpphps zmm22, zmm23, zmm24

// CHECK:      vdpphps zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0x47,0x52,0xf0]
               vdpphps zmm22 {k7}, zmm23, zmm24

// CHECK:      vdpphps zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x40,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vdpphps zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vdpphps zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x47,0x52,0xb4,0x80,0x23,0x01,0x00,0x00]
               vdpphps zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vdpphps zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x44,0x50,0x52,0x35,0x00,0x00,0x00,0x00]
               vdpphps zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vdpphps zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x44,0x40,0x52,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vdpphps zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vdpphps zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x44,0xc7,0x52,0x71,0x7f]
               vdpphps zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vdpphps zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x44,0xd7,0x52,0x72,0x80]
               vdpphps zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

