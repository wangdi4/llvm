// REQUIRES: intel_feature_isa_amx_future
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tblendvd tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x53,0xe6,0xf4]
               tblendvd tmm6, tmm5, tmm4

// CHECK:      tblendvd tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe6,0xd9]
               tblendvd tmm3, tmm2, tmm1

// CHECK:      tinterleaveeb tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x53,0xe8,0xf4]
               tinterleaveeb tmm6, tmm5, tmm4

// CHECK:      tinterleaveeb tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe8,0xd9]
               tinterleaveeb tmm3, tmm2, tmm1

// CHECK:      tinterleaveew tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x52,0xe8,0xf4]
               tinterleaveew tmm6, tmm5, tmm4

// CHECK:      tinterleaveew tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe8,0xd9]
               tinterleaveew tmm3, tmm2, tmm1

// CHECK:      tinterleaveob tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x50,0xe8,0xf4]
               tinterleaveob tmm6, tmm5, tmm4

// CHECK:      tinterleaveob tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x68,0xe8,0xd9]
               tinterleaveob tmm3, tmm2, tmm1

// CHECK:      tinterleaveow tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x51,0xe8,0xf4]
               tinterleaveow tmm6, tmm5, tmm4

// CHECK:      tinterleaveow tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x69,0xe8,0xd9]
               tinterleaveow tmm3, tmm2, tmm1

// CHECK:      tnarrowb tmm6, tmm5, 123
// CHECK: encoding: [0xc4,0xe3,0x7a,0x58,0xf5,0x7b]
               tnarrowb tmm6, tmm5, 123

// CHECK:      tnarrowb tmm3, tmm2, 123
// CHECK: encoding: [0xc4,0xe3,0x7a,0x58,0xda,0x7b]
               tnarrowb tmm3, tmm2, 123

// CHECK:      tnarroww tmm6, tmm5, 123
// CHECK: encoding: [0xc4,0xe3,0x78,0x59,0xf5,0x7b]
               tnarroww tmm6, tmm5, 123

// CHECK:      tnarroww tmm3, tmm2, 123
// CHECK: encoding: [0xc4,0xe3,0x78,0x59,0xda,0x7b]
               tnarroww tmm3, tmm2, 123

// CHECK:      tpermb tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x52,0xe6,0xf4]
               tpermb tmm6, tmm5, tmm4

// CHECK:      tpermb tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe6,0xd9]
               tpermb tmm3, tmm2, tmm1

// CHECK:      tpermb tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x52,0xe6,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tpermb tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tpermb tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x6a,0xe6,0x9c,0x80,0x23,0x01,0x00,0x00]
               tpermb tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tpermb tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x52,0xe6,0x35,0x00,0x00,0x00,0x00]
               tpermb tmm6, tmm5, byte ptr [rip]

// CHECK:      tpermb tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe6,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tpermb tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tpermd tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x51,0xe7,0xf4]
               tpermd tmm6, tmm5, tmm4

// CHECK:      tpermd tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x69,0xe7,0xd9]
               tpermd tmm3, tmm2, tmm1

// CHECK:      tpermd tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x51,0xe7,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tpermd tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tpermd tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x69,0xe7,0x9c,0x80,0x23,0x01,0x00,0x00]
               tpermd tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tpermd tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x51,0xe7,0x35,0x00,0x00,0x00,0x00]
               tpermd tmm6, tmm5, byte ptr [rip]

// CHECK:      tpermd tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x69,0xe7,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tpermd tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tpermw tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x50,0xe7,0xf4]
               tpermw tmm6, tmm5, tmm4

// CHECK:      tpermw tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x68,0xe7,0xd9]
               tpermw tmm3, tmm2, tmm1

// CHECK:      tpermw tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x50,0xe7,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tpermw tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tpermw tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x68,0xe7,0x9c,0x80,0x23,0x01,0x00,0x00]
               tpermw tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tpermw tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x50,0xe7,0x35,0x00,0x00,0x00,0x00]
               tpermw tmm6, tmm5, byte ptr [rip]

// CHECK:      tpermw tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x68,0xe7,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tpermw tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      twidenb tmm6, tmm5, 123
// CHECK: encoding: [0xc4,0xe3,0x79,0x59,0xf5,0x7b]
               twidenb tmm6, tmm5, 123

// CHECK:      twidenb tmm3, tmm2, 123
// CHECK: encoding: [0xc4,0xe3,0x79,0x59,0xda,0x7b]
               twidenb tmm3, tmm2, 123

// CHECK:      twidenw tmm6, tmm5, 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0x59,0xf5,0x7b]
               twidenw tmm6, tmm5, 123

// CHECK:      twidenw tmm3, tmm2, 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0x59,0xda,0x7b]
               twidenw tmm3, tmm2, 123
