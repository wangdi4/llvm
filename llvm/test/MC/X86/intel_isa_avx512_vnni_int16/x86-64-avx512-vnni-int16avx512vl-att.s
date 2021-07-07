// REQUIRES: intel_feature_isa_avx512_vnni_int16
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      vpdpwsud %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x46,0x20,0xd2,0xf0]
               vpdpwsud %ymm24, %ymm23, %ymm22

// CHECK:      vpdpwsud %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x46,0x27,0xd2,0xf0]
               vpdpwsud %ymm24, %ymm23, %ymm22 {%k7}

// CHECK:      vpdpwsud %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x46,0xa7,0xd2,0xf0]
               vpdpwsud %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwsud %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x46,0x00,0xd2,0xf0]
               vpdpwsud %xmm24, %xmm23, %xmm22

// CHECK:      vpdpwsud %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x46,0x07,0xd2,0xf0]
               vpdpwsud %xmm24, %xmm23, %xmm22 {%k7}

// CHECK:      vpdpwsud %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x46,0x87,0xd2,0xf0]
               vpdpwsud %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwsud  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x46,0x20,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsud  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK:      vpdpwsud  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x46,0x27,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwsud  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK:      vpdpwsud  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x46,0x30,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwsud  (%rip){1to8}, %ymm23, %ymm22

// CHECK:      vpdpwsud  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x46,0x20,0xd2,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwsud  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK:      vpdpwsud  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xa7,0xd2,0x71,0x7f]
               vpdpwsud  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwsud  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xb7,0xd2,0x72,0x80]
               vpdpwsud  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwsud  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x46,0x00,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsud  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK:      vpdpwsud  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x46,0x07,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwsud  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK:      vpdpwsud  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x10,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwsud  (%rip){1to4}, %xmm23, %xmm22

// CHECK:      vpdpwsud  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x00,0xd2,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwsud  -512(,%rbp,2), %xmm23, %xmm22

// CHECK:      vpdpwsud  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0x87,0xd2,0x71,0x7f]
               vpdpwsud  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwsud  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0x97,0xd2,0x72,0x80]
               vpdpwsud  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwsuds %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x46,0x20,0xd3,0xf0]
               vpdpwsuds %ymm24, %ymm23, %ymm22

// CHECK:      vpdpwsuds %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x46,0x27,0xd3,0xf0]
               vpdpwsuds %ymm24, %ymm23, %ymm22 {%k7}

// CHECK:      vpdpwsuds %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x46,0xa7,0xd3,0xf0]
               vpdpwsuds %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwsuds %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x46,0x00,0xd3,0xf0]
               vpdpwsuds %xmm24, %xmm23, %xmm22

// CHECK:      vpdpwsuds %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x46,0x07,0xd3,0xf0]
               vpdpwsuds %xmm24, %xmm23, %xmm22 {%k7}

// CHECK:      vpdpwsuds %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x46,0x87,0xd3,0xf0]
               vpdpwsuds %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwsuds  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x46,0x20,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsuds  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK:      vpdpwsuds  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x46,0x27,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwsuds  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK:      vpdpwsuds  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x46,0x30,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwsuds  (%rip){1to8}, %ymm23, %ymm22

// CHECK:      vpdpwsuds  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x46,0x20,0xd3,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwsuds  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK:      vpdpwsuds  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xa7,0xd3,0x71,0x7f]
               vpdpwsuds  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwsuds  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xb7,0xd3,0x72,0x80]
               vpdpwsuds  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwsuds  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x46,0x00,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsuds  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK:      vpdpwsuds  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x46,0x07,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwsuds  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK:      vpdpwsuds  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x10,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwsuds  (%rip){1to4}, %xmm23, %xmm22

// CHECK:      vpdpwsuds  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x00,0xd3,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwsuds  -512(,%rbp,2), %xmm23, %xmm22

// CHECK:      vpdpwsuds  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0x87,0xd3,0x71,0x7f]
               vpdpwsuds  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwsuds  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0x97,0xd3,0x72,0x80]
               vpdpwsuds  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwusd %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x45,0x20,0xd2,0xf0]
               vpdpwusd %ymm24, %ymm23, %ymm22

// CHECK:      vpdpwusd %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x45,0x27,0xd2,0xf0]
               vpdpwusd %ymm24, %ymm23, %ymm22 {%k7}

// CHECK:      vpdpwusd %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x45,0xa7,0xd2,0xf0]
               vpdpwusd %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwusd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x45,0x00,0xd2,0xf0]
               vpdpwusd %xmm24, %xmm23, %xmm22

// CHECK:      vpdpwusd %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x45,0x07,0xd2,0xf0]
               vpdpwusd %xmm24, %xmm23, %xmm22 {%k7}

// CHECK:      vpdpwusd %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x45,0x87,0xd2,0xf0]
               vpdpwusd %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwusd  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x45,0x20,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusd  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK:      vpdpwusd  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x45,0x27,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwusd  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK:      vpdpwusd  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x45,0x30,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwusd  (%rip){1to8}, %ymm23, %ymm22

// CHECK:      vpdpwusd  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x45,0x20,0xd2,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwusd  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK:      vpdpwusd  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0xa7,0xd2,0x71,0x7f]
               vpdpwusd  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwusd  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0xb7,0xd2,0x72,0x80]
               vpdpwusd  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwusd  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x45,0x00,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusd  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK:      vpdpwusd  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x45,0x07,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwusd  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK:      vpdpwusd  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x10,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwusd  (%rip){1to4}, %xmm23, %xmm22

// CHECK:      vpdpwusd  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x00,0xd2,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwusd  -512(,%rbp,2), %xmm23, %xmm22

// CHECK:      vpdpwusd  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0x87,0xd2,0x71,0x7f]
               vpdpwusd  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwusd  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0x97,0xd2,0x72,0x80]
               vpdpwusd  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwusds %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x45,0x20,0xd3,0xf0]
               vpdpwusds %ymm24, %ymm23, %ymm22

// CHECK:      vpdpwusds %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x45,0x27,0xd3,0xf0]
               vpdpwusds %ymm24, %ymm23, %ymm22 {%k7}

// CHECK:      vpdpwusds %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x45,0xa7,0xd3,0xf0]
               vpdpwusds %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwusds %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x45,0x00,0xd3,0xf0]
               vpdpwusds %xmm24, %xmm23, %xmm22

// CHECK:      vpdpwusds %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x45,0x07,0xd3,0xf0]
               vpdpwusds %xmm24, %xmm23, %xmm22 {%k7}

// CHECK:      vpdpwusds %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x45,0x87,0xd3,0xf0]
               vpdpwusds %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwusds  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x45,0x20,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusds  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK:      vpdpwusds  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x45,0x27,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwusds  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK:      vpdpwusds  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x45,0x30,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwusds  (%rip){1to8}, %ymm23, %ymm22

// CHECK:      vpdpwusds  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x45,0x20,0xd3,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwusds  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK:      vpdpwusds  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0xa7,0xd3,0x71,0x7f]
               vpdpwusds  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwusds  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0xb7,0xd3,0x72,0x80]
               vpdpwusds  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwusds  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x45,0x00,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusds  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK:      vpdpwusds  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x45,0x07,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwusds  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK:      vpdpwusds  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x10,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwusds  (%rip){1to4}, %xmm23, %xmm22

// CHECK:      vpdpwusds  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x00,0xd3,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwusds  -512(,%rbp,2), %xmm23, %xmm22

// CHECK:      vpdpwusds  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0x87,0xd3,0x71,0x7f]
               vpdpwusds  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwusds  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0x97,0xd3,0x72,0x80]
               vpdpwusds  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwuud %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x44,0x20,0xd2,0xf0]
               vpdpwuud %ymm24, %ymm23, %ymm22

// CHECK:      vpdpwuud %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x27,0xd2,0xf0]
               vpdpwuud %ymm24, %ymm23, %ymm22 {%k7}

// CHECK:      vpdpwuud %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0xa7,0xd2,0xf0]
               vpdpwuud %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwuud %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x44,0x00,0xd2,0xf0]
               vpdpwuud %xmm24, %xmm23, %xmm22

// CHECK:      vpdpwuud %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x07,0xd2,0xf0]
               vpdpwuud %xmm24, %xmm23, %xmm22 {%k7}

// CHECK:      vpdpwuud %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0x87,0xd2,0xf0]
               vpdpwuud %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwuud  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x44,0x20,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuud  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK:      vpdpwuud  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x27,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwuud  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK:      vpdpwuud  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x44,0x30,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwuud  (%rip){1to8}, %ymm23, %ymm22

// CHECK:      vpdpwuud  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x44,0x20,0xd2,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwuud  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK:      vpdpwuud  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xa7,0xd2,0x71,0x7f]
               vpdpwuud  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwuud  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xb7,0xd2,0x72,0x80]
               vpdpwuud  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwuud  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x44,0x00,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuud  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK:      vpdpwuud  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x07,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwuud  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK:      vpdpwuud  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x10,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwuud  (%rip){1to4}, %xmm23, %xmm22

// CHECK:      vpdpwuud  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x00,0xd2,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwuud  -512(,%rbp,2), %xmm23, %xmm22

// CHECK:      vpdpwuud  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0x87,0xd2,0x71,0x7f]
               vpdpwuud  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwuud  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0x97,0xd2,0x72,0x80]
               vpdpwuud  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwuuds %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x44,0x20,0xd3,0xf0]
               vpdpwuuds %ymm24, %ymm23, %ymm22

// CHECK:      vpdpwuuds %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x27,0xd3,0xf0]
               vpdpwuuds %ymm24, %ymm23, %ymm22 {%k7}

// CHECK:      vpdpwuuds %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0xa7,0xd3,0xf0]
               vpdpwuuds %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwuuds %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x44,0x00,0xd3,0xf0]
               vpdpwuuds %xmm24, %xmm23, %xmm22

// CHECK:      vpdpwuuds %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x07,0xd3,0xf0]
               vpdpwuuds %xmm24, %xmm23, %xmm22 {%k7}

// CHECK:      vpdpwuuds %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0x87,0xd3,0xf0]
               vpdpwuuds %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwuuds  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x44,0x20,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuuds  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK:      vpdpwuuds  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x27,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwuuds  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK:      vpdpwuuds  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x44,0x30,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwuuds  (%rip){1to8}, %ymm23, %ymm22

// CHECK:      vpdpwuuds  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x44,0x20,0xd3,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwuuds  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK:      vpdpwuuds  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xa7,0xd3,0x71,0x7f]
               vpdpwuuds  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwuuds  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xb7,0xd3,0x72,0x80]
               vpdpwuuds  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK:      vpdpwuuds  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x44,0x00,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuuds  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK:      vpdpwuuds  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x07,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwuuds  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK:      vpdpwuuds  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x10,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwuuds  (%rip){1to4}, %xmm23, %xmm22

// CHECK:      vpdpwuuds  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x00,0xd3,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwuuds  -512(,%rbp,2), %xmm23, %xmm22

// CHECK:      vpdpwuuds  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0x87,0xd3,0x71,0x7f]
               vpdpwuuds  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK:      vpdpwuuds  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0x97,0xd3,0x72,0x80]
               vpdpwuuds  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

