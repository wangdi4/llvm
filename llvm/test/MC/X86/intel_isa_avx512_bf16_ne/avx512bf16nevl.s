// REQUIRES: intel_feature_isa_avx512_bf16_ne
// RUN: llvm-mc -triple i386 --show-encoding %s | FileCheck %s

// CHECK: vaddnepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x58,0xd4]
          vaddnepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vaddnepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x2f,0x58,0xd4]
          vaddnepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vaddnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xaf,0x58,0xd4]
          vaddnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vaddnepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x58,0xd4]
          vaddnepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vaddnepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x0f,0x58,0xd4]
          vaddnepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vaddnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x8f,0x58,0xd4]
          vaddnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vaddnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x58,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vaddnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x2f,0x58,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vaddnepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x58,0x10]
          vaddnepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vaddnepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x58,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vaddnepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vaddnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xaf,0x58,0x51,0x7f]
          vaddnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vaddnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xbf,0x58,0x52,0x80]
          vaddnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vaddnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x58,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vaddnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x0f,0x58,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vaddnepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x58,0x10]
          vaddnepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vaddnepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x58,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vaddnepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vaddnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x8f,0x58,0x51,0x7f]
          vaddnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vaddnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x9f,0x58,0x52,0x80]
          vaddnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vcmpnepbf16 $123, %ymm4, %ymm3, %k5
// CHECK: encoding: [0x62,0xf3,0x67,0x28,0xc2,0xec,0x7b]
          vcmpnepbf16 $123, %ymm4, %ymm3, %k5

// CHECK: vcmpnepbf16 $123, %ymm4, %ymm3, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x67,0x2f,0xc2,0xec,0x7b]
          vcmpnepbf16 $123, %ymm4, %ymm3, %k5 {%k7}

// CHECK: vcmpnepbf16 $123, %xmm4, %xmm3, %k5
// CHECK: encoding: [0x62,0xf3,0x67,0x08,0xc2,0xec,0x7b]
          vcmpnepbf16 $123, %xmm4, %xmm3, %k5

// CHECK: vcmpnepbf16 $123, %xmm4, %xmm3, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x67,0x0f,0xc2,0xec,0x7b]
          vcmpnepbf16 $123, %xmm4, %xmm3, %k5 {%k7}

// CHECK: vcmpnepbf16  $123, 268435456(%esp,%esi,8), %xmm3, %k5
// CHECK: encoding: [0x62,0xf3,0x67,0x08,0xc2,0xac,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcmpnepbf16  $123, 268435456(%esp,%esi,8), %xmm3, %k5

// CHECK: vcmpnepbf16  $123, 291(%edi,%eax,4), %xmm3, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x67,0x0f,0xc2,0xac,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcmpnepbf16  $123, 291(%edi,%eax,4), %xmm3, %k5 {%k7}

// CHECK: vcmpnepbf16  $123, (%eax){1to8}, %xmm3, %k5
// CHECK: encoding: [0x62,0xf3,0x67,0x18,0xc2,0x28,0x7b]
          vcmpnepbf16  $123, (%eax){1to8}, %xmm3, %k5

// CHECK: vcmpnepbf16  $123, -512(,%ebp,2), %xmm3, %k5
// CHECK: encoding: [0x62,0xf3,0x67,0x08,0xc2,0x2c,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vcmpnepbf16  $123, -512(,%ebp,2), %xmm3, %k5

// CHECK: vcmpnepbf16  $123, 2032(%ecx), %xmm3, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x67,0x0f,0xc2,0x69,0x7f,0x7b]
          vcmpnepbf16  $123, 2032(%ecx), %xmm3, %k5 {%k7}

// CHECK: vcmpnepbf16  $123, -256(%edx){1to8}, %xmm3, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x67,0x1f,0xc2,0x6a,0x80,0x7b]
          vcmpnepbf16  $123, -256(%edx){1to8}, %xmm3, %k5 {%k7}

// CHECK: vcmpnepbf16  $123, 268435456(%esp,%esi,8), %ymm3, %k5
// CHECK: encoding: [0x62,0xf3,0x67,0x28,0xc2,0xac,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcmpnepbf16  $123, 268435456(%esp,%esi,8), %ymm3, %k5

// CHECK: vcmpnepbf16  $123, 291(%edi,%eax,4), %ymm3, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x67,0x2f,0xc2,0xac,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcmpnepbf16  $123, 291(%edi,%eax,4), %ymm3, %k5 {%k7}

// CHECK: vcmpnepbf16  $123, (%eax){1to16}, %ymm3, %k5
// CHECK: encoding: [0x62,0xf3,0x67,0x38,0xc2,0x28,0x7b]
          vcmpnepbf16  $123, (%eax){1to16}, %ymm3, %k5

// CHECK: vcmpnepbf16  $123, -1024(,%ebp,2), %ymm3, %k5
// CHECK: encoding: [0x62,0xf3,0x67,0x28,0xc2,0x2c,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vcmpnepbf16  $123, -1024(,%ebp,2), %ymm3, %k5

// CHECK: vcmpnepbf16  $123, 4064(%ecx), %ymm3, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x67,0x2f,0xc2,0x69,0x7f,0x7b]
          vcmpnepbf16  $123, 4064(%ecx), %ymm3, %k5 {%k7}

// CHECK: vcmpnepbf16  $123, -256(%edx){1to16}, %ymm3, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x67,0x3f,0xc2,0x6a,0x80,0x7b]
          vcmpnepbf16  $123, -256(%edx){1to16}, %ymm3, %k5 {%k7}

// CHECK: vcomnesbf16 %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x2f,0xd3]
          vcomnesbf16 %xmm3, %xmm2

// CHECK: vcomnesbf16  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x2f,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcomnesbf16  268435456(%esp,%esi,8), %xmm2

// CHECK: vcomnesbf16  291(%edi,%eax,4), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x2f,0x94,0x87,0x23,0x01,0x00,0x00]
          vcomnesbf16  291(%edi,%eax,4), %xmm2

// CHECK: vcomnesbf16  (%eax), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x2f,0x10]
          vcomnesbf16  (%eax), %xmm2

// CHECK: vcomnesbf16  -64(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x2f,0x14,0x6d,0xc0,0xff,0xff,0xff]
          vcomnesbf16  -64(,%ebp,2), %xmm2

// CHECK: vcomnesbf16  254(%ecx), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x2f,0x51,0x7f]
          vcomnesbf16  254(%ecx), %xmm2

// CHECK: vcomnesbf16  -256(%edx), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x2f,0x52,0x80]
          vcomnesbf16  -256(%edx), %xmm2

// CHECK: vdivnepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x5e,0xd4]
          vdivnepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vdivnepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x2f,0x5e,0xd4]
          vdivnepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vdivnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xaf,0x5e,0xd4]
          vdivnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vdivnepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x5e,0xd4]
          vdivnepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vdivnepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x0f,0x5e,0xd4]
          vdivnepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vdivnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x8f,0x5e,0xd4]
          vdivnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vdivnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x5e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vdivnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vdivnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x2f,0x5e,0x94,0x87,0x23,0x01,0x00,0x00]
          vdivnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vdivnepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x5e,0x10]
          vdivnepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vdivnepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x5e,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vdivnepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vdivnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xaf,0x5e,0x51,0x7f]
          vdivnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vdivnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xbf,0x5e,0x52,0x80]
          vdivnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vdivnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x5e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vdivnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vdivnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x0f,0x5e,0x94,0x87,0x23,0x01,0x00,0x00]
          vdivnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vdivnepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x5e,0x10]
          vdivnepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vdivnepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x5e,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vdivnepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vdivnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x8f,0x5e,0x51,0x7f]
          vdivnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vdivnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x9f,0x5e,0x52,0x80]
          vdivnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmadd132nepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x98,0xd4]
          vfmadd132nepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vfmadd132nepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0x98,0xd4]
          vfmadd132nepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vfmadd132nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0x98,0xd4]
          vfmadd132nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmadd132nepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x98,0xd4]
          vfmadd132nepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vfmadd132nepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0x98,0xd4]
          vfmadd132nepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vfmadd132nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0x98,0xd4]
          vfmadd132nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmadd132nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x98,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfmadd132nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vfmadd132nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0x98,0x94,0x87,0x23,0x01,0x00,0x00]
          vfmadd132nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vfmadd132nepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0x98,0x10]
          vfmadd132nepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vfmadd132nepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x98,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vfmadd132nepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vfmadd132nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0x98,0x51,0x7f]
          vfmadd132nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmadd132nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0x98,0x52,0x80]
          vfmadd132nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmadd132nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x98,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfmadd132nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vfmadd132nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0x98,0x94,0x87,0x23,0x01,0x00,0x00]
          vfmadd132nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vfmadd132nepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0x98,0x10]
          vfmadd132nepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vfmadd132nepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x98,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vfmadd132nepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vfmadd132nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0x98,0x51,0x7f]
          vfmadd132nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmadd132nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0x98,0x52,0x80]
          vfmadd132nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmadd213nepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xa8,0xd4]
          vfmadd213nepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vfmadd213nepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xa8,0xd4]
          vfmadd213nepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vfmadd213nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xa8,0xd4]
          vfmadd213nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmadd213nepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xa8,0xd4]
          vfmadd213nepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vfmadd213nepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xa8,0xd4]
          vfmadd213nepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vfmadd213nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xa8,0xd4]
          vfmadd213nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmadd213nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xa8,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfmadd213nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vfmadd213nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xa8,0x94,0x87,0x23,0x01,0x00,0x00]
          vfmadd213nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vfmadd213nepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0xa8,0x10]
          vfmadd213nepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vfmadd213nepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xa8,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vfmadd213nepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vfmadd213nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xa8,0x51,0x7f]
          vfmadd213nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmadd213nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0xa8,0x52,0x80]
          vfmadd213nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmadd213nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xa8,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfmadd213nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vfmadd213nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xa8,0x94,0x87,0x23,0x01,0x00,0x00]
          vfmadd213nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vfmadd213nepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xa8,0x10]
          vfmadd213nepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vfmadd213nepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xa8,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vfmadd213nepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vfmadd213nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xa8,0x51,0x7f]
          vfmadd213nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmadd213nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0xa8,0x52,0x80]
          vfmadd213nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmadd231nepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xb8,0xd4]
          vfmadd231nepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vfmadd231nepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xb8,0xd4]
          vfmadd231nepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vfmadd231nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xb8,0xd4]
          vfmadd231nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmadd231nepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xb8,0xd4]
          vfmadd231nepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vfmadd231nepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xb8,0xd4]
          vfmadd231nepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vfmadd231nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xb8,0xd4]
          vfmadd231nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmadd231nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xb8,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfmadd231nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vfmadd231nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xb8,0x94,0x87,0x23,0x01,0x00,0x00]
          vfmadd231nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vfmadd231nepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0xb8,0x10]
          vfmadd231nepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vfmadd231nepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xb8,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vfmadd231nepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vfmadd231nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xb8,0x51,0x7f]
          vfmadd231nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmadd231nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0xb8,0x52,0x80]
          vfmadd231nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmadd231nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xb8,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfmadd231nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vfmadd231nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xb8,0x94,0x87,0x23,0x01,0x00,0x00]
          vfmadd231nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vfmadd231nepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xb8,0x10]
          vfmadd231nepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vfmadd231nepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xb8,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vfmadd231nepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vfmadd231nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xb8,0x51,0x7f]
          vfmadd231nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmadd231nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0xb8,0x52,0x80]
          vfmadd231nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmsub132nepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x9a,0xd4]
          vfmsub132nepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vfmsub132nepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0x9a,0xd4]
          vfmsub132nepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vfmsub132nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0x9a,0xd4]
          vfmsub132nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmsub132nepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x9a,0xd4]
          vfmsub132nepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vfmsub132nepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0x9a,0xd4]
          vfmsub132nepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vfmsub132nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0x9a,0xd4]
          vfmsub132nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmsub132nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x9a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfmsub132nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vfmsub132nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0x9a,0x94,0x87,0x23,0x01,0x00,0x00]
          vfmsub132nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vfmsub132nepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0x9a,0x10]
          vfmsub132nepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vfmsub132nepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x9a,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vfmsub132nepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vfmsub132nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0x9a,0x51,0x7f]
          vfmsub132nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmsub132nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0x9a,0x52,0x80]
          vfmsub132nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmsub132nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x9a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfmsub132nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vfmsub132nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0x9a,0x94,0x87,0x23,0x01,0x00,0x00]
          vfmsub132nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vfmsub132nepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0x9a,0x10]
          vfmsub132nepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vfmsub132nepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x9a,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vfmsub132nepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vfmsub132nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0x9a,0x51,0x7f]
          vfmsub132nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmsub132nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0x9a,0x52,0x80]
          vfmsub132nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmsub213nepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xaa,0xd4]
          vfmsub213nepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vfmsub213nepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xaa,0xd4]
          vfmsub213nepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vfmsub213nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xaa,0xd4]
          vfmsub213nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmsub213nepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xaa,0xd4]
          vfmsub213nepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vfmsub213nepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xaa,0xd4]
          vfmsub213nepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vfmsub213nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xaa,0xd4]
          vfmsub213nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmsub213nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xaa,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfmsub213nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vfmsub213nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xaa,0x94,0x87,0x23,0x01,0x00,0x00]
          vfmsub213nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vfmsub213nepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0xaa,0x10]
          vfmsub213nepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vfmsub213nepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xaa,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vfmsub213nepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vfmsub213nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xaa,0x51,0x7f]
          vfmsub213nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmsub213nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0xaa,0x52,0x80]
          vfmsub213nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmsub213nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xaa,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfmsub213nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vfmsub213nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xaa,0x94,0x87,0x23,0x01,0x00,0x00]
          vfmsub213nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vfmsub213nepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xaa,0x10]
          vfmsub213nepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vfmsub213nepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xaa,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vfmsub213nepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vfmsub213nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xaa,0x51,0x7f]
          vfmsub213nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmsub213nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0xaa,0x52,0x80]
          vfmsub213nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmsub231nepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xba,0xd4]
          vfmsub231nepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vfmsub231nepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xba,0xd4]
          vfmsub231nepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vfmsub231nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xba,0xd4]
          vfmsub231nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmsub231nepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xba,0xd4]
          vfmsub231nepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vfmsub231nepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xba,0xd4]
          vfmsub231nepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vfmsub231nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xba,0xd4]
          vfmsub231nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmsub231nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xba,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfmsub231nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vfmsub231nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xba,0x94,0x87,0x23,0x01,0x00,0x00]
          vfmsub231nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vfmsub231nepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0xba,0x10]
          vfmsub231nepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vfmsub231nepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xba,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vfmsub231nepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vfmsub231nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xba,0x51,0x7f]
          vfmsub231nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmsub231nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0xba,0x52,0x80]
          vfmsub231nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfmsub231nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xba,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfmsub231nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vfmsub231nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xba,0x94,0x87,0x23,0x01,0x00,0x00]
          vfmsub231nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vfmsub231nepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xba,0x10]
          vfmsub231nepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vfmsub231nepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xba,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vfmsub231nepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vfmsub231nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xba,0x51,0x7f]
          vfmsub231nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vfmsub231nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0xba,0x52,0x80]
          vfmsub231nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmadd132nepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x9c,0xd4]
          vfnmadd132nepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vfnmadd132nepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0x9c,0xd4]
          vfnmadd132nepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vfnmadd132nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0x9c,0xd4]
          vfnmadd132nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmadd132nepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x9c,0xd4]
          vfnmadd132nepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vfnmadd132nepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0x9c,0xd4]
          vfnmadd132nepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vfnmadd132nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0x9c,0xd4]
          vfnmadd132nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmadd132nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x9c,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfnmadd132nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vfnmadd132nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0x9c,0x94,0x87,0x23,0x01,0x00,0x00]
          vfnmadd132nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vfnmadd132nepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0x9c,0x10]
          vfnmadd132nepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vfnmadd132nepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x9c,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vfnmadd132nepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vfnmadd132nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0x9c,0x51,0x7f]
          vfnmadd132nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmadd132nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0x9c,0x52,0x80]
          vfnmadd132nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmadd132nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x9c,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfnmadd132nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vfnmadd132nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0x9c,0x94,0x87,0x23,0x01,0x00,0x00]
          vfnmadd132nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vfnmadd132nepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0x9c,0x10]
          vfnmadd132nepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vfnmadd132nepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x9c,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vfnmadd132nepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vfnmadd132nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0x9c,0x51,0x7f]
          vfnmadd132nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmadd132nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0x9c,0x52,0x80]
          vfnmadd132nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmadd213nepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xac,0xd4]
          vfnmadd213nepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vfnmadd213nepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xac,0xd4]
          vfnmadd213nepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vfnmadd213nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xac,0xd4]
          vfnmadd213nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmadd213nepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xac,0xd4]
          vfnmadd213nepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vfnmadd213nepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xac,0xd4]
          vfnmadd213nepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vfnmadd213nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xac,0xd4]
          vfnmadd213nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmadd213nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xac,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfnmadd213nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vfnmadd213nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xac,0x94,0x87,0x23,0x01,0x00,0x00]
          vfnmadd213nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vfnmadd213nepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0xac,0x10]
          vfnmadd213nepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vfnmadd213nepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xac,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vfnmadd213nepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vfnmadd213nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xac,0x51,0x7f]
          vfnmadd213nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmadd213nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0xac,0x52,0x80]
          vfnmadd213nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmadd213nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xac,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfnmadd213nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vfnmadd213nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xac,0x94,0x87,0x23,0x01,0x00,0x00]
          vfnmadd213nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vfnmadd213nepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xac,0x10]
          vfnmadd213nepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vfnmadd213nepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xac,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vfnmadd213nepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vfnmadd213nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xac,0x51,0x7f]
          vfnmadd213nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmadd213nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0xac,0x52,0x80]
          vfnmadd213nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmadd231nepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xbc,0xd4]
          vfnmadd231nepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vfnmadd231nepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xbc,0xd4]
          vfnmadd231nepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vfnmadd231nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xbc,0xd4]
          vfnmadd231nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmadd231nepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xbc,0xd4]
          vfnmadd231nepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vfnmadd231nepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xbc,0xd4]
          vfnmadd231nepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vfnmadd231nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xbc,0xd4]
          vfnmadd231nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmadd231nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xbc,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfnmadd231nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vfnmadd231nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xbc,0x94,0x87,0x23,0x01,0x00,0x00]
          vfnmadd231nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vfnmadd231nepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0xbc,0x10]
          vfnmadd231nepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vfnmadd231nepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xbc,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vfnmadd231nepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vfnmadd231nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xbc,0x51,0x7f]
          vfnmadd231nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmadd231nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0xbc,0x52,0x80]
          vfnmadd231nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmadd231nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xbc,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfnmadd231nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vfnmadd231nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xbc,0x94,0x87,0x23,0x01,0x00,0x00]
          vfnmadd231nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vfnmadd231nepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xbc,0x10]
          vfnmadd231nepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vfnmadd231nepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xbc,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vfnmadd231nepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vfnmadd231nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xbc,0x51,0x7f]
          vfnmadd231nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmadd231nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0xbc,0x52,0x80]
          vfnmadd231nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmsub132nepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x9e,0xd4]
          vfnmsub132nepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vfnmsub132nepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0x9e,0xd4]
          vfnmsub132nepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vfnmsub132nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0x9e,0xd4]
          vfnmsub132nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmsub132nepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x9e,0xd4]
          vfnmsub132nepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vfnmsub132nepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0x9e,0xd4]
          vfnmsub132nepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vfnmsub132nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0x9e,0xd4]
          vfnmsub132nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmsub132nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x9e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfnmsub132nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vfnmsub132nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0x9e,0x94,0x87,0x23,0x01,0x00,0x00]
          vfnmsub132nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vfnmsub132nepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0x9e,0x10]
          vfnmsub132nepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vfnmsub132nepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x9e,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vfnmsub132nepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vfnmsub132nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0x9e,0x51,0x7f]
          vfnmsub132nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmsub132nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0x9e,0x52,0x80]
          vfnmsub132nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmsub132nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x9e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfnmsub132nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vfnmsub132nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0x9e,0x94,0x87,0x23,0x01,0x00,0x00]
          vfnmsub132nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vfnmsub132nepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0x9e,0x10]
          vfnmsub132nepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vfnmsub132nepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x9e,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vfnmsub132nepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vfnmsub132nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0x9e,0x51,0x7f]
          vfnmsub132nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmsub132nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0x9e,0x52,0x80]
          vfnmsub132nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmsub213nepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xae,0xd4]
          vfnmsub213nepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vfnmsub213nepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xae,0xd4]
          vfnmsub213nepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vfnmsub213nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xae,0xd4]
          vfnmsub213nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmsub213nepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xae,0xd4]
          vfnmsub213nepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vfnmsub213nepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xae,0xd4]
          vfnmsub213nepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vfnmsub213nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xae,0xd4]
          vfnmsub213nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmsub213nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xae,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfnmsub213nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vfnmsub213nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xae,0x94,0x87,0x23,0x01,0x00,0x00]
          vfnmsub213nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vfnmsub213nepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0xae,0x10]
          vfnmsub213nepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vfnmsub213nepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xae,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vfnmsub213nepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vfnmsub213nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xae,0x51,0x7f]
          vfnmsub213nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmsub213nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0xae,0x52,0x80]
          vfnmsub213nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmsub213nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xae,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfnmsub213nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vfnmsub213nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xae,0x94,0x87,0x23,0x01,0x00,0x00]
          vfnmsub213nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vfnmsub213nepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xae,0x10]
          vfnmsub213nepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vfnmsub213nepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xae,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vfnmsub213nepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vfnmsub213nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xae,0x51,0x7f]
          vfnmsub213nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmsub213nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0xae,0x52,0x80]
          vfnmsub213nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmsub231nepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xbe,0xd4]
          vfnmsub231nepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vfnmsub231nepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xbe,0xd4]
          vfnmsub231nepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vfnmsub231nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xbe,0xd4]
          vfnmsub231nepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmsub231nepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xbe,0xd4]
          vfnmsub231nepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vfnmsub231nepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xbe,0xd4]
          vfnmsub231nepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vfnmsub231nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xbe,0xd4]
          vfnmsub231nepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmsub231nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xbe,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfnmsub231nepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vfnmsub231nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xbe,0x94,0x87,0x23,0x01,0x00,0x00]
          vfnmsub231nepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vfnmsub231nepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0xbe,0x10]
          vfnmsub231nepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vfnmsub231nepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xbe,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vfnmsub231nepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vfnmsub231nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xbe,0x51,0x7f]
          vfnmsub231nepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmsub231nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0xbe,0x52,0x80]
          vfnmsub231nepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vfnmsub231nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xbe,0x94,0xf4,0x00,0x00,0x00,0x10]
          vfnmsub231nepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vfnmsub231nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xbe,0x94,0x87,0x23,0x01,0x00,0x00]
          vfnmsub231nepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vfnmsub231nepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xbe,0x10]
          vfnmsub231nepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vfnmsub231nepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xbe,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vfnmsub231nepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vfnmsub231nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xbe,0x51,0x7f]
          vfnmsub231nepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vfnmsub231nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0xbe,0x52,0x80]
          vfnmsub231nepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vfpclassnepbf16 $123, %ymm3, %k5
// CHECK: encoding: [0x62,0xf3,0x7f,0x28,0x66,0xeb,0x7b]
          vfpclassnepbf16 $123, %ymm3, %k5

// CHECK: vfpclassnepbf16 $123, %ymm3, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x2f,0x66,0xeb,0x7b]
          vfpclassnepbf16 $123, %ymm3, %k5 {%k7}

// CHECK: vfpclassnepbf16 $123, %xmm3, %k5
// CHECK: encoding: [0x62,0xf3,0x7f,0x08,0x66,0xeb,0x7b]
          vfpclassnepbf16 $123, %xmm3, %k5

// CHECK: vfpclassnepbf16 $123, %xmm3, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x0f,0x66,0xeb,0x7b]
          vfpclassnepbf16 $123, %xmm3, %k5 {%k7}

// CHECK: vfpclassnepbf16x  $123, 268435456(%esp,%esi,8), %k5
// CHECK: encoding: [0x62,0xf3,0x7f,0x08,0x66,0xac,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vfpclassnepbf16x  $123, 268435456(%esp,%esi,8), %k5

// CHECK: vfpclassnepbf16x  $123, 291(%edi,%eax,4), %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x0f,0x66,0xac,0x87,0x23,0x01,0x00,0x00,0x7b]
          vfpclassnepbf16x  $123, 291(%edi,%eax,4), %k5 {%k7}

// CHECK: vfpclassnepbf16  $123, (%eax){1to8}, %k5
// CHECK: encoding: [0x62,0xf3,0x7f,0x18,0x66,0x28,0x7b]
          vfpclassnepbf16  $123, (%eax){1to8}, %k5

// CHECK: vfpclassnepbf16x  $123, -512(,%ebp,2), %k5
// CHECK: encoding: [0x62,0xf3,0x7f,0x08,0x66,0x2c,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vfpclassnepbf16x  $123, -512(,%ebp,2), %k5

// CHECK: vfpclassnepbf16x  $123, 2032(%ecx), %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x0f,0x66,0x69,0x7f,0x7b]
          vfpclassnepbf16x  $123, 2032(%ecx), %k5 {%k7}

// CHECK: vfpclassnepbf16  $123, -256(%edx){1to8}, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x1f,0x66,0x6a,0x80,0x7b]
          vfpclassnepbf16  $123, -256(%edx){1to8}, %k5 {%k7}

// CHECK: vfpclassnepbf16  $123, (%eax){1to16}, %k5
// CHECK: encoding: [0x62,0xf3,0x7f,0x38,0x66,0x28,0x7b]
          vfpclassnepbf16  $123, (%eax){1to16}, %k5

// CHECK: vfpclassnepbf16y  $123, -1024(,%ebp,2), %k5
// CHECK: encoding: [0x62,0xf3,0x7f,0x28,0x66,0x2c,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vfpclassnepbf16y  $123, -1024(,%ebp,2), %k5

// CHECK: vfpclassnepbf16y  $123, 4064(%ecx), %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x2f,0x66,0x69,0x7f,0x7b]
          vfpclassnepbf16y  $123, 4064(%ecx), %k5 {%k7}

// CHECK: vfpclassnepbf16  $123, -256(%edx){1to16}, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x3f,0x66,0x6a,0x80,0x7b]
          vfpclassnepbf16  $123, -256(%edx){1to16}, %k5 {%k7}

// CHECK: vgetexpnepbf16 %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x42,0xd3]
          vgetexpnepbf16 %xmm3, %xmm2

// CHECK: vgetexpnepbf16 %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x42,0xd3]
          vgetexpnepbf16 %xmm3, %xmm2 {%k7}

// CHECK: vgetexpnepbf16 %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x8f,0x42,0xd3]
          vgetexpnepbf16 %xmm3, %xmm2 {%k7} {z}

// CHECK: vgetexpnepbf16 %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x42,0xd3]
          vgetexpnepbf16 %ymm3, %ymm2

// CHECK: vgetexpnepbf16 %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x42,0xd3]
          vgetexpnepbf16 %ymm3, %ymm2 {%k7}

// CHECK: vgetexpnepbf16 %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xaf,0x42,0xd3]
          vgetexpnepbf16 %ymm3, %ymm2 {%k7} {z}

// CHECK: vgetexpnepbf16  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x42,0x94,0xf4,0x00,0x00,0x00,0x10]
          vgetexpnepbf16  268435456(%esp,%esi,8), %xmm2

// CHECK: vgetexpnepbf16  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x42,0x94,0x87,0x23,0x01,0x00,0x00]
          vgetexpnepbf16  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vgetexpnepbf16  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x42,0x10]
          vgetexpnepbf16  (%eax){1to8}, %xmm2

// CHECK: vgetexpnepbf16  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x42,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vgetexpnepbf16  -512(,%ebp,2), %xmm2

// CHECK: vgetexpnepbf16  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x8f,0x42,0x51,0x7f]
          vgetexpnepbf16  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vgetexpnepbf16  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x9f,0x42,0x52,0x80]
          vgetexpnepbf16  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vgetexpnepbf16  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x42,0x94,0xf4,0x00,0x00,0x00,0x10]
          vgetexpnepbf16  268435456(%esp,%esi,8), %ymm2

// CHECK: vgetexpnepbf16  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x42,0x94,0x87,0x23,0x01,0x00,0x00]
          vgetexpnepbf16  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vgetexpnepbf16  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x42,0x10]
          vgetexpnepbf16  (%eax){1to16}, %ymm2

// CHECK: vgetexpnepbf16  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x42,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vgetexpnepbf16  -1024(,%ebp,2), %ymm2

// CHECK: vgetexpnepbf16  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xaf,0x42,0x51,0x7f]
          vgetexpnepbf16  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vgetexpnepbf16  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xbf,0x42,0x52,0x80]
          vgetexpnepbf16  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vgetmantnepbf16 $123, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x28,0x26,0xd3,0x7b]
          vgetmantnepbf16 $123, %ymm3, %ymm2

// CHECK: vgetmantnepbf16 $123, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x2f,0x26,0xd3,0x7b]
          vgetmantnepbf16 $123, %ymm3, %ymm2 {%k7}

// CHECK: vgetmantnepbf16 $123, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xaf,0x26,0xd3,0x7b]
          vgetmantnepbf16 $123, %ymm3, %ymm2 {%k7} {z}

// CHECK: vgetmantnepbf16 $123, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x08,0x26,0xd3,0x7b]
          vgetmantnepbf16 $123, %xmm3, %xmm2

// CHECK: vgetmantnepbf16 $123, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x0f,0x26,0xd3,0x7b]
          vgetmantnepbf16 $123, %xmm3, %xmm2 {%k7}

// CHECK: vgetmantnepbf16 $123, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0x8f,0x26,0xd3,0x7b]
          vgetmantnepbf16 $123, %xmm3, %xmm2 {%k7} {z}

// CHECK: vgetmantnepbf16  $123, 268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x08,0x26,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vgetmantnepbf16  $123, 268435456(%esp,%esi,8), %xmm2

// CHECK: vgetmantnepbf16  $123, 291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x0f,0x26,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vgetmantnepbf16  $123, 291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vgetmantnepbf16  $123, (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x18,0x26,0x10,0x7b]
          vgetmantnepbf16  $123, (%eax){1to8}, %xmm2

// CHECK: vgetmantnepbf16  $123, -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x08,0x26,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vgetmantnepbf16  $123, -512(,%ebp,2), %xmm2

// CHECK: vgetmantnepbf16  $123, 2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0x8f,0x26,0x51,0x7f,0x7b]
          vgetmantnepbf16  $123, 2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vgetmantnepbf16  $123, -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0x9f,0x26,0x52,0x80,0x7b]
          vgetmantnepbf16  $123, -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vgetmantnepbf16  $123, 268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x28,0x26,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vgetmantnepbf16  $123, 268435456(%esp,%esi,8), %ymm2

// CHECK: vgetmantnepbf16  $123, 291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x2f,0x26,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vgetmantnepbf16  $123, 291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vgetmantnepbf16  $123, (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x38,0x26,0x10,0x7b]
          vgetmantnepbf16  $123, (%eax){1to16}, %ymm2

// CHECK: vgetmantnepbf16  $123, -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x28,0x26,0x14,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vgetmantnepbf16  $123, -1024(,%ebp,2), %ymm2

// CHECK: vgetmantnepbf16  $123, 4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xaf,0x26,0x51,0x7f,0x7b]
          vgetmantnepbf16  $123, 4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vgetmantnepbf16  $123, -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xbf,0x26,0x52,0x80,0x7b]
          vgetmantnepbf16  $123, -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vmaxnepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x5f,0xd4]
          vmaxnepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vmaxnepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x2f,0x5f,0xd4]
          vmaxnepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vmaxnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xaf,0x5f,0xd4]
          vmaxnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vmaxnepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x5f,0xd4]
          vmaxnepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vmaxnepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x0f,0x5f,0xd4]
          vmaxnepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vmaxnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x8f,0x5f,0xd4]
          vmaxnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vmaxnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x5f,0x94,0xf4,0x00,0x00,0x00,0x10]
          vmaxnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vmaxnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x2f,0x5f,0x94,0x87,0x23,0x01,0x00,0x00]
          vmaxnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vmaxnepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x5f,0x10]
          vmaxnepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vmaxnepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x5f,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vmaxnepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vmaxnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xaf,0x5f,0x51,0x7f]
          vmaxnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vmaxnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xbf,0x5f,0x52,0x80]
          vmaxnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vmaxnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x5f,0x94,0xf4,0x00,0x00,0x00,0x10]
          vmaxnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vmaxnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x0f,0x5f,0x94,0x87,0x23,0x01,0x00,0x00]
          vmaxnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vmaxnepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x5f,0x10]
          vmaxnepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vmaxnepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x5f,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vmaxnepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vmaxnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x8f,0x5f,0x51,0x7f]
          vmaxnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vmaxnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x9f,0x5f,0x52,0x80]
          vmaxnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vminnepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x5d,0xd4]
          vminnepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vminnepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x2f,0x5d,0xd4]
          vminnepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vminnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xaf,0x5d,0xd4]
          vminnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vminnepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x5d,0xd4]
          vminnepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vminnepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x0f,0x5d,0xd4]
          vminnepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vminnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x8f,0x5d,0xd4]
          vminnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vminnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x5d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vminnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vminnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x2f,0x5d,0x94,0x87,0x23,0x01,0x00,0x00]
          vminnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vminnepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x5d,0x10]
          vminnepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vminnepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x5d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vminnepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vminnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xaf,0x5d,0x51,0x7f]
          vminnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vminnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xbf,0x5d,0x52,0x80]
          vminnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vminnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x5d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vminnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vminnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x0f,0x5d,0x94,0x87,0x23,0x01,0x00,0x00]
          vminnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vminnepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x5d,0x10]
          vminnepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vminnepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x5d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vminnepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vminnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x8f,0x5d,0x51,0x7f]
          vminnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vminnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x9f,0x5d,0x52,0x80]
          vminnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vmulnepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x59,0xd4]
          vmulnepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vmulnepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x2f,0x59,0xd4]
          vmulnepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vmulnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xaf,0x59,0xd4]
          vmulnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vmulnepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x59,0xd4]
          vmulnepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vmulnepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x0f,0x59,0xd4]
          vmulnepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vmulnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x8f,0x59,0xd4]
          vmulnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vmulnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x59,0x94,0xf4,0x00,0x00,0x00,0x10]
          vmulnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vmulnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x2f,0x59,0x94,0x87,0x23,0x01,0x00,0x00]
          vmulnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vmulnepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x59,0x10]
          vmulnepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vmulnepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x59,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vmulnepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vmulnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xaf,0x59,0x51,0x7f]
          vmulnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vmulnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xbf,0x59,0x52,0x80]
          vmulnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vmulnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x59,0x94,0xf4,0x00,0x00,0x00,0x10]
          vmulnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vmulnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x0f,0x59,0x94,0x87,0x23,0x01,0x00,0x00]
          vmulnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vmulnepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x59,0x10]
          vmulnepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vmulnepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x59,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vmulnepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vmulnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x8f,0x59,0x51,0x7f]
          vmulnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vmulnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x9f,0x59,0x52,0x80]
          vmulnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vrcpnepbf16 %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x08,0x4c,0xd3]
          vrcpnepbf16 %xmm3, %xmm2

// CHECK: vrcpnepbf16 %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x7c,0x0f,0x4c,0xd3]
          vrcpnepbf16 %xmm3, %xmm2 {%k7}

// CHECK: vrcpnepbf16 %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x7c,0x8f,0x4c,0xd3]
          vrcpnepbf16 %xmm3, %xmm2 {%k7} {z}

// CHECK: vrcpnepbf16 %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x28,0x4c,0xd3]
          vrcpnepbf16 %ymm3, %ymm2

// CHECK: vrcpnepbf16 %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x7c,0x2f,0x4c,0xd3]
          vrcpnepbf16 %ymm3, %ymm2 {%k7}

// CHECK: vrcpnepbf16 %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x7c,0xaf,0x4c,0xd3]
          vrcpnepbf16 %ymm3, %ymm2 {%k7} {z}

// CHECK: vrcpnepbf16  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x08,0x4c,0x94,0xf4,0x00,0x00,0x00,0x10]
          vrcpnepbf16  268435456(%esp,%esi,8), %xmm2

// CHECK: vrcpnepbf16  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x7c,0x0f,0x4c,0x94,0x87,0x23,0x01,0x00,0x00]
          vrcpnepbf16  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vrcpnepbf16  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x18,0x4c,0x10]
          vrcpnepbf16  (%eax){1to8}, %xmm2

// CHECK: vrcpnepbf16  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x08,0x4c,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vrcpnepbf16  -512(,%ebp,2), %xmm2

// CHECK: vrcpnepbf16  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x7c,0x8f,0x4c,0x51,0x7f]
          vrcpnepbf16  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vrcpnepbf16  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x7c,0x9f,0x4c,0x52,0x80]
          vrcpnepbf16  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vrcpnepbf16  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x28,0x4c,0x94,0xf4,0x00,0x00,0x00,0x10]
          vrcpnepbf16  268435456(%esp,%esi,8), %ymm2

// CHECK: vrcpnepbf16  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x7c,0x2f,0x4c,0x94,0x87,0x23,0x01,0x00,0x00]
          vrcpnepbf16  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vrcpnepbf16  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x38,0x4c,0x10]
          vrcpnepbf16  (%eax){1to16}, %ymm2

// CHECK: vrcpnepbf16  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x28,0x4c,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vrcpnepbf16  -1024(,%ebp,2), %ymm2

// CHECK: vrcpnepbf16  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x7c,0xaf,0x4c,0x51,0x7f]
          vrcpnepbf16  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vrcpnepbf16  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x7c,0xbf,0x4c,0x52,0x80]
          vrcpnepbf16  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vreducenepbf16 $123, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x28,0x56,0xd3,0x7b]
          vreducenepbf16 $123, %ymm3, %ymm2

// CHECK: vreducenepbf16 $123, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x2f,0x56,0xd3,0x7b]
          vreducenepbf16 $123, %ymm3, %ymm2 {%k7}

// CHECK: vreducenepbf16 $123, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xaf,0x56,0xd3,0x7b]
          vreducenepbf16 $123, %ymm3, %ymm2 {%k7} {z}

// CHECK: vreducenepbf16 $123, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x08,0x56,0xd3,0x7b]
          vreducenepbf16 $123, %xmm3, %xmm2

// CHECK: vreducenepbf16 $123, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x0f,0x56,0xd3,0x7b]
          vreducenepbf16 $123, %xmm3, %xmm2 {%k7}

// CHECK: vreducenepbf16 $123, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0x8f,0x56,0xd3,0x7b]
          vreducenepbf16 $123, %xmm3, %xmm2 {%k7} {z}

// CHECK: vreducenepbf16  $123, 268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x08,0x56,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vreducenepbf16  $123, 268435456(%esp,%esi,8), %xmm2

// CHECK: vreducenepbf16  $123, 291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x0f,0x56,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vreducenepbf16  $123, 291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vreducenepbf16  $123, (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x18,0x56,0x10,0x7b]
          vreducenepbf16  $123, (%eax){1to8}, %xmm2

// CHECK: vreducenepbf16  $123, -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x08,0x56,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vreducenepbf16  $123, -512(,%ebp,2), %xmm2

// CHECK: vreducenepbf16  $123, 2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0x8f,0x56,0x51,0x7f,0x7b]
          vreducenepbf16  $123, 2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vreducenepbf16  $123, -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0x9f,0x56,0x52,0x80,0x7b]
          vreducenepbf16  $123, -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vreducenepbf16  $123, 268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x28,0x56,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vreducenepbf16  $123, 268435456(%esp,%esi,8), %ymm2

// CHECK: vreducenepbf16  $123, 291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x2f,0x56,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vreducenepbf16  $123, 291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vreducenepbf16  $123, (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x38,0x56,0x10,0x7b]
          vreducenepbf16  $123, (%eax){1to16}, %ymm2

// CHECK: vreducenepbf16  $123, -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x28,0x56,0x14,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vreducenepbf16  $123, -1024(,%ebp,2), %ymm2

// CHECK: vreducenepbf16  $123, 4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xaf,0x56,0x51,0x7f,0x7b]
          vreducenepbf16  $123, 4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vreducenepbf16  $123, -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xbf,0x56,0x52,0x80,0x7b]
          vreducenepbf16  $123, -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vrndscalenepbf16 $123, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x28,0x08,0xd3,0x7b]
          vrndscalenepbf16 $123, %ymm3, %ymm2

// CHECK: vrndscalenepbf16 $123, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x2f,0x08,0xd3,0x7b]
          vrndscalenepbf16 $123, %ymm3, %ymm2 {%k7}

// CHECK: vrndscalenepbf16 $123, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xaf,0x08,0xd3,0x7b]
          vrndscalenepbf16 $123, %ymm3, %ymm2 {%k7} {z}

// CHECK: vrndscalenepbf16 $123, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x08,0x08,0xd3,0x7b]
          vrndscalenepbf16 $123, %xmm3, %xmm2

// CHECK: vrndscalenepbf16 $123, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x0f,0x08,0xd3,0x7b]
          vrndscalenepbf16 $123, %xmm3, %xmm2 {%k7}

// CHECK: vrndscalenepbf16 $123, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0x8f,0x08,0xd3,0x7b]
          vrndscalenepbf16 $123, %xmm3, %xmm2 {%k7} {z}

// CHECK: vrndscalenepbf16  $123, 268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x08,0x08,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vrndscalenepbf16  $123, 268435456(%esp,%esi,8), %xmm2

// CHECK: vrndscalenepbf16  $123, 291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x0f,0x08,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vrndscalenepbf16  $123, 291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vrndscalenepbf16  $123, (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x18,0x08,0x10,0x7b]
          vrndscalenepbf16  $123, (%eax){1to8}, %xmm2

// CHECK: vrndscalenepbf16  $123, -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x08,0x08,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vrndscalenepbf16  $123, -512(,%ebp,2), %xmm2

// CHECK: vrndscalenepbf16  $123, 2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0x8f,0x08,0x51,0x7f,0x7b]
          vrndscalenepbf16  $123, 2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vrndscalenepbf16  $123, -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0x9f,0x08,0x52,0x80,0x7b]
          vrndscalenepbf16  $123, -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vrndscalenepbf16  $123, 268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x28,0x08,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vrndscalenepbf16  $123, 268435456(%esp,%esi,8), %ymm2

// CHECK: vrndscalenepbf16  $123, 291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x2f,0x08,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vrndscalenepbf16  $123, 291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vrndscalenepbf16  $123, (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x38,0x08,0x10,0x7b]
          vrndscalenepbf16  $123, (%eax){1to16}, %ymm2

// CHECK: vrndscalenepbf16  $123, -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x28,0x08,0x14,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vrndscalenepbf16  $123, -1024(,%ebp,2), %ymm2

// CHECK: vrndscalenepbf16  $123, 4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xaf,0x08,0x51,0x7f,0x7b]
          vrndscalenepbf16  $123, 4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vrndscalenepbf16  $123, -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xbf,0x08,0x52,0x80,0x7b]
          vrndscalenepbf16  $123, -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vrsqrtnepbf16 %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x08,0x4e,0xd3]
          vrsqrtnepbf16 %xmm3, %xmm2

// CHECK: vrsqrtnepbf16 %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x7c,0x0f,0x4e,0xd3]
          vrsqrtnepbf16 %xmm3, %xmm2 {%k7}

// CHECK: vrsqrtnepbf16 %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x7c,0x8f,0x4e,0xd3]
          vrsqrtnepbf16 %xmm3, %xmm2 {%k7} {z}

// CHECK: vrsqrtnepbf16 %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x28,0x4e,0xd3]
          vrsqrtnepbf16 %ymm3, %ymm2

// CHECK: vrsqrtnepbf16 %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x7c,0x2f,0x4e,0xd3]
          vrsqrtnepbf16 %ymm3, %ymm2 {%k7}

// CHECK: vrsqrtnepbf16 %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x7c,0xaf,0x4e,0xd3]
          vrsqrtnepbf16 %ymm3, %ymm2 {%k7} {z}

// CHECK: vrsqrtnepbf16  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x08,0x4e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vrsqrtnepbf16  268435456(%esp,%esi,8), %xmm2

// CHECK: vrsqrtnepbf16  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x7c,0x0f,0x4e,0x94,0x87,0x23,0x01,0x00,0x00]
          vrsqrtnepbf16  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vrsqrtnepbf16  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x18,0x4e,0x10]
          vrsqrtnepbf16  (%eax){1to8}, %xmm2

// CHECK: vrsqrtnepbf16  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x08,0x4e,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vrsqrtnepbf16  -512(,%ebp,2), %xmm2

// CHECK: vrsqrtnepbf16  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x7c,0x8f,0x4e,0x51,0x7f]
          vrsqrtnepbf16  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vrsqrtnepbf16  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x7c,0x9f,0x4e,0x52,0x80]
          vrsqrtnepbf16  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vrsqrtnepbf16  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x28,0x4e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vrsqrtnepbf16  268435456(%esp,%esi,8), %ymm2

// CHECK: vrsqrtnepbf16  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x7c,0x2f,0x4e,0x94,0x87,0x23,0x01,0x00,0x00]
          vrsqrtnepbf16  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vrsqrtnepbf16  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x38,0x4e,0x10]
          vrsqrtnepbf16  (%eax){1to16}, %ymm2

// CHECK: vrsqrtnepbf16  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf6,0x7c,0x28,0x4e,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vrsqrtnepbf16  -1024(,%ebp,2), %ymm2

// CHECK: vrsqrtnepbf16  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x7c,0xaf,0x4e,0x51,0x7f]
          vrsqrtnepbf16  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vrsqrtnepbf16  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x7c,0xbf,0x4e,0x52,0x80]
          vrsqrtnepbf16  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vscalefnepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x2c,0xd4]
          vscalefnepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vscalefnepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0x2c,0xd4]
          vscalefnepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vscalefnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0x2c,0xd4]
          vscalefnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vscalefnepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x2c,0xd4]
          vscalefnepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vscalefnepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0x2c,0xd4]
          vscalefnepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vscalefnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0x2c,0xd4]
          vscalefnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vscalefnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x2c,0x94,0xf4,0x00,0x00,0x00,0x10]
          vscalefnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vscalefnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0x2c,0x94,0x87,0x23,0x01,0x00,0x00]
          vscalefnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vscalefnepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0x2c,0x10]
          vscalefnepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vscalefnepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0x2c,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vscalefnepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vscalefnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0x2c,0x51,0x7f]
          vscalefnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vscalefnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0x2c,0x52,0x80]
          vscalefnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vscalefnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x2c,0x94,0xf4,0x00,0x00,0x00,0x10]
          vscalefnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vscalefnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0x2c,0x94,0x87,0x23,0x01,0x00,0x00]
          vscalefnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vscalefnepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0x2c,0x10]
          vscalefnepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vscalefnepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0x2c,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vscalefnepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vscalefnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0x2c,0x51,0x7f]
          vscalefnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vscalefnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0x2c,0x52,0x80]
          vscalefnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vsqrtnepbf16 %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x51,0xd3]
          vsqrtnepbf16 %xmm3, %xmm2

// CHECK: vsqrtnepbf16 %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x51,0xd3]
          vsqrtnepbf16 %xmm3, %xmm2 {%k7}

// CHECK: vsqrtnepbf16 %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x8f,0x51,0xd3]
          vsqrtnepbf16 %xmm3, %xmm2 {%k7} {z}

// CHECK: vsqrtnepbf16 %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x51,0xd3]
          vsqrtnepbf16 %ymm3, %ymm2

// CHECK: vsqrtnepbf16 %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x51,0xd3]
          vsqrtnepbf16 %ymm3, %ymm2 {%k7}

// CHECK: vsqrtnepbf16 %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xaf,0x51,0xd3]
          vsqrtnepbf16 %ymm3, %ymm2 {%k7} {z}

// CHECK: vsqrtnepbf16  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsqrtnepbf16  268435456(%esp,%esi,8), %xmm2

// CHECK: vsqrtnepbf16  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
          vsqrtnepbf16  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vsqrtnepbf16  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x51,0x10]
          vsqrtnepbf16  (%eax){1to8}, %xmm2

// CHECK: vsqrtnepbf16  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x51,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vsqrtnepbf16  -512(,%ebp,2), %xmm2

// CHECK: vsqrtnepbf16  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x8f,0x51,0x51,0x7f]
          vsqrtnepbf16  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vsqrtnepbf16  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x9f,0x51,0x52,0x80]
          vsqrtnepbf16  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vsqrtnepbf16  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsqrtnepbf16  268435456(%esp,%esi,8), %ymm2

// CHECK: vsqrtnepbf16  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
          vsqrtnepbf16  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vsqrtnepbf16  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x51,0x10]
          vsqrtnepbf16  (%eax){1to16}, %ymm2

// CHECK: vsqrtnepbf16  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x51,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vsqrtnepbf16  -1024(,%ebp,2), %ymm2

// CHECK: vsqrtnepbf16  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xaf,0x51,0x51,0x7f]
          vsqrtnepbf16  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vsqrtnepbf16  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xbf,0x51,0x52,0x80]
          vsqrtnepbf16  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vsubnepbf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x5c,0xd4]
          vsubnepbf16 %ymm4, %ymm3, %ymm2

// CHECK: vsubnepbf16 %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x2f,0x5c,0xd4]
          vsubnepbf16 %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vsubnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xaf,0x5c,0xd4]
          vsubnepbf16 %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vsubnepbf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x5c,0xd4]
          vsubnepbf16 %xmm4, %xmm3, %xmm2

// CHECK: vsubnepbf16 %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x0f,0x5c,0xd4]
          vsubnepbf16 %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vsubnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x8f,0x5c,0xd4]
          vsubnepbf16 %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vsubnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x5c,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubnepbf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vsubnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x2f,0x5c,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubnepbf16  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vsubnepbf16  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x5c,0x10]
          vsubnepbf16  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vsubnepbf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x5c,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vsubnepbf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vsubnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xaf,0x5c,0x51,0x7f]
          vsubnepbf16  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vsubnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0xbf,0x5c,0x52,0x80]
          vsubnepbf16  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vsubnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x5c,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubnepbf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vsubnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x65,0x0f,0x5c,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubnepbf16  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vsubnepbf16  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x5c,0x10]
          vsubnepbf16  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vsubnepbf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x5c,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vsubnepbf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vsubnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x8f,0x5c,0x51,0x7f]
          vsubnepbf16  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vsubnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x65,0x9f,0x5c,0x52,0x80]
          vsubnepbf16  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

