// REQUIRES: intel_feature_isa_avx512_vnni_int16
// RUN: llvm-mc -triple i686-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      vpdpwsud %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x66,0xd2,0xd4]
               vpdpwsud %ymm4, %ymm3, %ymm2

// CHECK:      vpdpwsud %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0xd2,0xd4]
               vpdpwsud %ymm4, %ymm3, %ymm2 {%k7}

// CHECK:      vpdpwsud %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0xd2,0xd4]
               vpdpwsud %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwsud %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x62,0xd2,0xd4]
               vpdpwsud %xmm4, %xmm3, %xmm2

// CHECK:      vpdpwsud %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0xd2,0xd4]
               vpdpwsud %xmm4, %xmm3, %xmm2 {%k7}

// CHECK:      vpdpwsud %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0xd2,0xd4]
               vpdpwsud %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwsud  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x66,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwsud  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK:      vpdpwsud  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwsud  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK:      vpdpwsud  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x66,0x38,0xd2,0x10]
               vpdpwsud  (%eax){1to8}, %ymm3, %ymm2

// CHECK:      vpdpwsud  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x66,0xd2,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwsud  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK:      vpdpwsud  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0xd2,0x51,0x7f]
               vpdpwsud  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwsud  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xbf,0xd2,0x52,0x80]
               vpdpwsud  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwsud  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x62,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwsud  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      vpdpwsud  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwsud  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK:      vpdpwsud  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x18,0xd2,0x10]
               vpdpwsud  (%eax){1to4}, %xmm3, %xmm2

// CHECK:      vpdpwsud  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x62,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwsud  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      vpdpwsud  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0xd2,0x51,0x7f]
               vpdpwsud  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwsud  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0x9f,0xd2,0x52,0x80]
               vpdpwsud  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwsuds %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x66,0xd3,0xd4]
               vpdpwsuds %ymm4, %ymm3, %ymm2

// CHECK:      vpdpwsuds %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0xd3,0xd4]
               vpdpwsuds %ymm4, %ymm3, %ymm2 {%k7}

// CHECK:      vpdpwsuds %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0xd3,0xd4]
               vpdpwsuds %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwsuds %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x62,0xd3,0xd4]
               vpdpwsuds %xmm4, %xmm3, %xmm2

// CHECK:      vpdpwsuds %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0xd3,0xd4]
               vpdpwsuds %xmm4, %xmm3, %xmm2 {%k7}

// CHECK:      vpdpwsuds %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0xd3,0xd4]
               vpdpwsuds %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwsuds  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x66,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwsuds  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK:      vpdpwsuds  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwsuds  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK:      vpdpwsuds  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x66,0x38,0xd3,0x10]
               vpdpwsuds  (%eax){1to8}, %ymm3, %ymm2

// CHECK:      vpdpwsuds  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x66,0xd3,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwsuds  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK:      vpdpwsuds  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0xd3,0x51,0x7f]
               vpdpwsuds  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwsuds  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xbf,0xd3,0x52,0x80]
               vpdpwsuds  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwsuds  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x62,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwsuds  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      vpdpwsuds  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwsuds  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK:      vpdpwsuds  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x18,0xd3,0x10]
               vpdpwsuds  (%eax){1to4}, %xmm3, %xmm2

// CHECK:      vpdpwsuds  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x62,0xd3,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwsuds  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      vpdpwsuds  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0xd3,0x51,0x7f]
               vpdpwsuds  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwsuds  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0x9f,0xd3,0x52,0x80]
               vpdpwsuds  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwusd %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x65,0xd2,0xd4]
               vpdpwusd %ymm4, %ymm3, %ymm2

// CHECK:      vpdpwusd %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x2f,0xd2,0xd4]
               vpdpwusd %ymm4, %ymm3, %ymm2 {%k7}

// CHECK:      vpdpwusd %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xaf,0xd2,0xd4]
               vpdpwusd %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwusd %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x61,0xd2,0xd4]
               vpdpwusd %xmm4, %xmm3, %xmm2

// CHECK:      vpdpwusd %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x0f,0xd2,0xd4]
               vpdpwusd %xmm4, %xmm3, %xmm2 {%k7}

// CHECK:      vpdpwusd %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0x8f,0xd2,0xd4]
               vpdpwusd %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwusd  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x65,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwusd  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK:      vpdpwusd  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x2f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwusd  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK:      vpdpwusd  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x38,0xd2,0x10]
               vpdpwusd  (%eax){1to8}, %ymm3, %ymm2

// CHECK:      vpdpwusd  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x65,0xd2,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwusd  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK:      vpdpwusd  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xaf,0xd2,0x51,0x7f]
               vpdpwusd  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwusd  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xbf,0xd2,0x52,0x80]
               vpdpwusd  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwusd  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x61,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwusd  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      vpdpwusd  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x0f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwusd  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK:      vpdpwusd  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0xd2,0x10]
               vpdpwusd  (%eax){1to4}, %xmm3, %xmm2

// CHECK:      vpdpwusd  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x61,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwusd  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      vpdpwusd  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0x8f,0xd2,0x51,0x7f]
               vpdpwusd  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwusd  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0x9f,0xd2,0x52,0x80]
               vpdpwusd  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwusds %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x65,0xd3,0xd4]
               vpdpwusds %ymm4, %ymm3, %ymm2

// CHECK:      vpdpwusds %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x2f,0xd3,0xd4]
               vpdpwusds %ymm4, %ymm3, %ymm2 {%k7}

// CHECK:      vpdpwusds %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xaf,0xd3,0xd4]
               vpdpwusds %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwusds %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x61,0xd3,0xd4]
               vpdpwusds %xmm4, %xmm3, %xmm2

// CHECK:      vpdpwusds %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x0f,0xd3,0xd4]
               vpdpwusds %xmm4, %xmm3, %xmm2 {%k7}

// CHECK:      vpdpwusds %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0x8f,0xd3,0xd4]
               vpdpwusds %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwusds  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x65,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwusds  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK:      vpdpwusds  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x2f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwusds  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK:      vpdpwusds  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x38,0xd3,0x10]
               vpdpwusds  (%eax){1to8}, %ymm3, %ymm2

// CHECK:      vpdpwusds  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x65,0xd3,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwusds  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK:      vpdpwusds  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xaf,0xd3,0x51,0x7f]
               vpdpwusds  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwusds  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xbf,0xd3,0x52,0x80]
               vpdpwusds  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwusds  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x61,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwusds  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      vpdpwusds  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x0f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwusds  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK:      vpdpwusds  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0xd3,0x10]
               vpdpwusds  (%eax){1to4}, %xmm3, %xmm2

// CHECK:      vpdpwusds  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x61,0xd3,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwusds  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      vpdpwusds  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0x8f,0xd3,0x51,0x7f]
               vpdpwusds  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwusds  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0x9f,0xd3,0x52,0x80]
               vpdpwusds  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwuud %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x64,0xd2,0xd4]
               vpdpwuud %ymm4, %ymm3, %ymm2

// CHECK:      vpdpwuud %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0xd2,0xd4]
               vpdpwuud %ymm4, %ymm3, %ymm2 {%k7}

// CHECK:      vpdpwuud %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0xd2,0xd4]
               vpdpwuud %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwuud %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x60,0xd2,0xd4]
               vpdpwuud %xmm4, %xmm3, %xmm2

// CHECK:      vpdpwuud %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0xd2,0xd4]
               vpdpwuud %xmm4, %xmm3, %xmm2 {%k7}

// CHECK:      vpdpwuud %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0xd2,0xd4]
               vpdpwuud %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwuud  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x64,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwuud  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK:      vpdpwuud  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwuud  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK:      vpdpwuud  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x38,0xd2,0x10]
               vpdpwuud  (%eax){1to8}, %ymm3, %ymm2

// CHECK:      vpdpwuud  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x64,0xd2,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwuud  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK:      vpdpwuud  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0xd2,0x51,0x7f]
               vpdpwuud  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwuud  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xbf,0xd2,0x52,0x80]
               vpdpwuud  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwuud  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x60,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwuud  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      vpdpwuud  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwuud  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK:      vpdpwuud  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x18,0xd2,0x10]
               vpdpwuud  (%eax){1to4}, %xmm3, %xmm2

// CHECK:      vpdpwuud  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x60,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwuud  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      vpdpwuud  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0xd2,0x51,0x7f]
               vpdpwuud  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwuud  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x9f,0xd2,0x52,0x80]
               vpdpwuud  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwuuds %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x64,0xd3,0xd4]
               vpdpwuuds %ymm4, %ymm3, %ymm2

// CHECK:      vpdpwuuds %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0xd3,0xd4]
               vpdpwuuds %ymm4, %ymm3, %ymm2 {%k7}

// CHECK:      vpdpwuuds %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0xd3,0xd4]
               vpdpwuuds %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwuuds %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x60,0xd3,0xd4]
               vpdpwuuds %xmm4, %xmm3, %xmm2

// CHECK:      vpdpwuuds %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0xd3,0xd4]
               vpdpwuuds %xmm4, %xmm3, %xmm2 {%k7}

// CHECK:      vpdpwuuds %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0xd3,0xd4]
               vpdpwuuds %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwuuds  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x64,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwuuds  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK:      vpdpwuuds  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwuuds  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK:      vpdpwuuds  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x38,0xd3,0x10]
               vpdpwuuds  (%eax){1to8}, %ymm3, %ymm2

// CHECK:      vpdpwuuds  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x64,0xd3,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwuuds  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK:      vpdpwuuds  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0xd3,0x51,0x7f]
               vpdpwuuds  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwuuds  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xbf,0xd3,0x52,0x80]
               vpdpwuuds  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK:      vpdpwuuds  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x60,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwuuds  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      vpdpwuuds  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwuuds  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK:      vpdpwuuds  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x18,0xd3,0x10]
               vpdpwuuds  (%eax){1to4}, %xmm3, %xmm2

// CHECK:      vpdpwuuds  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x60,0xd3,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwuuds  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      vpdpwuuds  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0xd3,0x51,0x7f]
               vpdpwuuds  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK:      vpdpwuuds  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x9f,0xd3,0x52,0x80]
               vpdpwuuds  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

