// REQUIRES: intel_feature_isa_avx512_vnni_int8
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avx512vl,+avx512vnniint8 --show-encoding %s | FileCheck %s

// CHECK: vpdpbssd %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x50,0xd4]
     {evex} vpdpbssd %ymm4, %ymm3, %ymm2

// CHECK: vpdpbssd %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x67,0x2f,0x50,0xd4]
     {evex} vpdpbssd %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vpdpbssd %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0xaf,0x50,0xd4]
     {evex} vpdpbssd %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbssd %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x50,0xd4]
     {evex} vpdpbssd %xmm4, %xmm3, %xmm2

// CHECK: vpdpbssd %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x67,0x0f,0x50,0xd4]
     {evex} vpdpbssd %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vpdpbssd %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0x8f,0x50,0xd4]
     {evex} vpdpbssd %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbssd  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbssd  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vpdpbssd  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x67,0x2f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbssd  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vpdpbssd  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x38,0x50,0x10]
     {evex} vpdpbssd  (%eax){1to8}, %ymm3, %ymm2

// CHECK: vpdpbssd  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x50,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbssd  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vpdpbssd  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0xaf,0x50,0x51,0x7f]
     {evex} vpdpbssd  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbssd  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0xbf,0x50,0x52,0x80]
     {evex} vpdpbssd  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbssd  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbssd  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vpdpbssd  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x67,0x0f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbssd  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vpdpbssd  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x18,0x50,0x10]
     {evex} vpdpbssd  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vpdpbssd  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x50,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbssd  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vpdpbssd  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0x8f,0x50,0x51,0x7f]
     {evex} vpdpbssd  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbssd  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0x9f,0x50,0x52,0x80]
     {evex} vpdpbssd  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbssds %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x51,0xd4]
     {evex} vpdpbssds %ymm4, %ymm3, %ymm2

// CHECK: vpdpbssds %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x67,0x2f,0x51,0xd4]
     {evex} vpdpbssds %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vpdpbssds %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0xaf,0x51,0xd4]
     {evex} vpdpbssds %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbssds %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x51,0xd4]
     {evex} vpdpbssds %xmm4, %xmm3, %xmm2

// CHECK: vpdpbssds %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x67,0x0f,0x51,0xd4]
     {evex} vpdpbssds %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vpdpbssds %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0x8f,0x51,0xd4]
     {evex} vpdpbssds %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbssds  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbssds  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vpdpbssds  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x67,0x2f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbssds  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vpdpbssds  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x38,0x51,0x10]
     {evex} vpdpbssds  (%eax){1to8}, %ymm3, %ymm2

// CHECK: vpdpbssds  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x51,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbssds  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vpdpbssds  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0xaf,0x51,0x51,0x7f]
     {evex} vpdpbssds  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbssds  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0xbf,0x51,0x52,0x80]
     {evex} vpdpbssds  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbssds  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbssds  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vpdpbssds  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x67,0x0f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbssds  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vpdpbssds  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x18,0x51,0x10]
     {evex} vpdpbssds  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vpdpbssds  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x51,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbssds  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vpdpbssds  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0x8f,0x51,0x51,0x7f]
     {evex} vpdpbssds  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbssds  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x67,0x9f,0x51,0x52,0x80]
     {evex} vpdpbssds  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbsud %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x66,0x28,0x50,0xd4]
     {evex} vpdpbsud %ymm4, %ymm3, %ymm2

// CHECK: vpdpbsud %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0x50,0xd4]
     {evex} vpdpbsud %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vpdpbsud %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0x50,0xd4]
     {evex} vpdpbsud %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbsud %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x08,0x50,0xd4]
     {evex} vpdpbsud %xmm4, %xmm3, %xmm2

// CHECK: vpdpbsud %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0x50,0xd4]
     {evex} vpdpbsud %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vpdpbsud %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0x50,0xd4]
     {evex} vpdpbsud %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbsud  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x66,0x28,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbsud  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vpdpbsud  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbsud  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vpdpbsud  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x66,0x38,0x50,0x10]
     {evex} vpdpbsud  (%eax){1to8}, %ymm3, %ymm2

// CHECK: vpdpbsud  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x66,0x28,0x50,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbsud  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vpdpbsud  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0x50,0x51,0x7f]
     {evex} vpdpbsud  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbsud  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xbf,0x50,0x52,0x80]
     {evex} vpdpbsud  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbsud  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x08,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbsud  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vpdpbsud  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbsud  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vpdpbsud  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x18,0x50,0x10]
     {evex} vpdpbsud  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vpdpbsud  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x08,0x50,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbsud  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vpdpbsud  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0x50,0x51,0x7f]
     {evex} vpdpbsud  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbsud  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0x9f,0x50,0x52,0x80]
     {evex} vpdpbsud  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbsuds %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x66,0x28,0x51,0xd4]
     {evex} vpdpbsuds %ymm4, %ymm3, %ymm2

// CHECK: vpdpbsuds %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0x51,0xd4]
     {evex} vpdpbsuds %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vpdpbsuds %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0x51,0xd4]
     {evex} vpdpbsuds %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbsuds %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x08,0x51,0xd4]
     {evex} vpdpbsuds %xmm4, %xmm3, %xmm2

// CHECK: vpdpbsuds %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0x51,0xd4]
     {evex} vpdpbsuds %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vpdpbsuds %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0x51,0xd4]
     {evex} vpdpbsuds %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbsuds  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x66,0x28,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbsuds  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vpdpbsuds  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x2f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbsuds  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vpdpbsuds  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x66,0x38,0x51,0x10]
     {evex} vpdpbsuds  (%eax){1to8}, %ymm3, %ymm2

// CHECK: vpdpbsuds  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x66,0x28,0x51,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbsuds  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vpdpbsuds  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xaf,0x51,0x51,0x7f]
     {evex} vpdpbsuds  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbsuds  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xbf,0x51,0x52,0x80]
     {evex} vpdpbsuds  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbsuds  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x08,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbsuds  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vpdpbsuds  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x0f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbsuds  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vpdpbsuds  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x18,0x51,0x10]
     {evex} vpdpbsuds  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vpdpbsuds  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x08,0x51,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbsuds  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vpdpbsuds  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0x8f,0x51,0x51,0x7f]
     {evex} vpdpbsuds  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbsuds  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0x9f,0x51,0x52,0x80]
     {evex} vpdpbsuds  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbuud %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x50,0xd4]
     {evex} vpdpbuud %ymm4, %ymm3, %ymm2

// CHECK: vpdpbuud %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0x50,0xd4]
     {evex} vpdpbuud %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vpdpbuud %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0x50,0xd4]
     {evex} vpdpbuud %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbuud %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x50,0xd4]
     {evex} vpdpbuud %xmm4, %xmm3, %xmm2

// CHECK: vpdpbuud %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0x50,0xd4]
     {evex} vpdpbuud %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vpdpbuud %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0x50,0xd4]
     {evex} vpdpbuud %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbuud  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbuud  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vpdpbuud  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbuud  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vpdpbuud  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x38,0x50,0x10]
     {evex} vpdpbuud  (%eax){1to8}, %ymm3, %ymm2

// CHECK: vpdpbuud  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x50,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbuud  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vpdpbuud  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0x50,0x51,0x7f]
     {evex} vpdpbuud  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbuud  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xbf,0x50,0x52,0x80]
     {evex} vpdpbuud  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbuud  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x50,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbuud  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vpdpbuud  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0x50,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbuud  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vpdpbuud  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x18,0x50,0x10]
     {evex} vpdpbuud  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vpdpbuud  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x50,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbuud  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vpdpbuud  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0x50,0x51,0x7f]
     {evex} vpdpbuud  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbuud  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x9f,0x50,0x52,0x80]
     {evex} vpdpbuud  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbuuds %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x51,0xd4]
     {evex} vpdpbuuds %ymm4, %ymm3, %ymm2

// CHECK: vpdpbuuds %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0x51,0xd4]
     {evex} vpdpbuuds %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vpdpbuuds %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0x51,0xd4]
     {evex} vpdpbuuds %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbuuds %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x51,0xd4]
     {evex} vpdpbuuds %xmm4, %xmm3, %xmm2

// CHECK: vpdpbuuds %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0x51,0xd4]
     {evex} vpdpbuuds %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vpdpbuuds %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0x51,0xd4]
     {evex} vpdpbuuds %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbuuds  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbuuds  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vpdpbuuds  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbuuds  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vpdpbuuds  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x38,0x51,0x10]
     {evex} vpdpbuuds  (%eax){1to8}, %ymm3, %ymm2

// CHECK: vpdpbuuds  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x51,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbuuds  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vpdpbuuds  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0x51,0x51,0x7f]
     {evex} vpdpbuuds  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbuuds  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xbf,0x51,0x52,0x80]
     {evex} vpdpbuuds  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vpdpbuuds  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x51,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vpdpbuuds  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vpdpbuuds  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0x51,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vpdpbuuds  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vpdpbuuds  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x18,0x51,0x10]
     {evex} vpdpbuuds  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vpdpbuuds  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x51,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbuuds  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vpdpbuuds  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0x51,0x51,0x7f]
     {evex} vpdpbuuds  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vpdpbuuds  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x9f,0x51,0x52,0x80]
     {evex} vpdpbuuds  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

