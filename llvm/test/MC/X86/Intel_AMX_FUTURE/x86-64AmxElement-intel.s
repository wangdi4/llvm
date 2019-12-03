// REQUIRES: intel_feature_isa_amx_future
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s
// CHECK:      taddps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x50,0xe0,0xf4]
               taddps tmm6, tmm5, tmm4

// CHECK:      taddps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x68,0xe0,0xd9]
               taddps tmm3, tmm2, tmm1

// CHECK:      taddps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x50,0xe0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               taddps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      taddps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x68,0xe0,0x9c,0x80,0x23,0x01,0x00,0x00]
               taddps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      taddps tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x50,0xe0,0x35,0x00,0x00,0x00,0x00]
               taddps tmm6, tmm5, byte ptr [rip]

// CHECK:      taddps tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x68,0xe0,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               taddps tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tandd tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x51,0xe0,0xf4]
               tandd tmm6, tmm5, tmm4

// CHECK:      tandd tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x69,0xe0,0xd9]
               tandd tmm3, tmm2, tmm1

// CHECK:      tandd tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x51,0xe0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tandd tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tandd tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x69,0xe0,0x9c,0x80,0x23,0x01,0x00,0x00]
               tandd tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tandd tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x51,0xe0,0x35,0x00,0x00,0x00,0x00]
               tandd tmm6, tmm5, byte ptr [rip]

// CHECK:      tandd tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x69,0xe0,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tandd tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tandnd tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x53,0xe0,0xf4]
               tandnd tmm6, tmm5, tmm4

// CHECK:      tandnd tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe0,0xd9]
               tandnd tmm3, tmm2, tmm1

// CHECK:      tandnd tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x53,0xe0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tandnd tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tandnd tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x6b,0xe0,0x9c,0x80,0x23,0x01,0x00,0x00]
               tandnd tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tandnd tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x53,0xe0,0x35,0x00,0x00,0x00,0x00]
               tandnd tmm6, tmm5, byte ptr [rip]

// CHECK:      tandnd tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe0,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tandnd tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tcmpps tmm6, tmm5, tmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x52,0x59,0xf4,0x7b]
               tcmpps tmm6, tmm5, tmm4, 123

// CHECK:      tcmpps tmm3, tmm2, tmm1, 123
// CHECK: encoding: [0xc4,0xe3,0x6a,0x59,0xd9,0x7b]
               tcmpps tmm3, tmm2, tmm1, 123

// CHECK:      tcmpps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0xa3,0x52,0x59,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               tcmpps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      tcmpps tmm3, tmm2, byte ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0xc3,0x6a,0x59,0x9c,0x80,0x23,0x01,0x00,0x00,0x7b]
               tcmpps tmm3, tmm2, byte ptr [r8 + 4*rax + 291], 123

// CHECK:      tcmpps tmm6, tmm5, byte ptr [rip], 123
// CHECK: encoding: [0xc4,0xe3,0x52,0x59,0x35,0x00,0x00,0x00,0x00,0x7b]
               tcmpps tmm6, tmm5, byte ptr [rip], 123

// CHECK:      tcmpps tmm3, tmm2, byte ptr [2*rbp - 32], 123
// CHECK: encoding: [0xc4,0xe3,0x6a,0x59,0x1c,0x6d,0xe0,0xff,0xff,0xff,0x7b]
               tcmpps tmm3, tmm2, byte ptr [2*rbp - 32], 123

// CHECK:      tcvtb2ps tmm6, tmm5
// CHECK: encoding: [0xc4,0xe5,0x78,0xe1,0xf5]
               tcvtb2ps tmm6, tmm5

// CHECK:      tcvtb2ps tmm3, tmm2
// CHECK: encoding: [0xc4,0xe5,0x78,0xe1,0xda]
               tcvtb2ps tmm3, tmm2

// CHECK:      tcvtbf162ps tmm6, tmm5
// CHECK: encoding: [0xc4,0xe5,0x7b,0xe1,0xf5]
               tcvtbf162ps tmm6, tmm5

// CHECK:      tcvtbf162ps tmm3, tmm2
// CHECK: encoding: [0xc4,0xe5,0x7b,0xe1,0xda]
               tcvtbf162ps tmm3, tmm2

// CHECK:      tcvtd2ps tmm6, tmm5
// CHECK: encoding: [0xc4,0xe5,0x7a,0xe1,0xf5]
               tcvtd2ps tmm6, tmm5

// CHECK:      tcvtd2ps tmm3, tmm2
// CHECK: encoding: [0xc4,0xe5,0x7a,0xe1,0xda]
               tcvtd2ps tmm3, tmm2

// CHECK:      tcvtps2bf16 tmm6, tmm5
// CHECK: encoding: [0xc4,0xe5,0x7b,0xe2,0xf5]
               tcvtps2bf16 tmm6, tmm5

// CHECK:      tcvtps2bf16 tmm3, tmm2
// CHECK: encoding: [0xc4,0xe5,0x7b,0xe2,0xda]
               tcvtps2bf16 tmm3, tmm2

// CHECK:      tcvtps2bs tmm6, tmm5
// CHECK: encoding: [0xc4,0xe5,0x78,0xe2,0xf5]
               tcvtps2bs tmm6, tmm5

// CHECK:      tcvtps2bs tmm3, tmm2
// CHECK: encoding: [0xc4,0xe5,0x78,0xe2,0xda]
               tcvtps2bs tmm3, tmm2

// CHECK:      tcvtps2ubs tmm6, tmm5
// CHECK: encoding: [0xc4,0xe5,0x79,0xe2,0xf5]
               tcvtps2ubs tmm6, tmm5

// CHECK:      tcvtps2ubs tmm3, tmm2
// CHECK: encoding: [0xc4,0xe5,0x79,0xe2,0xda]
               tcvtps2ubs tmm3, tmm2

// CHECK:      tcvtub2ps tmm6, tmm5
// CHECK: encoding: [0xc4,0xe5,0x79,0xe1,0xf5]
               tcvtub2ps tmm6, tmm5

// CHECK:      tcvtub2ps tmm3, tmm2
// CHECK: encoding: [0xc4,0xe5,0x79,0xe1,0xda]
               tcvtub2ps tmm3, tmm2

// CHECK:      tfmaddps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x52,0xe2,0xf4]
               tfmaddps tmm6, tmm5, tmm4

// CHECK:      tfmaddps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe2,0xd9]
               tfmaddps tmm3, tmm2, tmm1

// CHECK:      tfmaddps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x52,0xe2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tfmaddps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tfmaddps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x6a,0xe2,0x9c,0x80,0x23,0x01,0x00,0x00]
               tfmaddps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tfmaddps tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x52,0xe2,0x35,0x00,0x00,0x00,0x00]
               tfmaddps tmm6, tmm5, byte ptr [rip]

// CHECK:      tfmaddps tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe2,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tfmaddps tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tfmsubps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x50,0xe3,0xf4]
               tfmsubps tmm6, tmm5, tmm4

// CHECK:      tfmsubps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x68,0xe3,0xd9]
               tfmsubps tmm3, tmm2, tmm1

// CHECK:      tfmsubps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x50,0xe3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tfmsubps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tfmsubps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x68,0xe3,0x9c,0x80,0x23,0x01,0x00,0x00]
               tfmsubps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tfmsubps tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x50,0xe3,0x35,0x00,0x00,0x00,0x00]
               tfmsubps tmm6, tmm5, byte ptr [rip]

// CHECK:      tfmsubps tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x68,0xe3,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tfmsubps tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tfnmaddps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x51,0xe3,0xf4]
               tfnmaddps tmm6, tmm5, tmm4

// CHECK:      tfnmaddps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x69,0xe3,0xd9]
               tfnmaddps tmm3, tmm2, tmm1

// CHECK:      tfnmaddps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x51,0xe3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tfnmaddps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tfnmaddps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x69,0xe3,0x9c,0x80,0x23,0x01,0x00,0x00]
               tfnmaddps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tfnmaddps tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x51,0xe3,0x35,0x00,0x00,0x00,0x00]
               tfnmaddps tmm6, tmm5, byte ptr [rip]

// CHECK:      tfnmaddps tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x69,0xe3,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tfnmaddps tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tfnmsubps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x53,0xe3,0xf4]
               tfnmsubps tmm6, tmm5, tmm4

// CHECK:      tfnmsubps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe3,0xd9]
               tfnmsubps tmm3, tmm2, tmm1

// CHECK:      tfnmsubps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x53,0xe3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tfnmsubps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tfnmsubps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x6b,0xe3,0x9c,0x80,0x23,0x01,0x00,0x00]
               tfnmsubps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tfnmsubps tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x53,0xe3,0x35,0x00,0x00,0x00,0x00]
               tfnmsubps tmm6, tmm5, byte ptr [rip]

// CHECK:      tfnmsubps tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe3,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tfnmsubps tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tmaxps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x52,0xe3,0xf4]
               tmaxps tmm6, tmm5, tmm4

// CHECK:      tmaxps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe3,0xd9]
               tmaxps tmm3, tmm2, tmm1

// CHECK:      tmaxps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x52,0xe3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tmaxps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tmaxps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x6a,0xe3,0x9c,0x80,0x23,0x01,0x00,0x00]
               tmaxps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tmaxps tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x52,0xe3,0x35,0x00,0x00,0x00,0x00]
               tmaxps tmm6, tmm5, byte ptr [rip]

// CHECK:      tmaxps tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe3,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tmaxps tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tminps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x50,0xe4,0xf4]
               tminps tmm6, tmm5, tmm4

// CHECK:      tminps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x68,0xe4,0xd9]
               tminps tmm3, tmm2, tmm1

// CHECK:      tminps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x50,0xe4,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tminps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tminps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x68,0xe4,0x9c,0x80,0x23,0x01,0x00,0x00]
               tminps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tminps tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x50,0xe4,0x35,0x00,0x00,0x00,0x00]
               tminps tmm6, tmm5, byte ptr [rip]

// CHECK:      tminps tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x68,0xe4,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tminps tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tmulps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x51,0xe4,0xf4]
               tmulps tmm6, tmm5, tmm4

// CHECK:      tmulps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x69,0xe4,0xd9]
               tmulps tmm3, tmm2, tmm1

// CHECK:      tmulps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x51,0xe4,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tmulps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tmulps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x69,0xe4,0x9c,0x80,0x23,0x01,0x00,0x00]
               tmulps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tmulps tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x51,0xe4,0x35,0x00,0x00,0x00,0x00]
               tmulps tmm6, tmm5, byte ptr [rip]

// CHECK:      tmulps tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x69,0xe4,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tmulps tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tord tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x53,0xe4,0xf4]
               tord tmm6, tmm5, tmm4

// CHECK:      tord tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe4,0xd9]
               tord tmm3, tmm2, tmm1

// CHECK:      tord tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x53,0xe4,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tord tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tord tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x6b,0xe4,0x9c,0x80,0x23,0x01,0x00,0x00]
               tord tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tord tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x53,0xe4,0x35,0x00,0x00,0x00,0x00]
               tord tmm6, tmm5, byte ptr [rip]

// CHECK:      tord tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe4,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tord tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      trcp14ps tmm6, tmm5
// CHECK: encoding: [0xc4,0xe5,0x7a,0xe4,0xf5]
               trcp14ps tmm6, tmm5

// CHECK:      trcp14ps tmm3, tmm2
// CHECK: encoding: [0xc4,0xe5,0x7a,0xe4,0xda]
               trcp14ps tmm3, tmm2

// CHECK:      treduceps tmm6, tmm5, 123
// CHECK: encoding: [0xc4,0xe3,0x78,0x58,0xf5,0x7b]
               treduceps tmm6, tmm5, 123

// CHECK:      treduceps tmm3, tmm2, 123
// CHECK: encoding: [0xc4,0xe3,0x78,0x58,0xda,0x7b]
               treduceps tmm3, tmm2, 123

// CHECK:      tscalefps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x50,0xe5,0xf4]
               tscalefps tmm6, tmm5, tmm4

// CHECK:      tscalefps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x68,0xe5,0xd9]
               tscalefps tmm3, tmm2, tmm1

// CHECK:      tscalefps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x50,0xe5,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tscalefps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tscalefps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x68,0xe5,0x9c,0x80,0x23,0x01,0x00,0x00]
               tscalefps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tscalefps tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x50,0xe5,0x35,0x00,0x00,0x00,0x00]
               tscalefps tmm6, tmm5, byte ptr [rip]

// CHECK:      tscalefps tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x68,0xe5,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tscalefps tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tslld tmm6, tmm5, 123
// CHECK: encoding: [0xc4,0xe3,0x79,0x58,0xf5,0x7b]
               tslld tmm6, tmm5, 123

// CHECK:      tslld tmm3, tmm2, 123
// CHECK: encoding: [0xc4,0xe3,0x79,0x58,0xda,0x7b]
               tslld tmm3, tmm2, 123

// CHECK:      tsrld tmm6, tmm5, 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0x58,0xf5,0x7b]
               tsrld tmm6, tmm5, 123

// CHECK:      tsrld tmm3, tmm2, 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0x58,0xda,0x7b]
               tsrld tmm3, tmm2, 123

// CHECK:      tsrlvd tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x51,0xe5,0xf4]
               tsrlvd tmm6, tmm5, tmm4

// CHECK:      tsrlvd tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x69,0xe5,0xd9]
               tsrlvd tmm3, tmm2, tmm1

// CHECK:      tsrlvd tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x51,0xe5,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tsrlvd tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tsrlvd tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x69,0xe5,0x9c,0x80,0x23,0x01,0x00,0x00]
               tsrlvd tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tsrlvd tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x51,0xe5,0x35,0x00,0x00,0x00,0x00]
               tsrlvd tmm6, tmm5, byte ptr [rip]

// CHECK:      tsrlvd tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x69,0xe5,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tsrlvd tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      tsubps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x53,0xe5,0xf4]
               tsubps tmm6, tmm5, tmm4

// CHECK:      tsubps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe5,0xd9]
               tsubps tmm3, tmm2, tmm1

// CHECK:      tsubps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x53,0xe5,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tsubps tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      tsubps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x6b,0xe5,0x9c,0x80,0x23,0x01,0x00,0x00]
               tsubps tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      tsubps tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x53,0xe5,0x35,0x00,0x00,0x00,0x00]
               tsubps tmm6, tmm5, byte ptr [rip]

// CHECK:      tsubps tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe5,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tsubps tmm3, tmm2, byte ptr [2*rbp - 32]

// CHECK:      txord tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x52,0xe5,0xf4]
               txord tmm6, tmm5, tmm4

// CHECK:      txord tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe5,0xd9]
               txord tmm3, tmm2, tmm1

// CHECK:      txord tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x52,0xe5,0xb4,0xf5,0x00,0x00,0x00,0x10]
               txord tmm6, tmm5, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      txord tmm3, tmm2, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x6a,0xe5,0x9c,0x80,0x23,0x01,0x00,0x00]
               txord tmm3, tmm2, byte ptr [r8 + 4*rax + 291]

// CHECK:      txord tmm6, tmm5, byte ptr [rip]
// CHECK: encoding: [0xc4,0xe5,0x52,0xe5,0x35,0x00,0x00,0x00,0x00]
               txord tmm6, tmm5, byte ptr [rip]

// CHECK:      txord tmm3, tmm2, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe5,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               txord tmm3, tmm2, byte ptr [2*rbp - 32]
