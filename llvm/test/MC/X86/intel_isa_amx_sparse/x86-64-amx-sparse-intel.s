// REQUIRES: intel_feature_isa_amx_sparse
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: tdpsbf16ps tmm6, tmm0, tmm2
// CHECK: encoding: [0xc4,0xe2,0xea,0x5c,0xf0]
          tdpsbf16ps tmm6, tmm0, tmm2

// CHECK: tdpsbf16ps tmm5, tmm2, tmm0
// CHECK: encoding: [0xc4,0xe2,0xfa,0x5c,0xea]
          tdpsbf16ps tmm5, tmm2, tmm0

// CHECK: tdpsbssd tmm6, tmm0, tmm2
// CHECK: encoding: [0xc4,0xe2,0xeb,0x5e,0xf0]
          tdpsbssd tmm6, tmm0, tmm2

// CHECK: tdpsbssd tmm5, tmm2, tmm0
// CHECK: encoding: [0xc4,0xe2,0xfb,0x5e,0xea]
          tdpsbssd tmm5, tmm2, tmm0

// CHECK: tdpsbsud tmm6, tmm0, tmm2
// CHECK: encoding: [0xc4,0xe2,0xea,0x5e,0xf0]
          tdpsbsud tmm6, tmm0, tmm2

// CHECK: tdpsbsud tmm5, tmm2, tmm0
// CHECK: encoding: [0xc4,0xe2,0xfa,0x5e,0xea]
          tdpsbsud tmm5, tmm2, tmm0

// CHECK: tdpsbusd tmm6, tmm0, tmm2
// CHECK: encoding: [0xc4,0xe2,0xe9,0x5e,0xf0]
          tdpsbusd tmm6, tmm0, tmm2

// CHECK: tdpsbusd tmm5, tmm2, tmm0
// CHECK: encoding: [0xc4,0xe2,0xf9,0x5e,0xea]
          tdpsbusd tmm5, tmm2, tmm0

// CHECK: tdpsbuud tmm6, tmm0, tmm2
// CHECK: encoding: [0xc4,0xe2,0xe8,0x5e,0xf0]
          tdpsbuud tmm6, tmm0, tmm2

// CHECK: tdpsbuud tmm5, tmm2, tmm0
// CHECK: encoding: [0xc4,0xe2,0xf8,0x5e,0xea]
          tdpsbuud tmm5, tmm2, tmm0

// CHECK: tdpsfp16ps tmm6, tmm0, tmm2
// CHECK: encoding: [0xc4,0xe2,0xeb,0x5c,0xf0]
          tdpsfp16ps tmm6, tmm0, tmm2

// CHECK: tdpsfp16ps tmm5, tmm2, tmm0
// CHECK: encoding: [0xc4,0xe2,0xfb,0x5c,0xea]
          tdpsfp16ps tmm5, tmm2, tmm0

// CHECK: tldexpandb tmm2, [rbp + 8*r14 + 268435456], ecx
// CHECK: encoding: [0xc4,0xa2,0x72,0x69,0x94,0xf5,0x00,0x00,0x00,0x10]
          tldexpandb tmm2, [rbp + 8*r14 + 268435456], ecx

// CHECK: tldexpandb tmm0, [r8 + 4*rax + 291], ecx
// CHECK: encoding: [0xc4,0xc2,0x72,0x69,0x84,0x80,0x23,0x01,0x00,0x00]
          tldexpandb tmm0, [r8 + 4*rax + 291], ecx

// CHECK: tldexpandb tmm2, [rcx], ecx
// CHECK: encoding: [0xc4,0xe2,0x72,0x69,0x14,0x21]
          tldexpandb tmm2, [rcx], ecx

// CHECK: tldexpandb tmm0, [2*rbp - 32], ecx
// CHECK: encoding: [0xc4,0xe2,0x72,0x69,0x04,0x6d,0xe0,0xff,0xff,0xff]
          tldexpandb tmm0, [2*rbp - 32], ecx

// CHECK: tldexpandbt1 tmm2, [rbp + 8*r14 + 268435456], ecx
// CHECK: encoding: [0xc4,0xa2,0x73,0x69,0x94,0xf5,0x00,0x00,0x00,0x10]
          tldexpandbt1 tmm2, [rbp + 8*r14 + 268435456], ecx

// CHECK: tldexpandbt1 tmm0, [r8 + 4*rax + 291], ecx
// CHECK: encoding: [0xc4,0xc2,0x73,0x69,0x84,0x80,0x23,0x01,0x00,0x00]
          tldexpandbt1 tmm0, [r8 + 4*rax + 291], ecx

// CHECK: tldexpandbt1 tmm2, [rcx], ecx
// CHECK: encoding: [0xc4,0xe2,0x73,0x69,0x14,0x21]
          tldexpandbt1 tmm2, [rcx], ecx

// CHECK: tldexpandbt1 tmm0, [2*rbp - 32], ecx
// CHECK: encoding: [0xc4,0xe2,0x73,0x69,0x04,0x6d,0xe0,0xff,0xff,0xff]
          tldexpandbt1 tmm0, [2*rbp - 32], ecx

// CHECK: tldexpandw tmm2, [rbp + 8*r14 + 268435456], ecx
// CHECK: encoding: [0xc4,0xa2,0x70,0x69,0x94,0xf5,0x00,0x00,0x00,0x10]
          tldexpandw tmm2, [rbp + 8*r14 + 268435456], ecx

// CHECK: tldexpandw tmm0, [r8 + 4*rax + 291], ecx
// CHECK: encoding: [0xc4,0xc2,0x70,0x69,0x84,0x80,0x23,0x01,0x00,0x00]
          tldexpandw tmm0, [r8 + 4*rax + 291], ecx

// CHECK: tldexpandw tmm2, [rcx], ecx
// CHECK: encoding: [0xc4,0xe2,0x70,0x69,0x14,0x21]
          tldexpandw tmm2, [rcx], ecx

// CHECK: tldexpandw tmm0, [2*rbp - 32], ecx
// CHECK: encoding: [0xc4,0xe2,0x70,0x69,0x04,0x6d,0xe0,0xff,0xff,0xff]
          tldexpandw tmm0, [2*rbp - 32], ecx

// CHECK: tldexpandwt1 tmm2, [rbp + 8*r14 + 268435456], ecx
// CHECK: encoding: [0xc4,0xa2,0x71,0x69,0x94,0xf5,0x00,0x00,0x00,0x10]
          tldexpandwt1 tmm2, [rbp + 8*r14 + 268435456], ecx

// CHECK: tldexpandwt1 tmm0, [r8 + 4*rax + 291], ecx
// CHECK: encoding: [0xc4,0xc2,0x71,0x69,0x84,0x80,0x23,0x01,0x00,0x00]
          tldexpandwt1 tmm0, [r8 + 4*rax + 291], ecx

// CHECK: tldexpandwt1 tmm2, [rcx], ecx
// CHECK: encoding: [0xc4,0xe2,0x71,0x69,0x14,0x21]
          tldexpandwt1 tmm2, [rcx], ecx

// CHECK: tldexpandwt1 tmm0, [2*rbp - 32], ecx
// CHECK: encoding: [0xc4,0xe2,0x71,0x69,0x04,0x6d,0xe0,0xff,0xff,0xff]
          tldexpandwt1 tmm0, [2*rbp - 32], ecx

