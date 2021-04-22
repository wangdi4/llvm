// REQUIRES: intel_feature_isa_avx512_rao_int
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vpaaddd zmmword ptr [rbp + 8*r14 + 268435456], zmm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x48,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaaddd zmmword ptr [rbp + 8*r14 + 268435456], zmm22

// CHECK:      vpaaddd zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22
// CHECK: encoding: [0x62,0xc2,0x7c,0x4f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaaddd zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22

// CHECK:      vpaaddd zmmword ptr [rip], zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x48,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaaddd zmmword ptr [rip], zmm22

// CHECK:      vpaaddd zmmword ptr [2*rbp - 2048], zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x48,0xfc,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpaaddd zmmword ptr [2*rbp - 2048], zmm22

// CHECK:      vpaaddd zmmword ptr [rcx + 8128] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x4f,0xfc,0x71,0x7f]
               vpaaddd zmmword ptr [rcx + 8128] {k7}, zmm22

// CHECK:      vpaaddd zmmword ptr [rdx - 8192] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x4f,0xfc,0x72,0x80]
               vpaaddd zmmword ptr [rdx - 8192] {k7}, zmm22

// CHECK:      vpaaddq zmmword ptr [rbp + 8*r14 + 268435456], zmm22
// CHECK: encoding: [0x62,0xa2,0xfc,0x48,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaaddq zmmword ptr [rbp + 8*r14 + 268435456], zmm22

// CHECK:      vpaaddq zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22
// CHECK: encoding: [0x62,0xc2,0xfc,0x4f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaaddq zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22

// CHECK:      vpaaddq zmmword ptr [rip], zmm22
// CHECK: encoding: [0x62,0xe2,0xfc,0x48,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaaddq zmmword ptr [rip], zmm22

// CHECK:      vpaaddq zmmword ptr [2*rbp - 2048], zmm22
// CHECK: encoding: [0x62,0xe2,0xfc,0x48,0xfc,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpaaddq zmmword ptr [2*rbp - 2048], zmm22

// CHECK:      vpaaddq zmmword ptr [rcx + 8128] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0xfc,0x4f,0xfc,0x71,0x7f]
               vpaaddq zmmword ptr [rcx + 8128] {k7}, zmm22

// CHECK:      vpaaddq zmmword ptr [rdx - 8192] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0xfc,0x4f,0xfc,0x72,0x80]
               vpaaddq zmmword ptr [rdx - 8192] {k7}, zmm22

// CHECK:      vpaandd zmmword ptr [rbp + 8*r14 + 268435456], zmm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaandd zmmword ptr [rbp + 8*r14 + 268435456], zmm22

// CHECK:      vpaandd zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22
// CHECK: encoding: [0x62,0xc2,0x7d,0x4f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaandd zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22

// CHECK:      vpaandd zmmword ptr [rip], zmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaandd zmmword ptr [rip], zmm22

// CHECK:      vpaandd zmmword ptr [2*rbp - 2048], zmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0xfc,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpaandd zmmword ptr [2*rbp - 2048], zmm22

// CHECK:      vpaandd zmmword ptr [rcx + 8128] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x4f,0xfc,0x71,0x7f]
               vpaandd zmmword ptr [rcx + 8128] {k7}, zmm22

// CHECK:      vpaandd zmmword ptr [rdx - 8192] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x4f,0xfc,0x72,0x80]
               vpaandd zmmword ptr [rdx - 8192] {k7}, zmm22

// CHECK:      vpaandq zmmword ptr [rbp + 8*r14 + 268435456], zmm22
// CHECK: encoding: [0x62,0xa2,0xfd,0x48,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaandq zmmword ptr [rbp + 8*r14 + 268435456], zmm22

// CHECK:      vpaandq zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22
// CHECK: encoding: [0x62,0xc2,0xfd,0x4f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaandq zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22

// CHECK:      vpaandq zmmword ptr [rip], zmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x48,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaandq zmmword ptr [rip], zmm22

// CHECK:      vpaandq zmmword ptr [2*rbp - 2048], zmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x48,0xfc,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpaandq zmmword ptr [2*rbp - 2048], zmm22

// CHECK:      vpaandq zmmword ptr [rcx + 8128] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x4f,0xfc,0x71,0x7f]
               vpaandq zmmword ptr [rcx + 8128] {k7}, zmm22

// CHECK:      vpaandq zmmword ptr [rdx - 8192] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x4f,0xfc,0x72,0x80]
               vpaandq zmmword ptr [rdx - 8192] {k7}, zmm22

// CHECK:      vpaord zmmword ptr [rbp + 8*r14 + 268435456], zmm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x48,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaord zmmword ptr [rbp + 8*r14 + 268435456], zmm22

// CHECK:      vpaord zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22
// CHECK: encoding: [0x62,0xc2,0x7f,0x4f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaord zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22

// CHECK:      vpaord zmmword ptr [rip], zmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x48,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaord zmmword ptr [rip], zmm22

// CHECK:      vpaord zmmword ptr [2*rbp - 2048], zmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x48,0xfc,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpaord zmmword ptr [2*rbp - 2048], zmm22

// CHECK:      vpaord zmmword ptr [rcx + 8128] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x4f,0xfc,0x71,0x7f]
               vpaord zmmword ptr [rcx + 8128] {k7}, zmm22

// CHECK:      vpaord zmmword ptr [rdx - 8192] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x4f,0xfc,0x72,0x80]
               vpaord zmmword ptr [rdx - 8192] {k7}, zmm22

// CHECK:      vpaorq zmmword ptr [rbp + 8*r14 + 268435456], zmm22
// CHECK: encoding: [0x62,0xa2,0xff,0x48,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaorq zmmword ptr [rbp + 8*r14 + 268435456], zmm22

// CHECK:      vpaorq zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22
// CHECK: encoding: [0x62,0xc2,0xff,0x4f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaorq zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22

// CHECK:      vpaorq zmmword ptr [rip], zmm22
// CHECK: encoding: [0x62,0xe2,0xff,0x48,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaorq zmmword ptr [rip], zmm22

// CHECK:      vpaorq zmmword ptr [2*rbp - 2048], zmm22
// CHECK: encoding: [0x62,0xe2,0xff,0x48,0xfc,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpaorq zmmword ptr [2*rbp - 2048], zmm22

// CHECK:      vpaorq zmmword ptr [rcx + 8128] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0xff,0x4f,0xfc,0x71,0x7f]
               vpaorq zmmword ptr [rcx + 8128] {k7}, zmm22

// CHECK:      vpaorq zmmword ptr [rdx - 8192] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0xff,0x4f,0xfc,0x72,0x80]
               vpaorq zmmword ptr [rdx - 8192] {k7}, zmm22

// CHECK:      vpaxord zmmword ptr [rbp + 8*r14 + 268435456], zmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaxord zmmword ptr [rbp + 8*r14 + 268435456], zmm22

// CHECK:      vpaxord zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22
// CHECK: encoding: [0x62,0xc2,0x7e,0x4f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaxord zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22

// CHECK:      vpaxord zmmword ptr [rip], zmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaxord zmmword ptr [rip], zmm22

// CHECK:      vpaxord zmmword ptr [2*rbp - 2048], zmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xfc,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpaxord zmmword ptr [2*rbp - 2048], zmm22

// CHECK:      vpaxord zmmword ptr [rcx + 8128] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x4f,0xfc,0x71,0x7f]
               vpaxord zmmword ptr [rcx + 8128] {k7}, zmm22

// CHECK:      vpaxord zmmword ptr [rdx - 8192] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x4f,0xfc,0x72,0x80]
               vpaxord zmmword ptr [rdx - 8192] {k7}, zmm22

// CHECK:      vpaxorq zmmword ptr [rbp + 8*r14 + 268435456], zmm22
// CHECK: encoding: [0x62,0xa2,0xfe,0x48,0xfc,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpaxorq zmmword ptr [rbp + 8*r14 + 268435456], zmm22

// CHECK:      vpaxorq zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22
// CHECK: encoding: [0x62,0xc2,0xfe,0x4f,0xfc,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpaxorq zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22

// CHECK:      vpaxorq zmmword ptr [rip], zmm22
// CHECK: encoding: [0x62,0xe2,0xfe,0x48,0xfc,0x35,0x00,0x00,0x00,0x00]
               vpaxorq zmmword ptr [rip], zmm22

// CHECK:      vpaxorq zmmword ptr [2*rbp - 2048], zmm22
// CHECK: encoding: [0x62,0xe2,0xfe,0x48,0xfc,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpaxorq zmmword ptr [2*rbp - 2048], zmm22

// CHECK:      vpaxorq zmmword ptr [rcx + 8128] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0xfe,0x4f,0xfc,0x71,0x7f]
               vpaxorq zmmword ptr [rcx + 8128] {k7}, zmm22

// CHECK:      vpaxorq zmmword ptr [rdx - 8192] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0xfe,0x4f,0xfc,0x72,0x80]
               vpaxorq zmmword ptr [rdx - 8192] {k7}, zmm22

