// REQUIRES: intel_feature_isa_avx_vnni
// RUN: llvm-mc -triple x86_64-unknown-unknown -mattr=+avxvnni -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vpdpbusd ymm6, ymm5, ymm4
// CHECK: encoding: [0xc4,0xe2,0x55,0x50,0xf4]
          vpdpbusd ymm6, ymm5, ymm4

// CHECK: vpdpbusd xmm6, xmm5, xmm4
// CHECK: encoding: [0xc4,0xe2,0x51,0x50,0xf4]
          vpdpbusd xmm6, xmm5, xmm4

// CHECK: vpdpbusd ymm6, ymm5, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x55,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vpdpbusd ymm6, ymm5, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbusd ymm6, ymm5, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x55,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vpdpbusd ymm6, ymm5, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbusd ymm6, ymm5, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x55,0x50,0x35,0x00,0x00,0x00,0x00]
          vpdpbusd ymm6, ymm5, ymmword ptr [rip]

// CHECK: vpdpbusd ymm6, ymm5, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x55,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vpdpbusd ymm6, ymm5, ymmword ptr [2*rbp - 1024]

// CHECK: vpdpbusd ymm6, ymm5, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x55,0x50,0xb1,0xe0,0x0f,0x00,0x00]
          vpdpbusd ymm6, ymm5, ymmword ptr [rcx + 4064]

// CHECK: vpdpbusd ymm6, ymm5, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x55,0x50,0xb2,0x00,0xf0,0xff,0xff]
          vpdpbusd ymm6, ymm5, ymmword ptr [rdx - 4096]

// CHECK: vpdpbusd xmm6, xmm5, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x51,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vpdpbusd xmm6, xmm5, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbusd xmm6, xmm5, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x51,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vpdpbusd xmm6, xmm5, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbusd xmm6, xmm5, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x51,0x50,0x35,0x00,0x00,0x00,0x00]
          vpdpbusd xmm6, xmm5, xmmword ptr [rip]

// CHECK: vpdpbusd xmm6, xmm5, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x51,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vpdpbusd xmm6, xmm5, xmmword ptr [2*rbp - 512]

// CHECK: vpdpbusd xmm6, xmm5, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x51,0x50,0xb1,0xf0,0x07,0x00,0x00]
          vpdpbusd xmm6, xmm5, xmmword ptr [rcx + 2032]

// CHECK: vpdpbusd xmm6, xmm5, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x51,0x50,0xb2,0x00,0xf8,0xff,0xff]
          vpdpbusd xmm6, xmm5, xmmword ptr [rdx - 2048]

// CHECK: vpdpbusds ymm6, ymm5, ymm4
// CHECK: encoding: [0xc4,0xe2,0x55,0x51,0xf4]
          vpdpbusds ymm6, ymm5, ymm4

// CHECK: vpdpbusds xmm6, xmm5, xmm4
// CHECK: encoding: [0xc4,0xe2,0x51,0x51,0xf4]
          vpdpbusds xmm6, xmm5, xmm4

// CHECK: vpdpbusds ymm6, ymm5, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x55,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vpdpbusds ymm6, ymm5, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbusds ymm6, ymm5, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x55,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
          vpdpbusds ymm6, ymm5, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbusds ymm6, ymm5, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x55,0x51,0x35,0x00,0x00,0x00,0x00]
          vpdpbusds ymm6, ymm5, ymmword ptr [rip]

// CHECK: vpdpbusds ymm6, ymm5, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x55,0x51,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vpdpbusds ymm6, ymm5, ymmword ptr [2*rbp - 1024]

// CHECK: vpdpbusds ymm6, ymm5, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x55,0x51,0xb1,0xe0,0x0f,0x00,0x00]
          vpdpbusds ymm6, ymm5, ymmword ptr [rcx + 4064]

// CHECK: vpdpbusds ymm6, ymm5, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x55,0x51,0xb2,0x00,0xf0,0xff,0xff]
          vpdpbusds ymm6, ymm5, ymmword ptr [rdx - 4096]

// CHECK: vpdpbusds xmm6, xmm5, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x51,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vpdpbusds xmm6, xmm5, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbusds xmm6, xmm5, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x51,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
          vpdpbusds xmm6, xmm5, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbusds xmm6, xmm5, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x51,0x51,0x35,0x00,0x00,0x00,0x00]
          vpdpbusds xmm6, xmm5, xmmword ptr [rip]

// CHECK: vpdpbusds xmm6, xmm5, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x51,0x51,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vpdpbusds xmm6, xmm5, xmmword ptr [2*rbp - 512]

// CHECK: vpdpbusds xmm6, xmm5, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x51,0x51,0xb1,0xf0,0x07,0x00,0x00]
          vpdpbusds xmm6, xmm5, xmmword ptr [rcx + 2032]

// CHECK: vpdpbusds xmm6, xmm5, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x51,0x51,0xb2,0x00,0xf8,0xff,0xff]
          vpdpbusds xmm6, xmm5, xmmword ptr [rdx - 2048]

// CHECK: vpdpwssd ymm6, ymm5, ymm4
// CHECK: encoding: [0xc4,0xe2,0x55,0x52,0xf4]
          vpdpwssd ymm6, ymm5, ymm4

// CHECK: vpdpwssd xmm6, xmm5, xmm4
// CHECK: encoding: [0xc4,0xe2,0x51,0x52,0xf4]
          vpdpwssd xmm6, xmm5, xmm4

// CHECK: vpdpwssd ymm6, ymm5, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x55,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vpdpwssd ymm6, ymm5, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpwssd ymm6, ymm5, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x55,0x52,0xb4,0x80,0x23,0x01,0x00,0x00]
          vpdpwssd ymm6, ymm5, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpwssd ymm6, ymm5, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x55,0x52,0x35,0x00,0x00,0x00,0x00]
          vpdpwssd ymm6, ymm5, ymmword ptr [rip]

// CHECK: vpdpwssd ymm6, ymm5, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x55,0x52,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vpdpwssd ymm6, ymm5, ymmword ptr [2*rbp - 1024]

// CHECK: vpdpwssd ymm6, ymm5, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x55,0x52,0xb1,0xe0,0x0f,0x00,0x00]
          vpdpwssd ymm6, ymm5, ymmword ptr [rcx + 4064]

// CHECK: vpdpwssd ymm6, ymm5, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x55,0x52,0xb2,0x00,0xf0,0xff,0xff]
          vpdpwssd ymm6, ymm5, ymmword ptr [rdx - 4096]

// CHECK: vpdpwssd xmm6, xmm5, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x51,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vpdpwssd xmm6, xmm5, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpwssd xmm6, xmm5, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x51,0x52,0xb4,0x80,0x23,0x01,0x00,0x00]
          vpdpwssd xmm6, xmm5, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpwssd xmm6, xmm5, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x51,0x52,0x35,0x00,0x00,0x00,0x00]
          vpdpwssd xmm6, xmm5, xmmword ptr [rip]

// CHECK: vpdpwssd xmm6, xmm5, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x51,0x52,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vpdpwssd xmm6, xmm5, xmmword ptr [2*rbp - 512]

// CHECK: vpdpwssd xmm6, xmm5, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x51,0x52,0xb1,0xf0,0x07,0x00,0x00]
          vpdpwssd xmm6, xmm5, xmmword ptr [rcx + 2032]

// CHECK: vpdpwssd xmm6, xmm5, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x51,0x52,0xb2,0x00,0xf8,0xff,0xff]
          vpdpwssd xmm6, xmm5, xmmword ptr [rdx - 2048]

// CHECK: vpdpwssds ymm6, ymm5, ymm4
// CHECK: encoding: [0xc4,0xe2,0x55,0x53,0xf4]
          vpdpwssds ymm6, ymm5, ymm4

// CHECK: vpdpwssds xmm6, xmm5, xmm4
// CHECK: encoding: [0xc4,0xe2,0x51,0x53,0xf4]
          vpdpwssds xmm6, xmm5, xmm4

// CHECK: vpdpwssds ymm6, ymm5, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x55,0x53,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vpdpwssds ymm6, ymm5, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpwssds ymm6, ymm5, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x55,0x53,0xb4,0x80,0x23,0x01,0x00,0x00]
          vpdpwssds ymm6, ymm5, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpwssds ymm6, ymm5, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x55,0x53,0x35,0x00,0x00,0x00,0x00]
          vpdpwssds ymm6, ymm5, ymmword ptr [rip]

// CHECK: vpdpwssds ymm6, ymm5, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x55,0x53,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vpdpwssds ymm6, ymm5, ymmword ptr [2*rbp - 1024]

// CHECK: vpdpwssds ymm6, ymm5, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x55,0x53,0xb1,0xe0,0x0f,0x00,0x00]
          vpdpwssds ymm6, ymm5, ymmword ptr [rcx + 4064]

// CHECK: vpdpwssds ymm6, ymm5, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x55,0x53,0xb2,0x00,0xf0,0xff,0xff]
          vpdpwssds ymm6, ymm5, ymmword ptr [rdx - 4096]

// CHECK: vpdpwssds xmm6, xmm5, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x51,0x53,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vpdpwssds xmm6, xmm5, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpwssds xmm6, xmm5, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x51,0x53,0xb4,0x80,0x23,0x01,0x00,0x00]
          vpdpwssds xmm6, xmm5, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpwssds xmm6, xmm5, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x51,0x53,0x35,0x00,0x00,0x00,0x00]
          vpdpwssds xmm6, xmm5, xmmword ptr [rip]

// CHECK: vpdpwssds xmm6, xmm5, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x51,0x53,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vpdpwssds xmm6, xmm5, xmmword ptr [2*rbp - 512]

// CHECK: vpdpwssds xmm6, xmm5, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x51,0x53,0xb1,0xf0,0x07,0x00,0x00]
          vpdpwssds xmm6, xmm5, xmmword ptr [rcx + 2032]

// CHECK: vpdpwssds xmm6, xmm5, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x51,0x53,0xb2,0x00,0xf8,0xff,0xff]
          vpdpwssds xmm6, xmm5, xmmword ptr [rdx - 2048]

