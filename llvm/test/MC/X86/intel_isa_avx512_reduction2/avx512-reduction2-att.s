// REQUIRES: intel_feature_isa_avx512_reduction2
// RUN: llvm-mc -triple i386 --show-encoding %s | FileCheck %s

// CHECK: vphraaddbd %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x43,0xd4]
          vphraaddbd %zmm4, %xmm3, %xmm2

// CHECK: vphraaddbd %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x43,0xd4]
          vphraaddbd %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddbdz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddbdz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraaddbdz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddbdz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddbdz  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x43,0x10]
          vphraaddbdz  (%eax), %xmm3, %xmm2

// CHECK: vphraaddbdz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x43,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddbdz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraaddbdz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x43,0x51,0x7f]
          vphraaddbdz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddbdz  -8192(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x43,0x52,0x80]
          vphraaddbdz  -8192(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddsbd %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x44,0xd4]
          vphraaddsbd %zmm4, %xmm3, %xmm2

// CHECK: vphraaddsbd %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x44,0xd4]
          vphraaddsbd %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddsbdz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddsbdz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraaddsbdz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddsbdz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddsbdz  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x44,0x10]
          vphraaddsbdz  (%eax), %xmm3, %xmm2

// CHECK: vphraaddsbdz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x44,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddsbdz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraaddsbdz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x44,0x51,0x7f]
          vphraaddsbdz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddsbdz  -8192(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x44,0x52,0x80]
          vphraaddsbdz  -8192(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddswd %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x44,0xd4]
          vphraaddswd %zmm4, %xmm3, %xmm2

// CHECK: vphraaddswd %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x44,0xd4]
          vphraaddswd %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddswdz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x44,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddswdz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraaddswdz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x44,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddswdz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddswd  (%eax){1to32}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x58,0x44,0x10]
          vphraaddswd  (%eax){1to32}, %xmm3, %xmm2

// CHECK: vphraaddswdz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x44,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddswdz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraaddswdz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x44,0x51,0x7f]
          vphraaddswdz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddswd  -256(%edx){1to32}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x5f,0x44,0x52,0x80]
          vphraaddswd  -256(%edx){1to32}, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddwd %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x43,0xd4]
          vphraaddwd %zmm4, %xmm3, %xmm2

// CHECK: vphraaddwd %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x43,0xd4]
          vphraaddwd %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraaddwdz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x43,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraaddwdz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraaddwdz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x43,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraaddwdz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddwd  (%eax){1to32}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x58,0x43,0x10]
          vphraaddwd  (%eax){1to32}, %xmm3, %xmm2

// CHECK: vphraaddwdz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x43,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddwdz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraaddwdz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x43,0x51,0x7f]
          vphraaddwdz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraaddwd  -256(%edx){1to32}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x5f,0x43,0x52,0x80]
          vphraaddwd  -256(%edx){1to32}, %xmm3, %xmm2 {%k7}

// CHECK: vphraandb %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4d,0xd4]
          vphraandb %zmm4, %xmm3, %xmm2

// CHECK: vphraandb %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4d,0xd4]
          vphraandb %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraandbz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandbz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraandbz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandbz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraandbz  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4d,0x10]
          vphraandbz  (%eax), %xmm3, %xmm2

// CHECK: vphraandbz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4d,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraandbz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraandbz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4d,0x51,0x7f]
          vphraandbz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandbz  -8192(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4d,0x52,0x80]
          vphraandbz  -8192(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandd %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x4d,0xd4]
          vphraandd %zmm4, %xmm3, %xmm2

// CHECK: vphraandd %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x4d,0xd4]
          vphraandd %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraanddz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraanddz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraanddz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraanddz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraandd  (%eax){1to16}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x4d,0x10]
          vphraandd  (%eax){1to16}, %xmm3, %xmm2

// CHECK: vphraanddz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x4d,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraanddz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraanddz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x4d,0x51,0x7f]
          vphraanddz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandd  -512(%edx){1to16}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x5f,0x4d,0x52,0x80]
          vphraandd  -512(%edx){1to16}, %xmm3, %xmm2 {%k7}

// CHECK: vphraandq %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x4d,0xd4]
          vphraandq %zmm4, %xmm3, %xmm2

// CHECK: vphraandq %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x4d,0xd4]
          vphraandq %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraandqz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandqz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraandqz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandqz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraandq  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x58,0x4d,0x10]
          vphraandq  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphraandqz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x4d,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraandqz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraandqz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x4d,0x51,0x7f]
          vphraandqz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandq  -1024(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x5f,0x4d,0x52,0x80]
          vphraandq  -1024(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphraandw %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x4d,0xd4]
          vphraandw %zmm4, %xmm3, %xmm2

// CHECK: vphraandw %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x4d,0xd4]
          vphraandw %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraandwz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x4d,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraandwz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraandwz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x4d,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraandwz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraandw  (%eax){1to32}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x58,0x4d,0x10]
          vphraandw  (%eax){1to32}, %xmm3, %xmm2

// CHECK: vphraandwz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x4d,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraandwz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraandwz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x4d,0x51,0x7f]
          vphraandwz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraandw  -256(%edx){1to32}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x5f,0x4d,0x52,0x80]
          vphraandw  -256(%edx){1to32}, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsb %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4b,0xd4]
          vphramaxsb %zmm4, %xmm3, %xmm2

// CHECK: vphramaxsb %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4b,0xd4]
          vphramaxsb %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsbz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsbz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphramaxsbz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsbz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsbz  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4b,0x10]
          vphramaxsbz  (%eax), %xmm3, %xmm2

// CHECK: vphramaxsbz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x4b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsbz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramaxsbz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4b,0x51,0x7f]
          vphramaxsbz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsbz  -8192(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x4b,0x52,0x80]
          vphramaxsbz  -8192(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsd %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x4b,0xd4]
          vphramaxsd %zmm4, %xmm3, %xmm2

// CHECK: vphramaxsd %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x4b,0xd4]
          vphramaxsd %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsdz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsdz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphramaxsdz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsdz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsd  (%eax){1to16}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x4b,0x10]
          vphramaxsd  (%eax){1to16}, %xmm3, %xmm2

// CHECK: vphramaxsdz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x4b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsdz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramaxsdz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x4b,0x51,0x7f]
          vphramaxsdz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsd  -512(%edx){1to16}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x5f,0x4b,0x52,0x80]
          vphramaxsd  -512(%edx){1to16}, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsq %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x4b,0xd4]
          vphramaxsq %zmm4, %xmm3, %xmm2

// CHECK: vphramaxsq %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x4b,0xd4]
          vphramaxsq %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsqz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxsqz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphramaxsqz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxsqz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsq  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x58,0x4b,0x10]
          vphramaxsq  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphramaxsqz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x4b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsqz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramaxsqz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x4b,0x51,0x7f]
          vphramaxsqz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsq  -1024(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x5f,0x4b,0x52,0x80]
          vphramaxsq  -1024(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsw %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x4b,0xd4]
          vphramaxsw %zmm4, %xmm3, %xmm2

// CHECK: vphramaxsw %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x4b,0xd4]
          vphramaxsw %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramaxswz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x4b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramaxswz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphramaxswz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x4b,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramaxswz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsw  (%eax){1to32}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x58,0x4b,0x10]
          vphramaxsw  (%eax){1to32}, %xmm3, %xmm2

// CHECK: vphramaxswz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x4b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxswz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramaxswz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x4b,0x51,0x7f]
          vphramaxswz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramaxsw  -256(%edx){1to32}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x5f,0x4b,0x52,0x80]
          vphramaxsw  -256(%edx){1to32}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminb %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x48,0xd4]
          vphraminb %zmm4, %xmm3, %xmm2

// CHECK: vphraminb %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x48,0xd4]
          vphraminb %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminbz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminbz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminbz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminbz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminbz  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x48,0x10]
          vphraminbz  (%eax), %xmm3, %xmm2

// CHECK: vphraminbz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x48,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminbz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminbz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x48,0x51,0x7f]
          vphraminbz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminbz  -8192(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x48,0x52,0x80]
          vphraminbz  -8192(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphramind %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x48,0xd4]
          vphramind %zmm4, %xmm3, %xmm2

// CHECK: vphramind %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x48,0xd4]
          vphramind %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphramindz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphramindz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphramindz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphramindz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphramind  (%eax){1to16}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x48,0x10]
          vphramind  (%eax){1to16}, %xmm3, %xmm2

// CHECK: vphramindz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x48,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphramindz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphramindz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x48,0x51,0x7f]
          vphramindz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphramind  -512(%edx){1to16}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x5f,0x48,0x52,0x80]
          vphramind  -512(%edx){1to16}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminq %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x48,0xd4]
          vphraminq %zmm4, %xmm3, %xmm2

// CHECK: vphraminq %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x48,0xd4]
          vphraminq %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminqz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminqz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminqz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminqz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminq  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x58,0x48,0x10]
          vphraminq  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphraminqz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x48,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminqz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminqz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x48,0x51,0x7f]
          vphraminqz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminq  -1024(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x5f,0x48,0x52,0x80]
          vphraminq  -1024(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsb %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x49,0xd4]
          vphraminsb %zmm4, %xmm3, %xmm2

// CHECK: vphraminsb %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x49,0xd4]
          vphraminsb %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsbz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsbz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminsbz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsbz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsbz  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x49,0x10]
          vphraminsbz  (%eax), %xmm3, %xmm2

// CHECK: vphraminsbz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x66,0x48,0x49,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsbz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminsbz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x49,0x51,0x7f]
          vphraminsbz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsbz  -8192(%edx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x66,0x4f,0x49,0x52,0x80]
          vphraminsbz  -8192(%edx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsd %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x49,0xd4]
          vphraminsd %zmm4, %xmm3, %xmm2

// CHECK: vphraminsd %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x49,0xd4]
          vphraminsd %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsdz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsdz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminsdz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsdz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsd  (%eax){1to16}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x49,0x10]
          vphraminsd  (%eax){1to16}, %xmm3, %xmm2

// CHECK: vphraminsdz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x49,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsdz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminsdz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x4f,0x49,0x51,0x7f]
          vphraminsdz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsd  -512(%edx){1to16}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x67,0x5f,0x49,0x52,0x80]
          vphraminsd  -512(%edx){1to16}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsq %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x49,0xd4]
          vphraminsq %zmm4, %xmm3, %xmm2

// CHECK: vphraminsq %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x49,0xd4]
          vphraminsq %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsqz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminsqz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminsqz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminsqz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsq  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x58,0x49,0x10]
          vphraminsq  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vphraminsqz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe7,0x48,0x49,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsqz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminsqz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x4f,0x49,0x51,0x7f]
          vphraminsqz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsq  -1024(%edx){1to8}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe7,0x5f,0x49,0x52,0x80]
          vphraminsq  -1024(%edx){1to8}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminsw %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x49,0xd4]
          vphraminsw %zmm4, %xmm3, %xmm2

// CHECK: vphraminsw %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x49,0xd4]
          vphraminsw %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminswz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x49,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminswz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminswz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x49,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminswz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsw  (%eax){1to32}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x58,0x49,0x10]
          vphraminsw  (%eax){1to32}, %xmm3, %xmm2

// CHECK: vphraminswz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x49,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminswz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminswz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x49,0x51,0x7f]
          vphraminswz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminsw  -256(%edx){1to32}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x5f,0x49,0x52,0x80]
          vphraminsw  -256(%edx){1to32}, %xmm3, %xmm2 {%k7}

// CHECK: vphraminw %zmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x48,0xd4]
          vphraminw %zmm4, %xmm3, %xmm2

// CHECK: vphraminw %zmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x48,0xd4]
          vphraminw %zmm4, %xmm3, %xmm2 {%k7}

// CHECK: vphraminwz  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x48,0x94,0xf4,0x00,0x00,0x00,0x10]
          vphraminwz  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vphraminwz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x48,0x94,0x87,0x23,0x01,0x00,0x00]
          vphraminwz  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vphraminw  (%eax){1to32}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x58,0x48,0x10]
          vphraminw  (%eax){1to32}, %xmm3, %xmm2

// CHECK: vphraminwz  -2048(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0xe6,0x48,0x48,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vphraminwz  -2048(,%ebp,2), %xmm3, %xmm2

// CHECK: vphraminwz  8128(%ecx), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x4f,0x48,0x51,0x7f]
          vphraminwz  8128(%ecx), %xmm3, %xmm2 {%k7}

// CHECK: vphraminw  -256(%edx){1to32}, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0xe6,0x5f,0x48,0x52,0x80]
          vphraminw  -256(%edx){1to32}, %xmm3, %xmm2 {%k7}

