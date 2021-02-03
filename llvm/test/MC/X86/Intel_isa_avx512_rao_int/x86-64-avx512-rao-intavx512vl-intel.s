// REQUIRES: intel_feature_isa_avx512_rao_int
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vpaaddd ymmword ptr [rbp + 8*r14 + 268435456], ymm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x28,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaaddd ymmword ptr [rbp + 8*r14 + 268435456], ymm22

// CHECK:      vpaaddd ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22
// CHECK: encoding: [0x62,0xc2,0x7c,0x2f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaaddd ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22

// CHECK:      vpaaddd ymmword ptr [rip], ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x28,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaaddd ymmword ptr [rip], ymm22

// CHECK:      vpaaddd ymmword ptr [2*rbp - 1024], ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x28,0xfc,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpaaddd ymmword ptr [2*rbp - 1024], ymm22

// CHECK:      vpaaddd ymmword ptr [rcx + 4064] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x2f,0xfc,0x71,0x7f]
               vpaaddd ymmword ptr [rcx + 4064] {k7}, ymm22

// CHECK:      vpaaddd ymmword ptr [rdx - 4096] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x2f,0xfc,0x72,0x80]
               vpaaddd ymmword ptr [rdx - 4096] {k7}, ymm22

// CHECK:      vpaaddd xmmword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x08,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaaddd xmmword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vpaaddd xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0x7c,0x0f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaaddd xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vpaaddd xmmword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x08,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaaddd xmmword ptr [rip], xmm22

// CHECK:      vpaaddd xmmword ptr [2*rbp - 512], xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x08,0xfc,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpaaddd xmmword ptr [2*rbp - 512], xmm22

// CHECK:      vpaaddd xmmword ptr [rcx + 2032] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x0f,0xfc,0x71,0x7f]
               vpaaddd xmmword ptr [rcx + 2032] {k7}, xmm22

// CHECK:      vpaaddd xmmword ptr [rdx - 2048] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x0f,0xfc,0x72,0x80]
               vpaaddd xmmword ptr [rdx - 2048] {k7}, xmm22

// CHECK:      vpaaddq ymmword ptr [rbp + 8*r14 + 268435456], ymm22
// CHECK: encoding: [0x62,0xa2,0xfc,0x28,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaaddq ymmword ptr [rbp + 8*r14 + 268435456], ymm22

// CHECK:      vpaaddq ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22
// CHECK: encoding: [0x62,0xc2,0xfc,0x2f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaaddq ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22

// CHECK:      vpaaddq ymmword ptr [rip], ymm22
// CHECK: encoding: [0x62,0xe2,0xfc,0x28,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaaddq ymmword ptr [rip], ymm22

// CHECK:      vpaaddq ymmword ptr [2*rbp - 1024], ymm22
// CHECK: encoding: [0x62,0xe2,0xfc,0x28,0xfc,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpaaddq ymmword ptr [2*rbp - 1024], ymm22

// CHECK:      vpaaddq ymmword ptr [rcx + 4064] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0xfc,0x2f,0xfc,0x71,0x7f]
               vpaaddq ymmword ptr [rcx + 4064] {k7}, ymm22

// CHECK:      vpaaddq ymmword ptr [rdx - 4096] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0xfc,0x2f,0xfc,0x72,0x80]
               vpaaddq ymmword ptr [rdx - 4096] {k7}, ymm22

// CHECK:      vpaaddq xmmword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0xfc,0x08,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaaddq xmmword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vpaaddq xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0xfc,0x0f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaaddq xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vpaaddq xmmword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0xfc,0x08,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaaddq xmmword ptr [rip], xmm22

// CHECK:      vpaaddq xmmword ptr [2*rbp - 512], xmm22
// CHECK: encoding: [0x62,0xe2,0xfc,0x08,0xfc,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpaaddq xmmword ptr [2*rbp - 512], xmm22

// CHECK:      vpaaddq xmmword ptr [rcx + 2032] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0xfc,0x0f,0xfc,0x71,0x7f]
               vpaaddq xmmword ptr [rcx + 2032] {k7}, xmm22

// CHECK:      vpaaddq xmmword ptr [rdx - 2048] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0xfc,0x0f,0xfc,0x72,0x80]
               vpaaddq xmmword ptr [rdx - 2048] {k7}, xmm22

// CHECK:      vpaandd ymmword ptr [rbp + 8*r14 + 268435456], ymm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x28,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaandd ymmword ptr [rbp + 8*r14 + 268435456], ymm22

// CHECK:      vpaandd ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22
// CHECK: encoding: [0x62,0xc2,0x7d,0x2f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaandd ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22

// CHECK:      vpaandd ymmword ptr [rip], ymm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x28,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaandd ymmword ptr [rip], ymm22

// CHECK:      vpaandd ymmword ptr [2*rbp - 1024], ymm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x28,0xfc,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpaandd ymmword ptr [2*rbp - 1024], ymm22

// CHECK:      vpaandd ymmword ptr [rcx + 4064] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x2f,0xfc,0x71,0x7f]
               vpaandd ymmword ptr [rcx + 4064] {k7}, ymm22

// CHECK:      vpaandd ymmword ptr [rdx - 4096] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x2f,0xfc,0x72,0x80]
               vpaandd ymmword ptr [rdx - 4096] {k7}, ymm22

// CHECK:      vpaandd xmmword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x08,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaandd xmmword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vpaandd xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0x7d,0x0f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaandd xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vpaandd xmmword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x08,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaandd xmmword ptr [rip], xmm22

// CHECK:      vpaandd xmmword ptr [2*rbp - 512], xmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x08,0xfc,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpaandd xmmword ptr [2*rbp - 512], xmm22

// CHECK:      vpaandd xmmword ptr [rcx + 2032] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x0f,0xfc,0x71,0x7f]
               vpaandd xmmword ptr [rcx + 2032] {k7}, xmm22

// CHECK:      vpaandd xmmword ptr [rdx - 2048] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x0f,0xfc,0x72,0x80]
               vpaandd xmmword ptr [rdx - 2048] {k7}, xmm22

// CHECK:      vpaandq ymmword ptr [rbp + 8*r14 + 268435456], ymm22
// CHECK: encoding: [0x62,0xa2,0xfd,0x28,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaandq ymmword ptr [rbp + 8*r14 + 268435456], ymm22

// CHECK:      vpaandq ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22
// CHECK: encoding: [0x62,0xc2,0xfd,0x2f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaandq ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22

// CHECK:      vpaandq ymmword ptr [rip], ymm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x28,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaandq ymmword ptr [rip], ymm22

// CHECK:      vpaandq ymmword ptr [2*rbp - 1024], ymm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x28,0xfc,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpaandq ymmword ptr [2*rbp - 1024], ymm22

// CHECK:      vpaandq ymmword ptr [rcx + 4064] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x2f,0xfc,0x71,0x7f]
               vpaandq ymmword ptr [rcx + 4064] {k7}, ymm22

// CHECK:      vpaandq ymmword ptr [rdx - 4096] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x2f,0xfc,0x72,0x80]
               vpaandq ymmword ptr [rdx - 4096] {k7}, ymm22

// CHECK:      vpaandq xmmword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0xfd,0x08,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaandq xmmword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vpaandq xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0xfd,0x0f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaandq xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vpaandq xmmword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x08,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaandq xmmword ptr [rip], xmm22

// CHECK:      vpaandq xmmword ptr [2*rbp - 512], xmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x08,0xfc,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpaandq xmmword ptr [2*rbp - 512], xmm22

// CHECK:      vpaandq xmmword ptr [rcx + 2032] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x0f,0xfc,0x71,0x7f]
               vpaandq xmmword ptr [rcx + 2032] {k7}, xmm22

// CHECK:      vpaandq xmmword ptr [rdx - 2048] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x0f,0xfc,0x72,0x80]
               vpaandq xmmword ptr [rdx - 2048] {k7}, xmm22

// CHECK:      vpaord ymmword ptr [rbp + 8*r14 + 268435456], ymm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x28,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaord ymmword ptr [rbp + 8*r14 + 268435456], ymm22

// CHECK:      vpaord ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22
// CHECK: encoding: [0x62,0xc2,0x7f,0x2f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaord ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22

// CHECK:      vpaord ymmword ptr [rip], ymm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x28,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaord ymmword ptr [rip], ymm22

// CHECK:      vpaord ymmword ptr [2*rbp - 1024], ymm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x28,0xfc,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpaord ymmword ptr [2*rbp - 1024], ymm22

// CHECK:      vpaord ymmword ptr [rcx + 4064] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x2f,0xfc,0x71,0x7f]
               vpaord ymmword ptr [rcx + 4064] {k7}, ymm22

// CHECK:      vpaord ymmword ptr [rdx - 4096] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x2f,0xfc,0x72,0x80]
               vpaord ymmword ptr [rdx - 4096] {k7}, ymm22

// CHECK:      vpaord xmmword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x08,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaord xmmword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vpaord xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0x7f,0x0f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaord xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vpaord xmmword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x08,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaord xmmword ptr [rip], xmm22

// CHECK:      vpaord xmmword ptr [2*rbp - 512], xmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x08,0xfc,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpaord xmmword ptr [2*rbp - 512], xmm22

// CHECK:      vpaord xmmword ptr [rcx + 2032] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x0f,0xfc,0x71,0x7f]
               vpaord xmmword ptr [rcx + 2032] {k7}, xmm22

// CHECK:      vpaord xmmword ptr [rdx - 2048] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x0f,0xfc,0x72,0x80]
               vpaord xmmword ptr [rdx - 2048] {k7}, xmm22

// CHECK:      vpaorq ymmword ptr [rbp + 8*r14 + 268435456], ymm22
// CHECK: encoding: [0x62,0xa2,0xff,0x28,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaorq ymmword ptr [rbp + 8*r14 + 268435456], ymm22

// CHECK:      vpaorq ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22
// CHECK: encoding: [0x62,0xc2,0xff,0x2f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaorq ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22

// CHECK:      vpaorq ymmword ptr [rip], ymm22
// CHECK: encoding: [0x62,0xe2,0xff,0x28,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaorq ymmword ptr [rip], ymm22

// CHECK:      vpaorq ymmword ptr [2*rbp - 1024], ymm22
// CHECK: encoding: [0x62,0xe2,0xff,0x28,0xfc,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpaorq ymmword ptr [2*rbp - 1024], ymm22

// CHECK:      vpaorq ymmword ptr [rcx + 4064] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0xff,0x2f,0xfc,0x71,0x7f]
               vpaorq ymmword ptr [rcx + 4064] {k7}, ymm22

// CHECK:      vpaorq ymmword ptr [rdx - 4096] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0xff,0x2f,0xfc,0x72,0x80]
               vpaorq ymmword ptr [rdx - 4096] {k7}, ymm22

// CHECK:      vpaorq xmmword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0xff,0x08,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaorq xmmword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vpaorq xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0xff,0x0f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaorq xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vpaorq xmmword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0xff,0x08,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaorq xmmword ptr [rip], xmm22

// CHECK:      vpaorq xmmword ptr [2*rbp - 512], xmm22
// CHECK: encoding: [0x62,0xe2,0xff,0x08,0xfc,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpaorq xmmword ptr [2*rbp - 512], xmm22

// CHECK:      vpaorq xmmword ptr [rcx + 2032] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0xff,0x0f,0xfc,0x71,0x7f]
               vpaorq xmmword ptr [rcx + 2032] {k7}, xmm22

// CHECK:      vpaorq xmmword ptr [rdx - 2048] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0xff,0x0f,0xfc,0x72,0x80]
               vpaorq xmmword ptr [rdx - 2048] {k7}, xmm22

// CHECK:      vpaxord ymmword ptr [rbp + 8*r14 + 268435456], ymm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x28,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaxord ymmword ptr [rbp + 8*r14 + 268435456], ymm22

// CHECK:      vpaxord ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22
// CHECK: encoding: [0x62,0xc2,0x7e,0x2f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaxord ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22

// CHECK:      vpaxord ymmword ptr [rip], ymm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaxord ymmword ptr [rip], ymm22

// CHECK:      vpaxord ymmword ptr [2*rbp - 1024], ymm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xfc,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpaxord ymmword ptr [2*rbp - 1024], ymm22

// CHECK:      vpaxord ymmword ptr [rcx + 4064] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x2f,0xfc,0x71,0x7f]
               vpaxord ymmword ptr [rcx + 4064] {k7}, ymm22

// CHECK:      vpaxord ymmword ptr [rdx - 4096] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x2f,0xfc,0x72,0x80]
               vpaxord ymmword ptr [rdx - 4096] {k7}, ymm22

// CHECK:      vpaxord xmmword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaxord xmmword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vpaxord xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0x7e,0x0f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaxord xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vpaxord xmmword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaxord xmmword ptr [rip], xmm22

// CHECK:      vpaxord xmmword ptr [2*rbp - 512], xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xfc,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpaxord xmmword ptr [2*rbp - 512], xmm22

// CHECK:      vpaxord xmmword ptr [rcx + 2032] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x0f,0xfc,0x71,0x7f]
               vpaxord xmmword ptr [rcx + 2032] {k7}, xmm22

// CHECK:      vpaxord xmmword ptr [rdx - 2048] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x0f,0xfc,0x72,0x80]
               vpaxord xmmword ptr [rdx - 2048] {k7}, xmm22

// CHECK:      vpaxorq ymmword ptr [rbp + 8*r14 + 268435456], ymm22
// CHECK: encoding: [0x62,0xa2,0xfe,0x28,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaxorq ymmword ptr [rbp + 8*r14 + 268435456], ymm22

// CHECK:      vpaxorq ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22
// CHECK: encoding: [0x62,0xc2,0xfe,0x2f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaxorq ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22

// CHECK:      vpaxorq ymmword ptr [rip], ymm22
// CHECK: encoding: [0x62,0xe2,0xfe,0x28,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaxorq ymmword ptr [rip], ymm22

// CHECK:      vpaxorq ymmword ptr [2*rbp - 1024], ymm22
// CHECK: encoding: [0x62,0xe2,0xfe,0x28,0xfc,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpaxorq ymmword ptr [2*rbp - 1024], ymm22

// CHECK:      vpaxorq ymmword ptr [rcx + 4064] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0xfe,0x2f,0xfc,0x71,0x7f]
               vpaxorq ymmword ptr [rcx + 4064] {k7}, ymm22

// CHECK:      vpaxorq ymmword ptr [rdx - 4096] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0xfe,0x2f,0xfc,0x72,0x80]
               vpaxorq ymmword ptr [rdx - 4096] {k7}, ymm22

// CHECK:      vpaxorq xmmword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0xfe,0x08,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaxorq xmmword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vpaxorq xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0xfe,0x0f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaxorq xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vpaxorq xmmword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0xfe,0x08,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaxorq xmmword ptr [rip], xmm22

// CHECK:      vpaxorq xmmword ptr [2*rbp - 512], xmm22
// CHECK: encoding: [0x62,0xe2,0xfe,0x08,0xfc,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpaxorq xmmword ptr [2*rbp - 512], xmm22

// CHECK:      vpaxorq xmmword ptr [rcx + 2032] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0xfe,0x0f,0xfc,0x71,0x7f]
               vpaxorq xmmword ptr [rcx + 2032] {k7}, xmm22

// CHECK:      vpaxorq xmmword ptr [rdx - 2048] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0xfe,0x0f,0xfc,0x72,0x80]
               vpaxorq xmmword ptr [rdx - 2048] {k7}, xmm22

