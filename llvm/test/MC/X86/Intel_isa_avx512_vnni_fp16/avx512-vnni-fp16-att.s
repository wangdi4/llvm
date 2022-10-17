// REQUIRES: intel_feature_isa_avx512_vnni_fp16
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avx512vnnifp16 --show-encoding %s | FileCheck %s

// CHECK:      vdpphps %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x52,0xd4]
               vdpphps %zmm4, %zmm3, %zmm2

// CHECK:      vdpphps %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0x52,0xd4]
               vdpphps %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vdpphps  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x52,0x94,0xf4,0x00,0x00,0x00,0x10]
               vdpphps  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vdpphps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0x52,0x94,0x87,0x23,0x01,0x00,0x00]
               vdpphps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vdpphps  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x58,0x52,0x10]
               vdpphps  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vdpphps  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x52,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vdpphps  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vdpphps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0x52,0x51,0x7f]
               vdpphps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vdpphps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xdf,0x52,0x52,0x80]
               vdpphps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

