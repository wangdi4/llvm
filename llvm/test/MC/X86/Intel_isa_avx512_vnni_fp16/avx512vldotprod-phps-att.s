// REQUIRES: intel_feature_isa_avx512_vnni_fp16
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avx512vl,+avx512vnnifp16 --show-encoding %s | FileCheck %s

// CHECK: vdpphps %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x52,0xd4]
     {evex} vdpphps %ymm4, %ymm3, %ymm2

// CHECK: vdpphps %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0x52,0xd4]
     {evex} vdpphps %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vdpphps %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0x52,0xd4]
     {evex} vdpphps %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vdpphps %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x52,0xd4]
     {evex} vdpphps %xmm4, %xmm3, %xmm2

// CHECK: vdpphps %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0x52,0xd4]
     {evex} vdpphps %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vdpphps %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0x52,0xd4]
     {evex} vdpphps %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vdpphps  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x52,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vdpphps  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vdpphps  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x2f,0x52,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vdpphps  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vdpphps  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x38,0x52,0x10]
     {evex} vdpphps  (%eax){1to8}, %ymm3, %ymm2

// CHECK: vdpphps  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x64,0x28,0x52,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vdpphps  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vdpphps  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xaf,0x52,0x51,0x7f]
     {evex} vdpphps  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vdpphps  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xbf,0x52,0x52,0x80]
     {evex} vdpphps  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vdpphps  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x52,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vdpphps  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vdpphps  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x0f,0x52,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vdpphps  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vdpphps  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x18,0x52,0x10]
     {evex} vdpphps  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vdpphps  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x08,0x52,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vdpphps  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vdpphps  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x8f,0x52,0x51,0x7f]
     {evex} vdpphps  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vdpphps  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0x9f,0x52,0x52,0x80]
     {evex} vdpphps  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

