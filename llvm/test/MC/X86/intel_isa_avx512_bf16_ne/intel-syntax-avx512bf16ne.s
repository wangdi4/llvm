// REQUIRES: intel_feature_isa_avx512_bf16_ne
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vaddnepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x40,0x58,0xf0]
          vaddnepbf16 zmm22, zmm23, zmm24

// CHECK: vaddnepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x47,0x58,0xf0]
          vaddnepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vaddnepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0xc7,0x58,0xf0]
          vaddnepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vaddnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x40,0x58,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vaddnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x47,0x58,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vaddnepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x58,0x35,0x00,0x00,0x00,0x00]
          vaddnepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vaddnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x58,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vaddnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vaddnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x45,0xc7,0x58,0x71,0x7f]
          vaddnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vaddnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0xd7,0x58,0x72,0x80]
          vaddnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vcmpnepbf16 k5, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x93,0x47,0x40,0xc2,0xe8,0x7b]
          vcmpnepbf16 k5, zmm23, zmm24, 123

// CHECK: vcmpnepbf16 k5 {k7}, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x93,0x47,0x47,0xc2,0xe8,0x7b]
          vcmpnepbf16 k5 {k7}, zmm23, zmm24, 123

// CHECK: vcmpnepbf16 k5, zmm23, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xb3,0x47,0x40,0xc2,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcmpnepbf16 k5, zmm23, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vcmpnepbf16 k5 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xd3,0x47,0x47,0xc2,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcmpnepbf16 k5 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vcmpnepbf16 k5, zmm23, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x47,0x50,0xc2,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vcmpnepbf16 k5, zmm23, word ptr [rip]{1to32}, 123

// CHECK: vcmpnepbf16 k5, zmm23, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x47,0x40,0xc2,0x2c,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcmpnepbf16 k5, zmm23, zmmword ptr [2*rbp - 2048], 123

// CHECK: vcmpnepbf16 k5 {k7}, zmm23, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x47,0x47,0xc2,0x69,0x7f,0x7b]
          vcmpnepbf16 k5 {k7}, zmm23, zmmword ptr [rcx + 8128], 123

// CHECK: vcmpnepbf16 k5 {k7}, zmm23, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x47,0x57,0xc2,0x6a,0x80,0x7b]
          vcmpnepbf16 k5 {k7}, zmm23, word ptr [rdx - 256]{1to32}, 123

// CHECK: vdivnepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x40,0x5e,0xf0]
          vdivnepbf16 zmm22, zmm23, zmm24

// CHECK: vdivnepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x47,0x5e,0xf0]
          vdivnepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vdivnepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0xc7,0x5e,0xf0]
          vdivnepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vdivnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x40,0x5e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdivnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdivnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x47,0x5e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdivnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vdivnepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x5e,0x35,0x00,0x00,0x00,0x00]
          vdivnepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vdivnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x5e,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vdivnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vdivnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x45,0xc7,0x5e,0x71,0x7f]
          vdivnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vdivnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0xd7,0x5e,0x72,0x80]
          vdivnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfmadd132nepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0x98,0xf0]
          vfmadd132nepbf16 zmm22, zmm23, zmm24

// CHECK: vfmadd132nepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0x98,0xf0]
          vfmadd132nepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vfmadd132nepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0x98,0xf0]
          vfmadd132nepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vfmadd132nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0x98,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd132nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmadd132nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0x98,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd132nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmadd132nepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0x98,0x35,0x00,0x00,0x00,0x00]
          vfmadd132nepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vfmadd132nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0x98,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmadd132nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vfmadd132nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0x98,0x71,0x7f]
          vfmadd132nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vfmadd132nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0x98,0x72,0x80]
          vfmadd132nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfmadd213nepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0xa8,0xf0]
          vfmadd213nepbf16 zmm22, zmm23, zmm24

// CHECK: vfmadd213nepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0xa8,0xf0]
          vfmadd213nepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vfmadd213nepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0xa8,0xf0]
          vfmadd213nepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vfmadd213nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0xa8,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd213nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmadd213nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0xa8,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd213nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmadd213nepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0xa8,0x35,0x00,0x00,0x00,0x00]
          vfmadd213nepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vfmadd213nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0xa8,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmadd213nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vfmadd213nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0xa8,0x71,0x7f]
          vfmadd213nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vfmadd213nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0xa8,0x72,0x80]
          vfmadd213nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfmadd231nepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0xb8,0xf0]
          vfmadd231nepbf16 zmm22, zmm23, zmm24

// CHECK: vfmadd231nepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0xb8,0xf0]
          vfmadd231nepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vfmadd231nepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0xb8,0xf0]
          vfmadd231nepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vfmadd231nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0xb8,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd231nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmadd231nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0xb8,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd231nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmadd231nepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0xb8,0x35,0x00,0x00,0x00,0x00]
          vfmadd231nepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vfmadd231nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0xb8,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmadd231nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vfmadd231nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0xb8,0x71,0x7f]
          vfmadd231nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vfmadd231nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0xb8,0x72,0x80]
          vfmadd231nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfmsub132nepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0x9a,0xf0]
          vfmsub132nepbf16 zmm22, zmm23, zmm24

// CHECK: vfmsub132nepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0x9a,0xf0]
          vfmsub132nepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vfmsub132nepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0x9a,0xf0]
          vfmsub132nepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vfmsub132nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0x9a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub132nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsub132nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0x9a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub132nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsub132nepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0x9a,0x35,0x00,0x00,0x00,0x00]
          vfmsub132nepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vfmsub132nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0x9a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmsub132nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vfmsub132nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0x9a,0x71,0x7f]
          vfmsub132nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vfmsub132nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0x9a,0x72,0x80]
          vfmsub132nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfmsub213nepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0xaa,0xf0]
          vfmsub213nepbf16 zmm22, zmm23, zmm24

// CHECK: vfmsub213nepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0xaa,0xf0]
          vfmsub213nepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vfmsub213nepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0xaa,0xf0]
          vfmsub213nepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vfmsub213nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0xaa,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub213nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsub213nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0xaa,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub213nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsub213nepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0xaa,0x35,0x00,0x00,0x00,0x00]
          vfmsub213nepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vfmsub213nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0xaa,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmsub213nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vfmsub213nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0xaa,0x71,0x7f]
          vfmsub213nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vfmsub213nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0xaa,0x72,0x80]
          vfmsub213nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfmsub231nepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0xba,0xf0]
          vfmsub231nepbf16 zmm22, zmm23, zmm24

// CHECK: vfmsub231nepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0xba,0xf0]
          vfmsub231nepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vfmsub231nepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0xba,0xf0]
          vfmsub231nepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vfmsub231nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0xba,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub231nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfmsub231nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0xba,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub231nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vfmsub231nepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0xba,0x35,0x00,0x00,0x00,0x00]
          vfmsub231nepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vfmsub231nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0xba,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmsub231nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vfmsub231nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0xba,0x71,0x7f]
          vfmsub231nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vfmsub231nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0xba,0x72,0x80]
          vfmsub231nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfnmadd132nepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0x9c,0xf0]
          vfnmadd132nepbf16 zmm22, zmm23, zmm24

// CHECK: vfnmadd132nepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0x9c,0xf0]
          vfnmadd132nepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vfnmadd132nepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0x9c,0xf0]
          vfnmadd132nepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vfnmadd132nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0x9c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd132nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmadd132nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0x9c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd132nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmadd132nepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0x9c,0x35,0x00,0x00,0x00,0x00]
          vfnmadd132nepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vfnmadd132nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0x9c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfnmadd132nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vfnmadd132nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0x9c,0x71,0x7f]
          vfnmadd132nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vfnmadd132nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0x9c,0x72,0x80]
          vfnmadd132nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfnmadd213nepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0xac,0xf0]
          vfnmadd213nepbf16 zmm22, zmm23, zmm24

// CHECK: vfnmadd213nepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0xac,0xf0]
          vfnmadd213nepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vfnmadd213nepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0xac,0xf0]
          vfnmadd213nepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vfnmadd213nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0xac,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd213nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmadd213nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0xac,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd213nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmadd213nepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0xac,0x35,0x00,0x00,0x00,0x00]
          vfnmadd213nepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vfnmadd213nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0xac,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfnmadd213nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vfnmadd213nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0xac,0x71,0x7f]
          vfnmadd213nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vfnmadd213nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0xac,0x72,0x80]
          vfnmadd213nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfnmadd231nepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0xbc,0xf0]
          vfnmadd231nepbf16 zmm22, zmm23, zmm24

// CHECK: vfnmadd231nepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0xbc,0xf0]
          vfnmadd231nepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vfnmadd231nepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0xbc,0xf0]
          vfnmadd231nepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vfnmadd231nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0xbc,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd231nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmadd231nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0xbc,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd231nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmadd231nepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0xbc,0x35,0x00,0x00,0x00,0x00]
          vfnmadd231nepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vfnmadd231nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0xbc,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfnmadd231nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vfnmadd231nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0xbc,0x71,0x7f]
          vfnmadd231nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vfnmadd231nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0xbc,0x72,0x80]
          vfnmadd231nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfnmsub132nepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0x9e,0xf0]
          vfnmsub132nepbf16 zmm22, zmm23, zmm24

// CHECK: vfnmsub132nepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0x9e,0xf0]
          vfnmsub132nepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vfnmsub132nepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0x9e,0xf0]
          vfnmsub132nepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vfnmsub132nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0x9e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub132nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmsub132nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0x9e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub132nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmsub132nepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0x9e,0x35,0x00,0x00,0x00,0x00]
          vfnmsub132nepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vfnmsub132nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0x9e,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfnmsub132nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vfnmsub132nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0x9e,0x71,0x7f]
          vfnmsub132nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vfnmsub132nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0x9e,0x72,0x80]
          vfnmsub132nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfnmsub213nepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0xae,0xf0]
          vfnmsub213nepbf16 zmm22, zmm23, zmm24

// CHECK: vfnmsub213nepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0xae,0xf0]
          vfnmsub213nepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vfnmsub213nepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0xae,0xf0]
          vfnmsub213nepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vfnmsub213nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0xae,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub213nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmsub213nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0xae,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub213nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmsub213nepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0xae,0x35,0x00,0x00,0x00,0x00]
          vfnmsub213nepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vfnmsub213nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0xae,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfnmsub213nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vfnmsub213nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0xae,0x71,0x7f]
          vfnmsub213nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vfnmsub213nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0xae,0x72,0x80]
          vfnmsub213nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfnmsub231nepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0xbe,0xf0]
          vfnmsub231nepbf16 zmm22, zmm23, zmm24

// CHECK: vfnmsub231nepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0xbe,0xf0]
          vfnmsub231nepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vfnmsub231nepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0xbe,0xf0]
          vfnmsub231nepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vfnmsub231nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0xbe,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub231nepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vfnmsub231nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0xbe,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub231nepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vfnmsub231nepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0xbe,0x35,0x00,0x00,0x00,0x00]
          vfnmsub231nepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vfnmsub231nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0xbe,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfnmsub231nepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vfnmsub231nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0xbe,0x71,0x7f]
          vfnmsub231nepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vfnmsub231nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0xbe,0x72,0x80]
          vfnmsub231nepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vfpclassnepbf16 k5, zmm23, 123
// CHECK: encoding: [0x62,0xb3,0x7f,0x48,0x66,0xef,0x7b]
          vfpclassnepbf16 k5, zmm23, 123

// CHECK: vfpclassnepbf16 k5 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xb3,0x7f,0x4f,0x66,0xef,0x7b]
          vfpclassnepbf16 k5 {k7}, zmm23, 123

// CHECK: vfpclassnepbf16 k5, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xb3,0x7f,0x48,0x66,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vfpclassnepbf16 k5, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vfpclassnepbf16 k5 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xd3,0x7f,0x4f,0x66,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vfpclassnepbf16 k5 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vfpclassnepbf16 k5, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x58,0x66,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vfpclassnepbf16 k5, word ptr [rip]{1to32}, 123

// CHECK: vfpclassnepbf16 k5, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x66,0x2c,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vfpclassnepbf16 k5, zmmword ptr [2*rbp - 2048], 123

// CHECK: vfpclassnepbf16 k5 {k7}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x66,0x69,0x7f,0x7b]
          vfpclassnepbf16 k5 {k7}, zmmword ptr [rcx + 8128], 123

// CHECK: vfpclassnepbf16 k5 {k7}, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xf3,0x7f,0x5f,0x66,0x6a,0x80,0x7b]
          vfpclassnepbf16 k5 {k7}, word ptr [rdx - 256]{1to32}, 123

// CHECK: vgetexpnepbf16 zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x42,0xf7]
          vgetexpnepbf16 zmm22, zmm23

// CHECK: vgetexpnepbf16 zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x42,0xf7]
          vgetexpnepbf16 zmm22 {k7}, zmm23

// CHECK: vgetexpnepbf16 zmm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0xcf,0x42,0xf7]
          vgetexpnepbf16 zmm22 {k7} {z}, zmm23

// CHECK: vgetexpnepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x42,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vgetexpnepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vgetexpnepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x42,0xb4,0x80,0x23,0x01,0x00,0x00]
          vgetexpnepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vgetexpnepbf16 zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x42,0x35,0x00,0x00,0x00,0x00]
          vgetexpnepbf16 zmm22, word ptr [rip]{1to32}

// CHECK: vgetexpnepbf16 zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x42,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vgetexpnepbf16 zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vgetexpnepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x42,0x71,0x7f]
          vgetexpnepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vgetexpnepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x42,0x72,0x80]
          vgetexpnepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vgetmantnepbf16 zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x26,0xf7,0x7b]
          vgetmantnepbf16 zmm22, zmm23, 123

// CHECK: vgetmantnepbf16 zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x4f,0x26,0xf7,0x7b]
          vgetmantnepbf16 zmm22 {k7}, zmm23, 123

// CHECK: vgetmantnepbf16 zmm22 {k7} {z}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0xcf,0x26,0xf7,0x7b]
          vgetmantnepbf16 zmm22 {k7} {z}, zmm23, 123

// CHECK: vgetmantnepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x26,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vgetmantnepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vgetmantnepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7f,0x4f,0x26,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vgetmantnepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vgetmantnepbf16 zmm22, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x58,0x26,0x35,0x00,0x00,0x00,0x00,0x7b]
          vgetmantnepbf16 zmm22, word ptr [rip]{1to32}, 123

// CHECK: vgetmantnepbf16 zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x26,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vgetmantnepbf16 zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vgetmantnepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xcf,0x26,0x71,0x7f,0x7b]
          vgetmantnepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vgetmantnepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xdf,0x26,0x72,0x80,0x7b]
          vgetmantnepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123

// CHECK: vmaxnepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x40,0x5f,0xf0]
          vmaxnepbf16 zmm22, zmm23, zmm24

// CHECK: vmaxnepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x47,0x5f,0xf0]
          vmaxnepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vmaxnepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0xc7,0x5f,0xf0]
          vmaxnepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vmaxnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x40,0x5f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmaxnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmaxnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x47,0x5f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmaxnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vmaxnepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x5f,0x35,0x00,0x00,0x00,0x00]
          vmaxnepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vmaxnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x5f,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vmaxnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vmaxnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x45,0xc7,0x5f,0x71,0x7f]
          vmaxnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vmaxnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0xd7,0x5f,0x72,0x80]
          vmaxnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vminnepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x40,0x5d,0xf0]
          vminnepbf16 zmm22, zmm23, zmm24

// CHECK: vminnepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x47,0x5d,0xf0]
          vminnepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vminnepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0xc7,0x5d,0xf0]
          vminnepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vminnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x40,0x5d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vminnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vminnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x47,0x5d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vminnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vminnepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x5d,0x35,0x00,0x00,0x00,0x00]
          vminnepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vminnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x5d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vminnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vminnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x45,0xc7,0x5d,0x71,0x7f]
          vminnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vminnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0xd7,0x5d,0x72,0x80]
          vminnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vmulnepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x40,0x59,0xf0]
          vmulnepbf16 zmm22, zmm23, zmm24

// CHECK: vmulnepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x47,0x59,0xf0]
          vmulnepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vmulnepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0xc7,0x59,0xf0]
          vmulnepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vmulnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x40,0x59,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmulnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmulnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x47,0x59,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmulnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vmulnepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x59,0x35,0x00,0x00,0x00,0x00]
          vmulnepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vmulnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x59,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vmulnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vmulnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x45,0xc7,0x59,0x71,0x7f]
          vmulnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vmulnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0xd7,0x59,0x72,0x80]
          vmulnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vrcpnepbf16 zmm22, zmm23
// CHECK: encoding: [0x62,0xa6,0x7c,0x48,0x4c,0xf7]
          vrcpnepbf16 zmm22, zmm23

// CHECK: vrcpnepbf16 zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa6,0x7c,0x4f,0x4c,0xf7]
          vrcpnepbf16 zmm22 {k7}, zmm23

// CHECK: vrcpnepbf16 zmm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa6,0x7c,0xcf,0x4c,0xf7]
          vrcpnepbf16 zmm22 {k7} {z}, zmm23

// CHECK: vrcpnepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x7c,0x48,0x4c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrcpnepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vrcpnepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x7c,0x4f,0x4c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrcpnepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vrcpnepbf16 zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x7c,0x58,0x4c,0x35,0x00,0x00,0x00,0x00]
          vrcpnepbf16 zmm22, word ptr [rip]{1to32}

// CHECK: vrcpnepbf16 zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x7c,0x48,0x4c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vrcpnepbf16 zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vrcpnepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x7c,0xcf,0x4c,0x71,0x7f]
          vrcpnepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vrcpnepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x7c,0xdf,0x4c,0x72,0x80]
          vrcpnepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vreducenepbf16 zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x56,0xf7,0x7b]
          vreducenepbf16 zmm22, zmm23, 123

// CHECK: vreducenepbf16 zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x4f,0x56,0xf7,0x7b]
          vreducenepbf16 zmm22 {k7}, zmm23, 123

// CHECK: vreducenepbf16 zmm22 {k7} {z}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0xcf,0x56,0xf7,0x7b]
          vreducenepbf16 zmm22 {k7} {z}, zmm23, 123

// CHECK: vreducenepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x56,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vreducenepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vreducenepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7f,0x4f,0x56,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vreducenepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vreducenepbf16 zmm22, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x58,0x56,0x35,0x00,0x00,0x00,0x00,0x7b]
          vreducenepbf16 zmm22, word ptr [rip]{1to32}, 123

// CHECK: vreducenepbf16 zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x56,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vreducenepbf16 zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vreducenepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xcf,0x56,0x71,0x7f,0x7b]
          vreducenepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vreducenepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xdf,0x56,0x72,0x80,0x7b]
          vreducenepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123

// CHECK: vrndscalenepbf16 zmm22, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x08,0xf7,0x7b]
          vrndscalenepbf16 zmm22, zmm23, 123

// CHECK: vrndscalenepbf16 zmm22 {k7}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x4f,0x08,0xf7,0x7b]
          vrndscalenepbf16 zmm22 {k7}, zmm23, 123

// CHECK: vrndscalenepbf16 zmm22 {k7} {z}, zmm23, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0xcf,0x08,0xf7,0x7b]
          vrndscalenepbf16 zmm22 {k7} {z}, zmm23, 123

// CHECK: vrndscalenepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x08,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vrndscalenepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK: vrndscalenepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x7f,0x4f,0x08,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vrndscalenepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK: vrndscalenepbf16 zmm22, word ptr [rip]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x58,0x08,0x35,0x00,0x00,0x00,0x00,0x7b]
          vrndscalenepbf16 zmm22, word ptr [rip]{1to32}, 123

// CHECK: vrndscalenepbf16 zmm22, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x08,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vrndscalenepbf16 zmm22, zmmword ptr [2*rbp - 2048], 123

// CHECK: vrndscalenepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xcf,0x08,0x71,0x7f,0x7b]
          vrndscalenepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128], 123

// CHECK: vrndscalenepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0xdf,0x08,0x72,0x80,0x7b]
          vrndscalenepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}, 123

// CHECK: vrsqrtnepbf16 zmm22, zmm23
// CHECK: encoding: [0x62,0xa6,0x7c,0x48,0x4e,0xf7]
          vrsqrtnepbf16 zmm22, zmm23

// CHECK: vrsqrtnepbf16 zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa6,0x7c,0x4f,0x4e,0xf7]
          vrsqrtnepbf16 zmm22 {k7}, zmm23

// CHECK: vrsqrtnepbf16 zmm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa6,0x7c,0xcf,0x4e,0xf7]
          vrsqrtnepbf16 zmm22 {k7} {z}, zmm23

// CHECK: vrsqrtnepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x7c,0x48,0x4e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrsqrtnepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vrsqrtnepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x7c,0x4f,0x4e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrsqrtnepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vrsqrtnepbf16 zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x7c,0x58,0x4e,0x35,0x00,0x00,0x00,0x00]
          vrsqrtnepbf16 zmm22, word ptr [rip]{1to32}

// CHECK: vrsqrtnepbf16 zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x7c,0x48,0x4e,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vrsqrtnepbf16 zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vrsqrtnepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x7c,0xcf,0x4e,0x71,0x7f]
          vrsqrtnepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vrsqrtnepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x7c,0xdf,0x4e,0x72,0x80]
          vrsqrtnepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vscalefnepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0x2c,0xf0]
          vscalefnepbf16 zmm22, zmm23, zmm24

// CHECK: vscalefnepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x47,0x2c,0xf0]
          vscalefnepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vscalefnepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0xc7,0x2c,0xf0]
          vscalefnepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vscalefnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa6,0x44,0x40,0x2c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vscalefnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vscalefnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc6,0x44,0x47,0x2c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vscalefnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vscalefnepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0x50,0x2c,0x35,0x00,0x00,0x00,0x00]
          vscalefnepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vscalefnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe6,0x44,0x40,0x2c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vscalefnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vscalefnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe6,0x44,0xc7,0x2c,0x71,0x7f]
          vscalefnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vscalefnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe6,0x44,0xd7,0x2c,0x72,0x80]
          vscalefnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

// CHECK: vsqrtnepbf16 zmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x51,0xf7]
          vsqrtnepbf16 zmm22, zmm23

// CHECK: vsqrtnepbf16 zmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x51,0xf7]
          vsqrtnepbf16 zmm22 {k7}, zmm23

// CHECK: vsqrtnepbf16 zmm22 {k7} {z}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0xcf,0x51,0xf7]
          vsqrtnepbf16 zmm22 {k7} {z}, zmm23

// CHECK: vsqrtnepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsqrtnepbf16 zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsqrtnepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsqrtnepbf16 zmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vsqrtnepbf16 zmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x51,0x35,0x00,0x00,0x00,0x00]
          vsqrtnepbf16 zmm22, word ptr [rip]{1to32}

// CHECK: vsqrtnepbf16 zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x51,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vsqrtnepbf16 zmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vsqrtnepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x51,0x71,0x7f]
          vsqrtnepbf16 zmm22 {k7} {z}, zmmword ptr [rcx + 8128]

// CHECK: vsqrtnepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x51,0x72,0x80]
          vsqrtnepbf16 zmm22 {k7} {z}, word ptr [rdx - 256]{1to32}

// CHECK: vsubnepbf16 zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x40,0x5c,0xf0]
          vsubnepbf16 zmm22, zmm23, zmm24

// CHECK: vsubnepbf16 zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0x47,0x5c,0xf0]
          vsubnepbf16 zmm22 {k7}, zmm23, zmm24

// CHECK: vsubnepbf16 zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x45,0xc7,0x5c,0xf0]
          vsubnepbf16 zmm22 {k7} {z}, zmm23, zmm24

// CHECK: vsubnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x40,0x5c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubnepbf16 zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vsubnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x47,0x5c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubnepbf16 zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vsubnepbf16 zmm22, zmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0x50,0x5c,0x35,0x00,0x00,0x00,0x00]
          vsubnepbf16 zmm22, zmm23, word ptr [rip]{1to32}

// CHECK: vsubnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x45,0x40,0x5c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vsubnepbf16 zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vsubnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x45,0xc7,0x5c,0x71,0x7f]
          vsubnepbf16 zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK: vsubnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0x45,0xd7,0x5c,0x72,0x80]
          vsubnepbf16 zmm22 {k7} {z}, zmm23, word ptr [rdx - 256]{1to32}

