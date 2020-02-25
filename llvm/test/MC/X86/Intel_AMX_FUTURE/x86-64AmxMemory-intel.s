// REQUIRES: intel_feature_isa_amx_future
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tgatherrowd tmm6, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x7a,0xe9,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tgatherrowd tmm6, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tgatherrowd tmm3, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x7a,0xe9,0x9c,0x80,0x23,0x01,0x00,0x00]
               tgatherrowd tmm3, byte ptr [r8 + 4*rax + 291]

// CHECK:      tgatherrowd tmm3, byte ptr [2*rbp - 8]
// CHECK: encoding: [0xc4,0xe5,0x7a,0xe9,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tgatherrowd tmm3, byte ptr [2*rbp - 8]

// CHECK:      tgatherrowdt1 tmm6, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x78,0xea,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tgatherrowdt1 tmm6, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tgatherrowdt1 tmm3, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x78,0xea,0x9c,0x80,0x23,0x01,0x00,0x00]
               tgatherrowdt1 tmm3, byte ptr [r8 + 4*rax + 291]

// CHECK:      tgatherrowdt1 tmm6, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x78,0xea,0x35,0x00,0x00,0x00,0x00]
               tgatherrowdt1 tmm6, byte ptr [rip]

// CHECK:      tgatherrowdt1 tmm3, byte ptr [2*rbp - 8]
// CHECK: encoding: [0xc4,0xe5,0x78,0xea,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tgatherrowdt1 tmm3, byte ptr [2*rbp - 8]

// CHECK:      tgatherrowq tmm6, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x79,0xea,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tgatherrowq tmm6, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tgatherrowq tmm3, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x79,0xea,0x9c,0x80,0x23,0x01,0x00,0x00]
               tgatherrowq tmm3, byte ptr [r8 + 4*rax + 291]

// CHECK:      tgatherrowq tmm6, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x79,0xea,0x35,0x00,0x00,0x00,0x00]
               tgatherrowq tmm6, byte ptr [rip]

// CHECK:      tgatherrowq tmm3, byte ptr [2*rbp - 8]
// CHECK: encoding: [0xc4,0xe5,0x79,0xea,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tgatherrowq tmm3, byte ptr [2*rbp - 8]

// CHECK:      tgatherrowqt1 tmm6, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x7b,0xea,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tgatherrowqt1 tmm6, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tgatherrowqt1 tmm3, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x7b,0xea,0x9c,0x80,0x23,0x01,0x00,0x00]
               tgatherrowqt1 tmm3, byte ptr [r8 + 4*rax + 291]

// CHECK:      tgatherrowqt1 tmm6, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x7b,0xea,0x35,0x00,0x00,0x00,0x00]
               tgatherrowqt1 tmm6, byte ptr [rip]

// CHECK:      tgatherrowqt1 tmm3, byte ptr [2*rbp - 8]
// CHECK: encoding: [0xc4,0xe5,0x7b,0xea,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tgatherrowqt1 tmm3, byte ptr [2*rbp - 8]

// CHECK:      tscatterrowd byte ptr [rbp + 8*r14 + 268435456], tmm6
// CHECK: encoding: [0xc4,0xa5,0x7a,0xea,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tscatterrowd byte ptr [rbp + 8*r14 + 268435456], tmm6

// CHECK:      tscatterrowd byte ptr [r8 + 4*rax + 291], tmm3
// CHECK: encoding: [0xc4,0xc5,0x7a,0xea,0x9c,0x80,0x23,0x01,0x00,0x00]
               tscatterrowd byte ptr [r8 + 4*rax + 291], tmm3

// CHECK:      tscatterrowd byte ptr [rip], tmm6
// CHECK: encoding: [0xc4,0xe5,0x7a,0xea,0x35,0x00,0x00,0x00,0x00]
               tscatterrowd byte ptr [rip], tmm6

// CHECK:      tscatterrowd byte ptr [2*rbp - 8], tmm3
// CHECK: encoding: [0xc4,0xe5,0x7a,0xea,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tscatterrowd byte ptr [2*rbp - 8], tmm3

// CHECK:      tscatterrowdt1 byte ptr [rbp + 8*r14 + 268435456], tmm6
// CHECK: encoding: [0xc4,0xa5,0x78,0xeb,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tscatterrowdt1 byte ptr [rbp + 8*r14 + 268435456], tmm6

// CHECK:      tscatterrowdt1 byte ptr [r8 + 4*rax + 291], tmm3
// CHECK: encoding: [0xc4,0xc5,0x78,0xeb,0x9c,0x80,0x23,0x01,0x00,0x00]
               tscatterrowdt1 byte ptr [r8 + 4*rax + 291], tmm3

// CHECK:      tscatterrowdt1 byte ptr [rip], tmm6
// CHECK: encoding: [0xc4,0xe5,0x78,0xeb,0x35,0x00,0x00,0x00,0x00]
               tscatterrowdt1 byte ptr [rip], tmm6

// CHECK:      tscatterrowdt1 byte ptr [2*rbp - 8], tmm3
// CHECK: encoding: [0xc4,0xe5,0x78,0xeb,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tscatterrowdt1 byte ptr [2*rbp - 8], tmm3

// CHECK:      tscatterrowq byte ptr [rbp + 8*r14 + 268435456], tmm6
// CHECK: encoding: [0xc4,0xa5,0x79,0xeb,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tscatterrowq byte ptr [rbp + 8*r14 + 268435456], tmm6

// CHECK:      tscatterrowq byte ptr [r8 + 4*rax + 291], tmm3
// CHECK: encoding: [0xc4,0xc5,0x79,0xeb,0x9c,0x80,0x23,0x01,0x00,0x00]
               tscatterrowq byte ptr [r8 + 4*rax + 291], tmm3

// CHECK:      tscatterrowq byte ptr [rip], tmm6
// CHECK: encoding: [0xc4,0xe5,0x79,0xeb,0x35,0x00,0x00,0x00,0x00]
               tscatterrowq byte ptr [rip], tmm6

// CHECK:      tscatterrowq byte ptr [2*rbp - 8], tmm3
// CHECK: encoding: [0xc4,0xe5,0x79,0xeb,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tscatterrowq byte ptr [2*rbp - 8], tmm3

// CHECK:      tscatterrowqt1 byte ptr [rbp + 8*r14 + 268435456], tmm6
// CHECK: encoding: [0xc4,0xa5,0x7b,0xeb,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tscatterrowqt1 byte ptr [rbp + 8*r14 + 268435456], tmm6

// CHECK:      tscatterrowqt1 byte ptr [r8 + 4*rax + 291], tmm3
// CHECK: encoding: [0xc4,0xc5,0x7b,0xeb,0x9c,0x80,0x23,0x01,0x00,0x00]
               tscatterrowqt1 byte ptr [r8 + 4*rax + 291], tmm3

// CHECK:      tscatterrowqt1 byte ptr [rip], tmm6
// CHECK: encoding: [0xc4,0xe5,0x7b,0xeb,0x35,0x00,0x00,0x00,0x00]
               tscatterrowqt1 byte ptr [rip], tmm6

// CHECK:      tscatterrowqt1 byte ptr [2*rbp - 8], tmm3
// CHECK: encoding: [0xc4,0xe5,0x7b,0xeb,0x1c,0x6d,0xf8,0xff,0xff,0xff]
               tscatterrowqt1 byte ptr [2*rbp - 8], tmm3
