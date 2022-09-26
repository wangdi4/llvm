// REQUIRES: intel_feature_isa_avx512_reduction2
// RUN: llvm-mc -triple i386 --show-encoding %s | FileCheck %s

// CHECK: vphraaddbd %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x43,0xd4]
          vphraaddbd %xmm4, %xmm3, %xmm2

// CHECK: vphraaddbd %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x43,0xd4]
          vphraaddbd %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddbd %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x43,0xd4]
          vphraaddbd %ymm4, %xmm3, %xmm2

// CHECK: vphraaddbd %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x43,0xd4]
          vphraaddbd %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddbdx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddbdx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraaddbdx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddbdx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddbdx  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x43,0x10]
          vphraaddbdx  (%eax), %xmm3, %xmm2

// CHECK: vphraaddbdx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x43,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddbdx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraaddbdx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x43,0x51,0x7f]
          vphraaddbdx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddbdx  -2048(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x43,0x52,0x80]
          vphraaddbdx  -2048(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddbdy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x43,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddbdy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraaddbdy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x43,0x51,0x7f]
          vphraaddbdy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddbdy  -4096(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x43,0x52,0x80]
          vphraaddbdy  -4096(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddsbd %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x44,0xd4]
          vphraaddsbd %xmm4, %xmm3, %xmm2

// CHECK: vphraaddsbd %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x44,0xd4]
          vphraaddsbd %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddsbd %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x44,0xd4]
          vphraaddsbd %ymm4, %xmm3, %xmm2

// CHECK: vphraaddsbd %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x44,0xd4]
          vphraaddsbd %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddsbdx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddsbdx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraaddsbdx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddsbdx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddsbdx  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x44,0x10]
          vphraaddsbdx  (%eax), %xmm3, %xmm2

// CHECK: vphraaddsbdx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x44,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddsbdx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraaddsbdx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x44,0x51,0x7f]
          vphraaddsbdx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddsbdx  -2048(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x44,0x52,0x80]
          vphraaddsbdx  -2048(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddsbdy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x44,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddsbdy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraaddsbdy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x44,0x51,0x7f]
          vphraaddsbdy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddsbdy  -4096(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x44,0x52,0x80]
          vphraaddsbdy  -4096(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddswd %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x44,0xd4]
          vphraaddswd %xmm4, %xmm3, %xmm2

// CHECK: vphraaddswd %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x44,0xd4]
          vphraaddswd %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddswd %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x44,0xd4]
          vphraaddswd %ymm4, %xmm3, %xmm2

// CHECK: vphraaddswd %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x44,0xd4]
          vphraaddswd %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddswdx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddswdx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraaddswdx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddswdx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddswd  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x18,0x44,0x10]
          vphraaddswd  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphraaddswdx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x44,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddswdx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraaddswdx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x44,0x51,0x7f]
          vphraaddswdx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddswd  -256(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x1f,0x44,0x52,0x80]
          vphraaddswd  -256(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddswd  (%eax){1to16}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x38,0x44,0x10]
          vphraaddswd  (%eax){1to16}, %xmm3, %xmm2

// CHECK: vphraaddswdy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x44,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddswdy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraaddswdy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x44,0x51,0x7f]
          vphraaddswdy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddswd  -256(%edx){1to16}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x3f,0x44,0x52,0x80]
          vphraaddswd  -256(%edx){1to16}, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddwd %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x43,0xd4]
          vphraaddwd %xmm4, %xmm3, %xmm2

// CHECK: vphraaddwd %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x43,0xd4]
          vphraaddwd %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddwd %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x43,0xd4]
          vphraaddwd %ymm4, %xmm3, %xmm2

// CHECK: vphraaddwd %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x43,0xd4]
          vphraaddwd %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddwdx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddwdx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraaddwdx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddwdx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddwd  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x18,0x43,0x10]
          vphraaddwd  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphraaddwdx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x43,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddwdx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraaddwdx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x43,0x51,0x7f]
          vphraaddwdx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddwd  -256(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x1f,0x43,0x52,0x80]
          vphraaddwd  -256(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddwd  (%eax){1to16}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x38,0x43,0x10]
          vphraaddwd  (%eax){1to16}, %xmm3, %xmm2

// CHECK: vphraaddwdy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x43,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddwdy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraaddwdy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x43,0x51,0x7f]
          vphraaddwdy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddwd  -256(%edx){1to16}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x3f,0x43,0x52,0x80]
          vphraaddwd  -256(%edx){1to16}, %xmm3, %xmm2 {%k7}

// CHECK: vphraandb %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4d,0xd4]
          vphraandb %xmm4, %xmm3, %xmm2

// CHECK: vphraandb %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4d,0xd4]
          vphraandb %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraandb %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x4d,0xd4]
          vphraandb %ymm4, %xmm3, %xmm2

// CHECK: vphraandb %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x4d,0xd4]
          vphraandb %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraandbx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandbx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraandbx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandbx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraandbx  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4d,0x10]
          vphraandbx  (%eax), %xmm3, %xmm2

// CHECK: vphraandbx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraandbx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraandbx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4d,0x51,0x7f]
          vphraandbx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandbx  -2048(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4d,0x52,0x80]
          vphraandbx  -2048(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandby  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x4d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraandby  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraandby  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x4d,0x51,0x7f]
          vphraandby  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandby  -4096(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x4d,0x52,0x80]
          vphraandby  -4096(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandd %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x4d,0xd4]
          vphraandd %xmm4, %xmm3, %xmm2

// CHECK: vphraandd %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x4d,0xd4]
          vphraandd %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraandd %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x4d,0xd4]
          vphraandd %ymm4, %xmm3, %xmm2

// CHECK: vphraandd %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x4d,0xd4]
          vphraandd %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraanddx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraanddx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraanddx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraanddx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraandd  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x4d,0x10]
          vphraandd  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vphraanddx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x4d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraanddx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraanddx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x4d,0x51,0x7f]
          vphraanddx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandd  -512(%edx){1to4}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x1f,0x4d,0x52,0x80]
          vphraandd  -512(%edx){1to4}, %xmm3, %xmm2 {%k7}

// CHECK: vphraandd  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x4d,0x10]
          vphraandd  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphraanddy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x4d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraanddy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraanddy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x4d,0x51,0x7f]
          vphraanddy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandd  -512(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x3f,0x4d,0x52,0x80]
          vphraandd  -512(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphraandq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x4d,0xd4]
          vphraandq %xmm4, %xmm3, %xmm2

// CHECK: vphraandq %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x4d,0xd4]
          vphraandq %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraandq %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x4d,0xd4]
          vphraandq %ymm4, %xmm3, %xmm2

// CHECK: vphraandq %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x4d,0xd4]
          vphraandq %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraandqx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandqx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraandqx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandqx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraandq  (%eax){1to2}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x18,0x4d,0x10]
          vphraandq  (%eax){1to2}, %xmm3, %xmm2

// CHECK: vphraandqx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x4d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraandqx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraandqx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x4d,0x51,0x7f]
          vphraandqx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandq  -1024(%edx){1to2}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x1f,0x4d,0x52,0x80]
          vphraandq  -1024(%edx){1to2}, %xmm3, %xmm2 {%k7}

// CHECK: vphraandq  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x38,0x4d,0x10]
          vphraandq  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vphraandqy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x4d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraandqy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraandqy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x4d,0x51,0x7f]
          vphraandqy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandq  -1024(%edx){1to4}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x3f,0x4d,0x52,0x80]
          vphraandq  -1024(%edx){1to4}, %xmm3, %xmm2 {%k7}

// CHECK: vphraandw %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x4d,0xd4]
          vphraandw %xmm4, %xmm3, %xmm2

// CHECK: vphraandw %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x4d,0xd4]
          vphraandw %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraandw %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x4d,0xd4]
          vphraandw %ymm4, %xmm3, %xmm2

// CHECK: vphraandw %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x4d,0xd4]
          vphraandw %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraandwx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandwx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraandwx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandwx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraandw  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x18,0x4d,0x10]
          vphraandw  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphraandwx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x4d,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraandwx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraandwx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x4d,0x51,0x7f]
          vphraandwx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandw  -256(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x1f,0x4d,0x52,0x80]
          vphraandw  -256(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphraandw  (%eax){1to16}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x38,0x4d,0x10]
          vphraandw  (%eax){1to16}, %xmm3, %xmm2

// CHECK: vphraandwy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x4d,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraandwy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraandwy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x4d,0x51,0x7f]
          vphraandwy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandw  -256(%edx){1to16}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x3f,0x4d,0x52,0x80]
          vphraandw  -256(%edx){1to16}, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsb %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4b,0xd4]
          vphramaxsb %xmm4, %xmm3, %xmm2

// CHECK: vphramaxsb %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4b,0xd4]
          vphramaxsb %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsb %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x4b,0xd4]
          vphramaxsb %ymm4, %xmm3, %xmm2

// CHECK: vphramaxsb %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x4b,0xd4]
          vphramaxsb %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsbx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsbx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphramaxsbx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsbx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsbx  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4b,0x10]
          vphramaxsbx  (%eax), %xmm3, %xmm2

// CHECK: vphramaxsbx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x4b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsbx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramaxsbx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4b,0x51,0x7f]
          vphramaxsbx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsbx  -2048(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x4b,0x52,0x80]
          vphramaxsbx  -2048(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsby  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x4b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsby  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramaxsby  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x4b,0x51,0x7f]
          vphramaxsby  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsby  -4096(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x4b,0x52,0x80]
          vphramaxsby  -4096(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsd %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x4b,0xd4]
          vphramaxsd %xmm4, %xmm3, %xmm2

// CHECK: vphramaxsd %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x4b,0xd4]
          vphramaxsd %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsd %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x4b,0xd4]
          vphramaxsd %ymm4, %xmm3, %xmm2

// CHECK: vphramaxsd %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x4b,0xd4]
          vphramaxsd %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsdx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsdx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphramaxsdx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsdx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsd  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x4b,0x10]
          vphramaxsd  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vphramaxsdx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x4b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsdx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramaxsdx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x4b,0x51,0x7f]
          vphramaxsdx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsd  -512(%edx){1to4}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x1f,0x4b,0x52,0x80]
          vphramaxsd  -512(%edx){1to4}, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsd  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x4b,0x10]
          vphramaxsd  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphramaxsdy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x4b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsdy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramaxsdy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x4b,0x51,0x7f]
          vphramaxsdy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsd  -512(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x3f,0x4b,0x52,0x80]
          vphramaxsd  -512(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x4b,0xd4]
          vphramaxsq %xmm4, %xmm3, %xmm2

// CHECK: vphramaxsq %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x4b,0xd4]
          vphramaxsq %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsq %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x4b,0xd4]
          vphramaxsq %ymm4, %xmm3, %xmm2

// CHECK: vphramaxsq %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x4b,0xd4]
          vphramaxsq %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsqx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsqx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphramaxsqx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsqx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsq  (%eax){1to2}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x18,0x4b,0x10]
          vphramaxsq  (%eax){1to2}, %xmm3, %xmm2

// CHECK: vphramaxsqx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x4b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsqx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramaxsqx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x4b,0x51,0x7f]
          vphramaxsqx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsq  -1024(%edx){1to2}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x1f,0x4b,0x52,0x80]
          vphramaxsq  -1024(%edx){1to2}, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsq  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x38,0x4b,0x10]
          vphramaxsq  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vphramaxsqy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x4b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsqy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramaxsqy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x4b,0x51,0x7f]
          vphramaxsqy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsq  -1024(%edx){1to4}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x3f,0x4b,0x52,0x80]
          vphramaxsq  -1024(%edx){1to4}, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsw %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x4b,0xd4]
          vphramaxsw %xmm4, %xmm3, %xmm2

// CHECK: vphramaxsw %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x4b,0xd4]
          vphramaxsw %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsw %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x4b,0xd4]
          vphramaxsw %ymm4, %xmm3, %xmm2

// CHECK: vphramaxsw %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x4b,0xd4]
          vphramaxsw %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxswx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxswx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphramaxswx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxswx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsw  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x18,0x4b,0x10]
          vphramaxsw  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphramaxswx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x4b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxswx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramaxswx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x4b,0x51,0x7f]
          vphramaxswx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsw  -256(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x1f,0x4b,0x52,0x80]
          vphramaxsw  -256(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsw  (%eax){1to16}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x38,0x4b,0x10]
          vphramaxsw  (%eax){1to16}, %xmm3, %xmm2

// CHECK: vphramaxswy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x4b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxswy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramaxswy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x4b,0x51,0x7f]
          vphramaxswy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsw  -256(%edx){1to16}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x3f,0x4b,0x52,0x80]
          vphramaxsw  -256(%edx){1to16}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminb %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x48,0xd4]
          vphraminb %xmm4, %xmm3, %xmm2

// CHECK: vphraminb %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x48,0xd4]
          vphraminb %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminb %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x48,0xd4]
          vphraminb %ymm4, %xmm3, %xmm2

// CHECK: vphraminb %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x48,0xd4]
          vphraminb %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminbx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminbx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminbx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminbx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminbx  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x48,0x10]
          vphraminbx  (%eax), %xmm3, %xmm2

// CHECK: vphraminbx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x48,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminbx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminbx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x48,0x51,0x7f]
          vphraminbx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminbx  -2048(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x48,0x52,0x80]
          vphraminbx  -2048(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminby  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x48,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminby  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminby  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x48,0x51,0x7f]
          vphraminby  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminby  -4096(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x48,0x52,0x80]
          vphraminby  -4096(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphramind %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x48,0xd4]
          vphramind %xmm4, %xmm3, %xmm2

// CHECK: vphramind %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x48,0xd4]
          vphramind %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramind %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x48,0xd4]
          vphramind %ymm4, %xmm3, %xmm2

// CHECK: vphramind %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x48,0xd4]
          vphramind %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramindx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramindx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphramindx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramindx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphramind  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x48,0x10]
          vphramind  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vphramindx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x48,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphramindx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramindx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x48,0x51,0x7f]
          vphramindx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramind  -512(%edx){1to4}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x1f,0x48,0x52,0x80]
          vphramind  -512(%edx){1to4}, %xmm3, %xmm2 {%k7}

// CHECK: vphramind  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x48,0x10]
          vphramind  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphramindy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x48,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphramindy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramindy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x48,0x51,0x7f]
          vphramindy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramind  -512(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x3f,0x48,0x52,0x80]
          vphramind  -512(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x48,0xd4]
          vphraminq %xmm4, %xmm3, %xmm2

// CHECK: vphraminq %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x48,0xd4]
          vphraminq %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminq %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x48,0xd4]
          vphraminq %ymm4, %xmm3, %xmm2

// CHECK: vphraminq %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x48,0xd4]
          vphraminq %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminqx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminqx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminqx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminqx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminq  (%eax){1to2}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x18,0x48,0x10]
          vphraminq  (%eax){1to2}, %xmm3, %xmm2

// CHECK: vphraminqx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x48,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminqx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminqx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x48,0x51,0x7f]
          vphraminqx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminq  -1024(%edx){1to2}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x1f,0x48,0x52,0x80]
          vphraminq  -1024(%edx){1to2}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminq  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x38,0x48,0x10]
          vphraminq  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vphraminqy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x48,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminqy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminqy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x48,0x51,0x7f]
          vphraminqy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminq  -1024(%edx){1to4}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x3f,0x48,0x52,0x80]
          vphraminq  -1024(%edx){1to4}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsb %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x49,0xd4]
          vphraminsb %xmm4, %xmm3, %xmm2

// CHECK: vphraminsb %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x49,0xd4]
          vphraminsb %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsb %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x49,0xd4]
          vphraminsb %ymm4, %xmm3, %xmm2

// CHECK: vphraminsb %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x49,0xd4]
          vphraminsb %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsbx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsbx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminsbx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsbx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsbx  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x49,0x10]
          vphraminsbx  (%eax), %xmm3, %xmm2

// CHECK: vphraminsbx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x08,0x49,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsbx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminsbx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x49,0x51,0x7f]
          vphraminsbx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsbx  -2048(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x0f,0x49,0x52,0x80]
          vphraminsbx  -2048(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsby  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x28,0x49,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsby  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminsby  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x49,0x51,0x7f]
          vphraminsby  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsby  -4096(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x2f,0x49,0x52,0x80]
          vphraminsby  -4096(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsd %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x49,0xd4]
          vphraminsd %xmm4, %xmm3, %xmm2

// CHECK: vphraminsd %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x49,0xd4]
          vphraminsd %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsd %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x49,0xd4]
          vphraminsd %ymm4, %xmm3, %xmm2

// CHECK: vphraminsd %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x49,0xd4]
          vphraminsd %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsdx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsdx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminsdx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsdx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsd  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x49,0x10]
          vphraminsd  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vphraminsdx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x49,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsdx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminsdx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x0f,0x49,0x51,0x7f]
          vphraminsdx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsd  -512(%edx){1to4}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x1f,0x49,0x52,0x80]
          vphraminsd  -512(%edx){1to4}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsd  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x49,0x10]
          vphraminsd  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphraminsdy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x49,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsdy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminsdy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x2f,0x49,0x51,0x7f]
          vphraminsdy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsd  -512(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x3f,0x49,0x52,0x80]
          vphraminsd  -512(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsq %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x49,0xd4]
          vphraminsq %xmm4, %xmm3, %xmm2

// CHECK: vphraminsq %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x49,0xd4]
          vphraminsq %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsq %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x49,0xd4]
          vphraminsq %ymm4, %xmm3, %xmm2

// CHECK: vphraminsq %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x49,0xd4]
          vphraminsq %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsqx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsqx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminsqx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsqx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsq  (%eax){1to2}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x18,0x49,0x10]
          vphraminsq  (%eax){1to2}, %xmm3, %xmm2

// CHECK: vphraminsqx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x08,0x49,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsqx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminsqx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x0f,0x49,0x51,0x7f]
          vphraminsqx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsq  -1024(%edx){1to2}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x1f,0x49,0x52,0x80]
          vphraminsq  -1024(%edx){1to2}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsq  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x38,0x49,0x10]
          vphraminsq  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vphraminsqy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x28,0x49,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsqy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminsqy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x2f,0x49,0x51,0x7f]
          vphraminsqy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsq  -1024(%edx){1to4}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x3f,0x49,0x52,0x80]
          vphraminsq  -1024(%edx){1to4}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsw %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x49,0xd4]
          vphraminsw %xmm4, %xmm3, %xmm2

// CHECK: vphraminsw %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x49,0xd4]
          vphraminsw %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsw %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x49,0xd4]
          vphraminsw %ymm4, %xmm3, %xmm2

// CHECK: vphraminsw %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x49,0xd4]
          vphraminsw %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminswx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminswx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminswx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminswx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsw  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x18,0x49,0x10]
          vphraminsw  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphraminswx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x49,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminswx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminswx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x49,0x51,0x7f]
          vphraminswx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsw  -256(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x1f,0x49,0x52,0x80]
          vphraminsw  -256(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsw  (%eax){1to16}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x38,0x49,0x10]
          vphraminsw  (%eax){1to16}, %xmm3, %xmm2

// CHECK: vphraminswy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x49,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminswy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminswy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x49,0x51,0x7f]
          vphraminswy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsw  -256(%edx){1to16}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x3f,0x49,0x52,0x80]
          vphraminsw  -256(%edx){1to16}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminw %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x48,0xd4]
          vphraminw %xmm4, %xmm3, %xmm2

// CHECK: vphraminw %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x48,0xd4]
          vphraminw %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminw %ymm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x48,0xd4]
          vphraminw %ymm4, %xmm3, %xmm2

// CHECK: vphraminw %ymm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x48,0xd4]
          vphraminw %ymm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminwx  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminwx  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminwx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminwx  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminw  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x18,0x48,0x10]
          vphraminw  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphraminwx  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x08,0x48,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vphraminwx  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminwx  2032(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x0f,0x48,0x51,0x7f]
          vphraminwx  2032(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminw  -256(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x1f,0x48,0x52,0x80]
          vphraminw  -256(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminw  (%eax){1to16}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x38,0x48,0x10]
          vphraminw  (%eax){1to16}, %xmm3, %xmm2

// CHECK: vphraminwy  -1024(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x28,0x48,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vphraminwy  -1024(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminwy  4064(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x2f,0x48,0x51,0x7f]
          vphraminwy  4064(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminw  -256(%edx){1to16}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x3f,0x48,0x52,0x80]
          vphraminw  -256(%edx){1to16}, %xmm3, %xmm2 {%k7}

