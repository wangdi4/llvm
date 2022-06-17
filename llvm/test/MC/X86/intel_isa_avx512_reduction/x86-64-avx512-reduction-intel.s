// REQUIRES: intel_feature_isa_avx512_reduction
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vphraaddbd xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x46,0x40,0x43,0xf0]
          vphraaddbd xmm22, xmm23, zmm24

// CHECK: vphraaddbd xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x46,0x47,0x43,0xf0]
          vphraaddbd xmm22 {k7}, xmm23, zmm24

// CHECK: vphraaddbd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x40,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddbd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraaddbd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x47,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddbd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraaddbd xmm22, xmm23, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraaddbd xmm22, xmm23, zmmword ptr [rip]

// CHECK: vphraaddbd xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x43,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddbd xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraaddbd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x43,0x71,0x7f]
          vphraaddbd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraaddbd xmm22 {k7}, xmm23, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x43,0x72,0x80]
          vphraaddbd xmm22 {k7}, xmm23, zmmword ptr [rdx - 8192]

// CHECK: vphraaddsbd xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x46,0x40,0x44,0xf0]
          vphraaddsbd xmm22, xmm23, zmm24

// CHECK: vphraaddsbd xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x46,0x47,0x44,0xf0]
          vphraaddsbd xmm22 {k7}, xmm23, zmm24

// CHECK: vphraaddsbd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x40,0x44,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddsbd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraaddsbd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x47,0x44,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddsbd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraaddsbd xmm22, xmm23, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraaddsbd xmm22, xmm23, zmmword ptr [rip]

// CHECK: vphraaddsbd xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x44,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddsbd xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraaddsbd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x44,0x71,0x7f]
          vphraaddsbd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraaddsbd xmm22 {k7}, xmm23, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x44,0x72,0x80]
          vphraaddsbd xmm22 {k7}, xmm23, zmmword ptr [rdx - 8192]

// CHECK: vphraaddswd xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x40,0x44,0xf0]
          vphraaddswd xmm22, xmm23, zmm24

// CHECK: vphraaddswd xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x47,0x44,0xf0]
          vphraaddswd xmm22 {k7}, xmm23, zmm24

// CHECK: vphraaddswd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc6,0x40,0x44,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddswd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraaddswd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc6,0x47,0x44,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddswd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraaddswd xmm22, xmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xc6,0x50,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraaddswd xmm22, xmm23, word ptr [rip]{1to32}

// CHECK: vphraaddswd xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xc6,0x40,0x44,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddswd xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraaddswd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xc6,0x47,0x44,0x71,0x7f]
          vphraaddswd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraaddswd xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xc6,0x57,0x44,0x72,0x80]
          vphraaddswd xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to32}

// CHECK: vphraaddwd xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x40,0x43,0xf0]
          vphraaddwd xmm22, xmm23, zmm24

// CHECK: vphraaddwd xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x47,0x43,0xf0]
          vphraaddwd xmm22 {k7}, xmm23, zmm24

// CHECK: vphraaddwd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc6,0x40,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddwd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraaddwd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc6,0x47,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddwd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraaddwd xmm22, xmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xc6,0x50,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraaddwd xmm22, xmm23, word ptr [rip]{1to32}

// CHECK: vphraaddwd xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xc6,0x40,0x43,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraaddwd xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraaddwd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xc6,0x47,0x43,0x71,0x7f]
          vphraaddwd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraaddwd xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xc6,0x57,0x43,0x72,0x80]
          vphraaddwd xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to32}

// CHECK: vphraandb xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x46,0x40,0x4d,0xf0]
          vphraandb xmm22, xmm23, zmm24

// CHECK: vphraandb xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x46,0x47,0x4d,0xf0]
          vphraandb xmm22 {k7}, xmm23, zmm24

// CHECK: vphraandb xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x40,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandb xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraandb xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x47,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandb xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraandb xmm22, xmm23, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandb xmm22, xmm23, zmmword ptr [rip]

// CHECK: vphraandb xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x4d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraandb xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraandb xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x4d,0x71,0x7f]
          vphraandb xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraandb xmm22 {k7}, xmm23, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x4d,0x72,0x80]
          vphraandb xmm22 {k7}, xmm23, zmmword ptr [rdx - 8192]

// CHECK: vphraandd xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x4d,0xf0]
          vphraandd xmm22, xmm23, zmm24

// CHECK: vphraandd xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x47,0x47,0x4d,0xf0]
          vphraandd xmm22 {k7}, xmm23, zmm24

// CHECK: vphraandd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraandd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x47,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraandd xmm22, xmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandd xmm22, xmm23, dword ptr [rip]{1to16}

// CHECK: vphraandd xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x4d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraandd xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraandd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x47,0x47,0x4d,0x71,0x7f]
          vphraandd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraandd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x57,0x4d,0x72,0x80]
          vphraandd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to16}

// CHECK: vphraandq xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x40,0x4d,0xf0]
          vphraandq xmm22, xmm23, zmm24

// CHECK: vphraandq xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x47,0x4d,0xf0]
          vphraandq xmm22 {k7}, xmm23, zmm24

// CHECK: vphraandq xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc7,0x40,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandq xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraandq xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc7,0x47,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandq xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraandq xmm22, xmm23, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc7,0x50,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandq xmm22, xmm23, qword ptr [rip]{1to8}

// CHECK: vphraandq xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xc7,0x40,0x4d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraandq xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraandq xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xc7,0x47,0x4d,0x71,0x7f]
          vphraandq xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraandq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc7,0x57,0x4d,0x72,0x80]
          vphraandq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to8}

// CHECK: vphraandw xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x40,0x4d,0xf0]
          vphraandw xmm22, xmm23, zmm24

// CHECK: vphraandw xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x47,0x4d,0xf0]
          vphraandw xmm22 {k7}, xmm23, zmm24

// CHECK: vphraandw xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc6,0x40,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandw xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraandw xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc6,0x47,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandw xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraandw xmm22, xmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xc6,0x50,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandw xmm22, xmm23, word ptr [rip]{1to32}

// CHECK: vphraandw xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xc6,0x40,0x4d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraandw xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraandw xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xc6,0x47,0x4d,0x71,0x7f]
          vphraandw xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraandw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xc6,0x57,0x4d,0x72,0x80]
          vphraandw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to32}

// CHECK: vphraddbd xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x43,0xf7]
          vphraddbd xmm22, zmm23

// CHECK: vphraddbd xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x43,0xf7]
          vphraddbd xmm22 {k7}, zmm23

// CHECK: vphraddbd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraddbd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraddbd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraddbd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraddbd xmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraddbd xmm22, zmmword ptr [rip]

// CHECK: vphraddbd xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x43,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraddbd xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphraddbd xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0x4f,0x43,0x71,0x7f]
          vphraddbd xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphraddbd xmm22 {k7}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7c,0x4f,0x43,0x72,0x80]
          vphraddbd xmm22 {k7}, zmmword ptr [rdx - 8192]

// CHECK: vphraddd xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x43,0xf7]
          vphraddd xmm22, zmm23

// CHECK: vphraddd xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x43,0xf7]
          vphraddd xmm22 {k7}, zmm23

// CHECK: vphraddd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraddd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraddd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraddd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraddd xmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraddd xmm22, dword ptr [rip]{1to16}

// CHECK: vphraddd xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x43,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraddd xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphraddd xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x4f,0x43,0x71,0x7f]
          vphraddd xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphraddd xmm22 {k7}, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x5f,0x43,0x72,0x80]
          vphraddd xmm22 {k7}, dword ptr [rdx - 512]{1to16}

// CHECK: vphraddq xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x43,0xf7]
          vphraddq xmm22, zmm23

// CHECK: vphraddq xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x4f,0x43,0xf7]
          vphraddq xmm22 {k7}, zmm23

// CHECK: vphraddq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraddq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraddq xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x4f,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraddq xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraddq xmm22, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraddq xmm22, qword ptr [rip]{1to8}

// CHECK: vphraddq xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x43,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraddq xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphraddq xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0x4f,0x43,0x71,0x7f]
          vphraddq xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphraddq xmm22 {k7}, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x5f,0x43,0x72,0x80]
          vphraddq xmm22 {k7}, qword ptr [rdx - 1024]{1to8}

// CHECK: vphraddsbd xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x44,0xf7]
          vphraddsbd xmm22, zmm23

// CHECK: vphraddsbd xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x44,0xf7]
          vphraddsbd xmm22 {k7}, zmm23

// CHECK: vphraddsbd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x44,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraddsbd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraddsbd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x44,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraddsbd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraddsbd xmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraddsbd xmm22, zmmword ptr [rip]

// CHECK: vphraddsbd xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x44,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraddsbd xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphraddsbd xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0x4f,0x44,0x71,0x7f]
          vphraddsbd xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphraddsbd xmm22 {k7}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7c,0x4f,0x44,0x72,0x80]
          vphraddsbd xmm22 {k7}, zmmword ptr [rdx - 8192]

// CHECK: vphraddsd xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x44,0xf7]
          vphraddsd xmm22, zmm23

// CHECK: vphraddsd xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x44,0xf7]
          vphraddsd xmm22 {k7}, zmm23

// CHECK: vphraddsd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x44,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraddsd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraddsd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x44,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraddsd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraddsd xmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraddsd xmm22, dword ptr [rip]{1to16}

// CHECK: vphraddsd xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x44,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraddsd xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphraddsd xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x4f,0x44,0x71,0x7f]
          vphraddsd xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphraddsd xmm22 {k7}, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x5f,0x44,0x72,0x80]
          vphraddsd xmm22 {k7}, dword ptr [rdx - 512]{1to16}

// CHECK: vphraddsq xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x44,0xf7]
          vphraddsq xmm22, zmm23

// CHECK: vphraddsq xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x4f,0x44,0xf7]
          vphraddsq xmm22 {k7}, zmm23

// CHECK: vphraddsq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x44,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraddsq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraddsq xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x4f,0x44,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraddsq xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraddsq xmm22, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraddsq xmm22, qword ptr [rip]{1to8}

// CHECK: vphraddsq xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x44,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraddsq xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphraddsq xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0x4f,0x44,0x71,0x7f]
          vphraddsq xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphraddsq xmm22 {k7}, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x5f,0x44,0x72,0x80]
          vphraddsq xmm22 {k7}, qword ptr [rdx - 1024]{1to8}

// CHECK: vphraddswd xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x44,0xf7]
          vphraddswd xmm22, zmm23

// CHECK: vphraddswd xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x4f,0x44,0xf7]
          vphraddswd xmm22 {k7}, zmm23

// CHECK: vphraddswd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x44,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraddswd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraddswd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfc,0x4f,0x44,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraddswd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraddswd xmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraddswd xmm22, word ptr [rip]{1to32}

// CHECK: vphraddswd xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x44,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraddswd xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphraddswd xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfc,0x4f,0x44,0x71,0x7f]
          vphraddswd xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphraddswd xmm22 {k7}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x5f,0x44,0x72,0x80]
          vphraddswd xmm22 {k7}, word ptr [rdx - 256]{1to32}

// CHECK: vphraddwd xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x43,0xf7]
          vphraddwd xmm22, zmm23

// CHECK: vphraddwd xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x4f,0x43,0xf7]
          vphraddwd xmm22 {k7}, zmm23

// CHECK: vphraddwd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraddwd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraddwd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfc,0x4f,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraddwd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraddwd xmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraddwd xmm22, word ptr [rip]{1to32}

// CHECK: vphraddwd xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x43,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraddwd xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphraddwd xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfc,0x4f,0x43,0x71,0x7f]
          vphraddwd xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphraddwd xmm22 {k7}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x5f,0x43,0x72,0x80]
          vphraddwd xmm22 {k7}, word ptr [rdx - 256]{1to32}

// CHECK: vphramaxsb xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x46,0x40,0x4b,0xf0]
          vphramaxsb xmm22, xmm23, zmm24

// CHECK: vphramaxsb xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x46,0x47,0x4b,0xf0]
          vphramaxsb xmm22 {k7}, xmm23, zmm24

// CHECK: vphramaxsb xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x40,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsb xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphramaxsb xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x47,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsb xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphramaxsb xmm22, xmm23, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsb xmm22, xmm23, zmmword ptr [rip]

// CHECK: vphramaxsb xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x4b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsb xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphramaxsb xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x4b,0x71,0x7f]
          vphramaxsb xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphramaxsb xmm22 {k7}, xmm23, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x4b,0x72,0x80]
          vphramaxsb xmm22 {k7}, xmm23, zmmword ptr [rdx - 8192]

// CHECK: vphramaxsd xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x4b,0xf0]
          vphramaxsd xmm22, xmm23, zmm24

// CHECK: vphramaxsd xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x47,0x47,0x4b,0xf0]
          vphramaxsd xmm22 {k7}, xmm23, zmm24

// CHECK: vphramaxsd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphramaxsd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x47,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphramaxsd xmm22, xmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsd xmm22, xmm23, dword ptr [rip]{1to16}

// CHECK: vphramaxsd xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x4b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsd xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphramaxsd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x47,0x47,0x4b,0x71,0x7f]
          vphramaxsd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphramaxsd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x57,0x4b,0x72,0x80]
          vphramaxsd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to16}

// CHECK: vphramaxsq xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x40,0x4b,0xf0]
          vphramaxsq xmm22, xmm23, zmm24

// CHECK: vphramaxsq xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x47,0x4b,0xf0]
          vphramaxsq xmm22 {k7}, xmm23, zmm24

// CHECK: vphramaxsq xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc7,0x40,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsq xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphramaxsq xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc7,0x47,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsq xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphramaxsq xmm22, xmm23, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc7,0x50,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsq xmm22, xmm23, qword ptr [rip]{1to8}

// CHECK: vphramaxsq xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xc7,0x40,0x4b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsq xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphramaxsq xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xc7,0x47,0x4b,0x71,0x7f]
          vphramaxsq xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphramaxsq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc7,0x57,0x4b,0x72,0x80]
          vphramaxsq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to8}

// CHECK: vphramaxsw xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x40,0x4b,0xf0]
          vphramaxsw xmm22, xmm23, zmm24

// CHECK: vphramaxsw xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x47,0x4b,0xf0]
          vphramaxsw xmm22 {k7}, xmm23, zmm24

// CHECK: vphramaxsw xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc6,0x40,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsw xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphramaxsw xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc6,0x47,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsw xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphramaxsw xmm22, xmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xc6,0x50,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsw xmm22, xmm23, word ptr [rip]{1to32}

// CHECK: vphramaxsw xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xc6,0x40,0x4b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphramaxsw xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphramaxsw xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xc6,0x47,0x4b,0x71,0x7f]
          vphramaxsw xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphramaxsw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xc6,0x57,0x4b,0x72,0x80]
          vphramaxsw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to32}

// CHECK: vphraminb xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x46,0x40,0x48,0xf0]
          vphraminb xmm22, xmm23, zmm24

// CHECK: vphraminb xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x46,0x47,0x48,0xf0]
          vphraminb xmm22 {k7}, xmm23, zmm24

// CHECK: vphraminb xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x40,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminb xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminb xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x47,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminb xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminb xmm22, xmm23, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminb xmm22, xmm23, zmmword ptr [rip]

// CHECK: vphraminb xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x48,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminb xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraminb xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x48,0x71,0x7f]
          vphraminb xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraminb xmm22 {k7}, xmm23, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x48,0x72,0x80]
          vphraminb xmm22 {k7}, xmm23, zmmword ptr [rdx - 8192]

// CHECK: vphramind xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x48,0xf0]
          vphramind xmm22, xmm23, zmm24

// CHECK: vphramind xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x47,0x47,0x48,0xf0]
          vphramind xmm22 {k7}, xmm23, zmm24

// CHECK: vphramind xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramind xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphramind xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x47,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramind xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphramind xmm22, xmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x48,0x35,0x00,0x00,0x00,0x00]
          vphramind xmm22, xmm23, dword ptr [rip]{1to16}

// CHECK: vphramind xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x48,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphramind xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphramind xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x47,0x47,0x48,0x71,0x7f]
          vphramind xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphramind xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x57,0x48,0x72,0x80]
          vphramind xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to16}

// CHECK: vphraminq xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x40,0x48,0xf0]
          vphraminq xmm22, xmm23, zmm24

// CHECK: vphraminq xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x47,0x48,0xf0]
          vphraminq xmm22 {k7}, xmm23, zmm24

// CHECK: vphraminq xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc7,0x40,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminq xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminq xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc7,0x47,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminq xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminq xmm22, xmm23, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc7,0x50,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminq xmm22, xmm23, qword ptr [rip]{1to8}

// CHECK: vphraminq xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xc7,0x40,0x48,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminq xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraminq xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xc7,0x47,0x48,0x71,0x7f]
          vphraminq xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraminq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc7,0x57,0x48,0x72,0x80]
          vphraminq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to8}

// CHECK: vphraminsb xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x46,0x40,0x49,0xf0]
          vphraminsb xmm22, xmm23, zmm24

// CHECK: vphraminsb xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x46,0x47,0x49,0xf0]
          vphraminsb xmm22 {k7}, xmm23, zmm24

// CHECK: vphraminsb xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x40,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsb xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminsb xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x47,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsb xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminsb xmm22, xmm23, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsb xmm22, xmm23, zmmword ptr [rip]

// CHECK: vphraminsb xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x46,0x40,0x49,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsb xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraminsb xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x49,0x71,0x7f]
          vphraminsb xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraminsb xmm22 {k7}, xmm23, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x46,0x47,0x49,0x72,0x80]
          vphraminsb xmm22 {k7}, xmm23, zmmword ptr [rdx - 8192]

// CHECK: vphraminsd xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x47,0x40,0x49,0xf0]
          vphraminsd xmm22, xmm23, zmm24

// CHECK: vphraminsd xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0x47,0x47,0x49,0xf0]
          vphraminsd xmm22 {k7}, xmm23, zmm24

// CHECK: vphraminsd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x40,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsd xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminsd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x47,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsd xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminsd xmm22, xmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x50,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsd xmm22, xmm23, dword ptr [rip]{1to16}

// CHECK: vphraminsd xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x47,0x40,0x49,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsd xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraminsd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x47,0x47,0x49,0x71,0x7f]
          vphraminsd xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraminsd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x47,0x57,0x49,0x72,0x80]
          vphraminsd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to16}

// CHECK: vphraminsq xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x40,0x49,0xf0]
          vphraminsq xmm22, xmm23, zmm24

// CHECK: vphraminsq xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x47,0x49,0xf0]
          vphraminsq xmm22 {k7}, xmm23, zmm24

// CHECK: vphraminsq xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc7,0x40,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsq xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminsq xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc7,0x47,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsq xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminsq xmm22, xmm23, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc7,0x50,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsq xmm22, xmm23, qword ptr [rip]{1to8}

// CHECK: vphraminsq xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xc7,0x40,0x49,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsq xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraminsq xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xc7,0x47,0x49,0x71,0x7f]
          vphraminsq xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraminsq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc7,0x57,0x49,0x72,0x80]
          vphraminsq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to8}

// CHECK: vphraminsw xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x40,0x49,0xf0]
          vphraminsw xmm22, xmm23, zmm24

// CHECK: vphraminsw xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x47,0x49,0xf0]
          vphraminsw xmm22 {k7}, xmm23, zmm24

// CHECK: vphraminsw xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc6,0x40,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsw xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminsw xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc6,0x47,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsw xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminsw xmm22, xmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xc6,0x50,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsw xmm22, xmm23, word ptr [rip]{1to32}

// CHECK: vphraminsw xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xc6,0x40,0x49,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminsw xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraminsw xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xc6,0x47,0x49,0x71,0x7f]
          vphraminsw xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraminsw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xc6,0x57,0x49,0x72,0x80]
          vphraminsw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to32}

// CHECK: vphraminw xmm22, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x40,0x48,0xf0]
          vphraminw xmm22, xmm23, zmm24

// CHECK: vphraminw xmm22 {k7}, xmm23, zmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x47,0x48,0xf0]
          vphraminw xmm22 {k7}, xmm23, zmm24

// CHECK: vphraminw xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc6,0x40,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminw xmm22, xmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminw xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc6,0x47,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminw xmm22 {k7}, xmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminw xmm22, xmm23, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xc6,0x50,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminw xmm22, xmm23, word ptr [rip]{1to32}

// CHECK: vphraminw xmm22, xmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xc6,0x40,0x48,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphraminw xmm22, xmm23, zmmword ptr [2*rbp - 2048]

// CHECK: vphraminw xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xc6,0x47,0x48,0x71,0x7f]
          vphraminw xmm22 {k7}, xmm23, zmmword ptr [rcx + 8128]

// CHECK: vphraminw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xc6,0x57,0x48,0x72,0x80]
          vphraminw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to32}

// CHECK: vphrandb xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x4d,0xf7]
          vphrandb xmm22, zmm23

// CHECK: vphrandb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrandb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrandb xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x48,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrandb xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrandb xmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphrandb xmm22, zmmword ptr [rip]

// CHECK: vphrandb xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrandb xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrandb xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4d,0x71,0x7f]
          vphrandb xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrandb xmm22, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4d,0x72,0x80]
          vphrandb xmm22, zmmword ptr [rdx - 8192]

// CHECK: vphrandd xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x4d,0xf7]
          vphrandd xmm22, zmm23

// CHECK: vphrandd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrandd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrandd xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x48,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrandd xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrandd xmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphrandd xmm22, dword ptr [rip]{1to16}

// CHECK: vphrandd xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x4d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrandd xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrandd xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x4d,0x71,0x7f]
          vphrandd xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrandd xmm22, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x4d,0x72,0x80]
          vphrandd xmm22, dword ptr [rdx - 512]{1to16}

// CHECK: vphranddq xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x45,0xf7]
          vphranddq xmm22, zmm23

// CHECK: vphranddq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x45,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphranddq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphranddq xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x48,0x45,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphranddq xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphranddq xmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x45,0x35,0x00,0x00,0x00,0x00]
          vphranddq xmm22, zmmword ptr [rip]

// CHECK: vphranddq xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x45,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphranddq xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphranddq xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x45,0x71,0x7f]
          vphranddq xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphranddq xmm22, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x45,0x72,0x80]
          vphranddq xmm22, zmmword ptr [rdx - 8192]

// CHECK: vphrandq xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x4d,0xf7]
          vphrandq xmm22, zmm23

// CHECK: vphrandq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrandq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrandq xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x48,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrandq xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrandq xmm22, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphrandq xmm22, qword ptr [rip]{1to8}

// CHECK: vphrandq xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x4d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrandq xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrandq xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x4d,0x71,0x7f]
          vphrandq xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrandq xmm22, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x4d,0x72,0x80]
          vphrandq xmm22, qword ptr [rdx - 1024]{1to8}

// CHECK: vphrandqq ymm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x45,0xf7]
          vphrandqq ymm22, zmm23

// CHECK: vphrandqq ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x45,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrandqq ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrandqq ymm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x48,0x45,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrandqq ymm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrandqq ymm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x45,0x35,0x00,0x00,0x00,0x00]
          vphrandqq ymm22, zmmword ptr [rip]

// CHECK: vphrandqq ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x45,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrandqq ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrandqq ymm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x45,0x71,0x7f]
          vphrandqq ymm22, zmmword ptr [rcx + 8128]

// CHECK: vphrandqq ymm22, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x45,0x72,0x80]
          vphrandqq ymm22, zmmword ptr [rdx - 8192]

// CHECK: vphrandw xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x4d,0xf7]
          vphrandw xmm22, zmm23

// CHECK: vphrandw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrandw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrandw xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfc,0x48,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrandw xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrandw xmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphrandw xmm22, word ptr [rip]{1to32}

// CHECK: vphrandw xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x4d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrandw xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrandw xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x4d,0x71,0x7f]
          vphrandw xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrandw xmm22, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x4d,0x72,0x80]
          vphrandw xmm22, word ptr [rdx - 256]{1to32}

// CHECK: vphrmaxb xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x4a,0xf7]
          vphrmaxb xmm22, zmm23

// CHECK: vphrmaxb xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x4a,0xf7]
          vphrmaxb xmm22 {k7}, zmm23

// CHECK: vphrmaxb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x4a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrmaxb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrmaxb xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x4a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrmaxb xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrmaxb xmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4a,0x35,0x00,0x00,0x00,0x00]
          vphrmaxb xmm22, zmmword ptr [rip]

// CHECK: vphrmaxb xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxb xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrmaxb xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0x4f,0x4a,0x71,0x7f]
          vphrmaxb xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphrmaxb xmm22 {k7}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7c,0x4f,0x4a,0x72,0x80]
          vphrmaxb xmm22 {k7}, zmmword ptr [rdx - 8192]

// CHECK: vphrmaxd xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x4a,0xf7]
          vphrmaxd xmm22, zmm23

// CHECK: vphrmaxd xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x4a,0xf7]
          vphrmaxd xmm22 {k7}, zmm23

// CHECK: vphrmaxd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x4a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrmaxd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrmaxd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x4a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrmaxd xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrmaxd xmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x4a,0x35,0x00,0x00,0x00,0x00]
          vphrmaxd xmm22, dword ptr [rip]{1to16}

// CHECK: vphrmaxd xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x4a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxd xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrmaxd xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x4f,0x4a,0x71,0x7f]
          vphrmaxd xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphrmaxd xmm22 {k7}, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x5f,0x4a,0x72,0x80]
          vphrmaxd xmm22 {k7}, dword ptr [rdx - 512]{1to16}

// CHECK: vphrmaxq xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x4a,0xf7]
          vphrmaxq xmm22, zmm23

// CHECK: vphrmaxq xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x4f,0x4a,0xf7]
          vphrmaxq xmm22 {k7}, zmm23

// CHECK: vphrmaxq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x4a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrmaxq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrmaxq xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x4f,0x4a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrmaxq xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrmaxq xmm22, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x4a,0x35,0x00,0x00,0x00,0x00]
          vphrmaxq xmm22, qword ptr [rip]{1to8}

// CHECK: vphrmaxq xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x4a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxq xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrmaxq xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0x4f,0x4a,0x71,0x7f]
          vphrmaxq xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphrmaxq xmm22 {k7}, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x5f,0x4a,0x72,0x80]
          vphrmaxq xmm22 {k7}, qword ptr [rdx - 1024]{1to8}

// CHECK: vphrmaxsb xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x4b,0xf7]
          vphrmaxsb xmm22, zmm23

// CHECK: vphrmaxsb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrmaxsb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrmaxsb xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x48,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrmaxsb xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrmaxsb xmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphrmaxsb xmm22, zmmword ptr [rip]

// CHECK: vphrmaxsb xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxsb xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrmaxsb xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4b,0x71,0x7f]
          vphrmaxsb xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrmaxsb xmm22, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4b,0x72,0x80]
          vphrmaxsb xmm22, zmmword ptr [rdx - 8192]

// CHECK: vphrmaxsd xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x4b,0xf7]
          vphrmaxsd xmm22, zmm23

// CHECK: vphrmaxsd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrmaxsd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrmaxsd xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x48,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrmaxsd xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrmaxsd xmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphrmaxsd xmm22, dword ptr [rip]{1to16}

// CHECK: vphrmaxsd xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x4b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxsd xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrmaxsd xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x4b,0x71,0x7f]
          vphrmaxsd xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrmaxsd xmm22, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x4b,0x72,0x80]
          vphrmaxsd xmm22, dword ptr [rdx - 512]{1to16}

// CHECK: vphrmaxsq xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x4b,0xf7]
          vphrmaxsq xmm22, zmm23

// CHECK: vphrmaxsq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrmaxsq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrmaxsq xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x48,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrmaxsq xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrmaxsq xmm22, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphrmaxsq xmm22, qword ptr [rip]{1to8}

// CHECK: vphrmaxsq xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x4b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxsq xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrmaxsq xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x4b,0x71,0x7f]
          vphrmaxsq xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrmaxsq xmm22, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x4b,0x72,0x80]
          vphrmaxsq xmm22, qword ptr [rdx - 1024]{1to8}

// CHECK: vphrmaxsw xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x4b,0xf7]
          vphrmaxsw xmm22, zmm23

// CHECK: vphrmaxsw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrmaxsw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrmaxsw xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfc,0x48,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrmaxsw xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrmaxsw xmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphrmaxsw xmm22, word ptr [rip]{1to32}

// CHECK: vphrmaxsw xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x4b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxsw xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrmaxsw xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x4b,0x71,0x7f]
          vphrmaxsw xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrmaxsw xmm22, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x4b,0x72,0x80]
          vphrmaxsw xmm22, word ptr [rdx - 256]{1to32}

// CHECK: vphrmaxw xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x4a,0xf7]
          vphrmaxw xmm22, zmm23

// CHECK: vphrmaxw xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x4f,0x4a,0xf7]
          vphrmaxw xmm22 {k7}, zmm23

// CHECK: vphrmaxw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x4a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrmaxw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrmaxw xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfc,0x4f,0x4a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrmaxw xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrmaxw xmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x4a,0x35,0x00,0x00,0x00,0x00]
          vphrmaxw xmm22, word ptr [rip]{1to32}

// CHECK: vphrmaxw xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x4a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrmaxw xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrmaxw xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfc,0x4f,0x4a,0x71,0x7f]
          vphrmaxw xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphrmaxw xmm22 {k7}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x5f,0x4a,0x72,0x80]
          vphrmaxw xmm22 {k7}, word ptr [rdx - 256]{1to32}

// CHECK: vphrminb xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x48,0xf7]
          vphrminb xmm22, zmm23

// CHECK: vphrminb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrminb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrminb xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x48,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrminb xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrminb xmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x48,0x35,0x00,0x00,0x00,0x00]
          vphrminb xmm22, zmmword ptr [rip]

// CHECK: vphrminb xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x48,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrminb xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrminb xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x48,0x71,0x7f]
          vphrminb xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrminb xmm22, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x48,0x72,0x80]
          vphrminb xmm22, zmmword ptr [rdx - 8192]

// CHECK: vphrmind xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x48,0xf7]
          vphrmind xmm22, zmm23

// CHECK: vphrmind xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrmind xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrmind xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x48,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrmind xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrmind xmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x48,0x35,0x00,0x00,0x00,0x00]
          vphrmind xmm22, dword ptr [rip]{1to16}

// CHECK: vphrmind xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x48,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrmind xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrmind xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x48,0x71,0x7f]
          vphrmind xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrmind xmm22, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x48,0x72,0x80]
          vphrmind xmm22, dword ptr [rdx - 512]{1to16}

// CHECK: vphrminq xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x48,0xf7]
          vphrminq xmm22, zmm23

// CHECK: vphrminq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrminq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrminq xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x48,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrminq xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrminq xmm22, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x48,0x35,0x00,0x00,0x00,0x00]
          vphrminq xmm22, qword ptr [rip]{1to8}

// CHECK: vphrminq xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x48,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrminq xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrminq xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x48,0x71,0x7f]
          vphrminq xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrminq xmm22, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x48,0x72,0x80]
          vphrminq xmm22, qword ptr [rdx - 1024]{1to8}

// CHECK: vphrminsb xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x49,0xf7]
          vphrminsb xmm22, zmm23

// CHECK: vphrminsb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrminsb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrminsb xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x48,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrminsb xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrminsb xmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x49,0x35,0x00,0x00,0x00,0x00]
          vphrminsb xmm22, zmmword ptr [rip]

// CHECK: vphrminsb xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x49,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrminsb xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrminsb xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x49,0x71,0x7f]
          vphrminsb xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrminsb xmm22, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x49,0x72,0x80]
          vphrminsb xmm22, zmmword ptr [rdx - 8192]

// CHECK: vphrminsd xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x49,0xf7]
          vphrminsd xmm22, zmm23

// CHECK: vphrminsd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrminsd xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrminsd xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x48,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrminsd xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrminsd xmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x49,0x35,0x00,0x00,0x00,0x00]
          vphrminsd xmm22, dword ptr [rip]{1to16}

// CHECK: vphrminsd xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x49,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrminsd xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrminsd xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x49,0x71,0x7f]
          vphrminsd xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrminsd xmm22, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x49,0x72,0x80]
          vphrminsd xmm22, dword ptr [rdx - 512]{1to16}

// CHECK: vphrminsq xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x49,0xf7]
          vphrminsq xmm22, zmm23

// CHECK: vphrminsq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrminsq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrminsq xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x48,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrminsq xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrminsq xmm22, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x49,0x35,0x00,0x00,0x00,0x00]
          vphrminsq xmm22, qword ptr [rip]{1to8}

// CHECK: vphrminsq xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x49,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrminsq xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrminsq xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x49,0x71,0x7f]
          vphrminsq xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrminsq xmm22, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x49,0x72,0x80]
          vphrminsq xmm22, qword ptr [rdx - 1024]{1to8}

// CHECK: vphrminsw xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x49,0xf7]
          vphrminsw xmm22, zmm23

// CHECK: vphrminsw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrminsw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrminsw xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfc,0x48,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrminsw xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrminsw xmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x49,0x35,0x00,0x00,0x00,0x00]
          vphrminsw xmm22, word ptr [rip]{1to32}

// CHECK: vphrminsw xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x49,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrminsw xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrminsw xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x49,0x71,0x7f]
          vphrminsw xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrminsw xmm22, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x49,0x72,0x80]
          vphrminsw xmm22, word ptr [rdx - 256]{1to32}

// CHECK: vphrminw xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x48,0xf7]
          vphrminw xmm22, zmm23

// CHECK: vphrminw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrminw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrminw xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfc,0x48,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrminw xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrminw xmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x48,0x35,0x00,0x00,0x00,0x00]
          vphrminw xmm22, word ptr [rip]{1to32}

// CHECK: vphrminw xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x48,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrminw xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrminw xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x48,0x71,0x7f]
          vphrminw xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrminw xmm22, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x48,0x72,0x80]
          vphrminw xmm22, word ptr [rdx - 256]{1to32}

// CHECK: vphrorb xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x4e,0xf7]
          vphrorb xmm22, zmm23

// CHECK: vphrorb xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x4e,0xf7]
          vphrorb xmm22 {k7}, zmm23

// CHECK: vphrorb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x4e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrorb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrorb xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x4e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrorb xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrorb xmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4e,0x35,0x00,0x00,0x00,0x00]
          vphrorb xmm22, zmmword ptr [rip]

// CHECK: vphrorb xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4e,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrorb xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrorb xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0x4f,0x4e,0x71,0x7f]
          vphrorb xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphrorb xmm22 {k7}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7c,0x4f,0x4e,0x72,0x80]
          vphrorb xmm22 {k7}, zmmword ptr [rdx - 8192]

// CHECK: vphrord xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x4e,0xf7]
          vphrord xmm22, zmm23

// CHECK: vphrord xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x4e,0xf7]
          vphrord xmm22 {k7}, zmm23

// CHECK: vphrord xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x4e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrord xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrord xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x4e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrord xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrord xmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x4e,0x35,0x00,0x00,0x00,0x00]
          vphrord xmm22, dword ptr [rip]{1to16}

// CHECK: vphrord xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x4e,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrord xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrord xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x4f,0x4e,0x71,0x7f]
          vphrord xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphrord xmm22 {k7}, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x5f,0x4e,0x72,0x80]
          vphrord xmm22 {k7}, dword ptr [rdx - 512]{1to16}

// CHECK: vphrordq xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x46,0xf7]
          vphrordq xmm22, zmm23

// CHECK: vphrordq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x46,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrordq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrordq xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x48,0x46,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrordq xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrordq xmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x46,0x35,0x00,0x00,0x00,0x00]
          vphrordq xmm22, zmmword ptr [rip]

// CHECK: vphrordq xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x46,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrordq xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrordq xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x46,0x71,0x7f]
          vphrordq xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrordq xmm22, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x46,0x72,0x80]
          vphrordq xmm22, zmmword ptr [rdx - 8192]

// CHECK: vphrorq xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x4e,0xf7]
          vphrorq xmm22, zmm23

// CHECK: vphrorq xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x4f,0x4e,0xf7]
          vphrorq xmm22 {k7}, zmm23

// CHECK: vphrorq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x4e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrorq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrorq xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x4f,0x4e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrorq xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrorq xmm22, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x4e,0x35,0x00,0x00,0x00,0x00]
          vphrorq xmm22, qword ptr [rip]{1to8}

// CHECK: vphrorq xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x4e,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrorq xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrorq xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0x4f,0x4e,0x71,0x7f]
          vphrorq xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphrorq xmm22 {k7}, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x5f,0x4e,0x72,0x80]
          vphrorq xmm22 {k7}, qword ptr [rdx - 1024]{1to8}

// CHECK: vphrorqq ymm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x46,0xf7]
          vphrorqq ymm22, zmm23

// CHECK: vphrorqq ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x46,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrorqq ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrorqq ymm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x48,0x46,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrorqq ymm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrorqq ymm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x46,0x35,0x00,0x00,0x00,0x00]
          vphrorqq ymm22, zmmword ptr [rip]

// CHECK: vphrorqq ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x46,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrorqq ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrorqq ymm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x46,0x71,0x7f]
          vphrorqq ymm22, zmmword ptr [rcx + 8128]

// CHECK: vphrorqq ymm22, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x46,0x72,0x80]
          vphrorqq ymm22, zmmword ptr [rdx - 8192]

// CHECK: vphrorw xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x4e,0xf7]
          vphrorw xmm22, zmm23

// CHECK: vphrorw xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x4f,0x4e,0xf7]
          vphrorw xmm22 {k7}, zmm23

// CHECK: vphrorw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x4e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrorw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrorw xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfc,0x4f,0x4e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrorw xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrorw xmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x4e,0x35,0x00,0x00,0x00,0x00]
          vphrorw xmm22, word ptr [rip]{1to32}

// CHECK: vphrorw xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x4e,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrorw xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrorw xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfc,0x4f,0x4e,0x71,0x7f]
          vphrorw xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphrorw xmm22 {k7}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x5f,0x4e,0x72,0x80]
          vphrorw xmm22 {k7}, word ptr [rdx - 256]{1to32}

// CHECK: vphrxorb xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x4f,0xf7]
          vphrxorb xmm22, zmm23

// CHECK: vphrxorb xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x4f,0xf7]
          vphrxorb xmm22 {k7}, zmm23

// CHECK: vphrxorb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x4f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrxorb xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrxorb xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x4f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrxorb xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrxorb xmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4f,0x35,0x00,0x00,0x00,0x00]
          vphrxorb xmm22, zmmword ptr [rip]

// CHECK: vphrxorb xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x4f,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrxorb xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrxorb xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7c,0x4f,0x4f,0x71,0x7f]
          vphrxorb xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphrxorb xmm22 {k7}, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7c,0x4f,0x4f,0x72,0x80]
          vphrxorb xmm22 {k7}, zmmword ptr [rdx - 8192]

// CHECK: vphrxord xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x4f,0xf7]
          vphrxord xmm22, zmm23

// CHECK: vphrxord xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x4f,0xf7]
          vphrxord xmm22 {k7}, zmm23

// CHECK: vphrxord xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x4f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrxord xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrxord xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x4f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrxord xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrxord xmm22, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x4f,0x35,0x00,0x00,0x00,0x00]
          vphrxord xmm22, dword ptr [rip]{1to16}

// CHECK: vphrxord xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x4f,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrxord xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrxord xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x4f,0x4f,0x71,0x7f]
          vphrxord xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphrxord xmm22 {k7}, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe5,0x7d,0x5f,0x4f,0x72,0x80]
          vphrxord xmm22 {k7}, dword ptr [rdx - 512]{1to16}

// CHECK: vphrxordq xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x47,0xf7]
          vphrxordq xmm22, zmm23

// CHECK: vphrxordq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x47,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrxordq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrxordq xmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7d,0x48,0x47,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrxordq xmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrxordq xmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x47,0x35,0x00,0x00,0x00,0x00]
          vphrxordq xmm22, zmmword ptr [rip]

// CHECK: vphrxordq xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x47,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrxordq xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrxordq xmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x47,0x71,0x7f]
          vphrxordq xmm22, zmmword ptr [rcx + 8128]

// CHECK: vphrxordq xmm22, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x47,0x72,0x80]
          vphrxordq xmm22, zmmword ptr [rdx - 8192]

// CHECK: vphrxorq xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x4f,0xf7]
          vphrxorq xmm22, zmm23

// CHECK: vphrxorq xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x4f,0x4f,0xf7]
          vphrxorq xmm22 {k7}, zmm23

// CHECK: vphrxorq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x4f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrxorq xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrxorq xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x4f,0x4f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrxorq xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrxorq xmm22, qword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x4f,0x35,0x00,0x00,0x00,0x00]
          vphrxorq xmm22, qword ptr [rip]{1to8}

// CHECK: vphrxorq xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x4f,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrxorq xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrxorq xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0x4f,0x4f,0x71,0x7f]
          vphrxorq xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphrxorq xmm22 {k7}, qword ptr [rdx - 1024]{1to8}
// CHECK: encoding: [0x62,0xe5,0xfd,0x5f,0x4f,0x72,0x80]
          vphrxorq xmm22 {k7}, qword ptr [rdx - 1024]{1to8}

// CHECK: vphrxorqq ymm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x47,0xf7]
          vphrxorqq ymm22, zmm23

// CHECK: vphrxorqq ymm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x47,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrxorqq ymm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrxorqq ymm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfd,0x48,0x47,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrxorqq ymm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrxorqq ymm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x47,0x35,0x00,0x00,0x00,0x00]
          vphrxorqq ymm22, zmmword ptr [rip]

// CHECK: vphrxorqq ymm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x47,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrxorqq ymm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrxorqq ymm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x47,0x71,0x7f]
          vphrxorqq ymm22, zmmword ptr [rcx + 8128]

// CHECK: vphrxorqq ymm22, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x47,0x72,0x80]
          vphrxorqq ymm22, zmmword ptr [rdx - 8192]

// CHECK: vphrxorw xmm22, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x4f,0xf7]
          vphrxorw xmm22, zmm23

// CHECK: vphrxorw xmm22 {k7}, zmm23
// CHECK: encoding: [0x62,0xa5,0xfc,0x4f,0x4f,0xf7]
          vphrxorw xmm22 {k7}, zmm23

// CHECK: vphrxorw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x4f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphrxorw xmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphrxorw xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfc,0x4f,0x4f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphrxorw xmm22 {k7}, zmmword ptr [r8 + 4*rax + 291]

// CHECK: vphrxorw xmm22, word ptr [rip]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x4f,0x35,0x00,0x00,0x00,0x00]
          vphrxorw xmm22, word ptr [rip]{1to32}

// CHECK: vphrxorw xmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x4f,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vphrxorw xmm22, zmmword ptr [2*rbp - 2048]

// CHECK: vphrxorw xmm22 {k7}, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe5,0xfc,0x4f,0x4f,0x71,0x7f]
          vphrxorw xmm22 {k7}, zmmword ptr [rcx + 8128]

// CHECK: vphrxorw xmm22 {k7}, word ptr [rdx - 256]{1to32}
// CHECK: encoding: [0x62,0xe5,0xfc,0x5f,0x4f,0x72,0x80]
          vphrxorw xmm22 {k7}, word ptr [rdx - 256]{1to32}

