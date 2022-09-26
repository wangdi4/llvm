// REQUIRES: intel_feature_isa_avx512_vnni_fp8
// RUN: llvm-mc -triple i386 --show-encoding %s | FileCheck %s

// CHECK: vdpbf8ps %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x64,0x48,0x50,0xd4]
          vdpbf8ps %zmm4, %zmm3, %zmm2

// CHECK: vdpbf8ps %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x64,0x4f,0x50,0xd4]
          vdpbf8ps %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vdpbf8ps %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x64,0xcf,0x50,0xd4]
          vdpbf8ps %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vdpbf8ps  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x64,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
          vdpbf8ps  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vdpbf8ps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x64,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
          vdpbf8ps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vdpbf8ps  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x64,0x58,0x50,0x10]
          vdpbf8ps  (%eax){1to16}, %zmm3, %zmm2

// CHECK: vdpbf8ps  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x64,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vdpbf8ps  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vdpbf8ps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x64,0xcf,0x50,0x51,0x7f]
          vdpbf8ps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vdpbf8ps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x64,0xdf,0x50,0x52,0x80]
          vdpbf8ps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vdpbhf8ps %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x50,0xd4]
          vdpbhf8ps %zmm4, %zmm3, %zmm2

// CHECK: vdpbhf8ps %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x50,0xd4]
          vdpbhf8ps %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vdpbhf8ps %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x67,0xcf,0x50,0xd4]
          vdpbhf8ps %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vdpbhf8ps  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
          vdpbhf8ps  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vdpbhf8ps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
          vdpbhf8ps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vdpbhf8ps  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x50,0x10]
          vdpbhf8ps  (%eax){1to16}, %zmm3, %zmm2

// CHECK: vdpbhf8ps  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vdpbhf8ps  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vdpbhf8ps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x67,0xcf,0x50,0x51,0x7f]
          vdpbhf8ps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vdpbhf8ps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x67,0xdf,0x50,0x52,0x80]
          vdpbhf8ps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vdphbf8ps %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x50,0xd4]
          vdphbf8ps %zmm4, %zmm3, %zmm2

// CHECK: vdphbf8ps %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x50,0xd4]
          vdphbf8ps %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vdphbf8ps %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x66,0xcf,0x50,0xd4]
          vdphbf8ps %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vdphbf8ps  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
          vdphbf8ps  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vdphbf8ps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
          vdphbf8ps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vdphbf8ps  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x58,0x50,0x10]
          vdphbf8ps  (%eax){1to16}, %zmm3, %zmm2

// CHECK: vdphbf8ps  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vdphbf8ps  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vdphbf8ps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x66,0xcf,0x50,0x51,0x7f]
          vdphbf8ps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vdphbf8ps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x66,0xdf,0x50,0x52,0x80]
          vdphbf8ps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vdphf8ps %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x50,0xd4]
          vdphf8ps %zmm4, %zmm3, %zmm2

// CHECK: vdphf8ps %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x4f,0x50,0xd4]
          vdphf8ps %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vdphf8ps %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xcf,0x50,0xd4]
          vdphf8ps %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vdphf8ps  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
          vdphf8ps  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vdphf8ps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
          vdphf8ps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vdphf8ps  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x50,0x10]
          vdphf8ps  (%eax){1to16}, %zmm3, %zmm2

// CHECK: vdphf8ps  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vdphf8ps  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vdphf8ps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xcf,0x50,0x51,0x7f]
          vdphf8ps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vdphf8ps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xdf,0x50,0x52,0x80]
          vdphf8ps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

