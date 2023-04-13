// REQUIRES: intel_feature_isa_avx512_sat_cvt,avx512vl
// RUN: llvm-mc -triple i386 --show-encoding %s | FileCheck %s

// CHECK: vcvtnebf162ibs %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x69,0xd3]
          vcvtnebf162ibs %xmm3, %xmm2

// CHECK: vcvtnebf162ibs %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x0f,0x69,0xd3]
          vcvtnebf162ibs %xmm3, %xmm2 {%k7}

// CHECK: vcvtnebf162ibs %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x69,0xd3]
          vcvtnebf162ibs %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtnebf162ibs %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x69,0xd3]
          vcvtnebf162ibs %ymm3, %ymm2

// CHECK: vcvtnebf162ibs %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x2f,0x69,0xd3]
          vcvtnebf162ibs %ymm3, %ymm2 {%k7}

// CHECK: vcvtnebf162ibs %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x69,0xd3]
          vcvtnebf162ibs %ymm3, %ymm2 {%k7} {z}

// CHECK: vcvtnebf162ibs  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x69,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnebf162ibs  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtnebf162ibs  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x0f,0x69,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnebf162ibs  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtnebf162ibs  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x18,0x69,0x10]
          vcvtnebf162ibs  (%eax){1to8}, %xmm2

// CHECK: vcvtnebf162ibs  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x69,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtnebf162ibs  -512(,%ebp,2), %xmm2

// CHECK: vcvtnebf162ibs  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x69,0x51,0x7f]
          vcvtnebf162ibs  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtnebf162ibs  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x9f,0x69,0x52,0x80]
          vcvtnebf162ibs  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvtnebf162ibs  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x69,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnebf162ibs  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtnebf162ibs  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x2f,0x69,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnebf162ibs  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtnebf162ibs  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x38,0x69,0x10]
          vcvtnebf162ibs  (%eax){1to16}, %ymm2

// CHECK: vcvtnebf162ibs  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x69,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtnebf162ibs  -1024(,%ebp,2), %ymm2

// CHECK: vcvtnebf162ibs  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x69,0x51,0x7f]
          vcvtnebf162ibs  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtnebf162ibs  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xbf,0x69,0x52,0x80]
          vcvtnebf162ibs  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vcvtnebf162iubs %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x6b,0xd3]
          vcvtnebf162iubs %xmm3, %xmm2

// CHECK: vcvtnebf162iubs %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x0f,0x6b,0xd3]
          vcvtnebf162iubs %xmm3, %xmm2 {%k7}

// CHECK: vcvtnebf162iubs %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x6b,0xd3]
          vcvtnebf162iubs %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtnebf162iubs %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x6b,0xd3]
          vcvtnebf162iubs %ymm3, %ymm2

// CHECK: vcvtnebf162iubs %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x2f,0x6b,0xd3]
          vcvtnebf162iubs %ymm3, %ymm2 {%k7}

// CHECK: vcvtnebf162iubs %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x6b,0xd3]
          vcvtnebf162iubs %ymm3, %ymm2 {%k7} {z}

// CHECK: vcvtnebf162iubs  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x6b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnebf162iubs  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtnebf162iubs  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x0f,0x6b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnebf162iubs  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtnebf162iubs  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x18,0x6b,0x10]
          vcvtnebf162iubs  (%eax){1to8}, %xmm2

// CHECK: vcvtnebf162iubs  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x6b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtnebf162iubs  -512(,%ebp,2), %xmm2

// CHECK: vcvtnebf162iubs  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x6b,0x51,0x7f]
          vcvtnebf162iubs  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtnebf162iubs  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x9f,0x6b,0x52,0x80]
          vcvtnebf162iubs  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvtnebf162iubs  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x6b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnebf162iubs  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtnebf162iubs  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x2f,0x6b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnebf162iubs  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtnebf162iubs  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x38,0x6b,0x10]
          vcvtnebf162iubs  (%eax){1to16}, %ymm2

// CHECK: vcvtnebf162iubs  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x6b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtnebf162iubs  -1024(,%ebp,2), %ymm2

// CHECK: vcvtnebf162iubs  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x6b,0x51,0x7f]
          vcvtnebf162iubs  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtnebf162iubs  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xbf,0x6b,0x52,0x80]
          vcvtnebf162iubs  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vcvtph2ibs %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x69,0xd3]
          vcvtph2ibs %xmm3, %xmm2

// CHECK: vcvtph2ibs %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x69,0xd3]
          vcvtph2ibs %xmm3, %xmm2 {%k7}

// CHECK: vcvtph2ibs %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x69,0xd3]
          vcvtph2ibs %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtph2ibs %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x69,0xd3]
          vcvtph2ibs %ymm3, %ymm2

// CHECK: vcvtph2ibs %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x69,0xd3]
          vcvtph2ibs %ymm3, %ymm2 {%k7}

// CHECK: vcvtph2ibs %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x69,0xd3]
          vcvtph2ibs %ymm3, %ymm2 {%k7} {z}

// CHECK: vcvtph2ibs  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x69,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtph2ibs  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtph2ibs  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x69,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtph2ibs  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtph2ibs  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x18,0x69,0x10]
          vcvtph2ibs  (%eax){1to8}, %xmm2

// CHECK: vcvtph2ibs  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x69,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2ibs  -512(,%ebp,2), %xmm2

// CHECK: vcvtph2ibs  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x69,0x51,0x7f]
          vcvtph2ibs  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtph2ibs  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x9f,0x69,0x52,0x80]
          vcvtph2ibs  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvtph2ibs  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x69,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtph2ibs  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtph2ibs  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x69,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtph2ibs  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtph2ibs  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x38,0x69,0x10]
          vcvtph2ibs  (%eax){1to16}, %ymm2

// CHECK: vcvtph2ibs  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x69,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtph2ibs  -1024(,%ebp,2), %ymm2

// CHECK: vcvtph2ibs  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x69,0x51,0x7f]
          vcvtph2ibs  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtph2ibs  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xbf,0x69,0x52,0x80]
          vcvtph2ibs  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vcvtph2iubs %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x6b,0xd3]
          vcvtph2iubs %xmm3, %xmm2

// CHECK: vcvtph2iubs %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x6b,0xd3]
          vcvtph2iubs %xmm3, %xmm2 {%k7}

// CHECK: vcvtph2iubs %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x6b,0xd3]
          vcvtph2iubs %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtph2iubs %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x6b,0xd3]
          vcvtph2iubs %ymm3, %ymm2

// CHECK: vcvtph2iubs %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x6b,0xd3]
          vcvtph2iubs %ymm3, %ymm2 {%k7}

// CHECK: vcvtph2iubs %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x6b,0xd3]
          vcvtph2iubs %ymm3, %ymm2 {%k7} {z}

// CHECK: vcvtph2iubs  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x6b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtph2iubs  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtph2iubs  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x6b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtph2iubs  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtph2iubs  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x18,0x6b,0x10]
          vcvtph2iubs  (%eax){1to8}, %xmm2

// CHECK: vcvtph2iubs  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x6b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2iubs  -512(,%ebp,2), %xmm2

// CHECK: vcvtph2iubs  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x6b,0x51,0x7f]
          vcvtph2iubs  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtph2iubs  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x9f,0x6b,0x52,0x80]
          vcvtph2iubs  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvtph2iubs  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x6b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtph2iubs  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtph2iubs  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x6b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtph2iubs  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtph2iubs  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x38,0x6b,0x10]
          vcvtph2iubs  (%eax){1to16}, %ymm2

// CHECK: vcvtph2iubs  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x6b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtph2iubs  -1024(,%ebp,2), %ymm2

// CHECK: vcvtph2iubs  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x6b,0x51,0x7f]
          vcvtph2iubs  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtph2iubs  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xbf,0x6b,0x52,0x80]
          vcvtph2iubs  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vcvtps2ibs %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x69,0xd3]
          vcvtps2ibs %xmm3, %xmm2

// CHECK: vcvtps2ibs %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x69,0xd3]
          vcvtps2ibs %xmm3, %xmm2 {%k7}

// CHECK: vcvtps2ibs %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x8f,0x69,0xd3]
          vcvtps2ibs %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtps2ibs %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x69,0xd3]
          vcvtps2ibs %ymm3, %ymm2

// CHECK: vcvtps2ibs %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x69,0xd3]
          vcvtps2ibs %ymm3, %ymm2 {%k7}

// CHECK: vcvtps2ibs %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xaf,0x69,0xd3]
          vcvtps2ibs %ymm3, %ymm2 {%k7} {z}

// CHECK: vcvtps2ibs  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x69,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtps2ibs  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtps2ibs  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x69,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtps2ibs  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtps2ibs  (%eax){1to4}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x69,0x10]
          vcvtps2ibs  (%eax){1to4}, %xmm2

// CHECK: vcvtps2ibs  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x69,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtps2ibs  -512(,%ebp,2), %xmm2

// CHECK: vcvtps2ibs  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x8f,0x69,0x51,0x7f]
          vcvtps2ibs  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtps2ibs  -512(%edx){1to4}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x9f,0x69,0x52,0x80]
          vcvtps2ibs  -512(%edx){1to4}, %xmm2 {%k7} {z}

// CHECK: vcvtps2ibs  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x69,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtps2ibs  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtps2ibs  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x69,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtps2ibs  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtps2ibs  (%eax){1to8}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x69,0x10]
          vcvtps2ibs  (%eax){1to8}, %ymm2

// CHECK: vcvtps2ibs  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x69,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtps2ibs  -1024(,%ebp,2), %ymm2

// CHECK: vcvtps2ibs  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xaf,0x69,0x51,0x7f]
          vcvtps2ibs  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtps2ibs  -512(%edx){1to8}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xbf,0x69,0x52,0x80]
          vcvtps2ibs  -512(%edx){1to8}, %ymm2 {%k7} {z}

// CHECK: vcvtps2iubs %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x6b,0xd3]
          vcvtps2iubs %xmm3, %xmm2

// CHECK: vcvtps2iubs %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x6b,0xd3]
          vcvtps2iubs %xmm3, %xmm2 {%k7}

// CHECK: vcvtps2iubs %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x8f,0x6b,0xd3]
          vcvtps2iubs %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtps2iubs %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x6b,0xd3]
          vcvtps2iubs %ymm3, %ymm2

// CHECK: vcvtps2iubs %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x6b,0xd3]
          vcvtps2iubs %ymm3, %ymm2 {%k7}

// CHECK: vcvtps2iubs %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xaf,0x6b,0xd3]
          vcvtps2iubs %ymm3, %ymm2 {%k7} {z}

// CHECK: vcvtps2iubs  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x6b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtps2iubs  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtps2iubs  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x6b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtps2iubs  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtps2iubs  (%eax){1to4}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x6b,0x10]
          vcvtps2iubs  (%eax){1to4}, %xmm2

// CHECK: vcvtps2iubs  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x6b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtps2iubs  -512(,%ebp,2), %xmm2

// CHECK: vcvtps2iubs  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x8f,0x6b,0x51,0x7f]
          vcvtps2iubs  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtps2iubs  -512(%edx){1to4}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x9f,0x6b,0x52,0x80]
          vcvtps2iubs  -512(%edx){1to4}, %xmm2 {%k7} {z}

// CHECK: vcvtps2iubs  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x6b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtps2iubs  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtps2iubs  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x6b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtps2iubs  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtps2iubs  (%eax){1to8}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x6b,0x10]
          vcvtps2iubs  (%eax){1to8}, %ymm2

// CHECK: vcvtps2iubs  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x6b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtps2iubs  -1024(,%ebp,2), %ymm2

// CHECK: vcvtps2iubs  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xaf,0x6b,0x51,0x7f]
          vcvtps2iubs  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtps2iubs  -512(%edx){1to8}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xbf,0x6b,0x52,0x80]
          vcvtps2iubs  -512(%edx){1to8}, %ymm2 {%k7} {z}

// CHECK: vcvttnebf162ibs %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x68,0xd3]
          vcvttnebf162ibs %xmm3, %xmm2

// CHECK: vcvttnebf162ibs %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x0f,0x68,0xd3]
          vcvttnebf162ibs %xmm3, %xmm2 {%k7}

// CHECK: vcvttnebf162ibs %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x68,0xd3]
          vcvttnebf162ibs %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvttnebf162ibs %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x68,0xd3]
          vcvttnebf162ibs %ymm3, %ymm2

// CHECK: vcvttnebf162ibs %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x2f,0x68,0xd3]
          vcvttnebf162ibs %ymm3, %ymm2 {%k7}

// CHECK: vcvttnebf162ibs %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x68,0xd3]
          vcvttnebf162ibs %ymm3, %ymm2 {%k7} {z}

// CHECK: vcvttnebf162ibs  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x68,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvttnebf162ibs  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvttnebf162ibs  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x0f,0x68,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvttnebf162ibs  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvttnebf162ibs  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x18,0x68,0x10]
          vcvttnebf162ibs  (%eax){1to8}, %xmm2

// CHECK: vcvttnebf162ibs  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x68,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvttnebf162ibs  -512(,%ebp,2), %xmm2

// CHECK: vcvttnebf162ibs  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x68,0x51,0x7f]
          vcvttnebf162ibs  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvttnebf162ibs  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x9f,0x68,0x52,0x80]
          vcvttnebf162ibs  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvttnebf162ibs  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x68,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvttnebf162ibs  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvttnebf162ibs  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x2f,0x68,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvttnebf162ibs  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvttnebf162ibs  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x38,0x68,0x10]
          vcvttnebf162ibs  (%eax){1to16}, %ymm2

// CHECK: vcvttnebf162ibs  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x68,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvttnebf162ibs  -1024(,%ebp,2), %ymm2

// CHECK: vcvttnebf162ibs  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x68,0x51,0x7f]
          vcvttnebf162ibs  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvttnebf162ibs  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xbf,0x68,0x52,0x80]
          vcvttnebf162ibs  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vcvttnebf162iubs %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x6a,0xd3]
          vcvttnebf162iubs %xmm3, %xmm2

// CHECK: vcvttnebf162iubs %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x0f,0x6a,0xd3]
          vcvttnebf162iubs %xmm3, %xmm2 {%k7}

// CHECK: vcvttnebf162iubs %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x6a,0xd3]
          vcvttnebf162iubs %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvttnebf162iubs %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x6a,0xd3]
          vcvttnebf162iubs %ymm3, %ymm2

// CHECK: vcvttnebf162iubs %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x2f,0x6a,0xd3]
          vcvttnebf162iubs %ymm3, %ymm2 {%k7}

// CHECK: vcvttnebf162iubs %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x6a,0xd3]
          vcvttnebf162iubs %ymm3, %ymm2 {%k7} {z}

// CHECK: vcvttnebf162iubs  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x6a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvttnebf162iubs  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvttnebf162iubs  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x0f,0x6a,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvttnebf162iubs  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvttnebf162iubs  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x18,0x6a,0x10]
          vcvttnebf162iubs  (%eax){1to8}, %xmm2

// CHECK: vcvttnebf162iubs  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x6a,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvttnebf162iubs  -512(,%ebp,2), %xmm2

// CHECK: vcvttnebf162iubs  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x6a,0x51,0x7f]
          vcvttnebf162iubs  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvttnebf162iubs  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x9f,0x6a,0x52,0x80]
          vcvttnebf162iubs  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvttnebf162iubs  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x6a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvttnebf162iubs  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvttnebf162iubs  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x2f,0x6a,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvttnebf162iubs  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvttnebf162iubs  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x38,0x6a,0x10]
          vcvttnebf162iubs  (%eax){1to16}, %ymm2

// CHECK: vcvttnebf162iubs  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x6a,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvttnebf162iubs  -1024(,%ebp,2), %ymm2

// CHECK: vcvttnebf162iubs  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x6a,0x51,0x7f]
          vcvttnebf162iubs  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvttnebf162iubs  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xbf,0x6a,0x52,0x80]
          vcvttnebf162iubs  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vcvttph2ibs %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x68,0xd3]
          vcvttph2ibs %xmm3, %xmm2

// CHECK: vcvttph2ibs %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x68,0xd3]
          vcvttph2ibs %xmm3, %xmm2 {%k7}

// CHECK: vcvttph2ibs %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x68,0xd3]
          vcvttph2ibs %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvttph2ibs %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x68,0xd3]
          vcvttph2ibs %ymm3, %ymm2

// CHECK: vcvttph2ibs %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x68,0xd3]
          vcvttph2ibs %ymm3, %ymm2 {%k7}

// CHECK: vcvttph2ibs %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x68,0xd3]
          vcvttph2ibs %ymm3, %ymm2 {%k7} {z}

// CHECK: vcvttph2ibs  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x68,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvttph2ibs  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvttph2ibs  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x68,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvttph2ibs  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvttph2ibs  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x18,0x68,0x10]
          vcvttph2ibs  (%eax){1to8}, %xmm2

// CHECK: vcvttph2ibs  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x68,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2ibs  -512(,%ebp,2), %xmm2

// CHECK: vcvttph2ibs  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x68,0x51,0x7f]
          vcvttph2ibs  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvttph2ibs  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x9f,0x68,0x52,0x80]
          vcvttph2ibs  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvttph2ibs  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x68,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvttph2ibs  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvttph2ibs  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x68,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvttph2ibs  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvttph2ibs  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x38,0x68,0x10]
          vcvttph2ibs  (%eax){1to16}, %ymm2

// CHECK: vcvttph2ibs  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x68,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvttph2ibs  -1024(,%ebp,2), %ymm2

// CHECK: vcvttph2ibs  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x68,0x51,0x7f]
          vcvttph2ibs  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvttph2ibs  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xbf,0x68,0x52,0x80]
          vcvttph2ibs  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vcvttph2iubs %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x6a,0xd3]
          vcvttph2iubs %xmm3, %xmm2

// CHECK: vcvttph2iubs %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x6a,0xd3]
          vcvttph2iubs %xmm3, %xmm2 {%k7}

// CHECK: vcvttph2iubs %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x6a,0xd3]
          vcvttph2iubs %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvttph2iubs %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x6a,0xd3]
          vcvttph2iubs %ymm3, %ymm2

// CHECK: vcvttph2iubs %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x6a,0xd3]
          vcvttph2iubs %ymm3, %ymm2 {%k7}

// CHECK: vcvttph2iubs %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x6a,0xd3]
          vcvttph2iubs %ymm3, %ymm2 {%k7} {z}

// CHECK: vcvttph2iubs  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x6a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvttph2iubs  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvttph2iubs  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x6a,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvttph2iubs  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvttph2iubs  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x18,0x6a,0x10]
          vcvttph2iubs  (%eax){1to8}, %xmm2

// CHECK: vcvttph2iubs  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x6a,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2iubs  -512(,%ebp,2), %xmm2

// CHECK: vcvttph2iubs  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x6a,0x51,0x7f]
          vcvttph2iubs  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvttph2iubs  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x9f,0x6a,0x52,0x80]
          vcvttph2iubs  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvttph2iubs  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x6a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvttph2iubs  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvttph2iubs  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x6a,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvttph2iubs  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvttph2iubs  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x38,0x6a,0x10]
          vcvttph2iubs  (%eax){1to16}, %ymm2

// CHECK: vcvttph2iubs  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x6a,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvttph2iubs  -1024(,%ebp,2), %ymm2

// CHECK: vcvttph2iubs  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x6a,0x51,0x7f]
          vcvttph2iubs  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvttph2iubs  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xbf,0x6a,0x52,0x80]
          vcvttph2iubs  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK: vcvttps2ibs %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x68,0xd3]
          vcvttps2ibs %xmm3, %xmm2

// CHECK: vcvttps2ibs %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x68,0xd3]
          vcvttps2ibs %xmm3, %xmm2 {%k7}

// CHECK: vcvttps2ibs %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x8f,0x68,0xd3]
          vcvttps2ibs %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvttps2ibs %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x68,0xd3]
          vcvttps2ibs %ymm3, %ymm2

// CHECK: vcvttps2ibs %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x68,0xd3]
          vcvttps2ibs %ymm3, %ymm2 {%k7}

// CHECK: vcvttps2ibs %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xaf,0x68,0xd3]
          vcvttps2ibs %ymm3, %ymm2 {%k7} {z}

// CHECK: vcvttps2ibs  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x68,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvttps2ibs  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvttps2ibs  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x68,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvttps2ibs  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvttps2ibs  (%eax){1to4}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x68,0x10]
          vcvttps2ibs  (%eax){1to4}, %xmm2

// CHECK: vcvttps2ibs  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x68,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvttps2ibs  -512(,%ebp,2), %xmm2

// CHECK: vcvttps2ibs  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x8f,0x68,0x51,0x7f]
          vcvttps2ibs  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvttps2ibs  -512(%edx){1to4}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x9f,0x68,0x52,0x80]
          vcvttps2ibs  -512(%edx){1to4}, %xmm2 {%k7} {z}

// CHECK: vcvttps2ibs  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x68,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvttps2ibs  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvttps2ibs  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x68,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvttps2ibs  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvttps2ibs  (%eax){1to8}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x68,0x10]
          vcvttps2ibs  (%eax){1to8}, %ymm2

// CHECK: vcvttps2ibs  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x68,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvttps2ibs  -1024(,%ebp,2), %ymm2

// CHECK: vcvttps2ibs  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xaf,0x68,0x51,0x7f]
          vcvttps2ibs  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvttps2ibs  -512(%edx){1to8}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xbf,0x68,0x52,0x80]
          vcvttps2ibs  -512(%edx){1to8}, %ymm2 {%k7} {z}

// CHECK: vcvttps2iubs %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x6a,0xd3]
          vcvttps2iubs %xmm3, %xmm2

// CHECK: vcvttps2iubs %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x6a,0xd3]
          vcvttps2iubs %xmm3, %xmm2 {%k7}

// CHECK: vcvttps2iubs %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x8f,0x6a,0xd3]
          vcvttps2iubs %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvttps2iubs %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x6a,0xd3]
          vcvttps2iubs %ymm3, %ymm2

// CHECK: vcvttps2iubs %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x6a,0xd3]
          vcvttps2iubs %ymm3, %ymm2 {%k7}

// CHECK: vcvttps2iubs %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xaf,0x6a,0xd3]
          vcvttps2iubs %ymm3, %ymm2 {%k7} {z}

// CHECK: vcvttps2iubs  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x6a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvttps2iubs  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvttps2iubs  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x0f,0x6a,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvttps2iubs  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvttps2iubs  (%eax){1to4}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x18,0x6a,0x10]
          vcvttps2iubs  (%eax){1to4}, %xmm2

// CHECK: vcvttps2iubs  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x08,0x6a,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvttps2iubs  -512(,%ebp,2), %xmm2

// CHECK: vcvttps2iubs  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x8f,0x6a,0x51,0x7f]
          vcvttps2iubs  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvttps2iubs  -512(%edx){1to4}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0x9f,0x6a,0x52,0x80]
          vcvttps2iubs  -512(%edx){1to4}, %xmm2 {%k7} {z}

// CHECK: vcvttps2iubs  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x6a,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvttps2iubs  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvttps2iubs  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7d,0x2f,0x6a,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvttps2iubs  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvttps2iubs  (%eax){1to8}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x38,0x6a,0x10]
          vcvttps2iubs  (%eax){1to8}, %ymm2

// CHECK: vcvttps2iubs  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7d,0x28,0x6a,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvttps2iubs  -1024(,%ebp,2), %ymm2

// CHECK: vcvttps2iubs  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xaf,0x6a,0x51,0x7f]
          vcvttps2iubs  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvttps2iubs  -512(%edx){1to8}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7d,0xbf,0x6a,0x52,0x80]
          vcvttps2iubs  -512(%edx){1to8}, %ymm2 {%k7} {z}

