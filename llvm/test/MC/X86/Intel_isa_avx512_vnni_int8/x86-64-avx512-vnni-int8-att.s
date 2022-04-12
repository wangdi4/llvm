// REQUIRES: intel_feature_isa_avx512_vnni_int8
// RUN: llvm-mc -triple=x86_64-unknown-unknown -mattr=+avx512vnniint8 --show-encoding < %s  | FileCheck %s

// CHECK:      vpdpbssd %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x47,0x40,0x50,0xf0]
               vpdpbssd %zmm24, %zmm23, %zmm22

// CHECK:      vpdpbssd %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x47,0x47,0x50,0xf0]
               vpdpbssd %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vpdpbssd %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x47,0xc7,0x50,0xf0]
               vpdpbssd %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbssd  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x47,0x40,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpbssd  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vpdpbssd  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x47,0x47,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpbssd  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vpdpbssd  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x50,0x50,0x35,0x00,0x00,0x00,0x00]
               vpdpbssd  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vpdpbssd  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x40,0x50,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbssd  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vpdpbssd  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x47,0xc7,0x50,0x71,0x7f]
               vpdpbssd  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbssd  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x47,0xd7,0x50,0x72,0x80]
               vpdpbssd  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbssds %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x47,0x40,0x51,0xf0]
               vpdpbssds %zmm24, %zmm23, %zmm22

// CHECK:      vpdpbssds %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x47,0x47,0x51,0xf0]
               vpdpbssds %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vpdpbssds %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x47,0xc7,0x51,0xf0]
               vpdpbssds %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbssds  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x47,0x40,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpbssds  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vpdpbssds  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x47,0x47,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpbssds  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vpdpbssds  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x50,0x51,0x35,0x00,0x00,0x00,0x00]
               vpdpbssds  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vpdpbssds  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x40,0x51,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbssds  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vpdpbssds  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x47,0xc7,0x51,0x71,0x7f]
               vpdpbssds  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbssds  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x47,0xd7,0x51,0x72,0x80]
               vpdpbssds  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbsud %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x46,0x40,0x50,0xf0]
               vpdpbsud %zmm24, %zmm23, %zmm22

// CHECK:      vpdpbsud %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x46,0x47,0x50,0xf0]
               vpdpbsud %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vpdpbsud %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x46,0xc7,0x50,0xf0]
               vpdpbsud %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbsud  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x46,0x40,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpbsud  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vpdpbsud  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x46,0x47,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpbsud  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vpdpbsud  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x50,0x50,0x35,0x00,0x00,0x00,0x00]
               vpdpbsud  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vpdpbsud  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x40,0x50,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbsud  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vpdpbsud  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xc7,0x50,0x71,0x7f]
               vpdpbsud  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbsud  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xd7,0x50,0x72,0x80]
               vpdpbsud  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbsuds %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x46,0x40,0x51,0xf0]
               vpdpbsuds %zmm24, %zmm23, %zmm22

// CHECK:      vpdpbsuds %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x46,0x47,0x51,0xf0]
               vpdpbsuds %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vpdpbsuds %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x46,0xc7,0x51,0xf0]
               vpdpbsuds %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbsuds  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x46,0x40,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpbsuds  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vpdpbsuds  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x46,0x47,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpbsuds  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vpdpbsuds  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x50,0x51,0x35,0x00,0x00,0x00,0x00]
               vpdpbsuds  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vpdpbsuds  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x46,0x40,0x51,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbsuds  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vpdpbsuds  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xc7,0x51,0x71,0x7f]
               vpdpbsuds  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbsuds  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x46,0xd7,0x51,0x72,0x80]
               vpdpbsuds  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbuud %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x44,0x40,0x50,0xf0]
               vpdpbuud %zmm24, %zmm23, %zmm22

// CHECK:      vpdpbuud %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x47,0x50,0xf0]
               vpdpbuud %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vpdpbuud %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0xc7,0x50,0xf0]
               vpdpbuud %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbuud  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x44,0x40,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpbuud  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vpdpbuud  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x47,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpbuud  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vpdpbuud  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x50,0x50,0x35,0x00,0x00,0x00,0x00]
               vpdpbuud  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vpdpbuud  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x40,0x50,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbuud  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vpdpbuud  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xc7,0x50,0x71,0x7f]
               vpdpbuud  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbuud  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xd7,0x50,0x72,0x80]
               vpdpbuud  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbuuds %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x44,0x40,0x51,0xf0]
               vpdpbuuds %zmm24, %zmm23, %zmm22

// CHECK:      vpdpbuuds %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x44,0x47,0x51,0xf0]
               vpdpbuuds %zmm24, %zmm23, %zmm22 {%k7}

// CHECK:      vpdpbuuds %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x44,0xc7,0x51,0xf0]
               vpdpbuuds %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbuuds  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x44,0x40,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpbuuds  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      vpdpbuuds  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x44,0x47,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpbuuds  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK:      vpdpbuuds  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x50,0x51,0x35,0x00,0x00,0x00,0x00]
               vpdpbuuds  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      vpdpbuuds  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x44,0x40,0x51,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbuuds  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      vpdpbuuds  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xc7,0x51,0x71,0x7f]
               vpdpbuuds  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK:      vpdpbuuds  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x44,0xd7,0x51,0x72,0x80]
               vpdpbuuds  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

