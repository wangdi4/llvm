// REQUIRES: intel_feature_isa_avx512_minmax
// RUN: llvm-mc -triple i386 --show-encoding %s | FileCheck %s

// CHECK: vminmaxnepbf16 $123, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x67,0x48,0x52,0xd4,0x7b]
          vminmaxnepbf16 $123, %zmm4, %zmm3, %zmm2

// CHECK: vminmaxnepbf16 $123, %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x67,0x4f,0x52,0xd4,0x7b]
          vminmaxnepbf16 $123, %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vminmaxnepbf16 $123, %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x67,0xcf,0x52,0xd4,0x7b]
          vminmaxnepbf16 $123, %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vminmaxnepbf16  $123, 268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x67,0x48,0x52,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vminmaxnepbf16  $123, 268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vminmaxnepbf16  $123, 291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x67,0x4f,0x52,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vminmaxnepbf16  $123, 291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vminmaxnepbf16  $123, (%eax){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x67,0x58,0x52,0x10,0x7b]
          vminmaxnepbf16  $123, (%eax){1to32}, %zmm3, %zmm2

// CHECK: vminmaxnepbf16  $123, -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x67,0x48,0x52,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxnepbf16  $123, -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vminmaxnepbf16  $123, 8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x67,0xcf,0x52,0x51,0x7f,0x7b]
          vminmaxnepbf16  $123, 8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vminmaxnepbf16  $123, -256(%edx){1to32}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x67,0xdf,0x52,0x52,0x80,0x7b]
          vminmaxnepbf16  $123, -256(%edx){1to32}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vminmaxpd $123, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0xe5,0x48,0x52,0xd4,0x7b]
          vminmaxpd $123, %zmm4, %zmm3, %zmm2

// CHECK: vminmaxpd $123, {sae}, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0xe5,0x18,0x52,0xd4,0x7b]
          vminmaxpd $123, {sae}, %zmm4, %zmm3, %zmm2

// CHECK: vminmaxpd $123, %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0xe5,0x4f,0x52,0xd4,0x7b]
          vminmaxpd $123, %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vminmaxpd $123, {sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0xe5,0x9f,0x52,0xd4,0x7b]
          vminmaxpd $123, {sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vminmaxpd  $123, 268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0xe5,0x48,0x52,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vminmaxpd  $123, 268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vminmaxpd  $123, 291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0xe5,0x4f,0x52,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vminmaxpd  $123, 291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vminmaxpd  $123, (%eax){1to8}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0xe5,0x58,0x52,0x10,0x7b]
          vminmaxpd  $123, (%eax){1to8}, %zmm3, %zmm2

// CHECK: vminmaxpd  $123, -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0xe5,0x48,0x52,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxpd  $123, -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vminmaxpd  $123, 8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0xe5,0xcf,0x52,0x51,0x7f,0x7b]
          vminmaxpd  $123, 8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vminmaxpd  $123, -1024(%edx){1to8}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0xe5,0xdf,0x52,0x52,0x80,0x7b]
          vminmaxpd  $123, -1024(%edx){1to8}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vminmaxph $123, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x64,0x48,0x52,0xd4,0x7b]
          vminmaxph $123, %zmm4, %zmm3, %zmm2

// CHECK: vminmaxph $123, {sae}, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x64,0x18,0x52,0xd4,0x7b]
          vminmaxph $123, {sae}, %zmm4, %zmm3, %zmm2

// CHECK: vminmaxph $123, %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x64,0x4f,0x52,0xd4,0x7b]
          vminmaxph $123, %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vminmaxph $123, {sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x64,0x9f,0x52,0xd4,0x7b]
          vminmaxph $123, {sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vminmaxph  $123, 268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x64,0x48,0x52,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vminmaxph  $123, 268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vminmaxph  $123, 291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x64,0x4f,0x52,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vminmaxph  $123, 291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vminmaxph  $123, (%eax){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x64,0x58,0x52,0x10,0x7b]
          vminmaxph  $123, (%eax){1to32}, %zmm3, %zmm2

// CHECK: vminmaxph  $123, -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x64,0x48,0x52,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxph  $123, -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vminmaxph  $123, 8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x64,0xcf,0x52,0x51,0x7f,0x7b]
          vminmaxph  $123, 8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vminmaxph  $123, -256(%edx){1to32}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x64,0xdf,0x52,0x52,0x80,0x7b]
          vminmaxph  $123, -256(%edx){1to32}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vminmaxps $123, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x65,0x48,0x52,0xd4,0x7b]
          vminmaxps $123, %zmm4, %zmm3, %zmm2

// CHECK: vminmaxps $123, {sae}, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x65,0x18,0x52,0xd4,0x7b]
          vminmaxps $123, {sae}, %zmm4, %zmm3, %zmm2

// CHECK: vminmaxps $123, %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x65,0x4f,0x52,0xd4,0x7b]
          vminmaxps $123, %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vminmaxps $123, {sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x65,0x9f,0x52,0xd4,0x7b]
          vminmaxps $123, {sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vminmaxps  $123, 268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x65,0x48,0x52,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vminmaxps  $123, 268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vminmaxps  $123, 291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x65,0x4f,0x52,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vminmaxps  $123, 291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vminmaxps  $123, (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x65,0x58,0x52,0x10,0x7b]
          vminmaxps  $123, (%eax){1to16}, %zmm3, %zmm2

// CHECK: vminmaxps  $123, -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x65,0x48,0x52,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxps  $123, -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vminmaxps  $123, 8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x65,0xcf,0x52,0x51,0x7f,0x7b]
          vminmaxps  $123, 8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vminmaxps  $123, -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x65,0xdf,0x52,0x52,0x80,0x7b]
          vminmaxps  $123, -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

