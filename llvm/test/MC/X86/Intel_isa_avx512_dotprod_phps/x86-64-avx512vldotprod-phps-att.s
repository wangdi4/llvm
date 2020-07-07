// REQUIRES: intel_feature_isa_avx512_dotprod_phps
// RUN: llvm-mc -triple=x86_64-unknown-unknown -mattr=+avx512vl,+avx512dotprodphps --show-encoding < %s  | FileCheck %s

// CHECK: vdpphps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x44,0x20,0x52,0xf0]
     {evex} vdpphps %ymm24, %ymm23, %ymm22

// CHECK: vdpphps %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x27,0x52,0xf0]
     {evex} vdpphps %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vdpphps %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0xa7,0x52,0xf0]
     {evex} vdpphps %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vdpphps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x44,0x00,0x52,0xf0]
     {evex} vdpphps %xmm24, %xmm23, %xmm22

// CHECK: vdpphps %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x07,0x52,0xf0]
     {evex} vdpphps %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vdpphps %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0x87,0x52,0xf0]
     {evex} vdpphps %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vdpphps  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x44,0x20,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vdpphps  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vdpphps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x27,0x52,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vdpphps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vdpphps  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x44,0x30,0x52,0x35,0x00,0x00,0x00,0x00]
     {evex} vdpphps  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vdpphps  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x44,0x20,0x52,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vdpphps  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vdpphps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xa7,0x52,0x71,0x7f]
     {evex} vdpphps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vdpphps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xb7,0x52,0x72,0x80]
     {evex} vdpphps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vdpphps  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x44,0x00,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vdpphps  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vdpphps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x07,0x52,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vdpphps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vdpphps  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x10,0x52,0x35,0x00,0x00,0x00,0x00]
     {evex} vdpphps  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vdpphps  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x00,0x52,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vdpphps  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vdpphps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0x87,0x52,0x71,0x7f]
     {evex} vdpphps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vdpphps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0x97,0x52,0x72,0x80]
     {evex} vdpphps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

