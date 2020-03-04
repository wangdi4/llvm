// REQUIRES: intel_feature_isa_amx_memory
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tgatherrowd tmm15, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x7b,0x6e,0xbc,0xf5,0x00,0x00,0x00,0x10]
               tgatherrowd tmm15, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tgatherrowd tmm3, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x7b,0x6e,0x9c,0x80,0x23,0x01,0x00,0x00]
               tgatherrowd tmm3, byte ptr [r8 + 4*rax + 291]

// CHECK:      tgatherrowd tmm3, byte ptr [2*rbp - 8]
// CHECK: encoding: [0xc4,0xe2,0x7b,0x6e,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tgatherrowd tmm3, byte ptr [2*rbp - 8]

// CHECK:      tgatherrowd tmm15, byte ptr [rcx + 31]
// CHECK: encoding: [0xc4,0x62,0x7b,0x6e,0x7c,0x21,0x1f]
               tgatherrowd tmm15, byte ptr [rcx + 31]

// CHECK:      tgatherrowd tmm3, byte ptr [rdx - 32]
// CHECK: encoding: [0xc4,0xe2,0x7b,0x6e,0x5c,0x22,0xe0]
               tgatherrowd tmm3, byte ptr [rdx - 32]

// CHECK:      tgatherrowdt1 tmm15, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x7b,0x6f,0xbc,0xf5,0x00,0x00,0x00,0x10]
               tgatherrowdt1 tmm15, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tgatherrowdt1 tmm3, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x7b,0x6f,0x9c,0x80,0x23,0x01,0x00,0x00]
               tgatherrowdt1 tmm3, byte ptr [r8 + 4*rax + 291]

// CHECK:      tgatherrowdt1 tmm3, byte ptr [2*rbp - 8]
// CHECK: encoding: [0xc4,0xe2,0x7b,0x6f,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tgatherrowdt1 tmm3, byte ptr [2*rbp - 8]

// CHECK:      tgatherrowdt1 tmm15, byte ptr [rcx + 31]
// CHECK: encoding: [0xc4,0x62,0x7b,0x6f,0x7c,0x21,0x1f]
               tgatherrowdt1 tmm15, byte ptr [rcx + 31]

// CHECK:      tgatherrowdt1 tmm3, byte ptr [rdx - 32]
// CHECK: encoding: [0xc4,0xe2,0x7b,0x6f,0x5c,0x22,0xe0]
               tgatherrowdt1 tmm3, byte ptr [rdx - 32]

// CHECK:      tgatherrowq tmm15, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0xfb,0x6e,0xbc,0xf5,0x00,0x00,0x00,0x10]
               tgatherrowq tmm15, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tgatherrowq tmm3, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0xfb,0x6e,0x9c,0x80,0x23,0x01,0x00,0x00]
               tgatherrowq tmm3, byte ptr [r8 + 4*rax + 291]

// CHECK:      tgatherrowq tmm3, byte ptr [2*rbp - 8]
// CHECK: encoding: [0xc4,0xe2,0xfb,0x6e,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tgatherrowq tmm3, byte ptr [2*rbp - 8]

// CHECK:      tgatherrowq tmm15, byte ptr [rcx + 31]
// CHECK: encoding: [0xc4,0x62,0xfb,0x6e,0x7c,0x21,0x1f]
               tgatherrowq tmm15, byte ptr [rcx + 31]

// CHECK:      tgatherrowq tmm3, byte ptr [rdx - 32]
// CHECK: encoding: [0xc4,0xe2,0xfb,0x6e,0x5c,0x22,0xe0]
               tgatherrowq tmm3, byte ptr [rdx - 32]

// CHECK:      tgatherrowqt1 tmm15, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0xfb,0x6f,0xbc,0xf5,0x00,0x00,0x00,0x10]
               tgatherrowqt1 tmm15, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tgatherrowqt1 tmm3, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0xfb,0x6f,0x9c,0x80,0x23,0x01,0x00,0x00]
               tgatherrowqt1 tmm3, byte ptr [r8 + 4*rax + 291]

// CHECK:      tgatherrowqt1 tmm3, byte ptr [2*rbp - 8]
// CHECK: encoding: [0xc4,0xe2,0xfb,0x6f,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tgatherrowqt1 tmm3, byte ptr [2*rbp - 8]

// CHECK:      tgatherrowqt1 tmm15, byte ptr [rcx + 31]
// CHECK: encoding: [0xc4,0x62,0xfb,0x6f,0x7c,0x21,0x1f]
               tgatherrowqt1 tmm15, byte ptr [rcx + 31]

// CHECK:      tgatherrowqt1 tmm3, byte ptr [rdx - 32]
// CHECK: encoding: [0xc4,0xe2,0xfb,0x6f,0x5c,0x22,0xe0]
               tgatherrowqt1 tmm3, byte ptr [rdx - 32]

// CHECK:      tscatterrowd byte ptr [rbp + 8*r14 + 268435456], tmm15
// CHECK: encoding: [0xc4,0x22,0x7a,0x6e,0xbc,0xf5,0x00,0x00,0x00,0x10]
               tscatterrowd byte ptr [rbp + 8*r14 + 268435456], tmm15

// CHECK:      tscatterrowd byte ptr [r8 + 4*rax + 291], tmm3
// CHECK: encoding: [0xc4,0xc2,0x7a,0x6e,0x9c,0x80,0x23,0x01,0x00,0x00]
               tscatterrowd byte ptr [r8 + 4*rax + 291], tmm3

// CHECK:      tscatterrowd byte ptr [2*rbp - 8], tmm3
// CHECK: encoding: [0xc4,0xe2,0x7a,0x6e,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tscatterrowd byte ptr [2*rbp - 8], tmm3

// CHECK:      tscatterrowd byte ptr [rcx + 31], tmm15
// CHECK: encoding: [0xc4,0x62,0x7a,0x6e,0x7c,0x21,0x1f]
               tscatterrowd byte ptr [rcx + 31], tmm15

// CHECK:      tscatterrowd byte ptr [rdx - 32], tmm3
// CHECK: encoding: [0xc4,0xe2,0x7a,0x6e,0x5c,0x22,0xe0]
               tscatterrowd byte ptr [rdx - 32], tmm3

// CHECK:      tscatterrowdt1 byte ptr [rbp + 8*r14 + 268435456], tmm15
// CHECK: encoding: [0xc4,0x22,0x7a,0x6f,0xbc,0xf5,0x00,0x00,0x00,0x10]
               tscatterrowdt1 byte ptr [rbp + 8*r14 + 268435456], tmm15

// CHECK:      tscatterrowdt1 byte ptr [r8 + 4*rax + 291], tmm3
// CHECK: encoding: [0xc4,0xc2,0x7a,0x6f,0x9c,0x80,0x23,0x01,0x00,0x00]
               tscatterrowdt1 byte ptr [r8 + 4*rax + 291], tmm3

// CHECK:      tscatterrowdt1 byte ptr [2*rbp - 8], tmm3
// CHECK: encoding: [0xc4,0xe2,0x7a,0x6f,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tscatterrowdt1 byte ptr [2*rbp - 8], tmm3

// CHECK:      tscatterrowdt1 byte ptr [rcx + 31], tmm15
// CHECK: encoding: [0xc4,0x62,0x7a,0x6f,0x7c,0x21,0x1f]
               tscatterrowdt1 byte ptr [rcx + 31], tmm15

// CHECK:      tscatterrowdt1 byte ptr [rdx - 32], tmm3
// CHECK: encoding: [0xc4,0xe2,0x7a,0x6f,0x5c,0x22,0xe0]
               tscatterrowdt1 byte ptr [rdx - 32], tmm3

// CHECK:      tscatterrowq byte ptr [rbp + 8*r14 + 268435456], tmm15
// CHECK: encoding: [0xc4,0x22,0xfa,0x6e,0xbc,0xf5,0x00,0x00,0x00,0x10]
               tscatterrowq byte ptr [rbp + 8*r14 + 268435456], tmm15

// CHECK:      tscatterrowq byte ptr [r8 + 4*rax + 291], tmm3
// CHECK: encoding: [0xc4,0xc2,0xfa,0x6e,0x9c,0x80,0x23,0x01,0x00,0x00]
               tscatterrowq byte ptr [r8 + 4*rax + 291], tmm3

// CHECK:      tscatterrowq byte ptr [2*rbp - 8], tmm3
// CHECK: encoding: [0xc4,0xe2,0xfa,0x6e,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tscatterrowq byte ptr [2*rbp - 8], tmm3

// CHECK:      tscatterrowq byte ptr [rcx + 31], tmm15
// CHECK: encoding: [0xc4,0x62,0xfa,0x6e,0x7c,0x21,0x1f]
               tscatterrowq byte ptr [rcx + 31], tmm15

// CHECK:      tscatterrowq byte ptr [rdx - 32], tmm3
// CHECK: encoding: [0xc4,0xe2,0xfa,0x6e,0x5c,0x22,0xe0]
               tscatterrowq byte ptr [rdx - 32], tmm3

// CHECK:      tscatterrowqt1 byte ptr [rbp + 8*r14 + 268435456], tmm15
// CHECK: encoding: [0xc4,0x22,0xfa,0x6f,0xbc,0xf5,0x00,0x00,0x00,0x10]
               tscatterrowqt1 byte ptr [rbp + 8*r14 + 268435456], tmm15

// CHECK:      tscatterrowqt1 byte ptr [r8 + 4*rax + 291], tmm3
// CHECK: encoding: [0xc4,0xc2,0xfa,0x6f,0x9c,0x80,0x23,0x01,0x00,0x00]
               tscatterrowqt1 byte ptr [r8 + 4*rax + 291], tmm3

// CHECK:      tscatterrowqt1 byte ptr [2*rbp - 8], tmm3
// CHECK: encoding: [0xc4,0xe2,0xfa,0x6f,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tscatterrowqt1 byte ptr [2*rbp - 8], tmm3

// CHECK:      tscatterrowqt1 byte ptr [rcx + 31], tmm15
// CHECK: encoding: [0xc4,0x62,0xfa,0x6f,0x7c,0x21,0x1f]
               tscatterrowqt1 byte ptr [rcx + 31], tmm15

// CHECK:      tscatterrowqt1 byte ptr [rdx - 32], tmm3
// CHECK: encoding: [0xc4,0xe2,0xfa,0x6f,0x5c,0x22,0xe0]
               tscatterrowqt1 byte ptr [rdx - 32], tmm3

