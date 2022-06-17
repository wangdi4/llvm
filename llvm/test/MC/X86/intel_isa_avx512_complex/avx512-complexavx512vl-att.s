// REQUIRES: intel_feature_isa_avx512_complex
// RUN: llvm-mc -triple i386 --show-encoding %s | FileCheck %s

// CHECK: vaddsubpd %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0xc5,0xe5,0xd0,0xd4]
          vaddsubpd %ymm4, %ymm3, %ymm2

// CHECK: vaddsubpd %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0xe5,0x2f,0xd0,0xd4]
          vaddsubpd %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vaddsubpd %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0xaf,0xd0,0xd4]
          vaddsubpd %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vaddsubpd %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc5,0xe1,0xd0,0xd4]
          vaddsubpd %xmm4, %xmm3, %xmm2

// CHECK: vaddsubpd %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0xe5,0x0f,0xd0,0xd4]
          vaddsubpd %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vaddsubpd %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0x8f,0xd0,0xd4]
          vaddsubpd %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vaddsubpd  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0xc5,0xe5,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubpd  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vaddsubpd  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0xe5,0x2f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubpd  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vaddsubpd  (%eax){1to4}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x38,0xd0,0x10]
          vaddsubpd  (%eax){1to4}, %ymm3, %ymm2

// CHECK: vaddsubpd  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0xc5,0xe5,0xd0,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vaddsubpd  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vaddsubpd  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0xaf,0xd0,0x51,0x7f]
          vaddsubpd  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vaddsubpd  -1024(%edx){1to4}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0xbf,0xd0,0x52,0x80]
          vaddsubpd  -1024(%edx){1to4}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vaddsubpd  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc5,0xe1,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubpd  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vaddsubpd  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0xe5,0x0f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubpd  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vaddsubpd  (%eax){1to2}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x18,0xd0,0x10]
          vaddsubpd  (%eax){1to2}, %xmm3, %xmm2

// CHECK: vaddsubpd  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc5,0xe1,0xd0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vaddsubpd  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vaddsubpd  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0x8f,0xd0,0x51,0x7f]
          vaddsubpd  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vaddsubpd  -1024(%edx){1to2}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0x9f,0xd0,0x52,0x80]
          vaddsubpd  -1024(%edx){1to2}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vaddsubph %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xd0,0xd4]
          vaddsubph %ymm4, %ymm3, %ymm2

// CHECK: vaddsubph %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xd0,0xd4]
          vaddsubph %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vaddsubph %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xd0,0xd4]
          vaddsubph %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vaddsubph %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xd0,0xd4]
          vaddsubph %xmm4, %xmm3, %xmm2

// CHECK: vaddsubph %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xd0,0xd4]
          vaddsubph %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vaddsubph %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xd0,0xd4]
          vaddsubph %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vaddsubph  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubph  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vaddsubph  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubph  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vaddsubph  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0xd0,0x10]
          vaddsubph  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vaddsubph  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xd0,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vaddsubph  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vaddsubph  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xd0,0x51,0x7f]
          vaddsubph  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vaddsubph  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0xd0,0x52,0x80]
          vaddsubph  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vaddsubph  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubph  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vaddsubph  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubph  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vaddsubph  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xd0,0x10]
          vaddsubph  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vaddsubph  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xd0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vaddsubph  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vaddsubph  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xd0,0x51,0x7f]
          vaddsubph  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vaddsubph  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0xd0,0x52,0x80]
          vaddsubph  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vaddsubps %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0xc5,0xe7,0xd0,0xd4]
          vaddsubps %ymm4, %ymm3, %ymm2

// CHECK: vaddsubps %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x65,0x2f,0xd0,0xd4]
          vaddsubps %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vaddsubps %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0xaf,0xd0,0xd4]
          vaddsubps %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vaddsubps %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc5,0xe3,0xd0,0xd4]
          vaddsubps %xmm4, %xmm3, %xmm2

// CHECK: vaddsubps %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x65,0x0f,0xd0,0xd4]
          vaddsubps %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vaddsubps %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0x8f,0xd0,0xd4]
          vaddsubps %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vaddsubps  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0xc5,0xe7,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubps  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vaddsubps  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x65,0x2f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubps  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vaddsubps  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x65,0x38,0xd0,0x10]
          vaddsubps  (%eax){1to8}, %ymm3, %ymm2

// CHECK: vaddsubps  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0xc5,0xe7,0xd0,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vaddsubps  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vaddsubps  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0xaf,0xd0,0x51,0x7f]
          vaddsubps  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vaddsubps  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0xbf,0xd0,0x52,0x80]
          vaddsubps  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vaddsubps  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc5,0xe3,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubps  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vaddsubps  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x65,0x0f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubps  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vaddsubps  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x18,0xd0,0x10]
          vaddsubps  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vaddsubps  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc5,0xe3,0xd0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vaddsubps  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vaddsubps  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0x8f,0xd0,0x51,0x7f]
          vaddsubps  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vaddsubps  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0x9f,0xd0,0x52,0x80]
          vaddsubps  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vmovdhdup %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf1,0xff,0x08,0x16,0xd3]
          vmovdhdup %xmm3, %xmm2

// CHECK: vmovdhdup %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf1,0xff,0x0f,0x16,0xd3]
          vmovdhdup %xmm3, %xmm2 {%k7}

// CHECK: vmovdhdup %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf1,0xff,0x8f,0x16,0xd3]
          vmovdhdup %xmm3, %xmm2 {%k7} {z}

// CHECK: vmovdhdup %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf1,0xff,0x28,0x16,0xd3]
          vmovdhdup %ymm3, %ymm2

// CHECK: vmovdhdup %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf1,0xff,0x2f,0x16,0xd3]
          vmovdhdup %ymm3, %ymm2 {%k7}

// CHECK: vmovdhdup %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf1,0xff,0xaf,0x16,0xd3]
          vmovdhdup %ymm3, %ymm2 {%k7} {z}

// CHECK: vmovdhdup  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf1,0xff,0x08,0x16,0x94,0xf4,0x00,0x00,0x00,0x10]
          vmovdhdup  268435456(%esp,%esi,8), %xmm2

// CHECK: vmovdhdup  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf1,0xff,0x0f,0x16,0x94,0x87,0x23,0x01,0x00,0x00]
          vmovdhdup  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vmovdhdup  (%eax), %xmm2
// CHECK: encoding: [0x62,0xf1,0xff,0x08,0x16,0x10]
          vmovdhdup  (%eax), %xmm2

// CHECK: vmovdhdup  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf1,0xff,0x08,0x16,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vmovdhdup  -512(,%ebp,2), %xmm2

// CHECK: vmovdhdup  1016(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf1,0xff,0x8f,0x16,0x51,0x7f]
          vmovdhdup  1016(%ecx), %xmm2 {%k7} {z}

// CHECK: vmovdhdup  -1024(%edx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf1,0xff,0x8f,0x16,0x52,0x80]
          vmovdhdup  -1024(%edx), %xmm2 {%k7} {z}

// CHECK: vmovdhdup  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf1,0xff,0x28,0x16,0x94,0xf4,0x00,0x00,0x00,0x10]
          vmovdhdup  268435456(%esp,%esi,8), %ymm2

// CHECK: vmovdhdup  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf1,0xff,0x2f,0x16,0x94,0x87,0x23,0x01,0x00,0x00]
          vmovdhdup  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vmovdhdup  (%eax), %ymm2
// CHECK: encoding: [0x62,0xf1,0xff,0x28,0x16,0x10]
          vmovdhdup  (%eax), %ymm2

// CHECK: vmovdhdup  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf1,0xff,0x28,0x16,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vmovdhdup  -1024(,%ebp,2), %ymm2

// CHECK: vmovdhdup  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf1,0xff,0xaf,0x16,0x51,0x7f]
          vmovdhdup  4064(%ecx), %ymm2 {%k7} {z}

// CHECK: vmovdhdup  -4096(%edx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf1,0xff,0xaf,0x16,0x52,0x80]
          vmovdhdup  -4096(%edx), %ymm2 {%k7} {z}

// CHECK: vsubaddpd %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x28,0xd1,0xd4]
          vsubaddpd %ymm4, %ymm3, %ymm2

// CHECK: vsubaddpd %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0xe5,0x2f,0xd1,0xd4]
          vsubaddpd %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vsubaddpd %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0xaf,0xd1,0xd4]
          vsubaddpd %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vsubaddpd %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x08,0xd1,0xd4]
          vsubaddpd %xmm4, %xmm3, %xmm2

// CHECK: vsubaddpd %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0xe5,0x0f,0xd1,0xd4]
          vsubaddpd %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vsubaddpd %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0x8f,0xd1,0xd4]
          vsubaddpd %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vsubaddpd  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x28,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddpd  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vsubaddpd  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0xe5,0x2f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddpd  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vsubaddpd  (%eax){1to4}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x38,0xd1,0x10]
          vsubaddpd  (%eax){1to4}, %ymm3, %ymm2

// CHECK: vsubaddpd  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x28,0xd1,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vsubaddpd  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vsubaddpd  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0xaf,0xd1,0x51,0x7f]
          vsubaddpd  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vsubaddpd  -1024(%edx){1to4}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0xbf,0xd1,0x52,0x80]
          vsubaddpd  -1024(%edx){1to4}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vsubaddpd  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x08,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddpd  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vsubaddpd  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0xe5,0x0f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddpd  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vsubaddpd  (%eax){1to2}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x18,0xd1,0x10]
          vsubaddpd  (%eax){1to2}, %xmm3, %xmm2

// CHECK: vsubaddpd  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x08,0xd1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vsubaddpd  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vsubaddpd  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0x8f,0xd1,0x51,0x7f]
          vsubaddpd  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vsubaddpd  -1024(%edx){1to2}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0x9f,0xd1,0x52,0x80]
          vsubaddpd  -1024(%edx){1to2}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vsubaddph %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xd1,0xd4]
          vsubaddph %ymm4, %ymm3, %ymm2

// CHECK: vsubaddph %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xd1,0xd4]
          vsubaddph %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vsubaddph %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xd1,0xd4]
          vsubaddph %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vsubaddph %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xd1,0xd4]
          vsubaddph %xmm4, %xmm3, %xmm2

// CHECK: vsubaddph %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xd1,0xd4]
          vsubaddph %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vsubaddph %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xd1,0xd4]
          vsubaddph %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vsubaddph  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddph  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vsubaddph  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x2f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddph  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vsubaddph  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x38,0xd1,0x10]
          vsubaddph  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vsubaddph  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x64,0x28,0xd1,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vsubaddph  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vsubaddph  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xaf,0xd1,0x51,0x7f]
          vsubaddph  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vsubaddph  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xbf,0xd1,0x52,0x80]
          vsubaddph  -256(%edx){1to16}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vsubaddph  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddph  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vsubaddph  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x0f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddph  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vsubaddph  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xd1,0x10]
          vsubaddph  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vsubaddph  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x08,0xd1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vsubaddph  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vsubaddph  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x8f,0xd1,0x51,0x7f]
          vsubaddph  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vsubaddph  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0x9f,0xd1,0x52,0x80]
          vsubaddph  -256(%edx){1to8}, %xmm3, %xmm2 {%k7} {z}

// CHECK: vsubaddps %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x65,0x28,0xd1,0xd4]
          vsubaddps %ymm4, %ymm3, %ymm2

// CHECK: vsubaddps %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x65,0x2f,0xd1,0xd4]
          vsubaddps %ymm4, %ymm3, %ymm2 {%k7}

// CHECK: vsubaddps %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0xaf,0xd1,0xd4]
          vsubaddps %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK: vsubaddps %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x08,0xd1,0xd4]
          vsubaddps %xmm4, %xmm3, %xmm2

// CHECK: vsubaddps %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x65,0x0f,0xd1,0xd4]
          vsubaddps %xmm4, %xmm3, %xmm2 {%k7}

// CHECK: vsubaddps %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0x8f,0xd1,0xd4]
          vsubaddps %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK: vsubaddps  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x65,0x28,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddps  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vsubaddps  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x65,0x2f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddps  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK: vsubaddps  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x65,0x38,0xd1,0x10]
          vsubaddps  (%eax){1to8}, %ymm3, %ymm2

// CHECK: vsubaddps  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf6,0x65,0x28,0xd1,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vsubaddps  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vsubaddps  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0xaf,0xd1,0x51,0x7f]
          vsubaddps  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK: vsubaddps  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0xbf,0xd1,0x52,0x80]
          vsubaddps  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK: vsubaddps  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x08,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddps  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vsubaddps  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x65,0x0f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddps  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK: vsubaddps  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x18,0xd1,0x10]
          vsubaddps  (%eax){1to4}, %xmm3, %xmm2

// CHECK: vsubaddps  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x08,0xd1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vsubaddps  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vsubaddps  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0x8f,0xd1,0x51,0x7f]
          vsubaddps  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK: vsubaddps  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0x9f,0xd1,0x52,0x80]
          vsubaddps  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

