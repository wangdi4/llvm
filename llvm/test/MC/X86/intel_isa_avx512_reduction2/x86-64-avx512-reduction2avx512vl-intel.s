// REQUIRES: intel_feature_isa_avx512_reduction2
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vphraaddbd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x43,0xf0]
          vphraaddbd xmm22, xmm23, xmm24

// CHECK: vphraaddbd xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x43,0xf0]
          vphraaddbd xmm22 {k7}, xmm23, xmm24

// CHECK: vphraaddbd xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x43,0xf0]
          vphraaddbd xmm22, xmm23, ymm24

// CHECK: vphraaddbd xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x43,0xf0]
          vphraaddbd xmm22 {k7}, xmm23, ymm24

// CHECK: vphraaddbd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddbd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraaddbd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddbd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraaddbd xmm22, xmm23, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraaddbd xmm22, xmm23, xmmword ptr [rip]

// CHECK: vphraaddbd xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x43,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddbd xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraaddbd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x43,0x71,0x7f]
          vphraaddbd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraaddbd xmm22 {k7}, xmm23, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x43,0x72,0x80]
          vphraaddbd xmm22 {k7}, xmm23, xmmword ptr [rdx - 2048]

// CHECK: vphraaddbd xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x43,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddbd xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraaddbd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x43,0x71,0x7f]
          vphraaddbd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraaddbd xmm22 {k7}, xmm23, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x43,0x72,0x80]
          vphraaddbd xmm22 {k7}, xmm23, ymmword ptr [rdx - 4096]

// CHECK: vphraaddsbd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x44,0xf0]
          vphraaddsbd xmm22, xmm23, xmm24

// CHECK: vphraaddsbd xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x44,0xf0]
          vphraaddsbd xmm22 {k7}, xmm23, xmm24

// CHECK: vphraaddsbd xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x44,0xf0]
          vphraaddsbd xmm22, xmm23, ymm24

// CHECK: vphraaddsbd xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x44,0xf0]
          vphraaddsbd xmm22 {k7}, xmm23, ymm24

// CHECK: vphraaddsbd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x44,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddsbd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraaddsbd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x44,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddsbd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraaddsbd xmm22, xmm23, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraaddsbd xmm22, xmm23, xmmword ptr [rip]

// CHECK: vphraaddsbd xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x44,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddsbd xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraaddsbd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x44,0x71,0x7f]
          vphraaddsbd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraaddsbd xmm22 {k7}, xmm23, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x44,0x72,0x80]
          vphraaddsbd xmm22 {k7}, xmm23, xmmword ptr [rdx - 2048]

// CHECK: vphraaddsbd xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x44,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddsbd xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraaddsbd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x44,0x71,0x7f]
          vphraaddsbd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraaddsbd xmm22 {k7}, xmm23, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x44,0x72,0x80]
          vphraaddsbd xmm22 {k7}, xmm23, ymmword ptr [rdx - 4096]

// CHECK: vphraaddswd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x00,0x44,0xf0]
          vphraaddswd xmm22, xmm23, xmm24

// CHECK: vphraaddswd xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x07,0x44,0xf0]
          vphraaddswd xmm22 {k7}, xmm23, xmm24

// CHECK: vphraaddswd xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc6,0x20,0x44,0xf0]
          vphraaddswd xmm22, xmm23, ymm24

// CHECK: vphraaddswd xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc6,0x27,0x44,0xf0]
          vphraaddswd xmm22 {k7}, xmm23, ymm24

// CHECK: vphraaddswd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc6,0x00,0x44,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddswd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraaddswd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc6,0x07,0x44,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddswd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraaddswd xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc6,0x10,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraaddswd xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vphraaddswd xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x44,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddswd xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraaddswd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xc6,0x07,0x44,0x71,0x7f]
          vphraaddswd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraaddswd xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc6,0x17,0x44,0x72,0x80]
          vphraaddswd xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vphraaddswd xmm22, xmm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0xc6,0x30,0x44,0x35,0x00,0x00,0x00,0x00]
          vphraaddswd xmm22, xmm23, word ptr [rip]{1to16}

// CHECK: vphraaddswd xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xc6,0x20,0x44,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddswd xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraaddswd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xc6,0x27,0x44,0x71,0x7f]
          vphraaddswd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraaddswd xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0xc6,0x37,0x44,0x72,0x80]
          vphraaddswd xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to16}

// CHECK: vphraaddwd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x00,0x43,0xf0]
          vphraaddwd xmm22, xmm23, xmm24

// CHECK: vphraaddwd xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x07,0x43,0xf0]
          vphraaddwd xmm22 {k7}, xmm23, xmm24

// CHECK: vphraaddwd xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc6,0x20,0x43,0xf0]
          vphraaddwd xmm22, xmm23, ymm24

// CHECK: vphraaddwd xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc6,0x27,0x43,0xf0]
          vphraaddwd xmm22 {k7}, xmm23, ymm24

// CHECK: vphraaddwd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc6,0x00,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraaddwd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraaddwd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc6,0x07,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraaddwd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraaddwd xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc6,0x10,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraaddwd xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vphraaddwd xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x43,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraaddwd xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraaddwd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xc6,0x07,0x43,0x71,0x7f]
          vphraaddwd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraaddwd xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc6,0x17,0x43,0x72,0x80]
          vphraaddwd xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vphraaddwd xmm22, xmm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0xc6,0x30,0x43,0x35,0x00,0x00,0x00,0x00]
          vphraaddwd xmm22, xmm23, word ptr [rip]{1to16}

// CHECK: vphraaddwd xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xc6,0x20,0x43,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraaddwd xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraaddwd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xc6,0x27,0x43,0x71,0x7f]
          vphraaddwd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraaddwd xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0xc6,0x37,0x43,0x72,0x80]
          vphraaddwd xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to16}


// CHECK: vphraandb xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x4d,0xf0]
          vphraandb xmm22, xmm23, xmm24

// CHECK: vphraandb xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x4d,0xf0]
          vphraandb xmm22 {k7}, xmm23, xmm24

// CHECK: vphraandb xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x4d,0xf0]
          vphraandb xmm22, xmm23, ymm24

// CHECK: vphraandb xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x4d,0xf0]
          vphraandb xmm22 {k7}, xmm23, ymm24

// CHECK: vphraandb xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandb xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraandb xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandb xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraandb xmm22, xmm23, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandb xmm22, xmm23, xmmword ptr [rip]

// CHECK: vphraandb xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x4d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraandb xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraandb xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x4d,0x71,0x7f]
          vphraandb xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraandb xmm22 {k7}, xmm23, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x4d,0x72,0x80]
          vphraandb xmm22 {k7}, xmm23, xmmword ptr [rdx - 2048]

// CHECK: vphraandb xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x4d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraandb xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraandb xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x4d,0x71,0x7f]
          vphraandb xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraandb xmm22 {k7}, xmm23, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x4d,0x72,0x80]
          vphraandb xmm22 {k7}, xmm23, ymmword ptr [rdx - 4096]

// CHECK: vphraandd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x4d,0xf0]
          vphraandd xmm22, xmm23, xmm24

// CHECK: vphraandd xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x07,0x4d,0xf0]
          vphraandd xmm22 {k7}, xmm23, xmm24

// CHECK: vphraandd xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x4d,0xf0]
          vphraandd xmm22, xmm23, ymm24

// CHECK: vphraandd xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x27,0x4d,0xf0]
          vphraandd xmm22 {k7}, xmm23, ymm24

// CHECK: vphraandd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraandd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x07,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraandd xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandd xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vphraandd xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x4d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraandd xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraandd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x47,0x07,0x4d,0x71,0x7f]
          vphraandd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraandd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe5,0x47,0x17,0x4d,0x72,0x80]
          vphraandd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vphraandd xmm22, xmm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandd xmm22, xmm23, dword ptr [rip]{1to8}

// CHECK: vphraandd xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x4d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraandd xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraandd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x47,0x27,0x4d,0x71,0x7f]
          vphraandd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraandd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x37,0x4d,0x72,0x80]
          vphraandd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to8}

// CHECK: vphraandq xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x00,0x4d,0xf0]
          vphraandq xmm22, xmm23, xmm24

// CHECK: vphraandq xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x07,0x4d,0xf0]
          vphraandq xmm22 {k7}, xmm23, xmm24

// CHECK: vphraandq xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc7,0x20,0x4d,0xf0]
          vphraandq xmm22, xmm23, ymm24

// CHECK: vphraandq xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc7,0x27,0x4d,0xf0]
          vphraandq xmm22 {k7}, xmm23, ymm24

// CHECK: vphraandq xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc7,0x00,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandq xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraandq xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc7,0x07,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandq xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraandq xmm22, xmm23, qword ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe5,0xc7,0x10,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandq xmm22, xmm23, qword ptr [rip]{1to2}

// CHECK: vphraandq xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xc7,0x00,0x4d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraandq xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraandq xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xc7,0x07,0x4d,0x71,0x7f]
          vphraandq xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraandq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to2}
// CHECK: encoding: [0x62,0xe5,0xc7,0x17,0x4d,0x72,0x80]
          vphraandq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to2}

// CHECK: vphraandq xmm22, xmm23, qword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0xc7,0x30,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandq xmm22, xmm23, qword ptr [rip]{1to4}

// CHECK: vphraandq xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xc7,0x20,0x4d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraandq xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraandq xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xc7,0x27,0x4d,0x71,0x7f]
          vphraandq xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraandq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to4}
// CHECK: encoding: [0x62,0xe5,0xc7,0x37,0x4d,0x72,0x80]
          vphraandq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to4}

// CHECK: vphraandw xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x00,0x4d,0xf0]
          vphraandw xmm22, xmm23, xmm24

// CHECK: vphraandw xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x07,0x4d,0xf0]
          vphraandw xmm22 {k7}, xmm23, xmm24

// CHECK: vphraandw xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc6,0x20,0x4d,0xf0]
          vphraandw xmm22, xmm23, ymm24

// CHECK: vphraandw xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc6,0x27,0x4d,0xf0]
          vphraandw xmm22 {k7}, xmm23, ymm24

// CHECK: vphraandw xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc6,0x00,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraandw xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraandw xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc6,0x07,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraandw xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraandw xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc6,0x10,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandw xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vphraandw xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x4d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraandw xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraandw xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xc6,0x07,0x4d,0x71,0x7f]
          vphraandw xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraandw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc6,0x17,0x4d,0x72,0x80]
          vphraandw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vphraandw xmm22, xmm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0xc6,0x30,0x4d,0x35,0x00,0x00,0x00,0x00]
          vphraandw xmm22, xmm23, word ptr [rip]{1to16}

// CHECK: vphraandw xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xc6,0x20,0x4d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraandw xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraandw xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xc6,0x27,0x4d,0x71,0x7f]
          vphraandw xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraandw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0xc6,0x37,0x4d,0x72,0x80]
          vphraandw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to16}

// CHECK: vphramaxsb xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x4b,0xf0]
          vphramaxsb xmm22, xmm23, xmm24

// CHECK: vphramaxsb xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x4b,0xf0]
          vphramaxsb xmm22 {k7}, xmm23, xmm24

// CHECK: vphramaxsb xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x4b,0xf0]
          vphramaxsb xmm22, xmm23, ymm24

// CHECK: vphramaxsb xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x4b,0xf0]
          vphramaxsb xmm22 {k7}, xmm23, ymm24

// CHECK: vphramaxsb xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsb xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphramaxsb xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsb xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphramaxsb xmm22, xmm23, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsb xmm22, xmm23, xmmword ptr [rip]

// CHECK: vphramaxsb xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x4b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsb xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphramaxsb xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x4b,0x71,0x7f]
          vphramaxsb xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphramaxsb xmm22 {k7}, xmm23, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x4b,0x72,0x80]
          vphramaxsb xmm22 {k7}, xmm23, xmmword ptr [rdx - 2048]

// CHECK: vphramaxsb xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x4b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsb xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphramaxsb xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x4b,0x71,0x7f]
          vphramaxsb xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphramaxsb xmm22 {k7}, xmm23, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x4b,0x72,0x80]
          vphramaxsb xmm22 {k7}, xmm23, ymmword ptr [rdx - 4096]

// CHECK: vphramaxsd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x4b,0xf0]
          vphramaxsd xmm22, xmm23, xmm24

// CHECK: vphramaxsd xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x07,0x4b,0xf0]
          vphramaxsd xmm22 {k7}, xmm23, xmm24

// CHECK: vphramaxsd xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x4b,0xf0]
          vphramaxsd xmm22, xmm23, ymm24

// CHECK: vphramaxsd xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x27,0x4b,0xf0]
          vphramaxsd xmm22 {k7}, xmm23, ymm24

// CHECK: vphramaxsd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphramaxsd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x07,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphramaxsd xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsd xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vphramaxsd xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x4b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsd xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphramaxsd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x47,0x07,0x4b,0x71,0x7f]
          vphramaxsd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphramaxsd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe5,0x47,0x17,0x4b,0x72,0x80]
          vphramaxsd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vphramaxsd xmm22, xmm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsd xmm22, xmm23, dword ptr [rip]{1to8}

// CHECK: vphramaxsd xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x4b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsd xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphramaxsd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x47,0x27,0x4b,0x71,0x7f]
          vphramaxsd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphramaxsd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x37,0x4b,0x72,0x80]
          vphramaxsd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to8}

// CHECK: vphramaxsq xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x00,0x4b,0xf0]
          vphramaxsq xmm22, xmm23, xmm24

// CHECK: vphramaxsq xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x07,0x4b,0xf0]
          vphramaxsq xmm22 {k7}, xmm23, xmm24

// CHECK: vphramaxsq xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc7,0x20,0x4b,0xf0]
          vphramaxsq xmm22, xmm23, ymm24

// CHECK: vphramaxsq xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc7,0x27,0x4b,0xf0]
          vphramaxsq xmm22 {k7}, xmm23, ymm24

// CHECK: vphramaxsq xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc7,0x00,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsq xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphramaxsq xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc7,0x07,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsq xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphramaxsq xmm22, xmm23, qword ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe5,0xc7,0x10,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsq xmm22, xmm23, qword ptr [rip]{1to2}

// CHECK: vphramaxsq xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xc7,0x00,0x4b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsq xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphramaxsq xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xc7,0x07,0x4b,0x71,0x7f]
          vphramaxsq xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphramaxsq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to2}
// CHECK: encoding: [0x62,0xe5,0xc7,0x17,0x4b,0x72,0x80]
          vphramaxsq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to2}

// CHECK: vphramaxsq xmm22, xmm23, qword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0xc7,0x30,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsq xmm22, xmm23, qword ptr [rip]{1to4}

// CHECK: vphramaxsq xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xc7,0x20,0x4b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsq xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphramaxsq xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xc7,0x27,0x4b,0x71,0x7f]
          vphramaxsq xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphramaxsq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to4}
// CHECK: encoding: [0x62,0xe5,0xc7,0x37,0x4b,0x72,0x80]
          vphramaxsq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to4}

// CHECK: vphramaxsw xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x00,0x4b,0xf0]
          vphramaxsw xmm22, xmm23, xmm24

// CHECK: vphramaxsw xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x07,0x4b,0xf0]
          vphramaxsw xmm22 {k7}, xmm23, xmm24

// CHECK: vphramaxsw xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc6,0x20,0x4b,0xf0]
          vphramaxsw xmm22, xmm23, ymm24

// CHECK: vphramaxsw xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc6,0x27,0x4b,0xf0]
          vphramaxsw xmm22 {k7}, xmm23, ymm24

// CHECK: vphramaxsw xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc6,0x00,0x4b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramaxsw xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphramaxsw xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc6,0x07,0x4b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramaxsw xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphramaxsw xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc6,0x10,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsw xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vphramaxsw xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x4b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphramaxsw xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphramaxsw xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xc6,0x07,0x4b,0x71,0x7f]
          vphramaxsw xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphramaxsw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc6,0x17,0x4b,0x72,0x80]
          vphramaxsw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vphramaxsw xmm22, xmm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0xc6,0x30,0x4b,0x35,0x00,0x00,0x00,0x00]
          vphramaxsw xmm22, xmm23, word ptr [rip]{1to16}

// CHECK: vphramaxsw xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xc6,0x20,0x4b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphramaxsw xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphramaxsw xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xc6,0x27,0x4b,0x71,0x7f]
          vphramaxsw xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphramaxsw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0xc6,0x37,0x4b,0x72,0x80]
          vphramaxsw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to16}

// CHECK: vphraminb xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x48,0xf0]
          vphraminb xmm22, xmm23, xmm24

// CHECK: vphraminb xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x48,0xf0]
          vphraminb xmm22 {k7}, xmm23, xmm24

// CHECK: vphraminb xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x48,0xf0]
          vphraminb xmm22, xmm23, ymm24

// CHECK: vphraminb xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x48,0xf0]
          vphraminb xmm22 {k7}, xmm23, ymm24

// CHECK: vphraminb xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminb xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminb xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminb xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminb xmm22, xmm23, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminb xmm22, xmm23, xmmword ptr [rip]

// CHECK: vphraminb xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x48,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminb xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraminb xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x48,0x71,0x7f]
          vphraminb xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraminb xmm22 {k7}, xmm23, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x48,0x72,0x80]
          vphraminb xmm22 {k7}, xmm23, xmmword ptr [rdx - 2048]

// CHECK: vphraminb xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x48,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminb xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraminb xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x48,0x71,0x7f]
          vphraminb xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraminb xmm22 {k7}, xmm23, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x48,0x72,0x80]
          vphraminb xmm22 {k7}, xmm23, ymmword ptr [rdx - 4096]

// CHECK: vphramind xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x48,0xf0]
          vphramind xmm22, xmm23, xmm24

// CHECK: vphramind xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x07,0x48,0xf0]
          vphramind xmm22 {k7}, xmm23, xmm24

// CHECK: vphramind xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x48,0xf0]
          vphramind xmm22, xmm23, ymm24

// CHECK: vphramind xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x27,0x48,0xf0]
          vphramind xmm22 {k7}, xmm23, ymm24

// CHECK: vphramind xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphramind xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphramind xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x07,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphramind xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphramind xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x48,0x35,0x00,0x00,0x00,0x00]
          vphramind xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vphramind xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x48,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphramind xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphramind xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x47,0x07,0x48,0x71,0x7f]
          vphramind xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphramind xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe5,0x47,0x17,0x48,0x72,0x80]
          vphramind xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vphramind xmm22, xmm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x48,0x35,0x00,0x00,0x00,0x00]
          vphramind xmm22, xmm23, dword ptr [rip]{1to8}

// CHECK: vphramind xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x48,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphramind xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphramind xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x47,0x27,0x48,0x71,0x7f]
          vphramind xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphramind xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x37,0x48,0x72,0x80]
          vphramind xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to8}

// CHECK: vphraminq xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x00,0x48,0xf0]
          vphraminq xmm22, xmm23, xmm24

// CHECK: vphraminq xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x07,0x48,0xf0]
          vphraminq xmm22 {k7}, xmm23, xmm24

// CHECK: vphraminq xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc7,0x20,0x48,0xf0]
          vphraminq xmm22, xmm23, ymm24

// CHECK: vphraminq xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc7,0x27,0x48,0xf0]
          vphraminq xmm22 {k7}, xmm23, ymm24

// CHECK: vphraminq xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc7,0x00,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminq xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminq xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc7,0x07,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminq xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminq xmm22, xmm23, qword ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe5,0xc7,0x10,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminq xmm22, xmm23, qword ptr [rip]{1to2}

// CHECK: vphraminq xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xc7,0x00,0x48,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminq xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraminq xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xc7,0x07,0x48,0x71,0x7f]
          vphraminq xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraminq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to2}
// CHECK: encoding: [0x62,0xe5,0xc7,0x17,0x48,0x72,0x80]
          vphraminq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to2}

// CHECK: vphraminq xmm22, xmm23, qword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0xc7,0x30,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminq xmm22, xmm23, qword ptr [rip]{1to4}

// CHECK: vphraminq xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xc7,0x20,0x48,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminq xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraminq xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xc7,0x27,0x48,0x71,0x7f]
          vphraminq xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraminq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to4}
// CHECK: encoding: [0x62,0xe5,0xc7,0x37,0x48,0x72,0x80]
          vphraminq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to4}

// CHECK: vphraminsb xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x49,0xf0]
          vphraminsb xmm22, xmm23, xmm24

// CHECK: vphraminsb xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x49,0xf0]
          vphraminsb xmm22 {k7}, xmm23, xmm24

// CHECK: vphraminsb xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x49,0xf0]
          vphraminsb xmm22, xmm23, ymm24

// CHECK: vphraminsb xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x49,0xf0]
          vphraminsb xmm22 {k7}, xmm23, ymm24

// CHECK: vphraminsb xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsb xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminsb xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsb xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminsb xmm22, xmm23, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsb xmm22, xmm23, xmmword ptr [rip]

// CHECK: vphraminsb xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x49,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsb xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraminsb xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x49,0x71,0x7f]
          vphraminsb xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraminsb xmm22 {k7}, xmm23, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe5,0x46,0x07,0x49,0x72,0x80]
          vphraminsb xmm22 {k7}, xmm23, xmmword ptr [rdx - 2048]

// CHECK: vphraminsb xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x49,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsb xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraminsb xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x49,0x71,0x7f]
          vphraminsb xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraminsb xmm22 {k7}, xmm23, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe5,0x46,0x27,0x49,0x72,0x80]
          vphraminsb xmm22 {k7}, xmm23, ymmword ptr [rdx - 4096]

// CHECK: vphraminsd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x49,0xf0]
          vphraminsd xmm22, xmm23, xmm24

// CHECK: vphraminsd xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x07,0x49,0xf0]
          vphraminsd xmm22 {k7}, xmm23, xmm24

// CHECK: vphraminsd xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x49,0xf0]
          vphraminsd xmm22, xmm23, ymm24

// CHECK: vphraminsd xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x27,0x49,0xf0]
          vphraminsd xmm22 {k7}, xmm23, ymm24

// CHECK: vphraminsd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminsd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x07,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminsd xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsd xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vphraminsd xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x49,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsd xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraminsd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x47,0x07,0x49,0x71,0x7f]
          vphraminsd xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraminsd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe5,0x47,0x17,0x49,0x72,0x80]
          vphraminsd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vphraminsd xmm22, xmm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsd xmm22, xmm23, dword ptr [rip]{1to8}

// CHECK: vphraminsd xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x49,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsd xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraminsd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x47,0x27,0x49,0x71,0x7f]
          vphraminsd xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraminsd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x37,0x49,0x72,0x80]
          vphraminsd xmm22 {k7}, xmm23, dword ptr [rdx - 512]{1to8}

// CHECK: vphraminsq xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x00,0x49,0xf0]
          vphraminsq xmm22, xmm23, xmm24

// CHECK: vphraminsq xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc7,0x07,0x49,0xf0]
          vphraminsq xmm22 {k7}, xmm23, xmm24

// CHECK: vphraminsq xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc7,0x20,0x49,0xf0]
          vphraminsq xmm22, xmm23, ymm24

// CHECK: vphraminsq xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc7,0x27,0x49,0xf0]
          vphraminsq xmm22 {k7}, xmm23, ymm24

// CHECK: vphraminsq xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc7,0x00,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsq xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminsq xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc7,0x07,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsq xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminsq xmm22, xmm23, qword ptr [rip]{1to2}
// CHECK: encoding: [0x62,0xe5,0xc7,0x10,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsq xmm22, xmm23, qword ptr [rip]{1to2}

// CHECK: vphraminsq xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xc7,0x00,0x49,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsq xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraminsq xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xc7,0x07,0x49,0x71,0x7f]
          vphraminsq xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraminsq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to2}
// CHECK: encoding: [0x62,0xe5,0xc7,0x17,0x49,0x72,0x80]
          vphraminsq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to2}

// CHECK: vphraminsq xmm22, xmm23, qword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0xc7,0x30,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsq xmm22, xmm23, qword ptr [rip]{1to4}

// CHECK: vphraminsq xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xc7,0x20,0x49,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsq xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraminsq xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xc7,0x27,0x49,0x71,0x7f]
          vphraminsq xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraminsq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to4}
// CHECK: encoding: [0x62,0xe5,0xc7,0x37,0x49,0x72,0x80]
          vphraminsq xmm22 {k7}, xmm23, qword ptr [rdx - 1024]{1to4}

// CHECK: vphraminsw xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x00,0x49,0xf0]
          vphraminsw xmm22, xmm23, xmm24

// CHECK: vphraminsw xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x07,0x49,0xf0]
          vphraminsw xmm22 {k7}, xmm23, xmm24

// CHECK: vphraminsw xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc6,0x20,0x49,0xf0]
          vphraminsw xmm22, xmm23, ymm24

// CHECK: vphraminsw xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc6,0x27,0x49,0xf0]
          vphraminsw xmm22 {k7}, xmm23, ymm24

// CHECK: vphraminsw xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc6,0x00,0x49,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminsw xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminsw xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc6,0x07,0x49,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminsw xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminsw xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc6,0x10,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsw xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vphraminsw xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x49,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminsw xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraminsw xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xc6,0x07,0x49,0x71,0x7f]
          vphraminsw xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraminsw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc6,0x17,0x49,0x72,0x80]
          vphraminsw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vphraminsw xmm22, xmm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0xc6,0x30,0x49,0x35,0x00,0x00,0x00,0x00]
          vphraminsw xmm22, xmm23, word ptr [rip]{1to16}

// CHECK: vphraminsw xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xc6,0x20,0x49,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminsw xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraminsw xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xc6,0x27,0x49,0x71,0x7f]
          vphraminsw xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraminsw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0xc6,0x37,0x49,0x72,0x80]
          vphraminsw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to16}

// CHECK: vphraminw xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x00,0x48,0xf0]
          vphraminw xmm22, xmm23, xmm24

// CHECK: vphraminw xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0xc6,0x07,0x48,0xf0]
          vphraminw xmm22 {k7}, xmm23, xmm24

// CHECK: vphraminw xmm22, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc6,0x20,0x48,0xf0]
          vphraminw xmm22, xmm23, ymm24

// CHECK: vphraminw xmm22 {k7}, xmm23, ymm24
// CHECK: encoding: [0x62,0x85,0xc6,0x27,0x48,0xf0]
          vphraminw xmm22 {k7}, xmm23, ymm24

// CHECK: vphraminw xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xc6,0x00,0x48,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vphraminw xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vphraminw xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xc6,0x07,0x48,0xb4,0x80,0x23,0x01,0x00,0x00]
          vphraminw xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vphraminw xmm22, xmm23, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc6,0x10,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminw xmm22, xmm23, word ptr [rip]{1to8}

// CHECK: vphraminw xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x48,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vphraminw xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vphraminw xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xc6,0x07,0x48,0x71,0x7f]
          vphraminw xmm22 {k7}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vphraminw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe5,0xc6,0x17,0x48,0x72,0x80]
          vphraminw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to8}

// CHECK: vphraminw xmm22, xmm23, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe5,0xc6,0x30,0x48,0x35,0x00,0x00,0x00,0x00]
          vphraminw xmm22, xmm23, word ptr [rip]{1to16}

// CHECK: vphraminw xmm22, xmm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xc6,0x20,0x48,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vphraminw xmm22, xmm23, ymmword ptr [2*rbp - 1024]

// CHECK: vphraminw xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xc6,0x27,0x48,0x71,0x7f]
          vphraminw xmm22 {k7}, xmm23, ymmword ptr [rcx + 4064]

// CHECK: vphraminw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe5,0xc6,0x37,0x48,0x72,0x80]
          vphraminw xmm22 {k7}, xmm23, word ptr [rdx - 256]{1to16}

