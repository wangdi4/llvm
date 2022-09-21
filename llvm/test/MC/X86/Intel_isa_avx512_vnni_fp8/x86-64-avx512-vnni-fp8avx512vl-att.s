// REQUIRES: intel_feature_isa_avx512_vnni_fp8
// RUN: llvm-mc -triple x86_64 --show-encoding %s | FileCheck %s

// CHECK: vdpbf8ps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x85,0x44,0x20,0x50,0xf0]
          vdpbf8ps %ymm24, %ymm23, %ymm22

// CHECK: vdpbf8ps %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x44,0x27,0x50,0xf0]
          vdpbf8ps %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vdpbf8ps %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x85,0x44,0xa7,0x50,0xf0]
          vdpbf8ps %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vdpbf8ps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x44,0x00,0x50,0xf0]
          vdpbf8ps %xmm24, %xmm23, %xmm22

// CHECK: vdpbf8ps %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x44,0x07,0x50,0xf0]
          vdpbf8ps %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vdpbf8ps %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x85,0x44,0x87,0x50,0xf0]
          vdpbf8ps %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vdpbf8ps  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x44,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdpbf8ps  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vdpbf8ps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x44,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdpbf8ps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vdpbf8ps  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x44,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
          vdpbf8ps  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vdpbf8ps  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x44,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vdpbf8ps  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vdpbf8ps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x44,0xa7,0x50,0x71,0x7f]
          vdpbf8ps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vdpbf8ps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x44,0xb7,0x50,0x72,0x80]
          vdpbf8ps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vdpbf8ps  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x44,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdpbf8ps  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vdpbf8ps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x44,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdpbf8ps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vdpbf8ps  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x44,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
          vdpbf8ps  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vdpbf8ps  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x44,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vdpbf8ps  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vdpbf8ps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x44,0x87,0x50,0x71,0x7f]
          vdpbf8ps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vdpbf8ps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x44,0x97,0x50,0x72,0x80]
          vdpbf8ps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vdpbhf8ps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x50,0xf0]
          vdpbhf8ps %ymm24, %ymm23, %ymm22

// CHECK: vdpbhf8ps %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x27,0x50,0xf0]
          vdpbhf8ps %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vdpbhf8ps %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x85,0x47,0xa7,0x50,0xf0]
          vdpbhf8ps %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vdpbhf8ps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x50,0xf0]
          vdpbhf8ps %xmm24, %xmm23, %xmm22

// CHECK: vdpbhf8ps %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x07,0x50,0xf0]
          vdpbhf8ps %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vdpbhf8ps %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x85,0x47,0x87,0x50,0xf0]
          vdpbhf8ps %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vdpbhf8ps  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x47,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdpbhf8ps  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vdpbhf8ps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x47,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdpbhf8ps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vdpbhf8ps  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
          vdpbhf8ps  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vdpbhf8ps  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vdpbhf8ps  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vdpbhf8ps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x47,0xa7,0x50,0x71,0x7f]
          vdpbhf8ps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vdpbhf8ps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x47,0xb7,0x50,0x72,0x80]
          vdpbhf8ps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vdpbhf8ps  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdpbhf8ps  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vdpbhf8ps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x47,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdpbhf8ps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vdpbhf8ps  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
          vdpbhf8ps  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vdpbhf8ps  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vdpbhf8ps  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vdpbhf8ps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x47,0x87,0x50,0x71,0x7f]
          vdpbhf8ps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vdpbhf8ps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x47,0x97,0x50,0x72,0x80]
          vdpbhf8ps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vdphbf8ps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x50,0xf0]
          vdphbf8ps %ymm24, %ymm23, %ymm22

// CHECK: vdphbf8ps %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x50,0xf0]
          vdphbf8ps %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vdphbf8ps %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x85,0x46,0xa7,0x50,0xf0]
          vdphbf8ps %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vdphbf8ps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x50,0xf0]
          vdphbf8ps %xmm24, %xmm23, %xmm22

// CHECK: vdphbf8ps %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x50,0xf0]
          vdphbf8ps %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vdphbf8ps %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x85,0x46,0x87,0x50,0xf0]
          vdphbf8ps %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vdphbf8ps  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x46,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdphbf8ps  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vdphbf8ps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdphbf8ps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vdphbf8ps  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x46,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
          vdphbf8ps  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vdphbf8ps  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vdphbf8ps  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vdphbf8ps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x46,0xa7,0x50,0x71,0x7f]
          vdphbf8ps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vdphbf8ps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x46,0xb7,0x50,0x72,0x80]
          vdphbf8ps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vdphbf8ps  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdphbf8ps  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vdphbf8ps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdphbf8ps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vdphbf8ps  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
          vdphbf8ps  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vdphbf8ps  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vdphbf8ps  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vdphbf8ps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x46,0x87,0x50,0x71,0x7f]
          vdphbf8ps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vdphbf8ps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x46,0x97,0x50,0x72,0x80]
          vdphbf8ps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vdphf8ps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x85,0x45,0x20,0x50,0xf0]
          vdphf8ps %ymm24, %ymm23, %ymm22

// CHECK: vdphf8ps %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x45,0x27,0x50,0xf0]
          vdphf8ps %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vdphf8ps %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x85,0x45,0xa7,0x50,0xf0]
          vdphf8ps %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vdphf8ps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x45,0x00,0x50,0xf0]
          vdphf8ps %xmm24, %xmm23, %xmm22

// CHECK: vdphf8ps %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x45,0x07,0x50,0xf0]
          vdphf8ps %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vdphf8ps %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x85,0x45,0x87,0x50,0xf0]
          vdphf8ps %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vdphf8ps  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x45,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdphf8ps  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vdphf8ps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x45,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdphf8ps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vdphf8ps  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
          vdphf8ps  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vdphf8ps  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vdphf8ps  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vdphf8ps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x45,0xa7,0x50,0x71,0x7f]
          vdphf8ps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vdphf8ps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x45,0xb7,0x50,0x72,0x80]
          vdphf8ps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vdphf8ps  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x45,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdphf8ps  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vdphf8ps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x45,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdphf8ps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vdphf8ps  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
          vdphf8ps  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vdphf8ps  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vdphf8ps  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vdphf8ps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x45,0x87,0x50,0x71,0x7f]
          vdphf8ps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vdphf8ps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x45,0x97,0x50,0x72,0x80]
          vdphf8ps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

