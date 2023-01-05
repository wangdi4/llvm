// REQUIRES: intel_feature_isa_avx512_ne_convert_fp8
// RUN: llvm-mc -triple i386 --show-encoding %s | FileCheck %s

// CHECK: vcvtbias2ph2bf8 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x74,0xd4]
          vcvtbias2ph2bf8 %ymm4, %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x74,0xd4]
          vcvtbias2ph2bf8 %xmm4, %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8  291(%edi,%eax,4), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8  291(%edi,%eax,4), %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x38,0x74,0x10]
          vcvtbias2ph2bf8  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2bf8  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8  4064(%ecx), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x74,0x51,0x7f]
          vcvtbias2ph2bf8  4064(%ecx), %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8  -256(%edx){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x38,0x74,0x52,0x80]
          vcvtbias2ph2bf8  -256(%edx){1to16}, %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0x74,0x10]
          vcvtbias2ph2bf8  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2bf8  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x74,0x51,0x7f]
          vcvtbias2ph2bf8  2032(%ecx), %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8  -256(%edx){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0x74,0x52,0x80]
          vcvtbias2ph2bf8  -256(%edx){1to8}, %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8s %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x74,0xd4]
          vcvtbias2ph2bf8s %ymm4, %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8s %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x74,0xd4]
          vcvtbias2ph2bf8s %xmm4, %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8s  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8s  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8s  291(%edi,%eax,4), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8s  291(%edi,%eax,4), %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8s  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x74,0x10]
          vcvtbias2ph2bf8s  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8s  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2bf8s  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8s  4064(%ecx), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x74,0x51,0x7f]
          vcvtbias2ph2bf8s  4064(%ecx), %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8s  -256(%edx){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x74,0x52,0x80]
          vcvtbias2ph2bf8s  -256(%edx){1to16}, %ymm3, %ymm2

// CHECK: vcvtbias2ph2bf8s  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8s  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8s  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8s  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8s  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x74,0x10]
          vcvtbias2ph2bf8s  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8s  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2bf8s  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8s  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x74,0x51,0x7f]
          vcvtbias2ph2bf8s  2032(%ecx), %xmm3, %xmm2

// CHECK: vcvtbias2ph2bf8s  -256(%edx){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x74,0x52,0x80]
          vcvtbias2ph2bf8s  -256(%edx){1to8}, %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x18,0xd4]
          vcvtbias2ph2hf8 %ymm4, %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x18,0xd4]
          vcvtbias2ph2hf8 %xmm4, %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8  291(%edi,%eax,4), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8  291(%edi,%eax,4), %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x18,0x10]
          vcvtbias2ph2hf8  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x18,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2hf8  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8  4064(%ecx), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x18,0x51,0x7f]
          vcvtbias2ph2hf8  4064(%ecx), %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8  -256(%edx){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x18,0x52,0x80]
          vcvtbias2ph2hf8  -256(%edx){1to16}, %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x18,0x10]
          vcvtbias2ph2hf8  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x18,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2hf8  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x18,0x51,0x7f]
          vcvtbias2ph2hf8  2032(%ecx), %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8  -256(%edx){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x18,0x52,0x80]
          vcvtbias2ph2hf8  -256(%edx){1to8}, %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8s %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x1b,0xd4]
          vcvtbias2ph2hf8s %ymm4, %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8s %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x1b,0xd4]
          vcvtbias2ph2hf8s %xmm4, %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8s  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8s  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8s  291(%edi,%eax,4), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8s  291(%edi,%eax,4), %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8s  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x1b,0x10]
          vcvtbias2ph2hf8s  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8s  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x1b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2hf8s  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8s  4064(%ecx), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x28,0x1b,0x51,0x7f]
          vcvtbias2ph2hf8s  4064(%ecx), %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8s  -256(%edx){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x65,0x38,0x1b,0x52,0x80]
          vcvtbias2ph2hf8s  -256(%edx){1to16}, %ymm3, %ymm2

// CHECK: vcvtbias2ph2hf8s  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8s  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8s  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8s  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8s  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x1b,0x10]
          vcvtbias2ph2hf8s  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8s  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x1b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2hf8s  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8s  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x08,0x1b,0x51,0x7f]
          vcvtbias2ph2hf8s  2032(%ecx), %xmm3, %xmm2

// CHECK: vcvtbias2ph2hf8s  -256(%edx){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x18,0x1b,0x52,0x80]
          vcvtbias2ph2hf8s  -256(%edx){1to8}, %xmm3, %xmm2

// CHECK: vcvtbiasph2bf8 %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x74,0xd3]
          vcvtbiasph2bf8 %xmm3, %xmm2

// CHECK: vcvtbiasph2bf8 %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x74,0xd3]
          vcvtbiasph2bf8 %xmm3, %xmm2 {%k7}

// CHECK: vcvtbiasph2bf8 %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0x8f,0x74,0xd3]
          vcvtbiasph2bf8 %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8 %ymm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x74,0xd3]
          vcvtbiasph2bf8 %ymm3, %xmm2

// CHECK: vcvtbiasph2bf8 %ymm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x74,0xd3]
          vcvtbiasph2bf8 %ymm3, %xmm2 {%k7}

// CHECK: vcvtbiasph2bf8 %ymm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0xaf,0x74,0xd3]
          vcvtbiasph2bf8 %ymm3, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8x  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8x  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtbiasph2bf8x  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8x  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtbiasph2bf8  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x18,0x74,0x10]
          vcvtbiasph2bf8  (%eax){1to8}, %xmm2

// CHECK: vcvtbiasph2bf8x  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2bf8x  -512(,%ebp,2), %xmm2

// CHECK: vcvtbiasph2bf8x  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0x8f,0x74,0x51,0x7f]
          vcvtbiasph2bf8x  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0x9f,0x74,0x52,0x80]
          vcvtbiasph2bf8  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8  (%eax){1to16}, %xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x38,0x74,0x10]
          vcvtbiasph2bf8  (%eax){1to16}, %xmm2

// CHECK: vcvtbiasph2bf8y  -1024(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2bf8y  -1024(,%ebp,2), %xmm2

// CHECK: vcvtbiasph2bf8y  4064(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0xaf,0x74,0x51,0x7f]
          vcvtbiasph2bf8y  4064(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8  -256(%edx){1to16}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0xbf,0x74,0x52,0x80]
          vcvtbiasph2bf8  -256(%edx){1to16}, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8s %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x74,0xd3]
          vcvtbiasph2bf8s %xmm3, %xmm2

// CHECK: vcvtbiasph2bf8s %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x74,0xd3]
          vcvtbiasph2bf8s %xmm3, %xmm2 {%k7}

// CHECK: vcvtbiasph2bf8s %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x74,0xd3]
          vcvtbiasph2bf8s %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8s %ymm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x74,0xd3]
          vcvtbiasph2bf8s %ymm3, %xmm2

// CHECK: vcvtbiasph2bf8s %ymm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x74,0xd3]
          vcvtbiasph2bf8s %ymm3, %xmm2 {%k7}

// CHECK: vcvtbiasph2bf8s %ymm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x74,0xd3]
          vcvtbiasph2bf8s %ymm3, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8sx  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8sx  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtbiasph2bf8sx  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8sx  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtbiasph2bf8s  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x18,0x74,0x10]
          vcvtbiasph2bf8s  (%eax){1to8}, %xmm2

// CHECK: vcvtbiasph2bf8sx  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2bf8sx  -512(,%ebp,2), %xmm2

// CHECK: vcvtbiasph2bf8sx  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x74,0x51,0x7f]
          vcvtbiasph2bf8sx  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8s  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x9f,0x74,0x52,0x80]
          vcvtbiasph2bf8s  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8s  (%eax){1to16}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x38,0x74,0x10]
          vcvtbiasph2bf8s  (%eax){1to16}, %xmm2

// CHECK: vcvtbiasph2bf8sy  -1024(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2bf8sy  -1024(,%ebp,2), %xmm2

// CHECK: vcvtbiasph2bf8sy  4064(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x74,0x51,0x7f]
          vcvtbiasph2bf8sy  4064(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8s  -256(%edx){1to16}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xbf,0x74,0x52,0x80]
          vcvtbiasph2bf8s  -256(%edx){1to16}, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8 %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x18,0xd3]
          vcvtbiasph2hf8 %xmm3, %xmm2

// CHECK: vcvtbiasph2hf8 %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x18,0xd3]
          vcvtbiasph2hf8 %xmm3, %xmm2 {%k7}

// CHECK: vcvtbiasph2hf8 %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x18,0xd3]
          vcvtbiasph2hf8 %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8 %ymm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x18,0xd3]
          vcvtbiasph2hf8 %ymm3, %xmm2

// CHECK: vcvtbiasph2hf8 %ymm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x18,0xd3]
          vcvtbiasph2hf8 %ymm3, %xmm2 {%k7}

// CHECK: vcvtbiasph2hf8 %ymm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x18,0xd3]
          vcvtbiasph2hf8 %ymm3, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8x  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8x  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtbiasph2hf8x  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8x  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtbiasph2hf8  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x18,0x18,0x10]
          vcvtbiasph2hf8  (%eax){1to8}, %xmm2

// CHECK: vcvtbiasph2hf8x  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x18,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2hf8x  -512(,%ebp,2), %xmm2

// CHECK: vcvtbiasph2hf8x  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x18,0x51,0x7f]
          vcvtbiasph2hf8x  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x9f,0x18,0x52,0x80]
          vcvtbiasph2hf8  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8  (%eax){1to16}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x38,0x18,0x10]
          vcvtbiasph2hf8  (%eax){1to16}, %xmm2

// CHECK: vcvtbiasph2hf8y  -1024(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x18,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2hf8y  -1024(,%ebp,2), %xmm2

// CHECK: vcvtbiasph2hf8y  4064(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x18,0x51,0x7f]
          vcvtbiasph2hf8y  4064(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8  -256(%edx){1to16}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xbf,0x18,0x52,0x80]
          vcvtbiasph2hf8  -256(%edx){1to16}, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8s %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x1b,0xd3]
          vcvtbiasph2hf8s %xmm3, %xmm2

// CHECK: vcvtbiasph2hf8s %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x1b,0xd3]
          vcvtbiasph2hf8s %xmm3, %xmm2 {%k7}

// CHECK: vcvtbiasph2hf8s %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x1b,0xd3]
          vcvtbiasph2hf8s %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8s %ymm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x1b,0xd3]
          vcvtbiasph2hf8s %ymm3, %xmm2

// CHECK: vcvtbiasph2hf8s %ymm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x2f,0x1b,0xd3]
          vcvtbiasph2hf8s %ymm3, %xmm2 {%k7}

// CHECK: vcvtbiasph2hf8s %ymm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x1b,0xd3]
          vcvtbiasph2hf8s %ymm3, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8sx  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8sx  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtbiasph2hf8sx  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x0f,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8sx  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtbiasph2hf8s  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x18,0x1b,0x10]
          vcvtbiasph2hf8s  (%eax){1to8}, %xmm2

// CHECK: vcvtbiasph2hf8sx  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x08,0x1b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2hf8sx  -512(,%ebp,2), %xmm2

// CHECK: vcvtbiasph2hf8sx  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x8f,0x1b,0x51,0x7f]
          vcvtbiasph2hf8sx  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8s  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0x9f,0x1b,0x52,0x80]
          vcvtbiasph2hf8s  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8s  (%eax){1to16}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x38,0x1b,0x10]
          vcvtbiasph2hf8s  (%eax){1to16}, %xmm2

// CHECK: vcvtbiasph2hf8sy  -1024(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x28,0x1b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2hf8sy  -1024(,%ebp,2), %xmm2

// CHECK: vcvtbiasph2hf8sy  4064(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xaf,0x1b,0x51,0x7f]
          vcvtbiasph2hf8sy  4064(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8s  -256(%edx){1to16}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xbf,0x1b,0x52,0x80]
          vcvtbiasph2hf8s  -256(%edx){1to16}, %xmm2 {%k7} {z}

// CHECK: vcvtne2ph2bf8 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x74,0xd4]
          vcvtne2ph2bf8 %ymm4, %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x74,0xd4]
          vcvtne2ph2bf8 %xmm4, %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8  291(%edi,%eax,4), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8  291(%edi,%eax,4), %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x38,0x74,0x10]
          vcvtne2ph2bf8  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2bf8  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8  4064(%ecx), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x28,0x74,0x51,0x7f]
          vcvtne2ph2bf8  4064(%ecx), %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8  -256(%edx){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x67,0x38,0x74,0x52,0x80]
          vcvtne2ph2bf8  -256(%edx){1to16}, %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x18,0x74,0x10]
          vcvtne2ph2bf8  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2bf8  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x08,0x74,0x51,0x7f]
          vcvtne2ph2bf8  2032(%ecx), %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8  -256(%edx){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x18,0x74,0x52,0x80]
          vcvtne2ph2bf8  -256(%edx){1to8}, %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8s %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x74,0xd4]
          vcvtne2ph2bf8s %ymm4, %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8s %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x74,0xd4]
          vcvtne2ph2bf8s %xmm4, %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8s  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8s  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8s  291(%edi,%eax,4), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8s  291(%edi,%eax,4), %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8s  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x74,0x10]
          vcvtne2ph2bf8s  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8s  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2bf8s  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8s  4064(%ecx), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x74,0x51,0x7f]
          vcvtne2ph2bf8s  4064(%ecx), %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8s  -256(%edx){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x74,0x52,0x80]
          vcvtne2ph2bf8s  -256(%edx){1to16}, %ymm3, %ymm2

// CHECK: vcvtne2ph2bf8s  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8s  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8s  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8s  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8s  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x74,0x10]
          vcvtne2ph2bf8s  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8s  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2bf8s  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8s  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x74,0x51,0x7f]
          vcvtne2ph2bf8s  2032(%ecx), %xmm3, %xmm2

// CHECK: vcvtne2ph2bf8s  -256(%edx){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x74,0x52,0x80]
          vcvtne2ph2bf8s  -256(%edx){1to8}, %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x18,0xd4]
          vcvtne2ph2hf8 %ymm4, %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x18,0xd4]
          vcvtne2ph2hf8 %xmm4, %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8  291(%edi,%eax,4), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8  291(%edi,%eax,4), %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x18,0x10]
          vcvtne2ph2hf8  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x18,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2hf8  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8  4064(%ecx), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x18,0x51,0x7f]
          vcvtne2ph2hf8  4064(%ecx), %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8  -256(%edx){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x18,0x52,0x80]
          vcvtne2ph2hf8  -256(%edx){1to16}, %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x18,0x10]
          vcvtne2ph2hf8  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x18,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2hf8  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x18,0x51,0x7f]
          vcvtne2ph2hf8  2032(%ecx), %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8  -256(%edx){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x18,0x52,0x80]
          vcvtne2ph2hf8  -256(%edx){1to8}, %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8s %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x1b,0xd4]
          vcvtne2ph2hf8s %ymm4, %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8s %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x1b,0xd4]
          vcvtne2ph2hf8s %xmm4, %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8s  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8s  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8s  291(%edi,%eax,4), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8s  291(%edi,%eax,4), %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8s  (%eax){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x1b,0x10]
          vcvtne2ph2hf8s  (%eax){1to16}, %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8s  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x1b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2hf8s  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8s  4064(%ecx), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x28,0x1b,0x51,0x7f]
          vcvtne2ph2hf8s  4064(%ecx), %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8s  -256(%edx){1to16}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x67,0x38,0x1b,0x52,0x80]
          vcvtne2ph2hf8s  -256(%edx){1to16}, %ymm3, %ymm2

// CHECK: vcvtne2ph2hf8s  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8s  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8s  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8s  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8s  (%eax){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x1b,0x10]
          vcvtne2ph2hf8s  (%eax){1to8}, %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8s  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x1b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2hf8s  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8s  2032(%ecx), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x08,0x1b,0x51,0x7f]
          vcvtne2ph2hf8s  2032(%ecx), %xmm3, %xmm2

// CHECK: vcvtne2ph2hf8s  -256(%edx){1to8}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x18,0x1b,0x52,0x80]
          vcvtne2ph2hf8s  -256(%edx){1to8}, %xmm3, %xmm2

// CHECK: vcvtnebf82ph %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1e,0xd3]
          vcvtnebf82ph %xmm3, %xmm2

// CHECK: vcvtnebf82ph %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x1e,0xd3]
          vcvtnebf82ph %xmm3, %xmm2 {%k7}

// CHECK: vcvtnebf82ph %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x1e,0xd3]
          vcvtnebf82ph %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtnebf82ph %xmm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x1e,0xd3]
          vcvtnebf82ph %xmm3, %ymm2

// CHECK: vcvtnebf82ph %xmm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x2f,0x1e,0xd3]
          vcvtnebf82ph %xmm3, %ymm2 {%k7}

// CHECK: vcvtnebf82ph %xmm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x1e,0xd3]
          vcvtnebf82ph %xmm3, %ymm2 {%k7} {z}

// CHECK: vcvtnebf82ph  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnebf82ph  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtnebf82ph  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x1e,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnebf82ph  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtnebf82ph  (%eax), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1e,0x10]
          vcvtnebf82ph  (%eax), %xmm2

// CHECK: vcvtnebf82ph  -256(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1e,0x14,0x6d,0x00,0xff,0xff,0xff]
          vcvtnebf82ph  -256(,%ebp,2), %xmm2

// CHECK: vcvtnebf82ph  1016(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x1e,0x51,0x7f]
          vcvtnebf82ph  1016(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtnebf82ph  -1024(%edx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x1e,0x52,0x80]
          vcvtnebf82ph  -1024(%edx), %xmm2 {%k7} {z}

// CHECK: vcvtnebf82ph  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x1e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnebf82ph  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtnebf82ph  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x2f,0x1e,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnebf82ph  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtnebf82ph  (%eax), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x1e,0x10]
          vcvtnebf82ph  (%eax), %ymm2

// CHECK: vcvtnebf82ph  -512(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x1e,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtnebf82ph  -512(,%ebp,2), %ymm2

// CHECK: vcvtnebf82ph  2032(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x1e,0x51,0x7f]
          vcvtnebf82ph  2032(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtnebf82ph  -2048(%edx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x1e,0x52,0x80]
          vcvtnebf82ph  -2048(%edx), %ymm2 {%k7} {z}

// CHECK: vcvtnehf82ph %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x1e,0xd3]
          vcvtnehf82ph %xmm3, %xmm2

// CHECK: vcvtnehf82ph %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x0f,0x1e,0xd3]
          vcvtnehf82ph %xmm3, %xmm2 {%k7}

// CHECK: vcvtnehf82ph %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x1e,0xd3]
          vcvtnehf82ph %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtnehf82ph %xmm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x1e,0xd3]
          vcvtnehf82ph %xmm3, %ymm2

// CHECK: vcvtnehf82ph %xmm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x2f,0x1e,0xd3]
          vcvtnehf82ph %xmm3, %ymm2 {%k7}

// CHECK: vcvtnehf82ph %xmm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x1e,0xd3]
          vcvtnehf82ph %xmm3, %ymm2 {%k7} {z}

// CHECK: vcvtnehf82ph  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x1e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnehf82ph  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtnehf82ph  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x0f,0x1e,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnehf82ph  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtnehf82ph  (%eax), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x1e,0x10]
          vcvtnehf82ph  (%eax), %xmm2

// CHECK: vcvtnehf82ph  -256(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x08,0x1e,0x14,0x6d,0x00,0xff,0xff,0xff]
          vcvtnehf82ph  -256(,%ebp,2), %xmm2

// CHECK: vcvtnehf82ph  1016(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x1e,0x51,0x7f]
          vcvtnehf82ph  1016(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtnehf82ph  -1024(%edx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0x8f,0x1e,0x52,0x80]
          vcvtnehf82ph  -1024(%edx), %xmm2 {%k7} {z}

// CHECK: vcvtnehf82ph  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x1e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnehf82ph  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtnehf82ph  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x2f,0x1e,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnehf82ph  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtnehf82ph  (%eax), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x1e,0x10]
          vcvtnehf82ph  (%eax), %ymm2

// CHECK: vcvtnehf82ph  -512(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x28,0x1e,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtnehf82ph  -512(,%ebp,2), %ymm2

// CHECK: vcvtnehf82ph  2032(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x1e,0x51,0x7f]
          vcvtnehf82ph  2032(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtnehf82ph  -2048(%edx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xaf,0x1e,0x52,0x80]
          vcvtnehf82ph  -2048(%edx), %ymm2 {%k7} {z}

// CHECK: vcvtneph2bf8 %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x74,0xd3]
          vcvtneph2bf8 %xmm3, %xmm2

// CHECK: vcvtneph2bf8 %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x74,0xd3]
          vcvtneph2bf8 %xmm3, %xmm2 {%k7}

// CHECK: vcvtneph2bf8 %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0x8f,0x74,0xd3]
          vcvtneph2bf8 %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtneph2bf8 %ymm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0x74,0xd3]
          vcvtneph2bf8 %ymm3, %xmm2

// CHECK: vcvtneph2bf8 %ymm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0x74,0xd3]
          vcvtneph2bf8 %ymm3, %xmm2 {%k7}

// CHECK: vcvtneph2bf8 %ymm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xaf,0x74,0xd3]
          vcvtneph2bf8 %ymm3, %xmm2 {%k7} {z}

// CHECK: vcvtneph2bf8x  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2bf8x  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtneph2bf8x  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2bf8x  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtneph2bf8  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x18,0x74,0x10]
          vcvtneph2bf8  (%eax){1to8}, %xmm2

// CHECK: vcvtneph2bf8x  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2bf8x  -512(,%ebp,2), %xmm2

// CHECK: vcvtneph2bf8x  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0x8f,0x74,0x51,0x7f]
          vcvtneph2bf8x  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtneph2bf8  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0x9f,0x74,0x52,0x80]
          vcvtneph2bf8  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvtneph2bf8  (%eax){1to16}, %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x38,0x74,0x10]
          vcvtneph2bf8  (%eax){1to16}, %xmm2

// CHECK: vcvtneph2bf8y  -1024(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2bf8y  -1024(,%ebp,2), %xmm2

// CHECK: vcvtneph2bf8y  4064(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xaf,0x74,0x51,0x7f]
          vcvtneph2bf8y  4064(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtneph2bf8  -256(%edx){1to16}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xbf,0x74,0x52,0x80]
          vcvtneph2bf8  -256(%edx){1to16}, %xmm2 {%k7} {z}

// CHECK: vcvtneph2bf8s %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x74,0xd3]
          vcvtneph2bf8s %xmm3, %xmm2

// CHECK: vcvtneph2bf8s %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x74,0xd3]
          vcvtneph2bf8s %xmm3, %xmm2 {%k7}

// CHECK: vcvtneph2bf8s %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x74,0xd3]
          vcvtneph2bf8s %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtneph2bf8s %ymm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x74,0xd3]
          vcvtneph2bf8s %ymm3, %xmm2

// CHECK: vcvtneph2bf8s %ymm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x2f,0x74,0xd3]
          vcvtneph2bf8s %ymm3, %xmm2 {%k7}

// CHECK: vcvtneph2bf8s %ymm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x74,0xd3]
          vcvtneph2bf8s %ymm3, %xmm2 {%k7} {z}

// CHECK: vcvtneph2bf8sx  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2bf8sx  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtneph2bf8sx  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2bf8sx  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtneph2bf8s  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x18,0x74,0x10]
          vcvtneph2bf8s  (%eax){1to8}, %xmm2

// CHECK: vcvtneph2bf8sx  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x74,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2bf8sx  -512(,%ebp,2), %xmm2

// CHECK: vcvtneph2bf8sx  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x74,0x51,0x7f]
          vcvtneph2bf8sx  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtneph2bf8s  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0x9f,0x74,0x52,0x80]
          vcvtneph2bf8s  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvtneph2bf8s  (%eax){1to16}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x38,0x74,0x10]
          vcvtneph2bf8s  (%eax){1to16}, %xmm2

// CHECK: vcvtneph2bf8sy  -1024(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x74,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2bf8sy  -1024(,%ebp,2), %xmm2

// CHECK: vcvtneph2bf8sy  4064(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x74,0x51,0x7f]
          vcvtneph2bf8sy  4064(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtneph2bf8s  -256(%edx){1to16}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xbf,0x74,0x52,0x80]
          vcvtneph2bf8s  -256(%edx){1to16}, %xmm2 {%k7} {z}

// CHECK: vcvtneph2hf8 %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x18,0xd3]
          vcvtneph2hf8 %xmm3, %xmm2

// CHECK: vcvtneph2hf8 %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x18,0xd3]
          vcvtneph2hf8 %xmm3, %xmm2 {%k7}

// CHECK: vcvtneph2hf8 %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x18,0xd3]
          vcvtneph2hf8 %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtneph2hf8 %ymm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x18,0xd3]
          vcvtneph2hf8 %ymm3, %xmm2

// CHECK: vcvtneph2hf8 %ymm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x2f,0x18,0xd3]
          vcvtneph2hf8 %ymm3, %xmm2 {%k7}

// CHECK: vcvtneph2hf8 %ymm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x18,0xd3]
          vcvtneph2hf8 %ymm3, %xmm2 {%k7} {z}

// CHECK: vcvtneph2hf8x  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2hf8x  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtneph2hf8x  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2hf8x  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtneph2hf8  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x18,0x18,0x10]
          vcvtneph2hf8  (%eax){1to8}, %xmm2

// CHECK: vcvtneph2hf8x  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x18,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2hf8x  -512(,%ebp,2), %xmm2

// CHECK: vcvtneph2hf8x  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x18,0x51,0x7f]
          vcvtneph2hf8x  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtneph2hf8  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0x9f,0x18,0x52,0x80]
          vcvtneph2hf8  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvtneph2hf8  (%eax){1to16}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x38,0x18,0x10]
          vcvtneph2hf8  (%eax){1to16}, %xmm2

// CHECK: vcvtneph2hf8y  -1024(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x18,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2hf8y  -1024(,%ebp,2), %xmm2

// CHECK: vcvtneph2hf8y  4064(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x18,0x51,0x7f]
          vcvtneph2hf8y  4064(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtneph2hf8  -256(%edx){1to16}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xbf,0x18,0x52,0x80]
          vcvtneph2hf8  -256(%edx){1to16}, %xmm2 {%k7} {z}

// CHECK: vcvtneph2hf8s %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1b,0xd3]
          vcvtneph2hf8s %xmm3, %xmm2

// CHECK: vcvtneph2hf8s %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x1b,0xd3]
          vcvtneph2hf8s %xmm3, %xmm2 {%k7}

// CHECK: vcvtneph2hf8s %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x1b,0xd3]
          vcvtneph2hf8s %xmm3, %xmm2 {%k7} {z}

// CHECK: vcvtneph2hf8s %ymm3, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x1b,0xd3]
          vcvtneph2hf8s %ymm3, %xmm2

// CHECK: vcvtneph2hf8s %ymm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x2f,0x1b,0xd3]
          vcvtneph2hf8s %ymm3, %xmm2 {%k7}

// CHECK: vcvtneph2hf8s %ymm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x1b,0xd3]
          vcvtneph2hf8s %ymm3, %xmm2 {%k7} {z}

// CHECK: vcvtneph2hf8sx  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2hf8sx  268435456(%esp,%esi,8), %xmm2

// CHECK: vcvtneph2hf8sx  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x0f,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2hf8sx  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK: vcvtneph2hf8s  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x18,0x1b,0x10]
          vcvtneph2hf8s  (%eax){1to8}, %xmm2

// CHECK: vcvtneph2hf8sx  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x1b,0x14,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2hf8sx  -512(,%ebp,2), %xmm2

// CHECK: vcvtneph2hf8sx  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0x8f,0x1b,0x51,0x7f]
          vcvtneph2hf8sx  2032(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtneph2hf8s  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0x9f,0x1b,0x52,0x80]
          vcvtneph2hf8s  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK: vcvtneph2hf8s  (%eax){1to16}, %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x38,0x1b,0x10]
          vcvtneph2hf8s  (%eax){1to16}, %xmm2

// CHECK: vcvtneph2hf8sy  -1024(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x28,0x1b,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2hf8sy  -1024(,%ebp,2), %xmm2

// CHECK: vcvtneph2hf8sy  4064(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xaf,0x1b,0x51,0x7f]
          vcvtneph2hf8sy  4064(%ecx), %xmm2 {%k7} {z}

// CHECK: vcvtneph2hf8s  -256(%edx){1to16}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xbf,0x1b,0x52,0x80]
          vcvtneph2hf8s  -256(%edx){1to16}, %xmm2 {%k7} {z}

