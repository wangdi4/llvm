// REQUIRES: intel_feature_isa_amx_future
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s
// CHECK:      tcoladdbcastps tmm6, tmm5
// CHECK: encoding: [0xc4,0xe5,0x79,0xe6,0xf5]
               tcoladdbcastps tmm6, tmm5

// CHECK:      tcoladdbcastps tmm3, tmm2
// CHECK: encoding: [0xc4,0xe5,0x79,0xe6,0xda]
               tcoladdbcastps tmm3, tmm2

// CHECK:      tcoladdps byte ptr [rbp + 8*r14 + 268435456], tmm6
// CHECK: encoding: [0xc4,0xa5,0x78,0xe6,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tcoladdps byte ptr [rbp + 8*r14 + 268435456], tmm6

// CHECK:      tcoladdps byte ptr [r8 + 4*rax + 291], tmm3
// CHECK: encoding: [0xc4,0xc5,0x78,0xe6,0x9c,0x80,0x23,0x01,0x00,0x00]
               tcoladdps byte ptr [r8 + 4*rax + 291], tmm3

// CHECK:      tcoladdps byte ptr [rip], tmm6
// CHECK: encoding: [0xc4,0xe5,0x78,0xe6,0x35,0x00,0x00,0x00,0x00]
               tcoladdps byte ptr [rip], tmm6

// CHECK:      tcoladdps byte ptr [2*rbp - 32], tmm3
// CHECK: encoding: [0xc4,0xe5,0x78,0xe6,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tcoladdps byte ptr [2*rbp - 32], tmm3
