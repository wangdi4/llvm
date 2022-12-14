// REQUIRES: intel_feature_isa_avx512_ne_convert_fp8
// RUN: llvm-mc -triple x86_64 --show-encoding %s | FileCheck %s

// CHECK: vcvtbias2ph2bf8 %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x45,0x40,0x74,0xf0]
          vcvtbias2ph2bf8 %zmm24, %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x45,0x40,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8  291(%r8,%rax,4), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xc2,0x45,0x40,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8  291(%r8,%rax,4), %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x50,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2bf8  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x40,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2bf8  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8  8128(%rcx), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x40,0x74,0x71,0x7f]
          vcvtbias2ph2bf8  8128(%rcx), %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8  -256(%rdx){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x50,0x74,0x72,0x80]
          vcvtbias2ph2bf8  -256(%rdx){1to32}, %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8s %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x85,0x45,0x40,0x74,0xf0]
          vcvtbias2ph2bf8s %zmm24, %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8s  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x45,0x40,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8s  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8s  291(%r8,%rax,4), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xc5,0x45,0x40,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8s  291(%r8,%rax,4), %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8s  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2bf8s  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8s  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2bf8s  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8s  8128(%rcx), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x74,0x71,0x7f]
          vcvtbias2ph2bf8s  8128(%rcx), %zmm23, %zmm22

// CHECK: vcvtbias2ph2bf8s  -256(%rdx){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x74,0x72,0x80]
          vcvtbias2ph2bf8s  -256(%rdx){1to32}, %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8 %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x85,0x45,0x40,0x18,0xf0]
          vcvtbias2ph2hf8 %zmm24, %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x45,0x40,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8  291(%r8,%rax,4), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xc5,0x45,0x40,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8  291(%r8,%rax,4), %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2hf8  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x18,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2hf8  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8  8128(%rcx), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x18,0x71,0x7f]
          vcvtbias2ph2hf8  8128(%rcx), %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8  -256(%rdx){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x18,0x72,0x80]
          vcvtbias2ph2hf8  -256(%rdx){1to32}, %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8s %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x85,0x45,0x40,0x1b,0xf0]
          vcvtbias2ph2hf8s %zmm24, %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8s  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x45,0x40,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8s  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8s  291(%r8,%rax,4), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xc5,0x45,0x40,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8s  291(%r8,%rax,4), %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8s  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtbias2ph2hf8s  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8s  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x1b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2hf8s  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8s  8128(%rcx), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x1b,0x71,0x7f]
          vcvtbias2ph2hf8s  8128(%rcx), %zmm23, %zmm22

// CHECK: vcvtbias2ph2hf8s  -256(%rdx){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x1b,0x72,0x80]
          vcvtbias2ph2hf8s  -256(%rdx){1to32}, %zmm23, %zmm22

// CHECK: vcvtbiasph2bf8 %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x48,0x74,0xf7]
          vcvtbiasph2bf8 %zmm23, %ymm22

// CHECK: vcvtbiasph2bf8 %zmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa2,0x7c,0x4f,0x74,0xf7]
          vcvtbiasph2bf8 %zmm23, %ymm22 {%k7}

// CHECK: vcvtbiasph2bf8 %zmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa2,0x7c,0xcf,0x74,0xf7]
          vcvtbiasph2bf8 %zmm23, %ymm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x48,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtbiasph2bf8  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7c,0x4f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtbiasph2bf8  (%rip){1to32}, %ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x58,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2bf8  (%rip){1to32}, %ymm22

// CHECK: vcvtbiasph2bf8  -2048(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x48,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2bf8  -2048(,%rbp,2), %ymm22

// CHECK: vcvtbiasph2bf8  8128(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7c,0xcf,0x74,0x71,0x7f]
          vcvtbiasph2bf8  8128(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8  -256(%rdx){1to32}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7c,0xdf,0x74,0x72,0x80]
          vcvtbiasph2bf8  -256(%rdx){1to32}, %ymm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8s %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x74,0xf7]
          vcvtbiasph2bf8s %zmm23, %ymm22

// CHECK: vcvtbiasph2bf8s %zmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x74,0xf7]
          vcvtbiasph2bf8s %zmm23, %ymm22 {%k7}

// CHECK: vcvtbiasph2bf8s %zmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xcf,0x74,0xf7]
          vcvtbiasph2bf8s %zmm23, %ymm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8s  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8s  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtbiasph2bf8s  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8s  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtbiasph2bf8s  (%rip){1to32}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2bf8s  (%rip){1to32}, %ymm22

// CHECK: vcvtbiasph2bf8s  -2048(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2bf8s  -2048(,%rbp,2), %ymm22

// CHECK: vcvtbiasph2bf8s  8128(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x74,0x71,0x7f]
          vcvtbiasph2bf8s  8128(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtbiasph2bf8s  -256(%rdx){1to32}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x74,0x72,0x80]
          vcvtbiasph2bf8s  -256(%rdx){1to32}, %ymm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8 %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x18,0xf7]
          vcvtbiasph2hf8 %zmm23, %ymm22

// CHECK: vcvtbiasph2hf8 %zmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x18,0xf7]
          vcvtbiasph2hf8 %zmm23, %ymm22 {%k7}

// CHECK: vcvtbiasph2hf8 %zmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xcf,0x18,0xf7]
          vcvtbiasph2hf8 %zmm23, %ymm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtbiasph2hf8  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtbiasph2hf8  (%rip){1to32}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2hf8  (%rip){1to32}, %ymm22

// CHECK: vcvtbiasph2hf8  -2048(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x18,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2hf8  -2048(,%rbp,2), %ymm22

// CHECK: vcvtbiasph2hf8  8128(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x18,0x71,0x7f]
          vcvtbiasph2hf8  8128(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8  -256(%rdx){1to32}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x18,0x72,0x80]
          vcvtbiasph2hf8  -256(%rdx){1to32}, %ymm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8s %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x1b,0xf7]
          vcvtbiasph2hf8s %zmm23, %ymm22

// CHECK: vcvtbiasph2hf8s %zmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x1b,0xf7]
          vcvtbiasph2hf8s %zmm23, %ymm22 {%k7}

// CHECK: vcvtbiasph2hf8s %zmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xcf,0x1b,0xf7]
          vcvtbiasph2hf8s %zmm23, %ymm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8s  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8s  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtbiasph2hf8s  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8s  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtbiasph2hf8s  (%rip){1to32}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtbiasph2hf8s  (%rip){1to32}, %ymm22

// CHECK: vcvtbiasph2hf8s  -2048(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x1b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2hf8s  -2048(,%rbp,2), %ymm22

// CHECK: vcvtbiasph2hf8s  8128(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x1b,0x71,0x7f]
          vcvtbiasph2hf8s  8128(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtbiasph2hf8s  -256(%rdx){1to32}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x1b,0x72,0x80]
          vcvtbiasph2hf8s  -256(%rdx){1to32}, %ymm22 {%k7} {z}

// CHECK: vcvtne2ph2bf8 %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x47,0x40,0x74,0xf0]
          vcvtne2ph2bf8 %zmm24, %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x47,0x40,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8  291(%r8,%rax,4), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xc2,0x47,0x40,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8  291(%r8,%rax,4), %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x50,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2bf8  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x40,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2bf8  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8  8128(%rcx), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x40,0x74,0x71,0x7f]
          vcvtne2ph2bf8  8128(%rcx), %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8  -256(%rdx){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x47,0x50,0x74,0x72,0x80]
          vcvtne2ph2bf8  -256(%rdx){1to32}, %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8s %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x74,0xf0]
          vcvtne2ph2bf8s %zmm24, %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8s  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8s  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8s  291(%r8,%rax,4), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xc5,0x47,0x40,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8s  291(%r8,%rax,4), %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8s  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2bf8s  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8s  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2bf8s  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8s  8128(%rcx), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x74,0x71,0x7f]
          vcvtne2ph2bf8s  8128(%rcx), %zmm23, %zmm22

// CHECK: vcvtne2ph2bf8s  -256(%rdx){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x74,0x72,0x80]
          vcvtne2ph2bf8s  -256(%rdx){1to32}, %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8 %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x18,0xf0]
          vcvtne2ph2hf8 %zmm24, %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8  291(%r8,%rax,4), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xc5,0x47,0x40,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8  291(%r8,%rax,4), %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2hf8  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x18,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2hf8  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8  8128(%rcx), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x18,0x71,0x7f]
          vcvtne2ph2hf8  8128(%rcx), %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8  -256(%rdx){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x18,0x72,0x80]
          vcvtne2ph2hf8  -256(%rdx){1to32}, %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8s %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x1b,0xf0]
          vcvtne2ph2hf8s %zmm24, %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8s  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8s  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8s  291(%r8,%rax,4), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xc5,0x47,0x40,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8s  291(%r8,%rax,4), %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8s  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtne2ph2hf8s  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8s  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x1b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2hf8s  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8s  8128(%rcx), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x1b,0x71,0x7f]
          vcvtne2ph2hf8s  8128(%rcx), %zmm23, %zmm22

// CHECK: vcvtne2ph2hf8s  -256(%rdx){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x1b,0x72,0x80]
          vcvtne2ph2hf8s  -256(%rdx){1to32}, %zmm23, %zmm22

// CHECK: vcvtnebf82ph %ymm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x1e,0xf7]
          vcvtnebf82ph %ymm23, %zmm22

// CHECK: vcvtnebf82ph %ymm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x4f,0x1e,0xf7]
          vcvtnebf82ph %ymm23, %zmm22 {%k7}

// CHECK: vcvtnebf82ph %ymm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0xcf,0x1e,0xf7]
          vcvtnebf82ph %ymm23, %zmm22 {%k7} {z}

// CHECK: vcvtnebf82ph  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x1e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnebf82ph  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvtnebf82ph  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x1e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnebf82ph  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvtnebf82ph  (%rip), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x1e,0x35,0x00,0x00,0x00,0x00]
          vcvtnebf82ph  (%rip), %zmm22

// CHECK: vcvtnebf82ph  -1024(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x1e,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtnebf82ph  -1024(,%rbp,2), %zmm22

// CHECK: vcvtnebf82ph  4064(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x1e,0x71,0x7f]
          vcvtnebf82ph  4064(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvtnebf82ph  -4096(%rdx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x1e,0x72,0x80]
          vcvtnebf82ph  -4096(%rdx), %zmm22 {%k7} {z}

// CHECK: vcvtnehf82ph %ymm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x1e,0xf7]
          vcvtnehf82ph %ymm23, %zmm22

// CHECK: vcvtnehf82ph %ymm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7f,0x4f,0x1e,0xf7]
          vcvtnehf82ph %ymm23, %zmm22 {%k7}

// CHECK: vcvtnehf82ph %ymm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7f,0xcf,0x1e,0xf7]
          vcvtnehf82ph %ymm23, %zmm22 {%k7} {z}

// CHECK: vcvtnehf82ph  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x1e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtnehf82ph  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvtnehf82ph  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7f,0x4f,0x1e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtnehf82ph  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvtnehf82ph  (%rip), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x1e,0x35,0x00,0x00,0x00,0x00]
          vcvtnehf82ph  (%rip), %zmm22

// CHECK: vcvtnehf82ph  -1024(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x1e,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtnehf82ph  -1024(,%rbp,2), %zmm22

// CHECK: vcvtnehf82ph  4064(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x1e,0x71,0x7f]
          vcvtnehf82ph  4064(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvtnehf82ph  -4096(%rdx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x1e,0x72,0x80]
          vcvtnehf82ph  -4096(%rdx), %zmm22 {%k7} {z}

// CHECK: vcvtneph2bf8 %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0x74,0xf7]
          vcvtneph2bf8 %zmm23, %ymm22

// CHECK: vcvtneph2bf8 %zmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa2,0x7e,0x4f,0x74,0xf7]
          vcvtneph2bf8 %zmm23, %ymm22 {%k7}

// CHECK: vcvtneph2bf8 %zmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa2,0x7e,0xcf,0x74,0xf7]
          vcvtneph2bf8 %zmm23, %ymm22 {%k7} {z}

// CHECK: vcvtneph2bf8  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2bf8  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtneph2bf8  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7e,0x4f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2bf8  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtneph2bf8  (%rip){1to32}, %ymm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x58,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2bf8  (%rip){1to32}, %ymm22

// CHECK: vcvtneph2bf8  -2048(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2bf8  -2048(,%rbp,2), %ymm22

// CHECK: vcvtneph2bf8  8128(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0xcf,0x74,0x71,0x7f]
          vcvtneph2bf8  8128(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtneph2bf8  -256(%rdx){1to32}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0xdf,0x74,0x72,0x80]
          vcvtneph2bf8  -256(%rdx){1to32}, %ymm22 {%k7} {z}

// CHECK: vcvtneph2bf8s %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x74,0xf7]
          vcvtneph2bf8s %zmm23, %ymm22

// CHECK: vcvtneph2bf8s %zmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x4f,0x74,0xf7]
          vcvtneph2bf8s %zmm23, %ymm22 {%k7}

// CHECK: vcvtneph2bf8s %zmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0xcf,0x74,0xf7]
          vcvtneph2bf8s %zmm23, %ymm22 {%k7} {z}

// CHECK: vcvtneph2bf8s  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x74,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2bf8s  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtneph2bf8s  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x74,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2bf8s  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtneph2bf8s  (%rip){1to32}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x58,0x74,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2bf8s  (%rip){1to32}, %ymm22

// CHECK: vcvtneph2bf8s  -2048(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x74,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2bf8s  -2048(,%rbp,2), %ymm22

// CHECK: vcvtneph2bf8s  8128(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x74,0x71,0x7f]
          vcvtneph2bf8s  8128(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtneph2bf8s  -256(%rdx){1to32}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xdf,0x74,0x72,0x80]
          vcvtneph2bf8s  -256(%rdx){1to32}, %ymm22 {%k7} {z}

// CHECK: vcvtneph2hf8 %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x18,0xf7]
          vcvtneph2hf8 %zmm23, %ymm22

// CHECK: vcvtneph2hf8 %zmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x4f,0x18,0xf7]
          vcvtneph2hf8 %zmm23, %ymm22 {%k7}

// CHECK: vcvtneph2hf8 %zmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0xcf,0x18,0xf7]
          vcvtneph2hf8 %zmm23, %ymm22 {%k7} {z}

// CHECK: vcvtneph2hf8  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x18,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2hf8  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtneph2hf8  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x18,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2hf8  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtneph2hf8  (%rip){1to32}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x58,0x18,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2hf8  (%rip){1to32}, %ymm22

// CHECK: vcvtneph2hf8  -2048(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x18,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2hf8  -2048(,%rbp,2), %ymm22

// CHECK: vcvtneph2hf8  8128(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x18,0x71,0x7f]
          vcvtneph2hf8  8128(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtneph2hf8  -256(%rdx){1to32}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xdf,0x18,0x72,0x80]
          vcvtneph2hf8  -256(%rdx){1to32}, %ymm22 {%k7} {z}

// CHECK: vcvtneph2hf8s %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x1b,0xf7]
          vcvtneph2hf8s %zmm23, %ymm22

// CHECK: vcvtneph2hf8s %zmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x4f,0x1b,0xf7]
          vcvtneph2hf8s %zmm23, %ymm22 {%k7}

// CHECK: vcvtneph2hf8s %zmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0xcf,0x1b,0xf7]
          vcvtneph2hf8s %zmm23, %ymm22 {%k7} {z}

// CHECK: vcvtneph2hf8s  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x1b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtneph2hf8s  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtneph2hf8s  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x1b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtneph2hf8s  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtneph2hf8s  (%rip){1to32}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x58,0x1b,0x35,0x00,0x00,0x00,0x00]
          vcvtneph2hf8s  (%rip){1to32}, %ymm22

// CHECK: vcvtneph2hf8s  -2048(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x1b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2hf8s  -2048(,%rbp,2), %ymm22

// CHECK: vcvtneph2hf8s  8128(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x1b,0x71,0x7f]
          vcvtneph2hf8s  8128(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtneph2hf8s  -256(%rdx){1to32}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xdf,0x1b,0x72,0x80]
          vcvtneph2hf8s  -256(%rdx){1to32}, %ymm22 {%k7} {z}

