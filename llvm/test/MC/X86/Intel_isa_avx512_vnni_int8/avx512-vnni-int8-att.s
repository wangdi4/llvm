// REQUIRES: intel_feature_isa_avx512_vnni_int8
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avx512vnniint8 --show-encoding %s | FileCheck %s

// CHECK:      vpdpbssd %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x50,0xd4]
               vpdpbssd %zmm4, %zmm3, %zmm2

// CHECK:      vpdpbssd %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x67,0x4f,0x50,0xd4]
               vpdpbssd %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vpdpbssd %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0xcf,0x50,0xd4]
               vpdpbssd %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbssd  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpbssd  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vpdpbssd  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x67,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpbssd  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vpdpbssd  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x58,0x50,0x10]
               vpdpbssd  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vpdpbssd  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbssd  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vpdpbssd  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0xcf,0x50,0x51,0x7f]
               vpdpbssd  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbssd  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0xdf,0x50,0x52,0x80]
               vpdpbssd  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbssds %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x51,0xd4]
               vpdpbssds %zmm4, %zmm3, %zmm2

// CHECK:      vpdpbssds %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x67,0x4f,0x51,0xd4]
               vpdpbssds %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vpdpbssds %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0xcf,0x51,0xd4]
               vpdpbssds %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbssds  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpbssds  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vpdpbssds  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x67,0x4f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpbssds  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vpdpbssds  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x58,0x51,0x10]
               vpdpbssds  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vpdpbssds  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x51,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbssds  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vpdpbssds  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0xcf,0x51,0x51,0x7f]
               vpdpbssds  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbssds  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0xdf,0x51,0x52,0x80]
               vpdpbssds  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbsud %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0x50,0xd4]
               vpdpbsud %zmm4, %zmm3, %zmm2

// CHECK:      vpdpbsud %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0x50,0xd4]
               vpdpbsud %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vpdpbsud %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0x50,0xd4]
               vpdpbsud %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbsud  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpbsud  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vpdpbsud  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpbsud  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vpdpbsud  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x58,0x50,0x10]
               vpdpbsud  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vpdpbsud  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbsud  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vpdpbsud  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0x50,0x51,0x7f]
               vpdpbsud  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbsud  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xdf,0x50,0x52,0x80]
               vpdpbsud  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbsuds %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0x51,0xd4]
               vpdpbsuds %zmm4, %zmm3, %zmm2

// CHECK:      vpdpbsuds %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0x51,0xd4]
               vpdpbsuds %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vpdpbsuds %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0x51,0xd4]
               vpdpbsuds %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbsuds  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpbsuds  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vpdpbsuds  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpbsuds  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vpdpbsuds  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x58,0x51,0x10]
               vpdpbsuds  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vpdpbsuds  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0x51,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbsuds  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vpdpbsuds  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0x51,0x51,0x7f]
               vpdpbsuds  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbsuds  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xdf,0x51,0x52,0x80]
               vpdpbsuds  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbuud %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x50,0xd4]
               vpdpbuud %zmm4, %zmm3, %zmm2

// CHECK:      vpdpbuud %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0x50,0xd4]
               vpdpbuud %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vpdpbuud %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0x50,0xd4]
               vpdpbuud %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbuud  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpbuud  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vpdpbuud  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpbuud  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vpdpbuud  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x58,0x50,0x10]
               vpdpbuud  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vpdpbuud  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x50,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbuud  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vpdpbuud  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0x50,0x51,0x7f]
               vpdpbuud  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbuud  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xdf,0x50,0x52,0x80]
               vpdpbuud  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbuuds %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x51,0xd4]
               vpdpbuuds %zmm4, %zmm3, %zmm2

// CHECK:      vpdpbuuds %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0x51,0xd4]
               vpdpbuuds %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vpdpbuuds %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0x51,0xd4]
               vpdpbuuds %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbuuds  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpbuuds  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vpdpbuuds  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpbuuds  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vpdpbuuds  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x58,0x51,0x10]
               vpdpbuuds  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vpdpbuuds  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0x51,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbuuds  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vpdpbuuds  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0x51,0x51,0x7f]
               vpdpbuuds  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpbuuds  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xdf,0x51,0x52,0x80]
               vpdpbuuds  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

