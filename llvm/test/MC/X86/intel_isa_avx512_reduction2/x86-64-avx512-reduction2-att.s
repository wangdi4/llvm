// REQUIRES: intel_feature_isa_avx512_reduction2
// RUN: llvm-mc -triple x86_64 --show-encoding %s | FileCheck %s

// CHECK: vphraaddbd %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x40,0x43,0xf0]
          vphraaddbd %zmm24, %xmm23, %xmm22

// CHECK: vphraaddbd %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x47,0x43,0xf0]
          vphraaddbd %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddbdz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x40,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddbdz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraaddbdz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x47,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddbdz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddbdz  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraaddbdz  (%rip), %xmm23, %xmm22

// CHECK: vphraaddbdz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x43,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddbdz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraaddbdz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x43,0x71,0x7f]
          vphraaddbdz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddbdz  -8192(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x43,0x72,0x80]
          vphraaddbdz  -8192(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddsbd %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x40,0x44,0xf0]
          vphraaddsbd %zmm24, %xmm23, %xmm22

// CHECK: vphraaddsbd %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x47,0x44,0xf0]
          vphraaddsbd %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddsbdz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x40,0x44,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddsbdz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraaddsbdz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x47,0x44,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddsbdz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddsbdz  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraaddsbdz  (%rip), %xmm23, %xmm22

// CHECK: vphraaddsbdz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x44,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddsbdz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraaddsbdz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x44,0x71,0x7f]
          vphraaddsbdz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddsbdz  -8192(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x44,0x72,0x80]
          vphraaddsbdz  -8192(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddswd %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x40,0x44,0xf0]
          vphraaddswd %zmm24, %xmm23, %xmm22

// CHECK: vphraaddswd %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x47,0x44,0xf0]
          vphraaddswd %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddswdz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc6,0x40,0x44,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddswdz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraaddswdz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc6,0x47,0x44,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddswdz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddswd  (%rip){1to32}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x50,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraaddswd  (%rip){1to32}, %xmm23, %xmm22

// CHECK: vphraaddswdz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x40,0x44,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddswdz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraaddswdz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x47,0x44,0x71,0x7f]
          vphraaddswdz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddswd  -256(%rdx){1to32}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x57,0x44,0x72,0x80]
          vphraaddswd  -256(%rdx){1to32}, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddwd %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x40,0x43,0xf0]
          vphraaddwd %zmm24, %xmm23, %xmm22

// CHECK: vphraaddwd %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x47,0x43,0xf0]
          vphraaddwd %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddwdz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc6,0x40,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddwdz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraaddwdz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc6,0x47,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddwdz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddwd  (%rip){1to32}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x50,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraaddwd  (%rip){1to32}, %xmm23, %xmm22

// CHECK: vphraaddwdz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x40,0x43,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddwdz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraaddwdz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x47,0x43,0x71,0x7f]
          vphraaddwdz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddwd  -256(%rdx){1to32}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x57,0x43,0x72,0x80]
          vphraaddwd  -256(%rdx){1to32}, %xmm23, %xmm22 {%k7}


// CHECK: vphraandb %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x40,0x4d,0xf0]
          vphraandb %zmm24, %xmm23, %xmm22

// CHECK: vphraandb %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x47,0x4d,0xf0]
          vphraandb %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraandbz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x40,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandbz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraandbz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x47,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandbz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraandbz  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandbz  (%rip), %xmm23, %xmm22

// CHECK: vphraandbz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x4d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraandbz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraandbz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x4d,0x71,0x7f]
          vphraandbz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandbz  -8192(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x4d,0x72,0x80]
          vphraandbz  -8192(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandd %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x4d,0xf0]
          vphraandd %zmm24, %xmm23, %xmm22

// CHECK: vphraandd %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x47,0x4d,0xf0]
          vphraandd %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraanddz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraanddz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraanddz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x47,0x47,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraanddz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraandd  (%rip){1to16}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandd  (%rip){1to16}, %xmm23, %xmm22

// CHECK: vphraanddz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x4d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraanddz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraanddz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x47,0x4d,0x71,0x7f]
          vphraanddz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandd  -512(%rdx){1to16}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x57,0x4d,0x72,0x80]
          vphraandd  -512(%rdx){1to16}, %xmm23, %xmm22 {%k7}

// CHECK: vphraandq %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc7,0x40,0x4d,0xf0]
          vphraandq %zmm24, %xmm23, %xmm22

// CHECK: vphraandq %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc7,0x47,0x4d,0xf0]
          vphraandq %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraandqz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc7,0x40,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandqz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraandqz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc7,0x47,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandqz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraandq  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x50,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandq  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphraandqz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x40,0x4d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraandqz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraandqz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x47,0x4d,0x71,0x7f]
          vphraandqz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandq  -1024(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x57,0x4d,0x72,0x80]
          vphraandq  -1024(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphraandw %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x40,0x4d,0xf0]
          vphraandw %zmm24, %xmm23, %xmm22

// CHECK: vphraandw %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x47,0x4d,0xf0]
          vphraandw %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraandwz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc6,0x40,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandwz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraandwz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc6,0x47,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandwz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraandw  (%rip){1to32}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x50,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandw  (%rip){1to32}, %xmm23, %xmm22

// CHECK: vphraandwz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x40,0x4d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraandwz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraandwz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x47,0x4d,0x71,0x7f]
          vphraandwz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandw  -256(%rdx){1to32}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x57,0x4d,0x72,0x80]
          vphraandw  -256(%rdx){1to32}, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsb %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x40,0x4b,0xf0]
          vphramaxsb %zmm24, %xmm23, %xmm22

// CHECK: vphramaxsb %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x47,0x4b,0xf0]
          vphramaxsb %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsbz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x40,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsbz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphramaxsbz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x47,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsbz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsbz  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsbz  (%rip), %xmm23, %xmm22

// CHECK: vphramaxsbz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x4b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsbz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramaxsbz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x4b,0x71,0x7f]
          vphramaxsbz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsbz  -8192(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x4b,0x72,0x80]
          vphramaxsbz  -8192(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsd %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x4b,0xf0]
          vphramaxsd %zmm24, %xmm23, %xmm22

// CHECK: vphramaxsd %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x47,0x4b,0xf0]
          vphramaxsd %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsdz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsdz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphramaxsdz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x47,0x47,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsdz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsd  (%rip){1to16}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsd  (%rip){1to16}, %xmm23, %xmm22

// CHECK: vphramaxsdz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x4b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsdz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramaxsdz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x47,0x4b,0x71,0x7f]
          vphramaxsdz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsd  -512(%rdx){1to16}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x57,0x4b,0x72,0x80]
          vphramaxsd  -512(%rdx){1to16}, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsq %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc7,0x40,0x4b,0xf0]
          vphramaxsq %zmm24, %xmm23, %xmm22

// CHECK: vphramaxsq %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc7,0x47,0x4b,0xf0]
          vphramaxsq %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsqz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc7,0x40,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsqz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphramaxsqz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc7,0x47,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsqz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsq  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x50,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsq  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphramaxsqz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x40,0x4b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsqz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramaxsqz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x47,0x4b,0x71,0x7f]
          vphramaxsqz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsq  -1024(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x57,0x4b,0x72,0x80]
          vphramaxsq  -1024(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsw %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x40,0x4b,0xf0]
          vphramaxsw %zmm24, %xmm23, %xmm22

// CHECK: vphramaxsw %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x47,0x4b,0xf0]
          vphramaxsw %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxswz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc6,0x40,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxswz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphramaxswz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc6,0x47,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxswz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsw  (%rip){1to32}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x50,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsw  (%rip){1to32}, %xmm23, %xmm22

// CHECK: vphramaxswz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x40,0x4b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxswz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramaxswz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x47,0x4b,0x71,0x7f]
          vphramaxswz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsw  -256(%rdx){1to32}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x57,0x4b,0x72,0x80]
          vphramaxsw  -256(%rdx){1to32}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminb %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x40,0x48,0xf0]
          vphraminb %zmm24, %xmm23, %xmm22

// CHECK: vphraminb %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x47,0x48,0xf0]
          vphraminb %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminbz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x40,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminbz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminbz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x47,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminbz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminbz  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminbz  (%rip), %xmm23, %xmm22

// CHECK: vphraminbz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x48,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminbz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminbz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x48,0x71,0x7f]
          vphraminbz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminbz  -8192(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x48,0x72,0x80]
          vphraminbz  -8192(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphramind %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x48,0xf0]
          vphramind %zmm24, %xmm23, %xmm22

// CHECK: vphramind %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x47,0x48,0xf0]
          vphramind %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramindz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramindz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphramindz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x47,0x47,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramindz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphramind  (%rip){1to16}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x48,0x35,0x00,0x00,0x00,0x00]
          vphramind  (%rip){1to16}, %xmm23, %xmm22

// CHECK: vphramindz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x48,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphramindz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramindz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x47,0x48,0x71,0x7f]
          vphramindz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramind  -512(%rdx){1to16}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x57,0x48,0x72,0x80]
          vphramind  -512(%rdx){1to16}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminq %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc7,0x40,0x48,0xf0]
          vphraminq %zmm24, %xmm23, %xmm22

// CHECK: vphraminq %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc7,0x47,0x48,0xf0]
          vphraminq %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminqz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc7,0x40,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminqz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminqz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc7,0x47,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminqz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminq  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x50,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminq  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphraminqz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x40,0x48,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminqz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminqz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x47,0x48,0x71,0x7f]
          vphraminqz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminq  -1024(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x57,0x48,0x72,0x80]
          vphraminq  -1024(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsb %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x40,0x49,0xf0]
          vphraminsb %zmm24, %xmm23, %xmm22

// CHECK: vphraminsb %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x47,0x49,0xf0]
          vphraminsb %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsbz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x40,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsbz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminsbz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x47,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsbz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsbz  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsbz  (%rip), %xmm23, %xmm22

// CHECK: vphraminsbz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x49,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsbz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminsbz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x49,0x71,0x7f]
          vphraminsbz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsbz  -8192(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x49,0x72,0x80]
          vphraminsbz  -8192(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsd %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x49,0xf0]
          vphraminsd %zmm24, %xmm23, %xmm22

// CHECK: vphraminsd %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x47,0x49,0xf0]
          vphraminsd %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsdz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsdz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminsdz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x47,0x47,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsdz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsd  (%rip){1to16}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsd  (%rip){1to16}, %xmm23, %xmm22

// CHECK: vphraminsdz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x49,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsdz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminsdz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x47,0x49,0x71,0x7f]
          vphraminsdz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsd  -512(%rdx){1to16}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x57,0x49,0x72,0x80]
          vphraminsd  -512(%rdx){1to16}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsq %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc7,0x40,0x49,0xf0]
          vphraminsq %zmm24, %xmm23, %xmm22

// CHECK: vphraminsq %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc7,0x47,0x49,0xf0]
          vphraminsq %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsqz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc7,0x40,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsqz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminsqz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc7,0x47,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsqz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsq  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x50,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsq  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphraminsqz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x40,0x49,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsqz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminsqz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x47,0x49,0x71,0x7f]
          vphraminsqz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsq  -1024(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x57,0x49,0x72,0x80]
          vphraminsq  -1024(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsw %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x40,0x49,0xf0]
          vphraminsw %zmm24, %xmm23, %xmm22

// CHECK: vphraminsw %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x47,0x49,0xf0]
          vphraminsw %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminswz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc6,0x40,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminswz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminswz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc6,0x47,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminswz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsw  (%rip){1to32}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x50,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsw  (%rip){1to32}, %xmm23, %xmm22

// CHECK: vphraminswz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x40,0x49,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminswz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminswz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x47,0x49,0x71,0x7f]
          vphraminswz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsw  -256(%rdx){1to32}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x57,0x49,0x72,0x80]
          vphraminsw  -256(%rdx){1to32}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminw %zmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x40,0x48,0xf0]
          vphraminw %zmm24, %xmm23, %xmm22

// CHECK: vphraminw %zmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x47,0x48,0xf0]
          vphraminw %zmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminwz  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc6,0x40,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminwz  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminwz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc6,0x47,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminwz  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminw  (%rip){1to32}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x50,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminw  (%rip){1to32}, %xmm23, %xmm22

// CHECK: vphraminwz  -2048(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x40,0x48,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminwz  -2048(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminwz  8128(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x47,0x48,0x71,0x7f]
          vphraminwz  8128(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminw  -256(%rdx){1to32}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x57,0x48,0x72,0x80]
          vphraminw  -256(%rdx){1to32}, %xmm23, %xmm22 {%k7}

