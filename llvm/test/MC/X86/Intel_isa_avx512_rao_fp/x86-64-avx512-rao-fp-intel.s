// REQUIRES: intel_feature_isa_avx512_rao_fp
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vaaddpbf16 zmmword ptr [rbp + 8*r14 + 268435456], zmm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0x94,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddpbf16 zmmword ptr [rbp + 8*r14 + 268435456], zmm22

// CHECK:      vaaddpbf16 zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22
// CHECK: encoding: [0x62,0xc2,0x7d,0x4f,0x94,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddpbf16 zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22

// CHECK:      vaaddpbf16 zmmword ptr [rip], zmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0x94,0x35,0x00,0x00,0x00,0x00]
               vaaddpbf16 zmmword ptr [rip], zmm22

// CHECK:      vaaddpbf16 zmmword ptr [2*rbp - 2048], zmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0x94,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vaaddpbf16 zmmword ptr [2*rbp - 2048], zmm22

// CHECK:      vaaddpbf16 zmmword ptr [rcx + 8128] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x4f,0x94,0x71,0x7f]
               vaaddpbf16 zmmword ptr [rcx + 8128] {k7}, zmm22

// CHECK:      vaaddpbf16 zmmword ptr [rdx - 8192] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x4f,0x94,0x72,0x80]
               vaaddpbf16 zmmword ptr [rdx - 8192] {k7}, zmm22

// CHECK:      vaaddpd zmmword ptr [rbp + 8*r14 + 268435456], zmm22
// CHECK: encoding: [0x62,0xa2,0xfd,0x48,0x84,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddpd zmmword ptr [rbp + 8*r14 + 268435456], zmm22

// CHECK:      vaaddpd zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22
// CHECK: encoding: [0x62,0xc2,0xfd,0x4f,0x84,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddpd zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22

// CHECK:      vaaddpd zmmword ptr [rip], zmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x48,0x84,0x35,0x00,0x00,0x00,0x00]
               vaaddpd zmmword ptr [rip], zmm22

// CHECK:      vaaddpd zmmword ptr [2*rbp - 2048], zmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x48,0x84,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vaaddpd zmmword ptr [2*rbp - 2048], zmm22

// CHECK:      vaaddpd zmmword ptr [rcx + 8128] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x4f,0x84,0x71,0x7f]
               vaaddpd zmmword ptr [rcx + 8128] {k7}, zmm22

// CHECK:      vaaddpd zmmword ptr [rdx - 8192] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0xfd,0x4f,0x84,0x72,0x80]
               vaaddpd zmmword ptr [rdx - 8192] {k7}, zmm22

// CHECK:      vaaddph zmmword ptr [rbp + 8*r14 + 268435456], zmm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x48,0x94,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddph zmmword ptr [rbp + 8*r14 + 268435456], zmm22

// CHECK:      vaaddph zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22
// CHECK: encoding: [0x62,0xc2,0x7c,0x4f,0x94,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddph zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22

// CHECK:      vaaddph zmmword ptr [rip], zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x48,0x94,0x35,0x00,0x00,0x00,0x00]
               vaaddph zmmword ptr [rip], zmm22

// CHECK:      vaaddph zmmword ptr [2*rbp - 2048], zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x48,0x94,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vaaddph zmmword ptr [2*rbp - 2048], zmm22

// CHECK:      vaaddph zmmword ptr [rcx + 8128] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x4f,0x94,0x71,0x7f]
               vaaddph zmmword ptr [rcx + 8128] {k7}, zmm22

// CHECK:      vaaddph zmmword ptr [rdx - 8192] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x4f,0x94,0x72,0x80]
               vaaddph zmmword ptr [rdx - 8192] {k7}, zmm22

// CHECK:      vaaddps zmmword ptr [rbp + 8*r14 + 268435456], zmm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x48,0x84,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddps zmmword ptr [rbp + 8*r14 + 268435456], zmm22

// CHECK:      vaaddps zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22
// CHECK: encoding: [0x62,0xc2,0x7c,0x4f,0x84,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddps zmmword ptr [r8 + 4*rax + 291] {k7}, zmm22

// CHECK:      vaaddps zmmword ptr [rip], zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x48,0x84,0x35,0x00,0x00,0x00,0x00]
               vaaddps zmmword ptr [rip], zmm22

// CHECK:      vaaddps zmmword ptr [2*rbp - 2048], zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x48,0x84,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vaaddps zmmword ptr [2*rbp - 2048], zmm22

// CHECK:      vaaddps zmmword ptr [rcx + 8128] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x4f,0x84,0x71,0x7f]
               vaaddps zmmword ptr [rcx + 8128] {k7}, zmm22

// CHECK:      vaaddps zmmword ptr [rdx - 8192] {k7}, zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x4f,0x84,0x72,0x80]
               vaaddps zmmword ptr [rdx - 8192] {k7}, zmm22

// CHECK:      vaaddsbf16 word ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x08,0x94,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddsbf16 word ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vaaddsbf16 word ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0x7f,0x0f,0x94,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddsbf16 word ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vaaddsbf16 word ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x08,0x94,0x35,0x00,0x00,0x00,0x00]
               vaaddsbf16 word ptr [rip], xmm22

// CHECK:      vaaddsbf16 word ptr [2*rbp - 64], xmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x08,0x94,0x34,0x6d,0xc0,0xff,0xff,0xff]
               vaaddsbf16 word ptr [2*rbp - 64], xmm22

// CHECK:      vaaddsbf16 word ptr [rcx + 254] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x0f,0x94,0x71,0x7f]
               vaaddsbf16 word ptr [rcx + 254] {k7}, xmm22

// CHECK:      vaaddsbf16 word ptr [rdx - 256] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x0f,0x94,0x72,0x80]
               vaaddsbf16 word ptr [rdx - 256] {k7}, xmm22

// CHECK:      vaaddsd qword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0xff,0x08,0x84,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddsd qword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vaaddsd qword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0xff,0x0f,0x84,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddsd qword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vaaddsd qword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0xff,0x08,0x84,0x35,0x00,0x00,0x00,0x00]
               vaaddsd qword ptr [rip], xmm22

// CHECK:      vaaddsd qword ptr [2*rbp - 256], xmm22
// CHECK: encoding: [0x62,0xe2,0xff,0x08,0x84,0x34,0x6d,0x00,0xff,0xff,0xff]
               vaaddsd qword ptr [2*rbp - 256], xmm22

// CHECK:      vaaddsd qword ptr [rcx + 1016] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0xff,0x0f,0x84,0x71,0x7f]
               vaaddsd qword ptr [rcx + 1016] {k7}, xmm22

// CHECK:      vaaddsd qword ptr [rdx - 1024] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0xff,0x0f,0x84,0x72,0x80]
               vaaddsd qword ptr [rdx - 1024] {k7}, xmm22

// CHECK:      vaaddsh word ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0x94,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddsh word ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vaaddsh word ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0x7e,0x0f,0x94,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddsh word ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vaaddsh word ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0x94,0x35,0x00,0x00,0x00,0x00]
               vaaddsh word ptr [rip], xmm22

// CHECK:      vaaddsh word ptr [2*rbp - 64], xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0x94,0x34,0x6d,0xc0,0xff,0xff,0xff]
               vaaddsh word ptr [2*rbp - 64], xmm22

// CHECK:      vaaddsh word ptr [rcx + 254] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x0f,0x94,0x71,0x7f]
               vaaddsh word ptr [rcx + 254] {k7}, xmm22

// CHECK:      vaaddsh word ptr [rdx - 256] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x0f,0x94,0x72,0x80]
               vaaddsh word ptr [rdx - 256] {k7}, xmm22

// CHECK:      vaaddss dword ptr [rbp + 8*r14 + 268435456], xmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0x84,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vaaddss dword ptr [rbp + 8*r14 + 268435456], xmm22

// CHECK:      vaaddss dword ptr [r8 + 4*rax + 291] {k7}, xmm22
// CHECK: encoding: [0x62,0xc2,0x7e,0x0f,0x84,0xb4,0x80,0x23,0x01,0x00,0x00]
               vaaddss dword ptr [r8 + 4*rax + 291] {k7}, xmm22

// CHECK:      vaaddss dword ptr [rip], xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0x84,0x35,0x00,0x00,0x00,0x00]
               vaaddss dword ptr [rip], xmm22

// CHECK:      vaaddss dword ptr [2*rbp - 128], xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0x84,0x34,0x6d,0x80,0xff,0xff,0xff]
               vaaddss dword ptr [2*rbp - 128], xmm22

// CHECK:      vaaddss dword ptr [rcx + 508] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x0f,0x84,0x71,0x7f]
               vaaddss dword ptr [rcx + 508] {k7}, xmm22

// CHECK:      vaaddss dword ptr [rdx - 512] {k7}, xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x0f,0x84,0x72,0x80]
               vaaddss dword ptr [rdx - 512] {k7}, xmm22

