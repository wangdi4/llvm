// REQUIRES: intel_feature_isa_amx_sparse
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK: tdpsbf16ps %tmm2, %tmm0, %tmm6
// CHECK: encoding: [0xc4,0xe2,0xea,0x5c,0xf0]
          tdpsbf16ps %tmm2, %tmm0, %tmm6

// CHECK: tdpsbf16ps %tmm0, %tmm2, %tmm5
// CHECK: encoding: [0xc4,0xe2,0xfa,0x5c,0xea]
          tdpsbf16ps %tmm0, %tmm2, %tmm5

// CHECK: tdpsbssd %tmm2, %tmm0, %tmm6
// CHECK: encoding: [0xc4,0xe2,0xeb,0x5e,0xf0]
          tdpsbssd %tmm2, %tmm0, %tmm6

// CHECK: tdpsbssd %tmm0, %tmm2, %tmm5
// CHECK: encoding: [0xc4,0xe2,0xfb,0x5e,0xea]
          tdpsbssd %tmm0, %tmm2, %tmm5

// CHECK: tdpsbsud %tmm2, %tmm0, %tmm6
// CHECK: encoding: [0xc4,0xe2,0xea,0x5e,0xf0]
          tdpsbsud %tmm2, %tmm0, %tmm6

// CHECK: tdpsbsud %tmm0, %tmm2, %tmm5
// CHECK: encoding: [0xc4,0xe2,0xfa,0x5e,0xea]
          tdpsbsud %tmm0, %tmm2, %tmm5

// CHECK: tdpsbusd %tmm2, %tmm0, %tmm6
// CHECK: encoding: [0xc4,0xe2,0xe9,0x5e,0xf0]
          tdpsbusd %tmm2, %tmm0, %tmm6

// CHECK: tdpsbusd %tmm0, %tmm2, %tmm5
// CHECK: encoding: [0xc4,0xe2,0xf9,0x5e,0xea]
          tdpsbusd %tmm0, %tmm2, %tmm5

// CHECK: tdpsbuud %tmm2, %tmm0, %tmm6
// CHECK: encoding: [0xc4,0xe2,0xe8,0x5e,0xf0]
          tdpsbuud %tmm2, %tmm0, %tmm6

// CHECK: tdpsbuud %tmm0, %tmm2, %tmm5
// CHECK: encoding: [0xc4,0xe2,0xf8,0x5e,0xea]
          tdpsbuud %tmm0, %tmm2, %tmm5

// CHECK: tdpsfp16ps %tmm2, %tmm0, %tmm6
// CHECK: encoding: [0xc4,0xe2,0xeb,0x5c,0xf0]
          tdpsfp16ps %tmm2, %tmm0, %tmm6

// CHECK: tdpsfp16ps %tmm0, %tmm2, %tmm5
// CHECK: encoding: [0xc4,0xe2,0xfb,0x5c,0xea]
          tdpsfp16ps %tmm0, %tmm2, %tmm5

// CHECK: tldexpandb %ecx, 268435456(%rbp,%r14,8), %tmm2
// CHECK: encoding: [0xc4,0xa2,0x72,0x69,0x94,0xf5,0x00,0x00,0x00,0x10]
          tldexpandb %ecx, 268435456(%rbp,%r14,8), %tmm2

// CHECK: tldexpandb %ecx, 291(%r8,%rax,4), %tmm0
// CHECK: encoding: [0xc4,0xc2,0x72,0x69,0x84,0x80,0x23,0x01,0x00,0x00]
          tldexpandb %ecx, 291(%r8,%rax,4), %tmm0

// CHECK: tldexpandb %ecx, (%rcx), %tmm2
// CHECK: encoding: [0xc4,0xe2,0x72,0x69,0x14,0x21]
          tldexpandb %ecx, (%rcx), %tmm2

// CHECK: tldexpandb %ecx, -32(,%rbp,2), %tmm0
// CHECK: encoding: [0xc4,0xe2,0x72,0x69,0x04,0x6d,0xe0,0xff,0xff,0xff]
          tldexpandb %ecx, -32(,%rbp,2), %tmm0

// CHECK: tldexpandbt1 %ecx, 268435456(%rbp,%r14,8), %tmm2
// CHECK: encoding: [0xc4,0xa2,0x73,0x69,0x94,0xf5,0x00,0x00,0x00,0x10]
          tldexpandbt1 %ecx, 268435456(%rbp,%r14,8), %tmm2

// CHECK: tldexpandbt1 %ecx, 291(%r8,%rax,4), %tmm0
// CHECK: encoding: [0xc4,0xc2,0x73,0x69,0x84,0x80,0x23,0x01,0x00,0x00]
          tldexpandbt1 %ecx, 291(%r8,%rax,4), %tmm0

// CHECK: tldexpandbt1 %ecx, (%rcx), %tmm2
// CHECK: encoding: [0xc4,0xe2,0x73,0x69,0x14,0x21]
          tldexpandbt1 %ecx, (%rcx), %tmm2

// CHECK: tldexpandbt1 %ecx, -32(,%rbp,2), %tmm0
// CHECK: encoding: [0xc4,0xe2,0x73,0x69,0x04,0x6d,0xe0,0xff,0xff,0xff]
          tldexpandbt1 %ecx, -32(,%rbp,2), %tmm0

// CHECK: tldexpandw %ecx, 268435456(%rbp,%r14,8), %tmm2
// CHECK: encoding: [0xc4,0xa2,0x70,0x69,0x94,0xf5,0x00,0x00,0x00,0x10]
          tldexpandw %ecx, 268435456(%rbp,%r14,8), %tmm2

// CHECK: tldexpandw %ecx, 291(%r8,%rax,4), %tmm0
// CHECK: encoding: [0xc4,0xc2,0x70,0x69,0x84,0x80,0x23,0x01,0x00,0x00]
          tldexpandw %ecx, 291(%r8,%rax,4), %tmm0

// CHECK: tldexpandw %ecx, (%rcx), %tmm2
// CHECK: encoding: [0xc4,0xe2,0x70,0x69,0x14,0x21]
          tldexpandw %ecx, (%rcx), %tmm2

// CHECK: tldexpandw %ecx, -32(,%rbp,2), %tmm0
// CHECK: encoding: [0xc4,0xe2,0x70,0x69,0x04,0x6d,0xe0,0xff,0xff,0xff]
          tldexpandw %ecx, -32(,%rbp,2), %tmm0

// CHECK: tldexpandwt1 %ecx, 268435456(%rbp,%r14,8), %tmm2
// CHECK: encoding: [0xc4,0xa2,0x71,0x69,0x94,0xf5,0x00,0x00,0x00,0x10]
          tldexpandwt1 %ecx, 268435456(%rbp,%r14,8), %tmm2

// CHECK: tldexpandwt1 %ecx, 291(%r8,%rax,4), %tmm0
// CHECK: encoding: [0xc4,0xc2,0x71,0x69,0x84,0x80,0x23,0x01,0x00,0x00]
          tldexpandwt1 %ecx, 291(%r8,%rax,4), %tmm0

// CHECK: tldexpandwt1 %ecx, (%rcx), %tmm2
// CHECK: encoding: [0xc4,0xe2,0x71,0x69,0x14,0x21]
          tldexpandwt1 %ecx, (%rcx), %tmm2

// CHECK: tldexpandwt1 %ecx, -32(,%rbp,2), %tmm0
// CHECK: encoding: [0xc4,0xe2,0x71,0x69,0x04,0x6d,0xe0,0xff,0xff,0xff]
          tldexpandwt1 %ecx, -32(,%rbp,2), %tmm0

