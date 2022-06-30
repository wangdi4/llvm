// REQUIRES: intel_feature_isa_avx512_minmax
// RUN: llvm-mc -triple x86_64 --show-encoding %s | FileCheck %s

// CHECK: vminmaxnepbf16 $123, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x83,0x47,0x40,0x52,0xf0,0x7b]
          vminmaxnepbf16 $123, %zmm24, %zmm23, %zmm22

// CHECK: vminmaxnepbf16 $123, %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x83,0x47,0x47,0x52,0xf0,0x7b]
          vminmaxnepbf16 $123, %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vminmaxnepbf16 $123, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x83,0x47,0xc7,0x52,0xf0,0x7b]
          vminmaxnepbf16 $123, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vminmaxnepbf16  $123, 268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0x47,0x40,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vminmaxnepbf16  $123, 268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vminmaxnepbf16  $123, 291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x47,0x47,0x52,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vminmaxnepbf16  $123, 291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vminmaxnepbf16  $123, (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe3,0x47,0x50,0x52,0x35,0x00,0x00,0x00,0x00,0x7b]
          vminmaxnepbf16  $123, (%rip){1to32}, %zmm23, %zmm22

// CHECK: vminmaxnepbf16  $123, -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe3,0x47,0x40,0x52,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxnepbf16  $123, -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vminmaxnepbf16  $123, 8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x47,0xc7,0x52,0x71,0x7f,0x7b]
          vminmaxnepbf16  $123, 8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vminmaxnepbf16  $123, -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x47,0xd7,0x52,0x72,0x80,0x7b]
          vminmaxnepbf16  $123, -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vminmaxpd $123, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x83,0xc5,0x40,0x52,0xf0,0x7b]
          vminmaxpd $123, %zmm24, %zmm23, %zmm22

// CHECK: vminmaxpd $123, {sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x83,0xc5,0x10,0x52,0xf0,0x7b]
          vminmaxpd $123, {sae}, %zmm24, %zmm23, %zmm22

// CHECK: vminmaxpd $123, %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x83,0xc5,0x47,0x52,0xf0,0x7b]
          vminmaxpd $123, %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vminmaxpd $123, {sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x83,0xc5,0x97,0x52,0xf0,0x7b]
          vminmaxpd $123, {sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vminmaxpd  $123, 268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0xc5,0x40,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vminmaxpd  $123, 268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vminmaxpd  $123, 291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0xc5,0x47,0x52,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vminmaxpd  $123, 291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vminmaxpd  $123, (%rip){1to8}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe3,0xc5,0x50,0x52,0x35,0x00,0x00,0x00,0x00,0x7b]
          vminmaxpd  $123, (%rip){1to8}, %zmm23, %zmm22

// CHECK: vminmaxpd  $123, -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe3,0xc5,0x40,0x52,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxpd  $123, -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vminmaxpd  $123, 8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0xc5,0xc7,0x52,0x71,0x7f,0x7b]
          vminmaxpd  $123, 8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vminmaxpd  $123, -1024(%rdx){1to8}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0xc5,0xd7,0x52,0x72,0x80,0x7b]
          vminmaxpd  $123, -1024(%rdx){1to8}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vminmaxph $123, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x83,0x44,0x40,0x52,0xf0,0x7b]
          vminmaxph $123, %zmm24, %zmm23, %zmm22

// CHECK: vminmaxph $123, {sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x83,0x44,0x10,0x52,0xf0,0x7b]
          vminmaxph $123, {sae}, %zmm24, %zmm23, %zmm22

// CHECK: vminmaxph $123, %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x83,0x44,0x47,0x52,0xf0,0x7b]
          vminmaxph $123, %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vminmaxph $123, {sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x83,0x44,0x97,0x52,0xf0,0x7b]
          vminmaxph $123, {sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vminmaxph  $123, 268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0x44,0x40,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vminmaxph  $123, 268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vminmaxph  $123, 291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x44,0x47,0x52,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vminmaxph  $123, 291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vminmaxph  $123, (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe3,0x44,0x50,0x52,0x35,0x00,0x00,0x00,0x00,0x7b]
          vminmaxph  $123, (%rip){1to32}, %zmm23, %zmm22

// CHECK: vminmaxph  $123, -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe3,0x44,0x40,0x52,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxph  $123, -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vminmaxph  $123, 8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x44,0xc7,0x52,0x71,0x7f,0x7b]
          vminmaxph  $123, 8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vminmaxph  $123, -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x44,0xd7,0x52,0x72,0x80,0x7b]
          vminmaxph  $123, -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vminmaxps $123, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x83,0x45,0x40,0x52,0xf0,0x7b]
          vminmaxps $123, %zmm24, %zmm23, %zmm22

// CHECK: vminmaxps $123, {sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x83,0x45,0x10,0x52,0xf0,0x7b]
          vminmaxps $123, {sae}, %zmm24, %zmm23, %zmm22

// CHECK: vminmaxps $123, %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x83,0x45,0x47,0x52,0xf0,0x7b]
          vminmaxps $123, %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vminmaxps $123, {sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x83,0x45,0x97,0x52,0xf0,0x7b]
          vminmaxps $123, {sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vminmaxps  $123, 268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0x45,0x40,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vminmaxps  $123, 268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vminmaxps  $123, 291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x45,0x47,0x52,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vminmaxps  $123, 291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vminmaxps  $123, (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe3,0x45,0x50,0x52,0x35,0x00,0x00,0x00,0x00,0x7b]
          vminmaxps  $123, (%rip){1to16}, %zmm23, %zmm22

// CHECK: vminmaxps  $123, -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe3,0x45,0x40,0x52,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vminmaxps  $123, -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vminmaxps  $123, 8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x45,0xc7,0x52,0x71,0x7f,0x7b]
          vminmaxps  $123, 8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vminmaxps  $123, -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x45,0xd7,0x52,0x72,0x80,0x7b]
          vminmaxps  $123, -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

