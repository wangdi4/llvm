// REQUIRES: intel_feature_isa_avx512_reduction2
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

