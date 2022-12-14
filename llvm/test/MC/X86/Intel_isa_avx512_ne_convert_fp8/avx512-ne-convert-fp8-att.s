// REQUIRES: intel_feature_isa_avx512_ne_convert_fp8
// RUN: llvm-mc -triple i386 --show-encoding %s | FileCheck %s

// CHECK: vcvtbias2ph2bf8 %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x74,0xd4]
          vcvtbias2ph2bf8 %zmm4, %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8  291(%edi,%eax,4), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8  291(%edi,%eax,4), %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8  (%eax){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x58,0x74,0x10]
          vcvtbias2ph2bf8  (%eax){1to32}, %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2bf8  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8  8128(%ecx), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x74,0x51,0x7f]
          vcvtbias2ph2bf8  8128(%ecx), %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8  -256(%edx){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x58,0x74,0x52,0x80]
          vcvtbias2ph2bf8  -256(%edx){1to32}, %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8s %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x74,0xd4]
          vcvtbias2ph2bf8s %zmm4, %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8s  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2bf8s  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8s  291(%edi,%eax,4), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2bf8s  291(%edi,%eax,4), %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8s  (%eax){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x74,0x10]
          vcvtbias2ph2bf8s  (%eax){1to32}, %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8s  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2bf8s  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8s  8128(%ecx), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x74,0x51,0x7f]
          vcvtbias2ph2bf8s  8128(%ecx), %zmm3, %zmm2

// CHECK: vcvtbias2ph2bf8s  -256(%edx){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x74,0x52,0x80]
          vcvtbias2ph2bf8s  -256(%edx){1to32}, %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8 %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x18,0xd4]
          vcvtbias2ph2hf8 %zmm4, %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8  291(%edi,%eax,4), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8  291(%edi,%eax,4), %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8  (%eax){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x18,0x10]
          vcvtbias2ph2hf8  (%eax){1to32}, %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x18,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2hf8  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8  8128(%ecx), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x18,0x51,0x7f]
          vcvtbias2ph2hf8  8128(%ecx), %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8  -256(%edx){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x18,0x52,0x80]
          vcvtbias2ph2hf8  -256(%edx){1to32}, %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8s %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x1b,0xd4]
          vcvtbias2ph2hf8s %zmm4, %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8s  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbias2ph2hf8s  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8s  291(%edi,%eax,4), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbias2ph2hf8s  291(%edi,%eax,4), %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8s  (%eax){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x1b,0x10]
          vcvtbias2ph2hf8s  (%eax){1to32}, %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8s  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x1b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbias2ph2hf8s  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8s  8128(%ecx), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x48,0x1b,0x51,0x7f]
          vcvtbias2ph2hf8s  8128(%ecx), %zmm3, %zmm2

// CHECK: vcvtbias2ph2hf8s  -256(%edx){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x65,0x58,0x1b,0x52,0x80]
          vcvtbias2ph2hf8s  -256(%edx){1to32}, %zmm3, %zmm2

// CHECK: vcvtbiasph2bf8 %zmm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0x74,0xd3]
          vcvtbiasph2bf8 %zmm3, %ymm2

// CHECK: vcvtbiasph2bf8 %zmm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0x74,0xd3]
          vcvtbiasph2bf8 %zmm3, %ymm2 {%k7}

// CHECK: vcvtbiasph2bf8 %zmm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0xcf,0x74,0xd3]
          vcvtbiasph2bf8 %zmm3, %ymm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtbiasph2bf8  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtbiasph2bf8  (%eax){1to32}, %ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x58,0x74,0x10]
          vcvtbiasph2bf8  (%eax){1to32}, %ymm2

// CHECK: vcvtbiasph2bf8  -2048(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2bf8  -2048(,%ebp,2), %ymm2

// CHECK: vcvtbiasph2bf8  8128(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0xcf,0x74,0x51,0x7f]
          vcvtbiasph2bf8  8128(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8  -256(%edx){1to32}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0xdf,0x74,0x52,0x80]
          vcvtbiasph2bf8  -256(%edx){1to32}, %ymm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8s %zmm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x74,0xd3]
          vcvtbiasph2bf8s %zmm3, %ymm2

// CHECK: vcvtbiasph2bf8s %zmm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x74,0xd3]
          vcvtbiasph2bf8s %zmm3, %ymm2 {%k7}

// CHECK: vcvtbiasph2bf8s %zmm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xcf,0x74,0xd3]
          vcvtbiasph2bf8s %zmm3, %ymm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8s  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2bf8s  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtbiasph2bf8s  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2bf8s  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtbiasph2bf8s  (%eax){1to32}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x58,0x74,0x10]
          vcvtbiasph2bf8s  (%eax){1to32}, %ymm2

// CHECK: vcvtbiasph2bf8s  -2048(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2bf8s  -2048(,%ebp,2), %ymm2

// CHECK: vcvtbiasph2bf8s  8128(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xcf,0x74,0x51,0x7f]
          vcvtbiasph2bf8s  8128(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtbiasph2bf8s  -256(%edx){1to32}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xdf,0x74,0x52,0x80]
          vcvtbiasph2bf8s  -256(%edx){1to32}, %ymm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8 %zmm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x18,0xd3]
          vcvtbiasph2hf8 %zmm3, %ymm2

// CHECK: vcvtbiasph2hf8 %zmm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x18,0xd3]
          vcvtbiasph2hf8 %zmm3, %ymm2 {%k7}

// CHECK: vcvtbiasph2hf8 %zmm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xcf,0x18,0xd3]
          vcvtbiasph2hf8 %zmm3, %ymm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtbiasph2hf8  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtbiasph2hf8  (%eax){1to32}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x58,0x18,0x10]
          vcvtbiasph2hf8  (%eax){1to32}, %ymm2

// CHECK: vcvtbiasph2hf8  -2048(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x18,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2hf8  -2048(,%ebp,2), %ymm2

// CHECK: vcvtbiasph2hf8  8128(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xcf,0x18,0x51,0x7f]
          vcvtbiasph2hf8  8128(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8  -256(%edx){1to32}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xdf,0x18,0x52,0x80]
          vcvtbiasph2hf8  -256(%edx){1to32}, %ymm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8s %zmm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x1b,0xd3]
          vcvtbiasph2hf8s %zmm3, %ymm2

// CHECK: vcvtbiasph2hf8s %zmm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x1b,0xd3]
          vcvtbiasph2hf8s %zmm3, %ymm2 {%k7}

// CHECK: vcvtbiasph2hf8s %zmm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xcf,0x1b,0xd3]
          vcvtbiasph2hf8s %zmm3, %ymm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8s  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtbiasph2hf8s  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtbiasph2hf8s  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7c,0x4f,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtbiasph2hf8s  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtbiasph2hf8s  (%eax){1to32}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x58,0x1b,0x10]
          vcvtbiasph2hf8s  (%eax){1to32}, %ymm2

// CHECK: vcvtbiasph2hf8s  -2048(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7c,0x48,0x1b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtbiasph2hf8s  -2048(,%ebp,2), %ymm2

// CHECK: vcvtbiasph2hf8s  8128(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xcf,0x1b,0x51,0x7f]
          vcvtbiasph2hf8s  8128(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtbiasph2hf8s  -256(%edx){1to32}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7c,0xdf,0x1b,0x52,0x80]
          vcvtbiasph2hf8s  -256(%edx){1to32}, %ymm2 {%k7} {z}

// CHECK: vcvtne2ph2bf8 %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x74,0xd4]
          vcvtne2ph2bf8 %zmm4, %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8  291(%edi,%eax,4), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8  291(%edi,%eax,4), %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8  (%eax){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x58,0x74,0x10]
          vcvtne2ph2bf8  (%eax){1to32}, %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2bf8  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8  8128(%ecx), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x48,0x74,0x51,0x7f]
          vcvtne2ph2bf8  8128(%ecx), %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8  -256(%edx){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x67,0x58,0x74,0x52,0x80]
          vcvtne2ph2bf8  -256(%edx){1to32}, %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8s %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x74,0xd4]
          vcvtne2ph2bf8s %zmm4, %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8s  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2bf8s  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8s  291(%edi,%eax,4), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2bf8s  291(%edi,%eax,4), %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8s  (%eax){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x74,0x10]
          vcvtne2ph2bf8s  (%eax){1to32}, %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8s  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2bf8s  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8s  8128(%ecx), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x74,0x51,0x7f]
          vcvtne2ph2bf8s  8128(%ecx), %zmm3, %zmm2

// CHECK: vcvtne2ph2bf8s  -256(%edx){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x74,0x52,0x80]
          vcvtne2ph2bf8s  -256(%edx){1to32}, %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8 %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x18,0xd4]
          vcvtne2ph2hf8 %zmm4, %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8  291(%edi,%eax,4), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8  291(%edi,%eax,4), %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8  (%eax){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x18,0x10]
          vcvtne2ph2hf8  (%eax){1to32}, %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x18,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2hf8  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8  8128(%ecx), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x18,0x51,0x7f]
          vcvtne2ph2hf8  8128(%ecx), %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8  -256(%edx){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x18,0x52,0x80]
          vcvtne2ph2hf8  -256(%edx){1to32}, %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8s %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x1b,0xd4]
          vcvtne2ph2hf8s %zmm4, %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8s  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtne2ph2hf8s  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8s  291(%edi,%eax,4), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtne2ph2hf8s  291(%edi,%eax,4), %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8s  (%eax){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x1b,0x10]
          vcvtne2ph2hf8s  (%eax){1to32}, %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8s  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x1b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtne2ph2hf8s  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8s  8128(%ecx), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x48,0x1b,0x51,0x7f]
          vcvtne2ph2hf8s  8128(%ecx), %zmm3, %zmm2

// CHECK: vcvtne2ph2hf8s  -256(%edx){1to32}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x67,0x58,0x1b,0x52,0x80]
          vcvtne2ph2hf8s  -256(%edx){1to32}, %zmm3, %zmm2

// CHECK: vcvtnebf82ph %ymm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1e,0xd3]
          vcvtnebf82ph %ymm3, %zmm2

// CHECK: vcvtnebf82ph %ymm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x1e,0xd3]
          vcvtnebf82ph %ymm3, %zmm2 {%k7}

// CHECK: vcvtnebf82ph %ymm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x1e,0xd3]
          vcvtnebf82ph %ymm3, %zmm2 {%k7} {z}

// CHECK: vcvtnebf82ph  268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnebf82ph  268435456(%esp,%esi,8), %zmm2

// CHECK: vcvtnebf82ph  291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x1e,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnebf82ph  291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvtnebf82ph  (%eax), %zmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1e,0x10]
          vcvtnebf82ph  (%eax), %zmm2

// CHECK: vcvtnebf82ph  -1024(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1e,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtnebf82ph  -1024(,%ebp,2), %zmm2

// CHECK: vcvtnebf82ph  4064(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x1e,0x51,0x7f]
          vcvtnebf82ph  4064(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvtnebf82ph  -4096(%edx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x1e,0x52,0x80]
          vcvtnebf82ph  -4096(%edx), %zmm2 {%k7} {z}

// CHECK: vcvtnehf82ph %ymm3, %zmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x48,0x1e,0xd3]
          vcvtnehf82ph %ymm3, %zmm2

// CHECK: vcvtnehf82ph %ymm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x4f,0x1e,0xd3]
          vcvtnehf82ph %ymm3, %zmm2 {%k7}

// CHECK: vcvtnehf82ph %ymm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xcf,0x1e,0xd3]
          vcvtnehf82ph %ymm3, %zmm2 {%k7} {z}

// CHECK: vcvtnehf82ph  268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x48,0x1e,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtnehf82ph  268435456(%esp,%esi,8), %zmm2

// CHECK: vcvtnehf82ph  291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7f,0x4f,0x1e,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtnehf82ph  291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvtnehf82ph  (%eax), %zmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x48,0x1e,0x10]
          vcvtnehf82ph  (%eax), %zmm2

// CHECK: vcvtnehf82ph  -1024(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf5,0x7f,0x48,0x1e,0x14,0x6d,0x00,0xfc,0xff,0xff]
          vcvtnehf82ph  -1024(,%ebp,2), %zmm2

// CHECK: vcvtnehf82ph  4064(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xcf,0x1e,0x51,0x7f]
          vcvtnehf82ph  4064(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvtnehf82ph  -4096(%edx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7f,0xcf,0x1e,0x52,0x80]
          vcvtnehf82ph  -4096(%edx), %zmm2 {%k7} {z}

// CHECK: vcvtneph2bf8 %zmm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0x74,0xd3]
          vcvtneph2bf8 %zmm3, %ymm2

// CHECK: vcvtneph2bf8 %zmm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0x74,0xd3]
          vcvtneph2bf8 %zmm3, %ymm2 {%k7}

// CHECK: vcvtneph2bf8 %zmm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xcf,0x74,0xd3]
          vcvtneph2bf8 %zmm3, %ymm2 {%k7} {z}

// CHECK: vcvtneph2bf8  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2bf8  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtneph2bf8  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2bf8  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtneph2bf8  (%eax){1to32}, %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x58,0x74,0x10]
          vcvtneph2bf8  (%eax){1to32}, %ymm2

// CHECK: vcvtneph2bf8  -2048(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2bf8  -2048(,%ebp,2), %ymm2

// CHECK: vcvtneph2bf8  8128(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xcf,0x74,0x51,0x7f]
          vcvtneph2bf8  8128(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtneph2bf8  -256(%edx){1to32}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xdf,0x74,0x52,0x80]
          vcvtneph2bf8  -256(%edx){1to32}, %ymm2 {%k7} {z}

// CHECK: vcvtneph2bf8s %zmm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x74,0xd3]
          vcvtneph2bf8s %zmm3, %ymm2

// CHECK: vcvtneph2bf8s %zmm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x74,0xd3]
          vcvtneph2bf8s %zmm3, %ymm2 {%k7}

// CHECK: vcvtneph2bf8s %zmm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x74,0xd3]
          vcvtneph2bf8s %zmm3, %ymm2 {%k7} {z}

// CHECK: vcvtneph2bf8s  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x74,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2bf8s  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtneph2bf8s  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x74,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2bf8s  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtneph2bf8s  (%eax){1to32}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x58,0x74,0x10]
          vcvtneph2bf8s  (%eax){1to32}, %ymm2

// CHECK: vcvtneph2bf8s  -2048(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x74,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2bf8s  -2048(,%ebp,2), %ymm2

// CHECK: vcvtneph2bf8s  8128(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x74,0x51,0x7f]
          vcvtneph2bf8s  8128(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtneph2bf8s  -256(%edx){1to32}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xdf,0x74,0x52,0x80]
          vcvtneph2bf8s  -256(%edx){1to32}, %ymm2 {%k7} {z}

// CHECK: vcvtneph2hf8 %zmm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x18,0xd3]
          vcvtneph2hf8 %zmm3, %ymm2

// CHECK: vcvtneph2hf8 %zmm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x18,0xd3]
          vcvtneph2hf8 %zmm3, %ymm2 {%k7}

// CHECK: vcvtneph2hf8 %zmm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x18,0xd3]
          vcvtneph2hf8 %zmm3, %ymm2 {%k7} {z}

// CHECK: vcvtneph2hf8  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x18,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2hf8  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtneph2hf8  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x18,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2hf8  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtneph2hf8  (%eax){1to32}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x58,0x18,0x10]
          vcvtneph2hf8  (%eax){1to32}, %ymm2

// CHECK: vcvtneph2hf8  -2048(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x18,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2hf8  -2048(,%ebp,2), %ymm2

// CHECK: vcvtneph2hf8  8128(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x18,0x51,0x7f]
          vcvtneph2hf8  8128(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtneph2hf8  -256(%edx){1to32}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xdf,0x18,0x52,0x80]
          vcvtneph2hf8  -256(%edx){1to32}, %ymm2 {%k7} {z}

// CHECK: vcvtneph2hf8s %zmm3, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1b,0xd3]
          vcvtneph2hf8s %zmm3, %ymm2

// CHECK: vcvtneph2hf8s %zmm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x1b,0xd3]
          vcvtneph2hf8s %zmm3, %ymm2 {%k7}

// CHECK: vcvtneph2hf8s %zmm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x1b,0xd3]
          vcvtneph2hf8s %zmm3, %ymm2 {%k7} {z}

// CHECK: vcvtneph2hf8s  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1b,0x94,0xf4,0x00,0x00,0x00,0x10]
          vcvtneph2hf8s  268435456(%esp,%esi,8), %ymm2

// CHECK: vcvtneph2hf8s  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf5,0x7e,0x4f,0x1b,0x94,0x87,0x23,0x01,0x00,0x00]
          vcvtneph2hf8s  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK: vcvtneph2hf8s  (%eax){1to32}, %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x58,0x1b,0x10]
          vcvtneph2hf8s  (%eax){1to32}, %ymm2

// CHECK: vcvtneph2hf8s  -2048(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf5,0x7e,0x48,0x1b,0x14,0x6d,0x00,0xf8,0xff,0xff]
          vcvtneph2hf8s  -2048(,%ebp,2), %ymm2

// CHECK: vcvtneph2hf8s  8128(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xcf,0x1b,0x51,0x7f]
          vcvtneph2hf8s  8128(%ecx), %ymm2 {%k7} {z}

// CHECK: vcvtneph2hf8s  -256(%edx){1to32}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf5,0x7e,0xdf,0x1b,0x52,0x80]
          vcvtneph2hf8s  -256(%edx){1to32}, %ymm2 {%k7} {z}

