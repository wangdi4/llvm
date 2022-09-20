// REQUIRES: intel_feature_isa_avx512_reduction2
// RUN: llvm-mc -triple x86_64 --show-encoding %s | FileCheck %s

// CHECK: vphraaddbd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x43,0xf0]
          vphraaddbd %xmm24, %xmm23, %xmm22

// CHECK: vphraaddbd %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x43,0xf0]
          vphraaddbd %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddbd %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x43,0xf0]
          vphraaddbd %ymm24, %xmm23, %xmm22

// CHECK: vphraaddbd %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x43,0xf0]
          vphraaddbd %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddbdx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddbdx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraaddbdx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddbdx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddbdx  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraaddbdx  (%rip), %xmm23, %xmm22

// CHECK: vphraaddbdx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x43,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddbdx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraaddbdx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x43,0x71,0x7f]
          vphraaddbdx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddbdx  -2048(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x43,0x72,0x80]
          vphraaddbdx  -2048(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddbdy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x43,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddbdy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraaddbdy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x43,0x71,0x7f]
          vphraaddbdy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddbdy  -4096(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x43,0x72,0x80]
          vphraaddbdy  -4096(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddsbd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x44,0xf0]
          vphraaddsbd %xmm24, %xmm23, %xmm22

// CHECK: vphraaddsbd %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x44,0xf0]
          vphraaddsbd %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddsbd %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x44,0xf0]
          vphraaddsbd %ymm24, %xmm23, %xmm22

// CHECK: vphraaddsbd %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x44,0xf0]
          vphraaddsbd %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddsbdx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x44,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddsbdx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraaddsbdx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x44,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddsbdx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddsbdx  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraaddsbdx  (%rip), %xmm23, %xmm22

// CHECK: vphraaddsbdx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x44,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddsbdx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraaddsbdx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x44,0x71,0x7f]
          vphraaddsbdx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddsbdx  -2048(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x44,0x72,0x80]
          vphraaddsbdx  -2048(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddsbdy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x44,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddsbdy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraaddsbdy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x44,0x71,0x7f]
          vphraaddsbdy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddsbdy  -4096(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x44,0x72,0x80]
          vphraaddsbdy  -4096(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddswd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x00,0x44,0xf0]
          vphraaddswd %xmm24, %xmm23, %xmm22

// CHECK: vphraaddswd %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x07,0x44,0xf0]
          vphraaddswd %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddswd %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x20,0x44,0xf0]
          vphraaddswd %ymm24, %xmm23, %xmm22

// CHECK: vphraaddswd %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x27,0x44,0xf0]
          vphraaddswd %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddswdx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc6,0x00,0x44,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddswdx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraaddswdx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc6,0x07,0x44,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddswdx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddswd  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x10,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraaddswd  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphraaddswdx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x44,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddswdx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraaddswdx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x07,0x44,0x71,0x7f]
          vphraaddswdx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddswd  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x17,0x44,0x72,0x80]
          vphraaddswd  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddswd  (%rip){1to16}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x30,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraaddswd  (%rip){1to16}, %xmm23, %xmm22

// CHECK: vphraaddswdy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x20,0x44,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddswdy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraaddswdy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x27,0x44,0x71,0x7f]
          vphraaddswdy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddswd  -256(%rdx){1to16}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x37,0x44,0x72,0x80]
          vphraaddswd  -256(%rdx){1to16}, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddwd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x00,0x43,0xf0]
          vphraaddwd %xmm24, %xmm23, %xmm22

// CHECK: vphraaddwd %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x07,0x43,0xf0]
          vphraaddwd %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddwd %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x20,0x43,0xf0]
          vphraaddwd %ymm24, %xmm23, %xmm22

// CHECK: vphraaddwd %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x27,0x43,0xf0]
          vphraaddwd %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddwdx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc6,0x00,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddwdx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraaddwdx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc6,0x07,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddwdx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddwd  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x10,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraaddwd  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphraaddwdx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x43,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddwdx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraaddwdx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x07,0x43,0x71,0x7f]
          vphraaddwdx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddwd  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x17,0x43,0x72,0x80]
          vphraaddwd  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphraaddwd  (%rip){1to16}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x30,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraaddwd  (%rip){1to16}, %xmm23, %xmm22

// CHECK: vphraaddwdy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x20,0x43,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddwdy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraaddwdy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x27,0x43,0x71,0x7f]
          vphraaddwdy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraaddwd  -256(%rdx){1to16}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x37,0x43,0x72,0x80]
          vphraaddwd  -256(%rdx){1to16}, %xmm23, %xmm22 {%k7}


// CHECK: vphraandb %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x4d,0xf0]
          vphraandb %xmm24, %xmm23, %xmm22

// CHECK: vphraandb %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x4d,0xf0]
          vphraandb %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraandb %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x4d,0xf0]
          vphraandb %ymm24, %xmm23, %xmm22

// CHECK: vphraandb %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x4d,0xf0]
          vphraandb %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraandbx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandbx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraandbx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandbx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraandbx  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandbx  (%rip), %xmm23, %xmm22

// CHECK: vphraandbx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x4d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraandbx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraandbx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x4d,0x71,0x7f]
          vphraandbx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandbx  -2048(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x4d,0x72,0x80]
          vphraandbx  -2048(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandby  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x4d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraandby  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraandby  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x4d,0x71,0x7f]
          vphraandby  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandby  -4096(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x4d,0x72,0x80]
          vphraandby  -4096(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x4d,0xf0]
          vphraandd %xmm24, %xmm23, %xmm22

// CHECK: vphraandd %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x07,0x4d,0xf0]
          vphraandd %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraandd %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x4d,0xf0]
          vphraandd %ymm24, %xmm23, %xmm22

// CHECK: vphraandd %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x27,0x4d,0xf0]
          vphraandd %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraanddx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraanddx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraanddx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x47,0x07,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraanddx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraandd  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandd  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vphraanddx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x4d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraanddx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraanddx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x07,0x4d,0x71,0x7f]
          vphraanddx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandd  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x17,0x4d,0x72,0x80]
          vphraandd  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7}

// CHECK: vphraandd  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandd  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphraanddy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x4d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraanddy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraanddy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x27,0x4d,0x71,0x7f]
          vphraanddy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandd  -512(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x37,0x4d,0x72,0x80]
          vphraandd  -512(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphraandq %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc7,0x00,0x4d,0xf0]
          vphraandq %xmm24, %xmm23, %xmm22

// CHECK: vphraandq %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc7,0x07,0x4d,0xf0]
          vphraandq %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraandq %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc7,0x20,0x4d,0xf0]
          vphraandq %ymm24, %xmm23, %xmm22

// CHECK: vphraandq %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc7,0x27,0x4d,0xf0]
          vphraandq %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraandqx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc7,0x00,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandqx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraandqx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc7,0x07,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandqx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraandq  (%rip){1to2}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x10,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandq  (%rip){1to2}, %xmm23, %xmm22

// CHECK: vphraandqx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x00,0x4d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraandqx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraandqx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x07,0x4d,0x71,0x7f]
          vphraandqx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandq  -1024(%rdx){1to2}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x17,0x4d,0x72,0x80]
          vphraandq  -1024(%rdx){1to2}, %xmm23, %xmm22 {%k7}

// CHECK: vphraandq  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x30,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandq  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vphraandqy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x20,0x4d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraandqy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraandqy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x27,0x4d,0x71,0x7f]
          vphraandqy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandq  -1024(%rdx){1to4}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x37,0x4d,0x72,0x80]
          vphraandq  -1024(%rdx){1to4}, %xmm23, %xmm22 {%k7}

// CHECK: vphraandw %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x00,0x4d,0xf0]
          vphraandw %xmm24, %xmm23, %xmm22

// CHECK: vphraandw %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x07,0x4d,0xf0]
          vphraandw %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraandw %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x20,0x4d,0xf0]
          vphraandw %ymm24, %xmm23, %xmm22

// CHECK: vphraandw %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x27,0x4d,0xf0]
          vphraandw %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraandwx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc6,0x00,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandwx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraandwx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc6,0x07,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandwx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraandw  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x10,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandw  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphraandwx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x4d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraandwx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraandwx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x07,0x4d,0x71,0x7f]
          vphraandwx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandw  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x17,0x4d,0x72,0x80]
          vphraandw  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphraandw  (%rip){1to16}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x30,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandw  (%rip){1to16}, %xmm23, %xmm22

// CHECK: vphraandwy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x20,0x4d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraandwy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraandwy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x27,0x4d,0x71,0x7f]
          vphraandwy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraandw  -256(%rdx){1to16}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x37,0x4d,0x72,0x80]
          vphraandw  -256(%rdx){1to16}, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsb %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x4b,0xf0]
          vphramaxsb %xmm24, %xmm23, %xmm22

// CHECK: vphramaxsb %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x4b,0xf0]
          vphramaxsb %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsb %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x4b,0xf0]
          vphramaxsb %ymm24, %xmm23, %xmm22

// CHECK: vphramaxsb %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x4b,0xf0]
          vphramaxsb %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsbx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsbx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphramaxsbx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsbx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsbx  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsbx  (%rip), %xmm23, %xmm22

// CHECK: vphramaxsbx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x4b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsbx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramaxsbx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x4b,0x71,0x7f]
          vphramaxsbx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsbx  -2048(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x4b,0x72,0x80]
          vphramaxsbx  -2048(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsby  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x4b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsby  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramaxsby  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x4b,0x71,0x7f]
          vphramaxsby  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsby  -4096(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x4b,0x72,0x80]
          vphramaxsby  -4096(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x4b,0xf0]
          vphramaxsd %xmm24, %xmm23, %xmm22

// CHECK: vphramaxsd %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x07,0x4b,0xf0]
          vphramaxsd %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsd %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x4b,0xf0]
          vphramaxsd %ymm24, %xmm23, %xmm22

// CHECK: vphramaxsd %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x27,0x4b,0xf0]
          vphramaxsd %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsdx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsdx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphramaxsdx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x47,0x07,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsdx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsd  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsd  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vphramaxsdx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x4b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsdx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramaxsdx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x07,0x4b,0x71,0x7f]
          vphramaxsdx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsd  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x17,0x4b,0x72,0x80]
          vphramaxsd  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsd  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsd  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphramaxsdy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x4b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsdy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramaxsdy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x27,0x4b,0x71,0x7f]
          vphramaxsdy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsd  -512(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x37,0x4b,0x72,0x80]
          vphramaxsd  -512(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsq %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc7,0x00,0x4b,0xf0]
          vphramaxsq %xmm24, %xmm23, %xmm22

// CHECK: vphramaxsq %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc7,0x07,0x4b,0xf0]
          vphramaxsq %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsq %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc7,0x20,0x4b,0xf0]
          vphramaxsq %ymm24, %xmm23, %xmm22

// CHECK: vphramaxsq %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc7,0x27,0x4b,0xf0]
          vphramaxsq %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsqx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc7,0x00,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsqx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphramaxsqx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc7,0x07,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsqx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsq  (%rip){1to2}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x10,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsq  (%rip){1to2}, %xmm23, %xmm22

// CHECK: vphramaxsqx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x00,0x4b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsqx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramaxsqx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x07,0x4b,0x71,0x7f]
          vphramaxsqx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsq  -1024(%rdx){1to2}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x17,0x4b,0x72,0x80]
          vphramaxsq  -1024(%rdx){1to2}, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsq  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x30,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsq  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vphramaxsqy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x20,0x4b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsqy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramaxsqy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x27,0x4b,0x71,0x7f]
          vphramaxsqy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsq  -1024(%rdx){1to4}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x37,0x4b,0x72,0x80]
          vphramaxsq  -1024(%rdx){1to4}, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsw %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x00,0x4b,0xf0]
          vphramaxsw %xmm24, %xmm23, %xmm22

// CHECK: vphramaxsw %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x07,0x4b,0xf0]
          vphramaxsw %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsw %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x20,0x4b,0xf0]
          vphramaxsw %ymm24, %xmm23, %xmm22

// CHECK: vphramaxsw %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x27,0x4b,0xf0]
          vphramaxsw %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxswx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc6,0x00,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxswx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphramaxswx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc6,0x07,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxswx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsw  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x10,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsw  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphramaxswx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x4b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxswx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramaxswx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x07,0x4b,0x71,0x7f]
          vphramaxswx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsw  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x17,0x4b,0x72,0x80]
          vphramaxsw  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsw  (%rip){1to16}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x30,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsw  (%rip){1to16}, %xmm23, %xmm22

// CHECK: vphramaxswy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x20,0x4b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxswy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramaxswy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x27,0x4b,0x71,0x7f]
          vphramaxswy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramaxsw  -256(%rdx){1to16}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x37,0x4b,0x72,0x80]
          vphramaxsw  -256(%rdx){1to16}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminb %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x48,0xf0]
          vphraminb %xmm24, %xmm23, %xmm22

// CHECK: vphraminb %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x48,0xf0]
          vphraminb %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminb %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x48,0xf0]
          vphraminb %ymm24, %xmm23, %xmm22

// CHECK: vphraminb %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x48,0xf0]
          vphraminb %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminbx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminbx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminbx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminbx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminbx  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminbx  (%rip), %xmm23, %xmm22

// CHECK: vphraminbx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x48,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminbx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminbx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x48,0x71,0x7f]
          vphraminbx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminbx  -2048(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x48,0x72,0x80]
          vphraminbx  -2048(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminby  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x48,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminby  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminby  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x48,0x71,0x7f]
          vphraminby  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminby  -4096(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x48,0x72,0x80]
          vphraminby  -4096(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphramind %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x48,0xf0]
          vphramind %xmm24, %xmm23, %xmm22

// CHECK: vphramind %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x07,0x48,0xf0]
          vphramind %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramind %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x48,0xf0]
          vphramind %ymm24, %xmm23, %xmm22

// CHECK: vphramind %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x27,0x48,0xf0]
          vphramind %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphramindx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramindx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphramindx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x47,0x07,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramindx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphramind  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x48,0x35,0x00,0x00,0x00,0x00]
          vphramind  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vphramindx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x48,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphramindx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramindx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x07,0x48,0x71,0x7f]
          vphramindx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramind  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x17,0x48,0x72,0x80]
          vphramind  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7}

// CHECK: vphramind  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x48,0x35,0x00,0x00,0x00,0x00]
          vphramind  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphramindy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x48,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphramindy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphramindy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x27,0x48,0x71,0x7f]
          vphramindy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphramind  -512(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x37,0x48,0x72,0x80]
          vphramind  -512(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminq %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc7,0x00,0x48,0xf0]
          vphraminq %xmm24, %xmm23, %xmm22

// CHECK: vphraminq %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc7,0x07,0x48,0xf0]
          vphraminq %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminq %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc7,0x20,0x48,0xf0]
          vphraminq %ymm24, %xmm23, %xmm22

// CHECK: vphraminq %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc7,0x27,0x48,0xf0]
          vphraminq %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminqx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc7,0x00,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminqx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminqx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc7,0x07,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminqx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminq  (%rip){1to2}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x10,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminq  (%rip){1to2}, %xmm23, %xmm22

// CHECK: vphraminqx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x00,0x48,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminqx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminqx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x07,0x48,0x71,0x7f]
          vphraminqx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminq  -1024(%rdx){1to2}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x17,0x48,0x72,0x80]
          vphraminq  -1024(%rdx){1to2}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminq  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x30,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminq  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vphraminqy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x20,0x48,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminqy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminqy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x27,0x48,0x71,0x7f]
          vphraminqy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminq  -1024(%rdx){1to4}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x37,0x48,0x72,0x80]
          vphraminq  -1024(%rdx){1to4}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsb %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x49,0xf0]
          vphraminsb %xmm24, %xmm23, %xmm22

// CHECK: vphraminsb %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x49,0xf0]
          vphraminsb %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsb %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x49,0xf0]
          vphraminsb %ymm24, %xmm23, %xmm22

// CHECK: vphraminsb %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x49,0xf0]
          vphraminsb %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsbx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsbx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminsbx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsbx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsbx  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsbx  (%rip), %xmm23, %xmm22

// CHECK: vphraminsbx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x49,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsbx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminsbx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x49,0x71,0x7f]
          vphraminsbx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsbx  -2048(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x49,0x72,0x80]
          vphraminsbx  -2048(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsby  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x49,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsby  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminsby  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x49,0x71,0x7f]
          vphraminsby  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsby  -4096(%rdx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x49,0x72,0x80]
          vphraminsby  -4096(%rdx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x49,0xf0]
          vphraminsd %xmm24, %xmm23, %xmm22

// CHECK: vphraminsd %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x07,0x49,0xf0]
          vphraminsd %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsd %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x49,0xf0]
          vphraminsd %ymm24, %xmm23, %xmm22

// CHECK: vphraminsd %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x47,0x27,0x49,0xf0]
          vphraminsd %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsdx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsdx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminsdx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x47,0x07,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsdx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsd  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsd  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vphraminsdx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x49,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsdx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminsdx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x07,0x49,0x71,0x7f]
          vphraminsdx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsd  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x17,0x49,0x72,0x80]
          vphraminsd  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsd  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsd  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphraminsdy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x49,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsdy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminsdy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x27,0x49,0x71,0x7f]
          vphraminsdy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsd  -512(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0x47,0x37,0x49,0x72,0x80]
          vphraminsd  -512(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsq %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc7,0x00,0x49,0xf0]
          vphraminsq %xmm24, %xmm23, %xmm22

// CHECK: vphraminsq %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc7,0x07,0x49,0xf0]
          vphraminsq %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsq %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc7,0x20,0x49,0xf0]
          vphraminsq %ymm24, %xmm23, %xmm22

// CHECK: vphraminsq %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc7,0x27,0x49,0xf0]
          vphraminsq %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsqx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc7,0x00,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsqx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminsqx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc7,0x07,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsqx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsq  (%rip){1to2}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x10,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsq  (%rip){1to2}, %xmm23, %xmm22

// CHECK: vphraminsqx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x00,0x49,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsqx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminsqx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x07,0x49,0x71,0x7f]
          vphraminsqx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsq  -1024(%rdx){1to2}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x17,0x49,0x72,0x80]
          vphraminsq  -1024(%rdx){1to2}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsq  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x30,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsq  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vphraminsqy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc7,0x20,0x49,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsqy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminsqy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x27,0x49,0x71,0x7f]
          vphraminsqy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsq  -1024(%rdx){1to4}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc7,0x37,0x49,0x72,0x80]
          vphraminsq  -1024(%rdx){1to4}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsw %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x00,0x49,0xf0]
          vphraminsw %xmm24, %xmm23, %xmm22

// CHECK: vphraminsw %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x07,0x49,0xf0]
          vphraminsw %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsw %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x20,0x49,0xf0]
          vphraminsw %ymm24, %xmm23, %xmm22

// CHECK: vphraminsw %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x27,0x49,0xf0]
          vphraminsw %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminswx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc6,0x00,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminswx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminswx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc6,0x07,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminswx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsw  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x10,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsw  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphraminswx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x49,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminswx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminswx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x07,0x49,0x71,0x7f]
          vphraminswx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsw  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x17,0x49,0x72,0x80]
          vphraminsw  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminsw  (%rip){1to16}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x30,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsw  (%rip){1to16}, %xmm23, %xmm22

// CHECK: vphraminswy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x20,0x49,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminswy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminswy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x27,0x49,0x71,0x7f]
          vphraminswy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminsw  -256(%rdx){1to16}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x37,0x49,0x72,0x80]
          vphraminsw  -256(%rdx){1to16}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminw %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x00,0x48,0xf0]
          vphraminw %xmm24, %xmm23, %xmm22

// CHECK: vphraminw %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x07,0x48,0xf0]
          vphraminw %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminw %ymm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0xc6,0x20,0x48,0xf0]
          vphraminw %ymm24, %xmm23, %xmm22

// CHECK: vphraminw %ymm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0xc6,0x27,0x48,0xf0]
          vphraminw %ymm24, %xmm23, %xmm22 {%k7}

// CHECK: vphraminwx  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xc6,0x00,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminwx  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vphraminwx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xc6,0x07,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminwx  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vphraminw  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x10,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminw  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vphraminwx  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x48,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminwx  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminwx  2032(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x07,0x48,0x71,0x7f]
          vphraminwx  2032(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminw  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x17,0x48,0x72,0x80]
          vphraminw  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7}

// CHECK: vphraminw  (%rip){1to16}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x30,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminw  (%rip){1to16}, %xmm23, %xmm22

// CHECK: vphraminwy  -1024(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x20,0x48,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminwy  -1024(,%rbp,2), %xmm23, %xmm22

// CHECK: vphraminwy  4064(%rcx), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x27,0x48,0x71,0x7f]
          vphraminwy  4064(%rcx), %xmm23, %xmm22 {%k7}

// CHECK: vphraminw  -256(%rdx){1to16}, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xe5,0xc6,0x37,0x48,0x72,0x80]
          vphraminw  -256(%rdx){1to16}, %xmm23, %xmm22 {%k7}

