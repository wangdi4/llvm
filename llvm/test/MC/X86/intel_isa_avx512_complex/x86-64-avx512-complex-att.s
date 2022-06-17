// REQUIRES: intel_feature_isa_avx512_complex
// RUN: llvm-mc -triple x86_64 --show-encoding %s | FileCheck %s

// CHECK: vaddsubpd %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0xc5,0x40,0xd0,0xf0]
          vaddsubpd %zmm24, %zmm23, %zmm22

// CHECK: vaddsubpd {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0xc5,0x10,0xd0,0xf0]
          vaddsubpd {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vaddsubpd %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0xc5,0x47,0xd0,0xf0]
          vaddsubpd %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vaddsubpd {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0xc5,0xf7,0xd0,0xf0]
          vaddsubpd {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vaddsubpd  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0xc5,0x40,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubpd  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vaddsubpd  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0xc5,0x47,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubpd  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vaddsubpd  (%rip){1to8}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0xc5,0x50,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubpd  (%rip){1to8}, %zmm23, %zmm22

// CHECK: vaddsubpd  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0xc5,0x40,0xd0,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vaddsubpd  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vaddsubpd  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0xc5,0xc7,0xd0,0x71,0x7f]
          vaddsubpd  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vaddsubpd  -1024(%rdx){1to8}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0xc5,0xd7,0xd0,0x72,0x80]
          vaddsubpd  -1024(%rdx){1to8}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vaddsubph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x44,0x40,0xd0,0xf0]
          vaddsubph %zmm24, %zmm23, %zmm22

// CHECK: vaddsubph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x44,0x10,0xd0,0xf0]
          vaddsubph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vaddsubph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x44,0x47,0xd0,0xf0]
          vaddsubph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vaddsubph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x44,0xf7,0xd0,0xf0]
          vaddsubph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vaddsubph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vaddsubph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vaddsubph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vaddsubph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0xd0,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vaddsubph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vaddsubph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0xd0,0x71,0x7f]
          vaddsubph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vaddsubph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0xd0,0x72,0x80]
          vaddsubph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vaddsubps %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xd0,0xf0]
          vaddsubps %zmm24, %zmm23, %zmm22

// CHECK: vaddsubps {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xd0,0xf0]
          vaddsubps {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vaddsubps %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xd0,0xf0]
          vaddsubps %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vaddsubps {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xd0,0xf0]
          vaddsubps {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vaddsubps  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xd0,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsubps  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vaddsubps  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xd0,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsubps  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vaddsubps  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xd0,0x35,0x00,0x00,0x00,0x00]
          vaddsubps  (%rip){1to16}, %zmm23, %zmm22

// CHECK: vaddsubps  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xd0,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vaddsubps  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vaddsubps  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xd0,0x71,0x7f]
          vaddsubps  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vaddsubps  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xd0,0x72,0x80]
          vaddsubps  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vmovdhdup %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa1,0xff,0x48,0x16,0xf7]
          vmovdhdup %zmm23, %zmm22

// CHECK: vmovdhdup %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa1,0xff,0x4f,0x16,0xf7]
          vmovdhdup %zmm23, %zmm22 {%k7}

// CHECK: vmovdhdup %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa1,0xff,0xcf,0x16,0xf7]
          vmovdhdup %zmm23, %zmm22 {%k7} {z}

// CHECK: vmovdhdup  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa1,0xff,0x48,0x16,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovdhdup  268435456(%rbp,%r14,8), %zmm22

// CHECK: vmovdhdup  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc1,0xff,0x4f,0x16,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovdhdup  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vmovdhdup  (%rip), %zmm22
// CHECK: encoding: [0x62,0xe1,0xff,0x48,0x16,0x35,0x00,0x00,0x00,0x00]
          vmovdhdup  (%rip), %zmm22

// CHECK: vmovdhdup  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe1,0xff,0x48,0x16,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vmovdhdup  -2048(,%rbp,2), %zmm22

// CHECK: vmovdhdup  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe1,0xff,0xcf,0x16,0x71,0x7f]
          vmovdhdup  8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vmovdhdup  -8192(%rdx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe1,0xff,0xcf,0x16,0x72,0x80]
          vmovdhdup  -8192(%rdx), %zmm22 {%k7} {z}

// CHECK: vsubaddpd %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0xc5,0x40,0xd1,0xf0]
          vsubaddpd %zmm24, %zmm23, %zmm22

// CHECK: vsubaddpd {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0xc5,0x10,0xd1,0xf0]
          vsubaddpd {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vsubaddpd %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0xc5,0x47,0xd1,0xf0]
          vsubaddpd %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vsubaddpd {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0xc5,0xf7,0xd1,0xf0]
          vsubaddpd {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vsubaddpd  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0xc5,0x40,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddpd  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vsubaddpd  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0xc5,0x47,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddpd  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vsubaddpd  (%rip){1to8}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0xc5,0x50,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddpd  (%rip){1to8}, %zmm23, %zmm22

// CHECK: vsubaddpd  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0xc5,0x40,0xd1,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vsubaddpd  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vsubaddpd  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0xc5,0xc7,0xd1,0x71,0x7f]
          vsubaddpd  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vsubaddpd  -1024(%rdx){1to8}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0xc5,0xd7,0xd1,0x72,0x80]
          vsubaddpd  -1024(%rdx){1to8}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vsubaddph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x44,0x40,0xd1,0xf0]
          vsubaddph %zmm24, %zmm23, %zmm22

// CHECK: vsubaddph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x44,0x10,0xd1,0xf0]
          vsubaddph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vsubaddph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x44,0x47,0xd1,0xf0]
          vsubaddph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vsubaddph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x44,0xf7,0xd1,0xf0]
          vsubaddph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vsubaddph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vsubaddph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vsubaddph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vsubaddph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0xd1,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vsubaddph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vsubaddph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0xd1,0x71,0x7f]
          vsubaddph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vsubaddph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0xd1,0x72,0x80]
          vsubaddph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vsubaddps %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xd1,0xf0]
          vsubaddps %zmm24, %zmm23, %zmm22

// CHECK: vsubaddps {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xd1,0xf0]
          vsubaddps {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vsubaddps %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xd1,0xf0]
          vsubaddps %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vsubaddps {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xd1,0xf0]
          vsubaddps {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vsubaddps  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xd1,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubaddps  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vsubaddps  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xd1,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubaddps  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vsubaddps  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xd1,0x35,0x00,0x00,0x00,0x00]
          vsubaddps  (%rip){1to16}, %zmm23, %zmm22

// CHECK: vsubaddps  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xd1,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vsubaddps  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vsubaddps  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xd1,0x71,0x7f]
          vsubaddps  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vsubaddps  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xd1,0x72,0x80]
          vsubaddps  -512(%rdx){1to16}, %zmm23, %zmm22 {%k7} {z}

