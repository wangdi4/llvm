// REQUIRES: intel_feature_isa_avx512_ne_convert_fp8
// RUN: llvm-mc -triple x86_64 --show-encoding %s | FileCheck %s

// CHECK: vcvtbias2ph2bf8 %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x45,0x20,0x74,0xf0]
          vcvtbias2ph2bf8 %ymm24, %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8 %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x45,0x00,0x74,0xf0]
          vcvtbias2ph2bf8 %xmm24, %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x45,0x20,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8  291(%r8,%rax,4), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xc2,0x45,0x20,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8  291(%r8,%rax,4), %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x45,0x30,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2bf8  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x45,0x20,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2bf8  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8  4064(%rcx), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x45,0x20,0x74,0x71,0x7f]
          vcvtbias2ph2bf8  4064(%rcx), %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8  -256(%rdx){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x45,0x30,0x74,0x72,0x80]
          vcvtbias2ph2bf8  -256(%rdx){1to16}, %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x45,0x00,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8  291(%r8,%rax,4), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc2,0x45,0x00,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8  291(%r8,%rax,4), %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x10,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2bf8  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x00,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2bf8  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8  2032(%rcx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x00,0x74,0x71,0x7f]
          vcvtbias2ph2bf8  2032(%rcx), %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8  -256(%rdx){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x10,0x74,0x72,0x80]
          vcvtbias2ph2bf8  -256(%rdx){1to8}, %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8s %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x85,0x45,0x20,0x74,0xf0]
          vcvtbias2ph2bf8s %ymm24, %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8s %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x45,0x00,0x74,0xf0]
          vcvtbias2ph2bf8s %xmm24, %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8s  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x45,0x20,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8s  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8s  291(%r8,%rax,4), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xc5,0x45,0x20,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8s  291(%r8,%rax,4), %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8s  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2bf8s  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8s  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2bf8s  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8s  4064(%rcx), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x74,0x71,0x7f]
          vcvtbias2ph2bf8s  4064(%rcx), %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8s  -256(%rdx){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x74,0x72,0x80]
          vcvtbias2ph2bf8s  -256(%rdx){1to16}, %ymm23, %ymm22

// CHECK: vcvtbias2ph2bf8s  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x45,0x00,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8s  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8s  291(%r8,%rax,4), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0x45,0x00,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8s  291(%r8,%rax,4), %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8s  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2bf8s  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8s  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2bf8s  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8s  2032(%rcx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x74,0x71,0x7f]
          vcvtbias2ph2bf8s  2032(%rcx), %xmm23, %xmm22

// CHECK: vcvtbias2ph2bf8s  -256(%rdx){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x74,0x72,0x80]
          vcvtbias2ph2bf8s  -256(%rdx){1to8}, %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8 %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x85,0x45,0x20,0x18,0xf0]
          vcvtbias2ph2hf8 %ymm24, %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8 %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x45,0x00,0x18,0xf0]
          vcvtbias2ph2hf8 %xmm24, %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x45,0x20,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8  291(%r8,%rax,4), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xc5,0x45,0x20,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8  291(%r8,%rax,4), %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2hf8  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x18,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2hf8  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8  4064(%rcx), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x18,0x71,0x7f]
          vcvtbias2ph2hf8  4064(%rcx), %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8  -256(%rdx){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x18,0x72,0x80]
          vcvtbias2ph2hf8  -256(%rdx){1to16}, %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x45,0x00,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8  291(%r8,%rax,4), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0x45,0x00,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8  291(%r8,%rax,4), %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2hf8  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x18,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2hf8  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8  2032(%rcx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x18,0x71,0x7f]
          vcvtbias2ph2hf8  2032(%rcx), %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8  -256(%rdx){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x18,0x72,0x80]
          vcvtbias2ph2hf8  -256(%rdx){1to8}, %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8s %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x85,0x45,0x20,0x1b,0xf0]
          vcvtbias2ph2hf8s %ymm24, %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8s %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x45,0x00,0x1b,0xf0]
          vcvtbias2ph2hf8s %xmm24, %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8s  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x45,0x20,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8s  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8s  291(%r8,%rax,4), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xc5,0x45,0x20,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8s  291(%r8,%rax,4), %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8s  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2hf8s  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8s  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x1b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbias2ph2hf8s  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8s  4064(%rcx), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x1b,0x71,0x7f]
          vcvtbias2ph2hf8s  4064(%rcx), %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8s  -256(%rdx){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x1b,0x72,0x80]
          vcvtbias2ph2hf8s  -256(%rdx){1to16}, %ymm23, %ymm22

// CHECK: vcvtbias2ph2hf8s  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x45,0x00,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8s  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8s  291(%r8,%rax,4), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0x45,0x00,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8s  291(%r8,%rax,4), %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8s  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2hf8s  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8s  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x1b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbias2ph2hf8s  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8s  2032(%rcx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x1b,0x71,0x7f]
          vcvtbias2ph2hf8s  2032(%rcx), %xmm23, %xmm22

// CHECK: vcvtbias2ph2hf8s  -256(%rdx){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x1b,0x72,0x80]
          vcvtbias2ph2hf8s  -256(%rdx){1to8}, %xmm23, %xmm22

// CHECK: vcvtbiasph2bf8 %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x08,0x74,0xf7]
          vcvtbiasph2bf8 %xmm23, %xmm22

// CHECK: vcvtbiasph2bf8 %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa2,0x7c,0x0f,0x74,0xf7]
          vcvtbiasph2bf8 %xmm23, %xmm22 {%k7}

// CHECK: vcvtbiasph2bf8 %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa2,0x7c,0x8f,0x74,0xf7]
          vcvtbiasph2bf8 %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8 %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x28,0x74,0xf7]
          vcvtbiasph2bf8 %ymm23, %xmm22

// CHECK: vcvtbiasph2bf8 %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa2,0x7c,0x2f,0x74,0xf7]
          vcvtbiasph2bf8 %ymm23, %xmm22 {%k7}

// CHECK: vcvtbiasph2bf8 %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa2,0x7c,0xaf,0x74,0xf7]
          vcvtbiasph2bf8 %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8x  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x08,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8x  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtbiasph2bf8x  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7c,0x0f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8x  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtbiasph2bf8  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x18,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2bf8  (%rip){1to8}, %xmm22

// CHECK: vcvtbiasph2bf8x  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x08,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2bf8x  -512(,%rbp,2), %xmm22

// CHECK: vcvtbiasph2bf8x  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7c,0x8f,0x74,0x71,0x7f]
          vcvtbiasph2bf8x  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7c,0x9f,0x74,0x72,0x80]
          vcvtbiasph2bf8  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8  (%rip){1to16}, %xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x38,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2bf8  (%rip){1to16}, %xmm22

// CHECK: vcvtbiasph2bf8y  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x28,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2bf8y  -1024(,%rbp,2), %xmm22

// CHECK: vcvtbiasph2bf8y  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7c,0xaf,0x74,0x71,0x7f]
          vcvtbiasph2bf8y  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8  -256(%rdx){1to16}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7c,0xbf,0x74,0x72,0x80]
          vcvtbiasph2bf8  -256(%rdx){1to16}, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8s %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x74,0xf7]
          vcvtbiasph2bf8s %xmm23, %xmm22

// CHECK: vcvtbiasph2bf8s %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x74,0xf7]
          vcvtbiasph2bf8s %xmm23, %xmm22 {%k7}

// CHECK: vcvtbiasph2bf8s %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x74,0xf7]
          vcvtbiasph2bf8s %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8s %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x74,0xf7]
          vcvtbiasph2bf8s %ymm23, %xmm22

// CHECK: vcvtbiasph2bf8s %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x74,0xf7]
          vcvtbiasph2bf8s %ymm23, %xmm22 {%k7}

// CHECK: vcvtbiasph2bf8s %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x74,0xf7]
          vcvtbiasph2bf8s %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8sx  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8sx  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtbiasph2bf8sx  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8sx  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtbiasph2bf8s  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2bf8s  (%rip){1to8}, %xmm22

// CHECK: vcvtbiasph2bf8sx  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2bf8sx  -512(,%rbp,2), %xmm22

// CHECK: vcvtbiasph2bf8sx  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x74,0x71,0x7f]
          vcvtbiasph2bf8sx  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8s  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x74,0x72,0x80]
          vcvtbiasph2bf8s  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8s  (%rip){1to16}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2bf8s  (%rip){1to16}, %xmm22

// CHECK: vcvtbiasph2bf8sy  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2bf8sy  -1024(,%rbp,2), %xmm22

// CHECK: vcvtbiasph2bf8sy  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x74,0x71,0x7f]
          vcvtbiasph2bf8sy  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8s  -256(%rdx){1to16}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x74,0x72,0x80]
          vcvtbiasph2bf8s  -256(%rdx){1to16}, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8 %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x18,0xf7]
          vcvtbiasph2hf8 %xmm23, %xmm22

// CHECK: vcvtbiasph2hf8 %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x18,0xf7]
          vcvtbiasph2hf8 %xmm23, %xmm22 {%k7}

// CHECK: vcvtbiasph2hf8 %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x18,0xf7]
          vcvtbiasph2hf8 %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8 %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x18,0xf7]
          vcvtbiasph2hf8 %ymm23, %xmm22

// CHECK: vcvtbiasph2hf8 %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x18,0xf7]
          vcvtbiasph2hf8 %ymm23, %xmm22 {%k7}

// CHECK: vcvtbiasph2hf8 %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x18,0xf7]
          vcvtbiasph2hf8 %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8x  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8x  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtbiasph2hf8x  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8x  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtbiasph2hf8  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2hf8  (%rip){1to8}, %xmm22

// CHECK: vcvtbiasph2hf8x  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x18,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2hf8x  -512(,%rbp,2), %xmm22

// CHECK: vcvtbiasph2hf8x  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x18,0x71,0x7f]
          vcvtbiasph2hf8x  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x18,0x72,0x80]
          vcvtbiasph2hf8  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8  (%rip){1to16}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2hf8  (%rip){1to16}, %xmm22

// CHECK: vcvtbiasph2hf8y  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x18,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2hf8y  -1024(,%rbp,2), %xmm22

// CHECK: vcvtbiasph2hf8y  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x18,0x71,0x7f]
          vcvtbiasph2hf8y  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8  -256(%rdx){1to16}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x18,0x72,0x80]
          vcvtbiasph2hf8  -256(%rdx){1to16}, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8s %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x1b,0xf7]
          vcvtbiasph2hf8s %xmm23, %xmm22

// CHECK: vcvtbiasph2hf8s %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x1b,0xf7]
          vcvtbiasph2hf8s %xmm23, %xmm22 {%k7}

// CHECK: vcvtbiasph2hf8s %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x1b,0xf7]
          vcvtbiasph2hf8s %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8s %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x1b,0xf7]
          vcvtbiasph2hf8s %ymm23, %xmm22

// CHECK: vcvtbiasph2hf8s %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x1b,0xf7]
          vcvtbiasph2hf8s %ymm23, %xmm22 {%k7}

// CHECK: vcvtbiasph2hf8s %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x1b,0xf7]
          vcvtbiasph2hf8s %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8sx  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8sx  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtbiasph2hf8sx  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8sx  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtbiasph2hf8s  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2hf8s  (%rip){1to8}, %xmm22

// CHECK: vcvtbiasph2hf8sx  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x1b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtbiasph2hf8sx  -512(,%rbp,2), %xmm22

// CHECK: vcvtbiasph2hf8sx  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x1b,0x71,0x7f]
          vcvtbiasph2hf8sx  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8s  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x1b,0x72,0x80]
          vcvtbiasph2hf8s  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8s  (%rip){1to16}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2hf8s  (%rip){1to16}, %xmm22

// CHECK: vcvtbiasph2hf8sy  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x1b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtbiasph2hf8sy  -1024(,%rbp,2), %xmm22

// CHECK: vcvtbiasph2hf8sy  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x1b,0x71,0x7f]
          vcvtbiasph2hf8sy  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8s  -256(%rdx){1to16}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x1b,0x72,0x80]
          vcvtbiasph2hf8s  -256(%rdx){1to16}, %xmm22 {%k7} {z}

// CHECK: vcvtne2ph2bf8 %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x47,0x20,0x74,0xf0]
          vcvtne2ph2bf8 %ymm24, %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8 %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x47,0x00,0x74,0xf0]
          vcvtne2ph2bf8 %xmm24, %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x47,0x20,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8  291(%r8,%rax,4), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xc2,0x47,0x20,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8  291(%r8,%rax,4), %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x47,0x30,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2bf8  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x47,0x20,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2bf8  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8  4064(%rcx), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x47,0x20,0x74,0x71,0x7f]
          vcvtne2ph2bf8  4064(%rcx), %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8  -256(%rdx){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x47,0x30,0x74,0x72,0x80]
          vcvtne2ph2bf8  -256(%rdx){1to16}, %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x47,0x00,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8  291(%r8,%rax,4), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc2,0x47,0x00,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8  291(%r8,%rax,4), %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x10,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2bf8  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x00,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2bf8  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8  2032(%rcx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x00,0x74,0x71,0x7f]
          vcvtne2ph2bf8  2032(%rcx), %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8  -256(%rdx){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x10,0x74,0x72,0x80]
          vcvtne2ph2bf8  -256(%rdx){1to8}, %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8s %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x74,0xf0]
          vcvtne2ph2bf8s %ymm24, %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8s %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x74,0xf0]
          vcvtne2ph2bf8s %xmm24, %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8s  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x47,0x20,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8s  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8s  291(%r8,%rax,4), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xc5,0x47,0x20,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8s  291(%r8,%rax,4), %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8s  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2bf8s  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8s  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2bf8s  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8s  4064(%rcx), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x74,0x71,0x7f]
          vcvtne2ph2bf8s  4064(%rcx), %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8s  -256(%rdx){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x74,0x72,0x80]
          vcvtne2ph2bf8s  -256(%rdx){1to16}, %ymm23, %ymm22

// CHECK: vcvtne2ph2bf8s  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8s  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8s  291(%r8,%rax,4), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0x47,0x00,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8s  291(%r8,%rax,4), %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8s  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2bf8s  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8s  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2bf8s  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8s  2032(%rcx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x74,0x71,0x7f]
          vcvtne2ph2bf8s  2032(%rcx), %xmm23, %xmm22

// CHECK: vcvtne2ph2bf8s  -256(%rdx){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x74,0x72,0x80]
          vcvtne2ph2bf8s  -256(%rdx){1to8}, %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8 %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x18,0xf0]
          vcvtne2ph2hf8 %ymm24, %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8 %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x18,0xf0]
          vcvtne2ph2hf8 %xmm24, %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x47,0x20,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8  291(%r8,%rax,4), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xc5,0x47,0x20,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8  291(%r8,%rax,4), %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2hf8  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x18,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2hf8  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8  4064(%rcx), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x18,0x71,0x7f]
          vcvtne2ph2hf8  4064(%rcx), %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8  -256(%rdx){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x18,0x72,0x80]
          vcvtne2ph2hf8  -256(%rdx){1to16}, %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8  291(%r8,%rax,4), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0x47,0x00,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8  291(%r8,%rax,4), %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2hf8  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x18,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2hf8  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8  2032(%rcx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x18,0x71,0x7f]
          vcvtne2ph2hf8  2032(%rcx), %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8  -256(%rdx){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x18,0x72,0x80]
          vcvtne2ph2hf8  -256(%rdx){1to8}, %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8s %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x1b,0xf0]
          vcvtne2ph2hf8s %ymm24, %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8s %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x1b,0xf0]
          vcvtne2ph2hf8s %xmm24, %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8s  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x47,0x20,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8s  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8s  291(%r8,%rax,4), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xc5,0x47,0x20,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8s  291(%r8,%rax,4), %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8s  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2hf8s  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8s  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x1b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtne2ph2hf8s  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8s  4064(%rcx), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x1b,0x71,0x7f]
          vcvtne2ph2hf8s  4064(%rcx), %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8s  -256(%rdx){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x1b,0x72,0x80]
          vcvtne2ph2hf8s  -256(%rdx){1to16}, %ymm23, %ymm22

// CHECK: vcvtne2ph2hf8s  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8s  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8s  291(%r8,%rax,4), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0x47,0x00,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8s  291(%r8,%rax,4), %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8s  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2hf8s  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8s  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x1b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtne2ph2hf8s  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8s  2032(%rcx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x1b,0x71,0x7f]
          vcvtne2ph2hf8s  2032(%rcx), %xmm23, %xmm22

// CHECK: vcvtne2ph2hf8s  -256(%rdx){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x1b,0x72,0x80]
          vcvtne2ph2hf8s  -256(%rdx){1to8}, %xmm23, %xmm22

// CHECK: vcvtnebf82ph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x1e,0xf7]
          vcvtnebf82ph %xmm23, %xmm22

// CHECK: vcvtnebf82ph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x0f,0x1e,0xf7]
          vcvtnebf82ph %xmm23, %xmm22 {%k7}

// CHECK: vcvtnebf82ph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0x8f,0x1e,0xf7]
          vcvtnebf82ph %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtnebf82ph %xmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x1e,0xf7]
          vcvtnebf82ph %xmm23, %ymm22

// CHECK: vcvtnebf82ph %xmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x2f,0x1e,0xf7]
          vcvtnebf82ph %xmm23, %ymm22 {%k7}

// CHECK: vcvtnebf82ph %xmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0xaf,0x1e,0xf7]
          vcvtnebf82ph %xmm23, %ymm22 {%k7} {z}

// CHECK: vcvtnebf82ph  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x1e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnebf82ph  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtnebf82ph  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x1e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnebf82ph  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtnebf82ph  (%rip), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x1e,0x35,0x00,0x00,0x00,0x00]
          vcvtnebf82ph  (%rip), %xmm22

// CHECK: vcvtnebf82ph  -256(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x1e,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtnebf82ph  -256(,%rbp,2), %xmm22

// CHECK: vcvtnebf82ph  1016(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x1e,0x71,0x7f]
          vcvtnebf82ph  1016(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtnebf82ph  -1024(%rdx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x1e,0x72,0x80]
          vcvtnebf82ph  -1024(%rdx), %xmm22 {%k7} {z}

// CHECK: vcvtnebf82ph  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x1e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnebf82ph  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtnebf82ph  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x2f,0x1e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnebf82ph  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtnebf82ph  (%rip), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x1e,0x35,0x00,0x00,0x00,0x00]
          vcvtnebf82ph  (%rip), %ymm22

// CHECK: vcvtnebf82ph  -512(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x1e,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtnebf82ph  -512(,%rbp,2), %ymm22

// CHECK: vcvtnebf82ph  2032(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x1e,0x71,0x7f]
          vcvtnebf82ph  2032(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtnebf82ph  -2048(%rdx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x1e,0x72,0x80]
          vcvtnebf82ph  -2048(%rdx), %ymm22 {%k7} {z}

// CHECK: vcvtnehf82ph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x1e,0xf7]
          vcvtnehf82ph %xmm23, %xmm22

// CHECK: vcvtnehf82ph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7f,0x0f,0x1e,0xf7]
          vcvtnehf82ph %xmm23, %xmm22 {%k7}

// CHECK: vcvtnehf82ph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7f,0x8f,0x1e,0xf7]
          vcvtnehf82ph %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtnehf82ph %xmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x28,0x1e,0xf7]
          vcvtnehf82ph %xmm23, %ymm22

// CHECK: vcvtnehf82ph %xmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7f,0x2f,0x1e,0xf7]
          vcvtnehf82ph %xmm23, %ymm22 {%k7}

// CHECK: vcvtnehf82ph %xmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7f,0xaf,0x1e,0xf7]
          vcvtnehf82ph %xmm23, %ymm22 {%k7} {z}

// CHECK: vcvtnehf82ph  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x1e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnehf82ph  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtnehf82ph  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7f,0x0f,0x1e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnehf82ph  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtnehf82ph  (%rip), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x08,0x1e,0x35,0x00,0x00,0x00,0x00]
          vcvtnehf82ph  (%rip), %xmm22

// CHECK: vcvtnehf82ph  -256(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x08,0x1e,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtnehf82ph  -256(,%rbp,2), %xmm22

// CHECK: vcvtnehf82ph  1016(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0x8f,0x1e,0x71,0x7f]
          vcvtnehf82ph  1016(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtnehf82ph  -1024(%rdx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0x8f,0x1e,0x72,0x80]
          vcvtnehf82ph  -1024(%rdx), %xmm22 {%k7} {z}

// CHECK: vcvtnehf82ph  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x28,0x1e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnehf82ph  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtnehf82ph  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7f,0x2f,0x1e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnehf82ph  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtnehf82ph  (%rip), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x28,0x1e,0x35,0x00,0x00,0x00,0x00]
          vcvtnehf82ph  (%rip), %ymm22

// CHECK: vcvtnehf82ph  -512(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x28,0x1e,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtnehf82ph  -512(,%rbp,2), %ymm22

// CHECK: vcvtnehf82ph  2032(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0xaf,0x1e,0x71,0x7f]
          vcvtnehf82ph  2032(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtnehf82ph  -2048(%rdx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0xaf,0x1e,0x72,0x80]
          vcvtnehf82ph  -2048(%rdx), %ymm22 {%k7} {z}

// CHECK: vcvtneph2bf8 %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0x74,0xf7]
          vcvtneph2bf8 %xmm23, %xmm22

// CHECK: vcvtneph2bf8 %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa2,0x7e,0x0f,0x74,0xf7]
          vcvtneph2bf8 %xmm23, %xmm22 {%k7}

// CHECK: vcvtneph2bf8 %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa2,0x7e,0x8f,0x74,0xf7]
          vcvtneph2bf8 %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf8 %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x28,0x74,0xf7]
          vcvtneph2bf8 %ymm23, %xmm22

// CHECK: vcvtneph2bf8 %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa2,0x7e,0x2f,0x74,0xf7]
          vcvtneph2bf8 %ymm23, %xmm22 {%k7}

// CHECK: vcvtneph2bf8 %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa2,0x7e,0xaf,0x74,0xf7]
          vcvtneph2bf8 %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf8x  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2bf8x  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtneph2bf8x  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7e,0x0f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2bf8x  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtneph2bf8  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x18,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2bf8  (%rip){1to8}, %xmm22

// CHECK: vcvtneph2bf8x  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2bf8x  -512(,%rbp,2), %xmm22

// CHECK: vcvtneph2bf8x  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0x8f,0x74,0x71,0x7f]
          vcvtneph2bf8x  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf8  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0x9f,0x74,0x72,0x80]
          vcvtneph2bf8  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf8  (%rip){1to16}, %xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x38,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2bf8  (%rip){1to16}, %xmm22

// CHECK: vcvtneph2bf8y  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2bf8y  -1024(,%rbp,2), %xmm22

// CHECK: vcvtneph2bf8y  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0xaf,0x74,0x71,0x7f]
          vcvtneph2bf8y  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf8  -256(%rdx){1to16}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0xbf,0x74,0x72,0x80]
          vcvtneph2bf8  -256(%rdx){1to16}, %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf8s %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x74,0xf7]
          vcvtneph2bf8s %xmm23, %xmm22

// CHECK: vcvtneph2bf8s %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x0f,0x74,0xf7]
          vcvtneph2bf8s %xmm23, %xmm22 {%k7}

// CHECK: vcvtneph2bf8s %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0x8f,0x74,0xf7]
          vcvtneph2bf8s %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf8s %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x74,0xf7]
          vcvtneph2bf8s %ymm23, %xmm22

// CHECK: vcvtneph2bf8s %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x2f,0x74,0xf7]
          vcvtneph2bf8s %ymm23, %xmm22 {%k7}

// CHECK: vcvtneph2bf8s %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0xaf,0x74,0xf7]
          vcvtneph2bf8s %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf8sx  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2bf8sx  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtneph2bf8sx  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2bf8sx  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtneph2bf8s  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x18,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2bf8s  (%rip){1to8}, %xmm22

// CHECK: vcvtneph2bf8sx  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x74,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2bf8sx  -512(,%rbp,2), %xmm22

// CHECK: vcvtneph2bf8sx  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x74,0x71,0x7f]
          vcvtneph2bf8sx  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf8s  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x9f,0x74,0x72,0x80]
          vcvtneph2bf8s  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf8s  (%rip){1to16}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x38,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2bf8s  (%rip){1to16}, %xmm22

// CHECK: vcvtneph2bf8sy  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x74,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2bf8sy  -1024(,%rbp,2), %xmm22

// CHECK: vcvtneph2bf8sy  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x74,0x71,0x7f]
          vcvtneph2bf8sy  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf8s  -256(%rdx){1to16}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xbf,0x74,0x72,0x80]
          vcvtneph2bf8s  -256(%rdx){1to16}, %xmm22 {%k7} {z}

// CHECK: vcvtneph2hf8 %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x18,0xf7]
          vcvtneph2hf8 %xmm23, %xmm22

// CHECK: vcvtneph2hf8 %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x0f,0x18,0xf7]
          vcvtneph2hf8 %xmm23, %xmm22 {%k7}

// CHECK: vcvtneph2hf8 %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0x8f,0x18,0xf7]
          vcvtneph2hf8 %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtneph2hf8 %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x18,0xf7]
          vcvtneph2hf8 %ymm23, %xmm22

// CHECK: vcvtneph2hf8 %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x2f,0x18,0xf7]
          vcvtneph2hf8 %ymm23, %xmm22 {%k7}

// CHECK: vcvtneph2hf8 %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0xaf,0x18,0xf7]
          vcvtneph2hf8 %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtneph2hf8x  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2hf8x  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtneph2hf8x  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2hf8x  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtneph2hf8  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x18,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2hf8  (%rip){1to8}, %xmm22

// CHECK: vcvtneph2hf8x  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x18,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2hf8x  -512(,%rbp,2), %xmm22

// CHECK: vcvtneph2hf8x  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x18,0x71,0x7f]
          vcvtneph2hf8x  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtneph2hf8  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x9f,0x18,0x72,0x80]
          vcvtneph2hf8  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtneph2hf8  (%rip){1to16}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x38,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2hf8  (%rip){1to16}, %xmm22

// CHECK: vcvtneph2hf8y  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x18,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2hf8y  -1024(,%rbp,2), %xmm22

// CHECK: vcvtneph2hf8y  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x18,0x71,0x7f]
          vcvtneph2hf8y  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtneph2hf8  -256(%rdx){1to16}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xbf,0x18,0x72,0x80]
          vcvtneph2hf8  -256(%rdx){1to16}, %xmm22 {%k7} {z}

// CHECK: vcvtneph2hf8s %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x1b,0xf7]
          vcvtneph2hf8s %xmm23, %xmm22

// CHECK: vcvtneph2hf8s %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x0f,0x1b,0xf7]
          vcvtneph2hf8s %xmm23, %xmm22 {%k7}

// CHECK: vcvtneph2hf8s %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0x8f,0x1b,0xf7]
          vcvtneph2hf8s %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtneph2hf8s %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x1b,0xf7]
          vcvtneph2hf8s %ymm23, %xmm22

// CHECK: vcvtneph2hf8s %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x2f,0x1b,0xf7]
          vcvtneph2hf8s %ymm23, %xmm22 {%k7}

// CHECK: vcvtneph2hf8s %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0xaf,0x1b,0xf7]
          vcvtneph2hf8s %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtneph2hf8sx  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2hf8sx  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtneph2hf8sx  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2hf8sx  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtneph2hf8s  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x18,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2hf8s  (%rip){1to8}, %xmm22

// CHECK: vcvtneph2hf8sx  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x1b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtneph2hf8sx  -512(,%rbp,2), %xmm22

// CHECK: vcvtneph2hf8sx  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x1b,0x71,0x7f]
          vcvtneph2hf8sx  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtneph2hf8s  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x9f,0x1b,0x72,0x80]
          vcvtneph2hf8s  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtneph2hf8s  (%rip){1to16}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x38,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2hf8s  (%rip){1to16}, %xmm22

// CHECK: vcvtneph2hf8sy  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x1b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtneph2hf8sy  -1024(,%rbp,2), %xmm22

// CHECK: vcvtneph2hf8sy  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x1b,0x71,0x7f]
          vcvtneph2hf8sy  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtneph2hf8s  -256(%rdx){1to16}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xbf,0x1b,0x72,0x80]
          vcvtneph2hf8s  -256(%rdx){1to16}, %xmm22 {%k7} {z}

