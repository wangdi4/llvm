// REQUIRES: intel_feature_isa_avx512_rao_fp
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vaaddpbf16 ymmword ptr [rbp + 8*r14 + 268435456], ymm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x28,0x94,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddpbf16 ymmword ptr [rbp + 8*r14 + 268435456], ymm22

// CHECK:      vaaddpbf16 ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22
// CHECK: encoding: [0x62,0xc2,0x7d,0x2f,0x94,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddpbf16 ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22

// CHECK:      vaaddpbf16 ymmword ptr [rip], ymm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x28,0x94,0x35,0x00,0x00,0x00,0x00]
               vaaddpbf16 ymmword ptr [rip], ymm22

// CHECK:      vaaddpbf16 ymmword ptr [2*rbp - 1024], ymm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x28,0x94,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vaaddpbf16 ymmword ptr [2*rbp - 1024], ymm22

// CHECK:      vaaddpbf16 ymmword ptr [rcx + 4064] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x2f,0x94,0x71,0x7f]
               vaaddpbf16 ymmword ptr [rcx + 4064] {k7}, ymm22

// CHECK:      vaaddpbf16 ymmword ptr [rdx - 4096] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x2f,0x94,0x72,0x80]
               vaaddpbf16 ymmword ptr [rdx - 4096] {k7}, ymm22

// CHECK:      vaaddpbf16 xmmword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x08,0x94,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddpbf16 xmmword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vaaddpbf16 xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0x7d,0x0f,0x94,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddpbf16 xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vaaddpbf16 xmmword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x08,0x94,0x35,0x00,0x00,0x00,0x00]
               vaaddpbf16 xmmword ptr [rip], xmm22

// CHECK:      vaaddpbf16 xmmword ptr [2*rbp - 512], xmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x08,0x94,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vaaddpbf16 xmmword ptr [2*rbp - 512], xmm22

// CHECK:      vaaddpbf16 xmmword ptr [rcx + 2032] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x0f,0x94,0x71,0x7f]
               vaaddpbf16 xmmword ptr [rcx + 2032] {k7}, xmm22

// CHECK:      vaaddpbf16 xmmword ptr [rdx - 2048] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x0f,0x94,0x72,0x80]
               vaaddpbf16 xmmword ptr [rdx - 2048] {k7}, xmm22

// CHECK:      vaaddpd ymmword ptr [rbp + 8*r14 + 268435456], ymm22
// CHECK: encoding: [0x62,0xa2,0xfd,0x28,0x84,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddpd ymmword ptr [rbp + 8*r14 + 268435456], ymm22

// CHECK:      vaaddpd ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22
// CHECK: encoding: [0x62,0xc2,0xfd,0x2f,0x84,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddpd ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22

// CHECK:      vaaddpd ymmword ptr [rip], ymm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x28,0x84,0x35,0x00,0x00,0x00,0x00]
               vaaddpd ymmword ptr [rip], ymm22

// CHECK:      vaaddpd ymmword ptr [2*rbp - 1024], ymm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x28,0x84,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vaaddpd ymmword ptr [2*rbp - 1024], ymm22

// CHECK:      vaaddpd ymmword ptr [rcx + 4064] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x2f,0x84,0x71,0x7f]
               vaaddpd ymmword ptr [rcx + 4064] {k7}, ymm22

// CHECK:      vaaddpd ymmword ptr [rdx - 4096] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x2f,0x84,0x72,0x80]
               vaaddpd ymmword ptr [rdx - 4096] {k7}, ymm22

// CHECK:      vaaddpd xmmword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0xfd,0x08,0x84,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddpd xmmword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vaaddpd xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0xfd,0x0f,0x84,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddpd xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vaaddpd xmmword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x08,0x84,0x35,0x00,0x00,0x00,0x00]
               vaaddpd xmmword ptr [rip], xmm22

// CHECK:      vaaddpd xmmword ptr [2*rbp - 512], xmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x08,0x84,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vaaddpd xmmword ptr [2*rbp - 512], xmm22

// CHECK:      vaaddpd xmmword ptr [rcx + 2032] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x0f,0x84,0x71,0x7f]
               vaaddpd xmmword ptr [rcx + 2032] {k7}, xmm22

// CHECK:      vaaddpd xmmword ptr [rdx - 2048] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x0f,0x84,0x72,0x80]
               vaaddpd xmmword ptr [rdx - 2048] {k7}, xmm22

// CHECK:      vaaddph ymmword ptr [rbp + 8*r14 + 268435456], ymm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x28,0x94,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddph ymmword ptr [rbp + 8*r14 + 268435456], ymm22

// CHECK:      vaaddph ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22
// CHECK: encoding: [0x62,0xc2,0x7c,0x2f,0x94,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddph ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22

// CHECK:      vaaddph ymmword ptr [rip], ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x28,0x94,0x35,0x00,0x00,0x00,0x00]
               vaaddph ymmword ptr [rip], ymm22

// CHECK:      vaaddph ymmword ptr [2*rbp - 1024], ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x28,0x94,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vaaddph ymmword ptr [2*rbp - 1024], ymm22

// CHECK:      vaaddph ymmword ptr [rcx + 4064] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x2f,0x94,0x71,0x7f]
               vaaddph ymmword ptr [rcx + 4064] {k7}, ymm22

// CHECK:      vaaddph ymmword ptr [rdx - 4096] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x2f,0x94,0x72,0x80]
               vaaddph ymmword ptr [rdx - 4096] {k7}, ymm22

// CHECK:      vaaddph xmmword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x08,0x94,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddph xmmword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vaaddph xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0x7c,0x0f,0x94,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddph xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vaaddph xmmword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x08,0x94,0x35,0x00,0x00,0x00,0x00]
               vaaddph xmmword ptr [rip], xmm22

// CHECK:      vaaddph xmmword ptr [2*rbp - 512], xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x08,0x94,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vaaddph xmmword ptr [2*rbp - 512], xmm22

// CHECK:      vaaddph xmmword ptr [rcx + 2032] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x0f,0x94,0x71,0x7f]
               vaaddph xmmword ptr [rcx + 2032] {k7}, xmm22

// CHECK:      vaaddph xmmword ptr [rdx - 2048] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x0f,0x94,0x72,0x80]
               vaaddph xmmword ptr [rdx - 2048] {k7}, xmm22

// CHECK:      vaaddps ymmword ptr [rbp + 8*r14 + 268435456], ymm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x28,0x84,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddps ymmword ptr [rbp + 8*r14 + 268435456], ymm22

// CHECK:      vaaddps ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22
// CHECK: encoding: [0x62,0xc2,0x7c,0x2f,0x84,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddps ymmword ptr [r8 + 4*rax + 291] {k7}, ymm22

// CHECK:      vaaddps ymmword ptr [rip], ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x28,0x84,0x35,0x00,0x00,0x00,0x00]
               vaaddps ymmword ptr [rip], ymm22

// CHECK:      vaaddps ymmword ptr [2*rbp - 1024], ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x28,0x84,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vaaddps ymmword ptr [2*rbp - 1024], ymm22

// CHECK:      vaaddps ymmword ptr [rcx + 4064] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x2f,0x84,0x71,0x7f]
               vaaddps ymmword ptr [rcx + 4064] {k7}, ymm22

// CHECK:      vaaddps ymmword ptr [rdx - 4096] {k7}, ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x2f,0x84,0x72,0x80]
               vaaddps ymmword ptr [rdx - 4096] {k7}, ymm22

// CHECK:      vaaddps xmmword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x08,0x84,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddps xmmword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vaaddps xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0x7c,0x0f,0x84,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddps xmmword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vaaddps xmmword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x08,0x84,0x35,0x00,0x00,0x00,0x00]
               vaaddps xmmword ptr [rip], xmm22

// CHECK:      vaaddps xmmword ptr [2*rbp - 512], xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x08,0x84,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vaaddps xmmword ptr [2*rbp - 512], xmm22

// CHECK:      vaaddps xmmword ptr [rcx + 2032] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x0f,0x84,0x71,0x7f]
               vaaddps xmmword ptr [rcx + 2032] {k7}, xmm22

// CHECK:      vaaddps xmmword ptr [rdx - 2048] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x0f,0x84,0x72,0x80]
               vaaddps xmmword ptr [rdx - 2048] {k7}, xmm22

