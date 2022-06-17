// REQUIRES: intel_feature_isa_avx512_complex
// RUN: llvm-mc -triple i386 --show-encoding %s | FileCheck %s

// CHECK: vaddsubpd %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x48,0xd0,0xd4]
          vaddsubpd %zmm4, %zmm3, %zmm2

// CHECK: vaddsubpd {rn-sae}, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x18,0xd0,0xd4]
          vaddsubpd {rn-sae}, %zmm4, %zmm3, %zmm2

// CHECK: vaddsubpd %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0xe5,0x4f,0xd0,0xd4]
          vaddsubpd %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vaddsubpd {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0xff,0xd0,0xd4]
          vaddsubpd {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vaddsubpd  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x48,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubpd  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vaddsubpd  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0xe5,0x4f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubpd  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vaddsubpd  (%eax){1to8}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x58,0xd0,0x10]
          vaddsubpd  (%eax){1to8}, %zmm3, %zmm2

// CHECK: vaddsubpd  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x48,0xd0,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vaddsubpd  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vaddsubpd  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0xcf,0xd0,0x51,0x7f]
          vaddsubpd  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vaddsubpd  -1024(%edx){1to8}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0xdf,0xd0,0x52,0x80]
          vaddsubpd  -1024(%edx){1to8}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vaddsubph %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x48,0xd0,0xd4]
          vaddsubph %zmm4, %zmm3, %zmm2

// CHECK: vaddsubph {rn-sae}, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xd0,0xd4]
          vaddsubph {rn-sae}, %zmm4, %zmm3, %zmm2

// CHECK: vaddsubph %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x4f,0xd0,0xd4]
          vaddsubph %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vaddsubph {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xff,0xd0,0xd4]
          vaddsubph {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vaddsubph  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x48,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubph  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vaddsubph  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x4f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubph  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vaddsubph  (%eax){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x58,0xd0,0x10]
          vaddsubph  (%eax){1to32}, %zmm3, %zmm2

// CHECK: vaddsubph  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x48,0xd0,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vaddsubph  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vaddsubph  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xcf,0xd0,0x51,0x7f]
          vaddsubph  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vaddsubph  -256(%edx){1to32}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xdf,0xd0,0x52,0x80]
          vaddsubph  -256(%edx){1to32}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vaddsubps %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x48,0xd0,0xd4]
          vaddsubps %zmm4, %zmm3, %zmm2

// CHECK: vaddsubps {rn-sae}, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x18,0xd0,0xd4]
          vaddsubps {rn-sae}, %zmm4, %zmm3, %zmm2

// CHECK: vaddsubps %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x65,0x4f,0xd0,0xd4]
          vaddsubps %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vaddsubps {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0xff,0xd0,0xd4]
          vaddsubps {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vaddsubps  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x48,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          vaddsubps  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vaddsubps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x65,0x4f,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          vaddsubps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vaddsubps  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x58,0xd0,0x10]
          vaddsubps  (%eax){1to16}, %zmm3, %zmm2

// CHECK: vaddsubps  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x48,0xd0,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vaddsubps  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vaddsubps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0xcf,0xd0,0x51,0x7f]
          vaddsubps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vaddsubps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0xdf,0xd0,0x52,0x80]
          vaddsubps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vmovdhdup %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf1,0xff,0x48,0x16,0xd3]
          vmovdhdup %zmm3, %zmm2

// CHECK: vmovdhdup %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf1,0xff,0x4f,0x16,0xd3]
          vmovdhdup %zmm3, %zmm2 {%k7}

// CHECK: vmovdhdup %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf1,0xff,0xcf,0x16,0xd3]
          vmovdhdup %zmm3, %zmm2 {%k7} {z}

// CHECK: vmovdhdup  268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf1,0xff,0x48,0x16,0x94,0xf4,0x00,0x00,0x00,0x10]
          vmovdhdup  268435456(%esp,%esi,8), %zmm2

// CHECK: vmovdhdup  291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf1,0xff,0x4f,0x16,0x94,0x87,0x23,0x01,0x00,0x00]
          vmovdhdup  291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vmovdhdup  (%eax), %zmm2
// CHECK: encoding: [0x62,0xf1,0xff,0x48,0x16,0x10]
          vmovdhdup  (%eax), %zmm2

// CHECK: vmovdhdup  -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf1,0xff,0x48,0x16,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vmovdhdup  -2048(,%ebp,2), %zmm2

// CHECK: vmovdhdup  8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf1,0xff,0xcf,0x16,0x51,0x7f]
          vmovdhdup  8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vmovdhdup  -8192(%edx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf1,0xff,0xcf,0x16,0x52,0x80]
          vmovdhdup  -8192(%edx), %zmm2 {%k7} {z}

// CHECK: vsubaddpd %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x48,0xd1,0xd4]
          vsubaddpd %zmm4, %zmm3, %zmm2

// CHECK: vsubaddpd {rn-sae}, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x18,0xd1,0xd4]
          vsubaddpd {rn-sae}, %zmm4, %zmm3, %zmm2

// CHECK: vsubaddpd %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0xe5,0x4f,0xd1,0xd4]
          vsubaddpd %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vsubaddpd {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0xff,0xd1,0xd4]
          vsubaddpd {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vsubaddpd  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x48,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddpd  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vsubaddpd  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0xe5,0x4f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddpd  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vsubaddpd  (%eax){1to8}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x58,0xd1,0x10]
          vsubaddpd  (%eax){1to8}, %zmm3, %zmm2

// CHECK: vsubaddpd  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0xe5,0x48,0xd1,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vsubaddpd  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vsubaddpd  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0xcf,0xd1,0x51,0x7f]
          vsubaddpd  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vsubaddpd  -1024(%edx){1to8}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0xe5,0xdf,0xd1,0x52,0x80]
          vsubaddpd  -1024(%edx){1to8}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vsubaddph %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x48,0xd1,0xd4]
          vsubaddph %zmm4, %zmm3, %zmm2

// CHECK: vsubaddph {rn-sae}, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x18,0xd1,0xd4]
          vsubaddph {rn-sae}, %zmm4, %zmm3, %zmm2

// CHECK: vsubaddph %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x4f,0xd1,0xd4]
          vsubaddph %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vsubaddph {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xff,0xd1,0xd4]
          vsubaddph {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vsubaddph  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x48,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddph  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vsubaddph  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x64,0x4f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddph  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vsubaddph  (%eax){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x58,0xd1,0x10]
          vsubaddph  (%eax){1to32}, %zmm3, %zmm2

// CHECK: vsubaddph  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x64,0x48,0xd1,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vsubaddph  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vsubaddph  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xcf,0xd1,0x51,0x7f]
          vsubaddph  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vsubaddph  -256(%edx){1to32}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x64,0xdf,0xd1,0x52,0x80]
          vsubaddph  -256(%edx){1to32}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vsubaddps %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x48,0xd1,0xd4]
          vsubaddps %zmm4, %zmm3, %zmm2

// CHECK: vsubaddps {rn-sae}, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x18,0xd1,0xd4]
          vsubaddps {rn-sae}, %zmm4, %zmm3, %zmm2

// CHECK: vsubaddps %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x65,0x4f,0xd1,0xd4]
          vsubaddps %zmm4, %zmm3, %zmm2 {%k7}

// CHECK: vsubaddps {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0xff,0xd1,0xd4]
          vsubaddps {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK: vsubaddps  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x48,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          vsubaddps  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vsubaddps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf6,0x65,0x4f,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          vsubaddps  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK: vsubaddps  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x58,0xd1,0x10]
          vsubaddps  (%eax){1to16}, %zmm3, %zmm2

// CHECK: vsubaddps  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf6,0x65,0x48,0xd1,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vsubaddps  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vsubaddps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0xcf,0xd1,0x51,0x7f]
          vsubaddps  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK: vsubaddps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf6,0x65,0xdf,0xd1,0x52,0x80]
          vsubaddps  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

