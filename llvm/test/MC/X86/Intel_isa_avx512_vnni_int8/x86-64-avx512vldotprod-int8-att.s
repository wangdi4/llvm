// REQUIRES: intel_feature_isa_avx512_vnni_int8
// RUN: llvm-mc -triple=x86_64-unknown-unknown -mattr=+avx512vl,+avx512vnniint8 --show-encoding < %s  | FileCheck %s

// CHECK: vpdpbssd %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x47,0x20,0x50,0xf0]
     {evex} vpdpbssd %ymm24, %ymm23, %ymm22

// CHECK: vpdpbssd %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x47,0x27,0x50,0xf0]
     {evex} vpdpbssd %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vpdpbssd %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x47,0xa7,0x50,0xf0]
     {evex} vpdpbssd %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbssd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x47,0x00,0x50,0xf0]
     {evex} vpdpbssd %xmm24, %xmm23, %xmm22

// CHECK: vpdpbssd %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x47,0x07,0x50,0xf0]
     {evex} vpdpbssd %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vpdpbssd %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x47,0x87,0x50,0xf0]
     {evex} vpdpbssd %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbssd  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x47,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbssd  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vpdpbssd  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x47,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbssd  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vpdpbssd  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x47,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbssd  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vpdpbssd  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x47,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbssd  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vpdpbssd  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x47,0xa7,0x50,0x71,0x7f]
     {evex} vpdpbssd  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbssd  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x47,0xb7,0x50,0x72,0x80]
     {evex} vpdpbssd  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbssd  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x47,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbssd  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vpdpbssd  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x47,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbssd  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vpdpbssd  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbssd  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vpdpbssd  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbssd  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vpdpbssd  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x47,0x87,0x50,0x71,0x7f]
     {evex} vpdpbssd  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbssd  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x47,0x97,0x50,0x72,0x80]
     {evex} vpdpbssd  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbssds %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x47,0x20,0x51,0xf0]
     {evex} vpdpbssds %ymm24, %ymm23, %ymm22

// CHECK: vpdpbssds %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x47,0x27,0x51,0xf0]
     {evex} vpdpbssds %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vpdpbssds %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x47,0xa7,0x51,0xf0]
     {evex} vpdpbssds %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbssds %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x47,0x00,0x51,0xf0]
     {evex} vpdpbssds %xmm24, %xmm23, %xmm22

// CHECK: vpdpbssds %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x47,0x07,0x51,0xf0]
     {evex} vpdpbssds %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vpdpbssds %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x47,0x87,0x51,0xf0]
     {evex} vpdpbssds %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbssds  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x47,0x20,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbssds  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vpdpbssds  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x47,0x27,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbssds  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vpdpbssds  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x47,0x30,0x51,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbssds  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vpdpbssds  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x47,0x20,0x51,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbssds  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vpdpbssds  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x47,0xa7,0x51,0x71,0x7f]
     {evex} vpdpbssds  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbssds  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x47,0xb7,0x51,0x72,0x80]
     {evex} vpdpbssds  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbssds  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x47,0x00,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbssds  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vpdpbssds  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x47,0x07,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbssds  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vpdpbssds  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x10,0x51,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbssds  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vpdpbssds  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x00,0x51,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbssds  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vpdpbssds  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x47,0x87,0x51,0x71,0x7f]
     {evex} vpdpbssds  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbssds  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x47,0x97,0x51,0x72,0x80]
     {evex} vpdpbssds  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbsud %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x46,0x20,0x50,0xf0]
     {evex} vpdpbsud %ymm24, %ymm23, %ymm22

// CHECK: vpdpbsud %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x46,0x27,0x50,0xf0]
     {evex} vpdpbsud %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vpdpbsud %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x46,0xa7,0x50,0xf0]
     {evex} vpdpbsud %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbsud %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x46,0x00,0x50,0xf0]
     {evex} vpdpbsud %xmm24, %xmm23, %xmm22

// CHECK: vpdpbsud %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x46,0x07,0x50,0xf0]
     {evex} vpdpbsud %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vpdpbsud %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x46,0x87,0x50,0xf0]
     {evex} vpdpbsud %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbsud  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x46,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbsud  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vpdpbsud  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x46,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbsud  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vpdpbsud  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x46,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbsud  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vpdpbsud  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x46,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbsud  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vpdpbsud  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xa7,0x50,0x71,0x7f]
     {evex} vpdpbsud  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbsud  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xb7,0x50,0x72,0x80]
     {evex} vpdpbsud  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbsud  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x46,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbsud  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vpdpbsud  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x46,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbsud  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vpdpbsud  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbsud  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vpdpbsud  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbsud  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vpdpbsud  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0x87,0x50,0x71,0x7f]
     {evex} vpdpbsud  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbsud  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0x97,0x50,0x72,0x80]
     {evex} vpdpbsud  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbsuds %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x46,0x20,0x51,0xf0]
     {evex} vpdpbsuds %ymm24, %ymm23, %ymm22

// CHECK: vpdpbsuds %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x46,0x27,0x51,0xf0]
     {evex} vpdpbsuds %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vpdpbsuds %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x46,0xa7,0x51,0xf0]
     {evex} vpdpbsuds %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbsuds %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x46,0x00,0x51,0xf0]
     {evex} vpdpbsuds %xmm24, %xmm23, %xmm22

// CHECK: vpdpbsuds %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x46,0x07,0x51,0xf0]
     {evex} vpdpbsuds %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vpdpbsuds %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x46,0x87,0x51,0xf0]
     {evex} vpdpbsuds %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbsuds  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x46,0x20,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbsuds  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vpdpbsuds  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x46,0x27,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbsuds  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vpdpbsuds  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x46,0x30,0x51,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbsuds  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vpdpbsuds  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x46,0x20,0x51,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbsuds  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vpdpbsuds  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xa7,0x51,0x71,0x7f]
     {evex} vpdpbsuds  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbsuds  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xb7,0x51,0x72,0x80]
     {evex} vpdpbsuds  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbsuds  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x46,0x00,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbsuds  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vpdpbsuds  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x46,0x07,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbsuds  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vpdpbsuds  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x10,0x51,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbsuds  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vpdpbsuds  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x00,0x51,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbsuds  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vpdpbsuds  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0x87,0x51,0x71,0x7f]
     {evex} vpdpbsuds  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbsuds  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0x97,0x51,0x72,0x80]
     {evex} vpdpbsuds  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbuud %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x44,0x20,0x50,0xf0]
     {evex} vpdpbuud %ymm24, %ymm23, %ymm22

// CHECK: vpdpbuud %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x27,0x50,0xf0]
     {evex} vpdpbuud %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vpdpbuud %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0xa7,0x50,0xf0]
     {evex} vpdpbuud %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbuud %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x44,0x00,0x50,0xf0]
     {evex} vpdpbuud %xmm24, %xmm23, %xmm22

// CHECK: vpdpbuud %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x07,0x50,0xf0]
     {evex} vpdpbuud %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vpdpbuud %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0x87,0x50,0xf0]
     {evex} vpdpbuud %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbuud  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x44,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbuud  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vpdpbuud  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbuud  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vpdpbuud  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x44,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbuud  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vpdpbuud  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x44,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbuud  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vpdpbuud  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xa7,0x50,0x71,0x7f]
     {evex} vpdpbuud  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbuud  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xb7,0x50,0x72,0x80]
     {evex} vpdpbuud  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbuud  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x44,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbuud  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vpdpbuud  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbuud  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vpdpbuud  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbuud  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vpdpbuud  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbuud  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vpdpbuud  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0x87,0x50,0x71,0x7f]
     {evex} vpdpbuud  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbuud  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0x97,0x50,0x72,0x80]
     {evex} vpdpbuud  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbuuds %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x44,0x20,0x51,0xf0]
     {evex} vpdpbuuds %ymm24, %ymm23, %ymm22

// CHECK: vpdpbuuds %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x27,0x51,0xf0]
     {evex} vpdpbuuds %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vpdpbuuds %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0xa7,0x51,0xf0]
     {evex} vpdpbuuds %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbuuds %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x44,0x00,0x51,0xf0]
     {evex} vpdpbuuds %xmm24, %xmm23, %xmm22

// CHECK: vpdpbuuds %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x07,0x51,0xf0]
     {evex} vpdpbuuds %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vpdpbuuds %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0x87,0x51,0xf0]
     {evex} vpdpbuuds %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbuuds  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x44,0x20,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbuuds  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vpdpbuuds  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x27,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbuuds  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vpdpbuuds  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x44,0x30,0x51,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbuuds  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vpdpbuuds  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x44,0x20,0x51,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbuuds  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vpdpbuuds  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xa7,0x51,0x71,0x7f]
     {evex} vpdpbuuds  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbuuds  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xb7,0x51,0x72,0x80]
     {evex} vpdpbuuds  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vpdpbuuds  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x44,0x00,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbuuds  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vpdpbuuds  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x07,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbuuds  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vpdpbuuds  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x10,0x51,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbuuds  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vpdpbuuds  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x00,0x51,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbuuds  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vpdpbuuds  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0x87,0x51,0x71,0x7f]
     {evex} vpdpbuuds  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vpdpbuuds  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0x97,0x51,0x72,0x80]
     {evex} vpdpbuuds  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

