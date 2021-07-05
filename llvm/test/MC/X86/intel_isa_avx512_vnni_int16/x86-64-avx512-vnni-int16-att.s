// REQUIRES: intel_feature_isa_avx512_vnni_int16
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      vpdpwsud %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x46,0x40,0xd2,0xf0]
               vpdpwsud %zmm24, %zmm23, %zmm22

// CHECK:      vpdpwsud %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x46,0x47,0xd2,0xf0]
               vpdpwsud %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vpdpwsud %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x46,0xc7,0xd2,0xf0]
               vpdpwsud %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwsud  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x46,0x40,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsud  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vpdpwsud  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x46,0x47,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwsud  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vpdpwsud  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x50,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwsud  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vpdpwsud  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x40,0xd2,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwsud  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vpdpwsud  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xc7,0xd2,0x71,0x7f]
               vpdpwsud  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwsud  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xd7,0xd2,0x72,0x80]
               vpdpwsud  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwsuds %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x46,0x40,0xd3,0xf0]
               vpdpwsuds %zmm24, %zmm23, %zmm22

// CHECK:      vpdpwsuds %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x46,0x47,0xd3,0xf0]
               vpdpwsuds %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vpdpwsuds %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x46,0xc7,0xd3,0xf0]
               vpdpwsuds %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwsuds  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x46,0x40,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsuds  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vpdpwsuds  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x46,0x47,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwsuds  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vpdpwsuds  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x50,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwsuds  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vpdpwsuds  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x40,0xd3,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwsuds  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vpdpwsuds  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xc7,0xd3,0x71,0x7f]
               vpdpwsuds  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwsuds  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xd7,0xd3,0x72,0x80]
               vpdpwsuds  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwusd %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x45,0x40,0xd2,0xf0]
               vpdpwusd %zmm24, %zmm23, %zmm22

// CHECK:      vpdpwusd %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x45,0x47,0xd2,0xf0]
               vpdpwusd %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vpdpwusd %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x45,0xc7,0xd2,0xf0]
               vpdpwusd %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwusd  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x45,0x40,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusd  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vpdpwusd  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x45,0x47,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwusd  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vpdpwusd  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x50,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwusd  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vpdpwusd  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x40,0xd2,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwusd  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vpdpwusd  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0xc7,0xd2,0x71,0x7f]
               vpdpwusd  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwusd  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0xd7,0xd2,0x72,0x80]
               vpdpwusd  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwusds %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x45,0x40,0xd3,0xf0]
               vpdpwusds %zmm24, %zmm23, %zmm22

// CHECK:      vpdpwusds %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x45,0x47,0xd3,0xf0]
               vpdpwusds %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vpdpwusds %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x45,0xc7,0xd3,0xf0]
               vpdpwusds %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwusds  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x45,0x40,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusds  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vpdpwusds  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x45,0x47,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwusds  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vpdpwusds  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x50,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwusds  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vpdpwusds  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x40,0xd3,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwusds  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vpdpwusds  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0xc7,0xd3,0x71,0x7f]
               vpdpwusds  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwusds  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0xd7,0xd3,0x72,0x80]
               vpdpwusds  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwuud %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x44,0x40,0xd2,0xf0]
               vpdpwuud %zmm24, %zmm23, %zmm22

// CHECK:      vpdpwuud %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x47,0xd2,0xf0]
               vpdpwuud %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vpdpwuud %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0xc7,0xd2,0xf0]
               vpdpwuud %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwuud  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x44,0x40,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuud  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vpdpwuud  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x47,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwuud  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vpdpwuud  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x50,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwuud  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vpdpwuud  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x40,0xd2,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwuud  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vpdpwuud  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xc7,0xd2,0x71,0x7f]
               vpdpwuud  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwuud  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xd7,0xd2,0x72,0x80]
               vpdpwuud  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwuuds %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x44,0x40,0xd3,0xf0]
               vpdpwuuds %zmm24, %zmm23, %zmm22

// CHECK:      vpdpwuuds %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x47,0xd3,0xf0]
               vpdpwuuds %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vpdpwuuds %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0xc7,0xd3,0xf0]
               vpdpwuuds %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwuuds  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x44,0x40,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuuds  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vpdpwuuds  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x47,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwuuds  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vpdpwuuds  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x50,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwuuds  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vpdpwuuds  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x40,0xd3,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwuuds  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vpdpwuuds  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xc7,0xd3,0x71,0x7f]
               vpdpwuuds  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpwuuds  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xd7,0xd3,0x72,0x80]
               vpdpwuuds  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

