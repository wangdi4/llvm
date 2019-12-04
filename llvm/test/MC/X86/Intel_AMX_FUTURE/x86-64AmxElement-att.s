// REQUIRES: intel_feature_isa_amx_future
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s
// CHECK:      taddps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x50,0xe0,0xf4]
               taddps %tmm4, %tmm5, %tmm6

// CHECK:      taddps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x68,0xe0,0xd9]
               taddps %tmm1, %tmm2, %tmm3

// CHECK:      taddps 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x50,0xe0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               taddps 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      taddps 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x68,0xe0,0x9c,0x80,0x23,0x01,0x00,0x00]
               taddps 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      taddps (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x50,0xe0,0x35,0x00,0x00,0x00,0x00]
               taddps (%rip), %tmm5, %tmm6

// CHECK:      taddps -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x68,0xe0,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               taddps -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tandd %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x51,0xe0,0xf4]
               tandd %tmm4, %tmm5, %tmm6

// CHECK:      tandd %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x69,0xe0,0xd9]
               tandd %tmm1, %tmm2, %tmm3

// CHECK:      tandd 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x51,0xe0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tandd 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tandd 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x69,0xe0,0x9c,0x80,0x23,0x01,0x00,0x00]
               tandd 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tandd (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x51,0xe0,0x35,0x00,0x00,0x00,0x00]
               tandd (%rip), %tmm5, %tmm6

// CHECK:      tandd -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x69,0xe0,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tandd -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tandnd %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x53,0xe0,0xf4]
               tandnd %tmm4, %tmm5, %tmm6

// CHECK:      tandnd %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe0,0xd9]
               tandnd %tmm1, %tmm2, %tmm3

// CHECK:      tandnd 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x53,0xe0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tandnd 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tandnd 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x6b,0xe0,0x9c,0x80,0x23,0x01,0x00,0x00]
               tandnd 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tandnd (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x53,0xe0,0x35,0x00,0x00,0x00,0x00]
               tandnd (%rip), %tmm5, %tmm6

// CHECK:      tandnd -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe0,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tandnd -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tcmpps $123, %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe3,0x52,0x59,0xf4,0x7b]
               tcmpps $123, %tmm4, %tmm5, %tmm6

// CHECK:      tcmpps $123, %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe3,0x6a,0x59,0xd9,0x7b]
               tcmpps $123, %tmm1, %tmm2, %tmm3

// CHECK:      tcmpps $123, 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa3,0x52,0x59,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               tcmpps $123, 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tcmpps $123, 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc3,0x6a,0x59,0x9c,0x80,0x23,0x01,0x00,0x00,0x7b]
               tcmpps $123, 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tcmpps $123, (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe3,0x52,0x59,0x35,0x00,0x00,0x00,0x00,0x7b]
               tcmpps $123, (%rip), %tmm5, %tmm6

// CHECK:      tcmpps $123, -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe3,0x6a,0x59,0x1c,0x6d,0xe0,0xff,0xff,0xff,0x7b]
               tcmpps $123, -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tcvtb2ps %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x78,0xe1,0xf5]
               tcvtb2ps %tmm5, %tmm6

// CHECK:      tcvtb2ps %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x78,0xe1,0xda]
               tcvtb2ps %tmm2, %tmm3

// CHECK:      tcvtbf162ps %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x7b,0xe1,0xf5]
               tcvtbf162ps %tmm5, %tmm6

// CHECK:      tcvtbf162ps %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x7b,0xe1,0xda]
               tcvtbf162ps %tmm2, %tmm3

// CHECK:      tcvtd2ps %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x7a,0xe1,0xf5]
               tcvtd2ps %tmm5, %tmm6

// CHECK:      tcvtd2ps %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x7a,0xe1,0xda]
               tcvtd2ps %tmm2, %tmm3

// CHECK:      tcvtps2bf16 %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x7b,0xe2,0xf5]
               tcvtps2bf16 %tmm5, %tmm6

// CHECK:      tcvtps2bf16 %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x7b,0xe2,0xda]
               tcvtps2bf16 %tmm2, %tmm3

// CHECK:      tcvtps2bs %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x78,0xe2,0xf5]
               tcvtps2bs %tmm5, %tmm6

// CHECK:      tcvtps2bs %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x78,0xe2,0xda]
               tcvtps2bs %tmm2, %tmm3

// CHECK:      tcvtps2ubs %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x79,0xe2,0xf5]
               tcvtps2ubs %tmm5, %tmm6

// CHECK:      tcvtps2ubs %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x79,0xe2,0xda]
               tcvtps2ubs %tmm2, %tmm3

// CHECK:      tcvtub2ps %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x79,0xe1,0xf5]
               tcvtub2ps %tmm5, %tmm6

// CHECK:      tcvtub2ps %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x79,0xe1,0xda]
               tcvtub2ps %tmm2, %tmm3

// CHECK:      tfmaddps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x52,0xe2,0xf4]
               tfmaddps %tmm4, %tmm5, %tmm6

// CHECK:      tfmaddps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe2,0xd9]
               tfmaddps %tmm1, %tmm2, %tmm3

// CHECK:      tfmaddps 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x52,0xe2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tfmaddps 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tfmaddps 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x6a,0xe2,0x9c,0x80,0x23,0x01,0x00,0x00]
               tfmaddps 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tfmaddps (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x52,0xe2,0x35,0x00,0x00,0x00,0x00]
               tfmaddps (%rip), %tmm5, %tmm6

// CHECK:      tfmaddps -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe2,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tfmaddps -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tfmsubps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x50,0xe3,0xf4]
               tfmsubps %tmm4, %tmm5, %tmm6

// CHECK:      tfmsubps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x68,0xe3,0xd9]
               tfmsubps %tmm1, %tmm2, %tmm3

// CHECK:      tfmsubps 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x50,0xe3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tfmsubps 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tfmsubps 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x68,0xe3,0x9c,0x80,0x23,0x01,0x00,0x00]
               tfmsubps 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tfmsubps (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x50,0xe3,0x35,0x00,0x00,0x00,0x00]
               tfmsubps (%rip), %tmm5, %tmm6

// CHECK:      tfmsubps -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x68,0xe3,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tfmsubps -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tfnmaddps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x51,0xe3,0xf4]
               tfnmaddps %tmm4, %tmm5, %tmm6

// CHECK:      tfnmaddps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x69,0xe3,0xd9]
               tfnmaddps %tmm1, %tmm2, %tmm3

// CHECK:      tfnmaddps 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x51,0xe3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tfnmaddps 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tfnmaddps 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x69,0xe3,0x9c,0x80,0x23,0x01,0x00,0x00]
               tfnmaddps 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tfnmaddps (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x51,0xe3,0x35,0x00,0x00,0x00,0x00]
               tfnmaddps (%rip), %tmm5, %tmm6

// CHECK:      tfnmaddps -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x69,0xe3,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tfnmaddps -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tfnmsubps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x53,0xe3,0xf4]
               tfnmsubps %tmm4, %tmm5, %tmm6

// CHECK:      tfnmsubps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe3,0xd9]
               tfnmsubps %tmm1, %tmm2, %tmm3

// CHECK:      tfnmsubps 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x53,0xe3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tfnmsubps 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tfnmsubps 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x6b,0xe3,0x9c,0x80,0x23,0x01,0x00,0x00]
               tfnmsubps 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tfnmsubps (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x53,0xe3,0x35,0x00,0x00,0x00,0x00]
               tfnmsubps (%rip), %tmm5, %tmm6

// CHECK:      tfnmsubps -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe3,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tfnmsubps -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tmaxps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x52,0xe3,0xf4]
               tmaxps %tmm4, %tmm5, %tmm6

// CHECK:      tmaxps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe3,0xd9]
               tmaxps %tmm1, %tmm2, %tmm3

// CHECK:      tmaxps 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x52,0xe3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tmaxps 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tmaxps 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x6a,0xe3,0x9c,0x80,0x23,0x01,0x00,0x00]
               tmaxps 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tmaxps (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x52,0xe3,0x35,0x00,0x00,0x00,0x00]
               tmaxps (%rip), %tmm5, %tmm6

// CHECK:      tmaxps -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe3,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tmaxps -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tminps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x50,0xe4,0xf4]
               tminps %tmm4, %tmm5, %tmm6

// CHECK:      tminps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x68,0xe4,0xd9]
               tminps %tmm1, %tmm2, %tmm3

// CHECK:      tminps 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x50,0xe4,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tminps 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tminps 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x68,0xe4,0x9c,0x80,0x23,0x01,0x00,0x00]
               tminps 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tminps (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x50,0xe4,0x35,0x00,0x00,0x00,0x00]
               tminps (%rip), %tmm5, %tmm6

// CHECK:      tminps -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x68,0xe4,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tminps -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tmulps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x51,0xe4,0xf4]
               tmulps %tmm4, %tmm5, %tmm6

// CHECK:      tmulps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x69,0xe4,0xd9]
               tmulps %tmm1, %tmm2, %tmm3

// CHECK:      tmulps 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x51,0xe4,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tmulps 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tmulps 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x69,0xe4,0x9c,0x80,0x23,0x01,0x00,0x00]
               tmulps 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tmulps (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x51,0xe4,0x35,0x00,0x00,0x00,0x00]
               tmulps (%rip), %tmm5, %tmm6

// CHECK:      tmulps -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x69,0xe4,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tmulps -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tord %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x53,0xe4,0xf4]
               tord %tmm4, %tmm5, %tmm6

// CHECK:      tord %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe4,0xd9]
               tord %tmm1, %tmm2, %tmm3

// CHECK:      tord 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x53,0xe4,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tord 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tord 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x6b,0xe4,0x9c,0x80,0x23,0x01,0x00,0x00]
               tord 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tord (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x53,0xe4,0x35,0x00,0x00,0x00,0x00]
               tord (%rip), %tmm5, %tmm6

// CHECK:      tord -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe4,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tord -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      trcp14ps %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x7a,0xe4,0xf5]
               trcp14ps %tmm5, %tmm6

// CHECK:      trcp14ps %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x7a,0xe4,0xda]
               trcp14ps %tmm2, %tmm3

// CHECK:      treduceps $123, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe3,0x78,0x58,0xf5,0x7b]
               treduceps $123, %tmm5, %tmm6

// CHECK:      treduceps $123, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe3,0x78,0x58,0xda,0x7b]
               treduceps $123, %tmm2, %tmm3

// CHECK:      tscalefps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x50,0xe5,0xf4]
               tscalefps %tmm4, %tmm5, %tmm6

// CHECK:      tscalefps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x68,0xe5,0xd9]
               tscalefps %tmm1, %tmm2, %tmm3

// CHECK:      tscalefps 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x50,0xe5,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tscalefps 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tscalefps 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x68,0xe5,0x9c,0x80,0x23,0x01,0x00,0x00]
               tscalefps 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tscalefps (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x50,0xe5,0x35,0x00,0x00,0x00,0x00]
               tscalefps (%rip), %tmm5, %tmm6

// CHECK:      tscalefps -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x68,0xe5,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tscalefps -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tslld $123, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe3,0x79,0x58,0xf5,0x7b]
               tslld $123, %tmm5, %tmm6

// CHECK:      tslld $123, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe3,0x79,0x58,0xda,0x7b]
               tslld $123, %tmm2, %tmm3

// CHECK:      tsrld $123, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe3,0x7b,0x58,0xf5,0x7b]
               tsrld $123, %tmm5, %tmm6

// CHECK:      tsrld $123, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe3,0x7b,0x58,0xda,0x7b]
               tsrld $123, %tmm2, %tmm3

// CHECK:      tsrlvd %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x51,0xe5,0xf4]
               tsrlvd %tmm4, %tmm5, %tmm6

// CHECK:      tsrlvd %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x69,0xe5,0xd9]
               tsrlvd %tmm1, %tmm2, %tmm3

// CHECK:      tsrlvd 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x51,0xe5,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tsrlvd 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tsrlvd 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x69,0xe5,0x9c,0x80,0x23,0x01,0x00,0x00]
               tsrlvd 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tsrlvd (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x51,0xe5,0x35,0x00,0x00,0x00,0x00]
               tsrlvd (%rip), %tmm5, %tmm6

// CHECK:      tsrlvd -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x69,0xe5,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tsrlvd -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tsubps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x53,0xe5,0xf4]
               tsubps %tmm4, %tmm5, %tmm6

// CHECK:      tsubps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe5,0xd9]
               tsubps %tmm1, %tmm2, %tmm3

// CHECK:      tsubps 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x53,0xe5,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tsubps 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tsubps 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x6b,0xe5,0x9c,0x80,0x23,0x01,0x00,0x00]
               tsubps 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tsubps (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x53,0xe5,0x35,0x00,0x00,0x00,0x00]
               tsubps (%rip), %tmm5, %tmm6

// CHECK:      tsubps -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe5,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tsubps -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      txord %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x52,0xe5,0xf4]
               txord %tmm4, %tmm5, %tmm6

// CHECK:      txord %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe5,0xd9]
               txord %tmm1, %tmm2, %tmm3

// CHECK:      txord 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x52,0xe5,0xb4,0xf5,0x00,0x00,0x00,0x10]
               txord 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      txord 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x6a,0xe5,0x9c,0x80,0x23,0x01,0x00,0x00]
               txord 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      txord (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x52,0xe5,0x35,0x00,0x00,0x00,0x00]
               txord (%rip), %tmm5, %tmm6

// CHECK:      txord -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe5,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               txord -32(,%rbp,2), %tmm2, %tmm3
