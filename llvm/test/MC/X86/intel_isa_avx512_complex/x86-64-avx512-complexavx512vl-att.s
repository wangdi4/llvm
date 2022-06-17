// REQUIRES: intel_feature_isa_avx512_complex
// RUN: llvm-mc -triple x86_64 --show-encoding %s | FileCheck %s

// CHECK: vaddsubpd %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0xc5,0x20,0xd0,0xf0]
          vaddsubpd %ymm24, %ymm23, %ymm22

// CHECK: vaddsubpd %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0xc5,0x27,0xd0,0xf0]
          vaddsubpd %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vaddsubpd %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0xc5,0xa7,0xd0,0xf0]
          vaddsubpd %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vaddsubpd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0xc5,0x00,0xd0,0xf0]
          vaddsubpd %xmm24, %xmm23, %xmm22

// CHECK: vaddsubpd %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0xc5,0x07,0xd0,0xf0]
          vaddsubpd %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vaddsubpd %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0xc5,0x87,0xd0,0xf0]
          vaddsubpd %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vaddsubpd  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0xc5,0x20,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubpd  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vaddsubpd  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0xc5,0x27,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubpd  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vaddsubpd  (%rip){1to4}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0xc5,0x30,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubpd  (%rip){1to4}, %ymm23, %ymm22

// CHECK: vaddsubpd  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0xc5,0x20,0xd0,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vaddsubpd  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vaddsubpd  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0xc5,0xa7,0xd0,0x71,0x7f]
          vaddsubpd  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vaddsubpd  -1024(%rdx){1to4}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0xc5,0xb7,0xd0,0x72,0x80]
          vaddsubpd  -1024(%rdx){1to4}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vaddsubpd  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0xc5,0x00,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubpd  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vaddsubpd  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0xc5,0x07,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubpd  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vaddsubpd  (%rip){1to2}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0xc5,0x10,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubpd  (%rip){1to2}, %xmm23, %xmm22

// CHECK: vaddsubpd  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0xc5,0x00,0xd0,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vaddsubpd  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vaddsubpd  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0xc5,0x87,0xd0,0x71,0x7f]
          vaddsubpd  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vaddsubpd  -1024(%rdx){1to2}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0xc5,0x97,0xd0,0x72,0x80]
          vaddsubpd  -1024(%rdx){1to2}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vaddsubph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x44,0x20,0xd0,0xf0]
          vaddsubph %ymm24, %ymm23, %ymm22

// CHECK: vaddsubph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x44,0x27,0xd0,0xf0]
          vaddsubph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vaddsubph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x44,0xa7,0xd0,0xf0]
          vaddsubph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vaddsubph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x44,0x00,0xd0,0xf0]
          vaddsubph %xmm24, %xmm23, %xmm22

// CHECK: vaddsubph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x44,0x07,0xd0,0xf0]
          vaddsubph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vaddsubph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x44,0x87,0xd0,0xf0]
          vaddsubph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vaddsubph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x44,0x20,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vaddsubph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x44,0x27,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vaddsubph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x44,0x30,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vaddsubph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x44,0x20,0xd0,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vaddsubph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vaddsubph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x44,0xa7,0xd0,0x71,0x7f]
          vaddsubph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vaddsubph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x44,0xb7,0xd0,0x72,0x80]
          vaddsubph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vaddsubph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x44,0x00,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vaddsubph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x44,0x07,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vaddsubph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x44,0x10,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vaddsubph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x44,0x00,0xd0,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vaddsubph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vaddsubph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x44,0x87,0xd0,0x71,0x7f]
          vaddsubph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vaddsubph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x44,0x97,0xd0,0x72,0x80]
          vaddsubph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vaddsubps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xd0,0xf0]
          vaddsubps %ymm24, %ymm23, %ymm22

// CHECK: vaddsubps %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xd0,0xf0]
          vaddsubps %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vaddsubps %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xd0,0xf0]
          vaddsubps %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vaddsubps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xd0,0xf0]
          vaddsubps %xmm24, %xmm23, %xmm22

// CHECK: vaddsubps %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xd0,0xf0]
          vaddsubps %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vaddsubps %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xd0,0xf0]
          vaddsubps %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vaddsubps  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubps  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vaddsubps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vaddsubps  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubps  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vaddsubps  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xd0,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vaddsubps  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vaddsubps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xd0,0x71,0x7f]
          vaddsubps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vaddsubps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xd0,0x72,0x80]
          vaddsubps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vaddsubps  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubps  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vaddsubps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vaddsubps  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubps  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vaddsubps  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xd0,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vaddsubps  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vaddsubps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xd0,0x71,0x7f]
          vaddsubps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vaddsubps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xd0,0x72,0x80]
          vaddsubps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vmovdhdup %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa1,0xff,0x08,0x16,0xf7]
          vmovdhdup %xmm23, %xmm22

// CHECK: vmovdhdup %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa1,0xff,0x0f,0x16,0xf7]
          vmovdhdup %xmm23, %xmm22 {%k7}

// CHECK: vmovdhdup %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa1,0xff,0x8f,0x16,0xf7]
          vmovdhdup %xmm23, %xmm22 {%k7} {z}

// CHECK: vmovdhdup %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa1,0xff,0x28,0x16,0xf7]
          vmovdhdup %ymm23, %ymm22

// CHECK: vmovdhdup %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa1,0xff,0x2f,0x16,0xf7]
          vmovdhdup %ymm23, %ymm22 {%k7}

// CHECK: vmovdhdup %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa1,0xff,0xaf,0x16,0xf7]
          vmovdhdup %ymm23, %ymm22 {%k7} {z}

// CHECK: vmovdhdup  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa1,0xff,0x08,0x16,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovdhdup  268435456(%rbp,%r14,8), %xmm22

// CHECK: vmovdhdup  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc1,0xff,0x0f,0x16,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovdhdup  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vmovdhdup  (%rip), %xmm22
// CHECK: encoding: [0x62,0xe1,0xff,0x08,0x16,0x35,0x00,0x00,0x00,0x00]
          vmovdhdup  (%rip), %xmm22

// CHECK: vmovdhdup  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe1,0xff,0x08,0x16,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vmovdhdup  -512(,%rbp,2), %xmm22

// CHECK: vmovdhdup  1016(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe1,0xff,0x8f,0x16,0x71,0x7f]
          vmovdhdup  1016(%rcx), %xmm22 {%k7} {z}

// CHECK: vmovdhdup  -1024(%rdx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe1,0xff,0x8f,0x16,0x72,0x80]
          vmovdhdup  -1024(%rdx), %xmm22 {%k7} {z}

// CHECK: vmovdhdup  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa1,0xff,0x28,0x16,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovdhdup  268435456(%rbp,%r14,8), %ymm22

// CHECK: vmovdhdup  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc1,0xff,0x2f,0x16,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovdhdup  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vmovdhdup  (%rip), %ymm22
// CHECK: encoding: [0x62,0xe1,0xff,0x28,0x16,0x35,0x00,0x00,0x00,0x00]
          vmovdhdup  (%rip), %ymm22

// CHECK: vmovdhdup  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe1,0xff,0x28,0x16,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vmovdhdup  -1024(,%rbp,2), %ymm22

// CHECK: vmovdhdup  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe1,0xff,0xaf,0x16,0x71,0x7f]
          vmovdhdup  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vmovdhdup  -4096(%rdx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe1,0xff,0xaf,0x16,0x72,0x80]
          vmovdhdup  -4096(%rdx), %ymm22 {%k7} {z}

// CHECK: vsubaddpd %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0xc5,0x20,0xd1,0xf0]
          vsubaddpd %ymm24, %ymm23, %ymm22

// CHECK: vsubaddpd %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0xc5,0x27,0xd1,0xf0]
          vsubaddpd %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vsubaddpd %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0xc5,0xa7,0xd1,0xf0]
          vsubaddpd %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vsubaddpd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0xc5,0x00,0xd1,0xf0]
          vsubaddpd %xmm24, %xmm23, %xmm22

// CHECK: vsubaddpd %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0xc5,0x07,0xd1,0xf0]
          vsubaddpd %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vsubaddpd %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0xc5,0x87,0xd1,0xf0]
          vsubaddpd %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vsubaddpd  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0xc5,0x20,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddpd  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vsubaddpd  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0xc5,0x27,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddpd  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vsubaddpd  (%rip){1to4}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0xc5,0x30,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddpd  (%rip){1to4}, %ymm23, %ymm22

// CHECK: vsubaddpd  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0xc5,0x20,0xd1,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vsubaddpd  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vsubaddpd  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0xc5,0xa7,0xd1,0x71,0x7f]
          vsubaddpd  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vsubaddpd  -1024(%rdx){1to4}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0xc5,0xb7,0xd1,0x72,0x80]
          vsubaddpd  -1024(%rdx){1to4}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vsubaddpd  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0xc5,0x00,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddpd  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vsubaddpd  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0xc5,0x07,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddpd  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vsubaddpd  (%rip){1to2}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0xc5,0x10,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddpd  (%rip){1to2}, %xmm23, %xmm22

// CHECK: vsubaddpd  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0xc5,0x00,0xd1,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vsubaddpd  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vsubaddpd  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0xc5,0x87,0xd1,0x71,0x7f]
          vsubaddpd  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vsubaddpd  -1024(%rdx){1to2}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0xc5,0x97,0xd1,0x72,0x80]
          vsubaddpd  -1024(%rdx){1to2}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vsubaddph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x44,0x20,0xd1,0xf0]
          vsubaddph %ymm24, %ymm23, %ymm22

// CHECK: vsubaddph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x44,0x27,0xd1,0xf0]
          vsubaddph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vsubaddph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x44,0xa7,0xd1,0xf0]
          vsubaddph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vsubaddph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x44,0x00,0xd1,0xf0]
          vsubaddph %xmm24, %xmm23, %xmm22

// CHECK: vsubaddph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x44,0x07,0xd1,0xf0]
          vsubaddph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vsubaddph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x44,0x87,0xd1,0xf0]
          vsubaddph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vsubaddph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x44,0x20,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vsubaddph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x44,0x27,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vsubaddph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x44,0x30,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vsubaddph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x44,0x20,0xd1,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vsubaddph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vsubaddph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x44,0xa7,0xd1,0x71,0x7f]
          vsubaddph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vsubaddph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x44,0xb7,0xd1,0x72,0x80]
          vsubaddph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vsubaddph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x44,0x00,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vsubaddph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x44,0x07,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vsubaddph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x44,0x10,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vsubaddph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x44,0x00,0xd1,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vsubaddph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vsubaddph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x44,0x87,0xd1,0x71,0x7f]
          vsubaddph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vsubaddph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x44,0x97,0xd1,0x72,0x80]
          vsubaddph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vsubaddps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xd1,0xf0]
          vsubaddps %ymm24, %ymm23, %ymm22

// CHECK: vsubaddps %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xd1,0xf0]
          vsubaddps %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vsubaddps %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xd1,0xf0]
          vsubaddps %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vsubaddps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xd1,0xf0]
          vsubaddps %xmm24, %xmm23, %xmm22

// CHECK: vsubaddps %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xd1,0xf0]
          vsubaddps %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vsubaddps %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xd1,0xf0]
          vsubaddps %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vsubaddps  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddps  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vsubaddps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddps  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vsubaddps  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddps  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vsubaddps  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xd1,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vsubaddps  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vsubaddps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xd1,0x71,0x7f]
          vsubaddps  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vsubaddps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xd1,0x72,0x80]
          vsubaddps  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vsubaddps  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddps  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vsubaddps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddps  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vsubaddps  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddps  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vsubaddps  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xd1,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vsubaddps  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vsubaddps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xd1,0x71,0x7f]
          vsubaddps  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vsubaddps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xd1,0x72,0x80]
          vsubaddps  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

