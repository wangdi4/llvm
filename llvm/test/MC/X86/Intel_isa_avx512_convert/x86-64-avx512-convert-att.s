// REQUIRES: intel_feature_isa_avx512_convert
// RUN: llvm-mc -triple=x86_64-unknown-unknown -mattr=+avx512convert --show-encoding < %s  | FileCheck %s

// CHECK:      vcvtbf162ph %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0x67,0xf7]
               vcvtbf162ph %zmm23, %zmm22

// CHECK:      vcvtbf162ph {rn-sae}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x18,0x67,0xf7]
               vcvtbf162ph {rn-sae}, %zmm23, %zmm22

// CHECK:      vcvtbf162ph %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa2,0x7e,0x4f,0x67,0xf7]
               vcvtbf162ph %zmm23, %zmm22 {%k7}

// CHECK:      vcvtbf162ph {rz-sae}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa2,0x7e,0xff,0x67,0xf7]
               vcvtbf162ph {rz-sae}, %zmm23, %zmm22 {%k7} {z}

// CHECK:      vcvtbf162ph  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vcvtbf162ph  268435456(%rbp,%r14,8), %zmm22

// CHECK:      vcvtbf162ph  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7e,0x4f,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
               vcvtbf162ph  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK:      vcvtbf162ph  (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x58,0x67,0x35,0x00,0x00,0x00,0x00]
               vcvtbf162ph  (%rip){1to32}, %zmm22

// CHECK:      vcvtbf162ph  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0x67,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vcvtbf162ph  -2048(,%rbp,2), %zmm22

// CHECK:      vcvtbf162ph  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0xcf,0x67,0x71,0x7f]
               vcvtbf162ph  8128(%rcx), %zmm22 {%k7} {z}

// CHECK:      vcvtbf162ph  -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0xdf,0x67,0x72,0x80]
               vcvtbf162ph  -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK:      vcvtneph2bf16 %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x48,0x67,0xf7]
               vcvtneph2bf16 %zmm23, %zmm22

// CHECK:      vcvtneph2bf16 %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa2,0x7f,0x4f,0x67,0xf7]
               vcvtneph2bf16 %zmm23, %zmm22 {%k7}

// CHECK:      vcvtneph2bf16 %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa2,0x7f,0xcf,0x67,0xf7]
               vcvtneph2bf16 %zmm23, %zmm22 {%k7} {z}

// CHECK:      vcvtneph2bf16  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x48,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vcvtneph2bf16  268435456(%rbp,%r14,8), %zmm22

// CHECK:      vcvtneph2bf16  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7f,0x4f,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
               vcvtneph2bf16  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK:      vcvtneph2bf16  (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x58,0x67,0x35,0x00,0x00,0x00,0x00]
               vcvtneph2bf16  (%rip){1to32}, %zmm22

// CHECK:      vcvtneph2bf16  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x48,0x67,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vcvtneph2bf16  -2048(,%rbp,2), %zmm22

// CHECK:      vcvtneph2bf16  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7f,0xcf,0x67,0x71,0x7f]
               vcvtneph2bf16  8128(%rcx), %zmm22 {%k7} {z}

// CHECK:      vcvtneph2bf16  -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7f,0xdf,0x67,0x72,0x80]
               vcvtneph2bf16  -256(%rdx){1to32}, %zmm22 {%k7} {z}

