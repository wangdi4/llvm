// REQUIRES: intel_feature_isa_avx_vnni_int16
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vpdpwsud ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x66,0xd2,0xd4]
               vpdpwsud ymm2, ymm3, ymm4

// CHECK:      vpdpwsud xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x62,0xd2,0xd4]
               vpdpwsud xmm2, xmm3, xmm4

// CHECK:      vpdpwsud ymm2, ymm3, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x66,0xd2,0x94,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsud ymm2, ymm3, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwsud ymm2, ymm3, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x66,0xd2,0x94,0x80,0x23,0x01,0x00,0x00]
               vpdpwsud ymm2, ymm3, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwsud ymm2, ymm3, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x66,0xd2,0x15,0x00,0x00,0x00,0x00]
               vpdpwsud ymm2, ymm3, ymmword ptr [rip]

// CHECK:      vpdpwsud ymm2, ymm3, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x66,0xd2,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwsud ymm2, ymm3, ymmword ptr [2*rbp - 1024]

// CHECK:      vpdpwsud ymm2, ymm3, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x66,0xd2,0x91,0xe0,0x0f,0x00,0x00]
               vpdpwsud ymm2, ymm3, ymmword ptr [rcx + 4064]

// CHECK:      vpdpwsud ymm2, ymm3, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x66,0xd2,0x92,0x00,0xf0,0xff,0xff]
               vpdpwsud ymm2, ymm3, ymmword ptr [rdx - 4096]

// CHECK:      vpdpwsud xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x62,0xd2,0x94,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsud xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwsud xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x62,0xd2,0x94,0x80,0x23,0x01,0x00,0x00]
               vpdpwsud xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwsud xmm2, xmm3, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x62,0xd2,0x15,0x00,0x00,0x00,0x00]
               vpdpwsud xmm2, xmm3, xmmword ptr [rip]

// CHECK:      vpdpwsud xmm2, xmm3, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x62,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwsud xmm2, xmm3, xmmword ptr [2*rbp - 512]

// CHECK:      vpdpwsud xmm2, xmm3, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x62,0xd2,0x91,0xf0,0x07,0x00,0x00]
               vpdpwsud xmm2, xmm3, xmmword ptr [rcx + 2032]

// CHECK:      vpdpwsud xmm2, xmm3, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x62,0xd2,0x92,0x00,0xf8,0xff,0xff]
               vpdpwsud xmm2, xmm3, xmmword ptr [rdx - 2048]

// CHECK:      vpdpwsuds ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x66,0xd3,0xd4]
               vpdpwsuds ymm2, ymm3, ymm4

// CHECK:      vpdpwsuds xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x62,0xd3,0xd4]
               vpdpwsuds xmm2, xmm3, xmm4

// CHECK:      vpdpwsuds ymm2, ymm3, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x66,0xd3,0x94,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsuds ymm2, ymm3, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwsuds ymm2, ymm3, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x66,0xd3,0x94,0x80,0x23,0x01,0x00,0x00]
               vpdpwsuds ymm2, ymm3, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwsuds ymm2, ymm3, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x66,0xd3,0x15,0x00,0x00,0x00,0x00]
               vpdpwsuds ymm2, ymm3, ymmword ptr [rip]

// CHECK:      vpdpwsuds ymm2, ymm3, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x66,0xd3,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwsuds ymm2, ymm3, ymmword ptr [2*rbp - 1024]

// CHECK:      vpdpwsuds ymm2, ymm3, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x66,0xd3,0x91,0xe0,0x0f,0x00,0x00]
               vpdpwsuds ymm2, ymm3, ymmword ptr [rcx + 4064]

// CHECK:      vpdpwsuds ymm2, ymm3, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x66,0xd3,0x92,0x00,0xf0,0xff,0xff]
               vpdpwsuds ymm2, ymm3, ymmword ptr [rdx - 4096]

// CHECK:      vpdpwsuds xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x62,0xd3,0x94,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsuds xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwsuds xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x62,0xd3,0x94,0x80,0x23,0x01,0x00,0x00]
               vpdpwsuds xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwsuds xmm2, xmm3, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x62,0xd3,0x15,0x00,0x00,0x00,0x00]
               vpdpwsuds xmm2, xmm3, xmmword ptr [rip]

// CHECK:      vpdpwsuds xmm2, xmm3, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x62,0xd3,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwsuds xmm2, xmm3, xmmword ptr [2*rbp - 512]

// CHECK:      vpdpwsuds xmm2, xmm3, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x62,0xd3,0x91,0xf0,0x07,0x00,0x00]
               vpdpwsuds xmm2, xmm3, xmmword ptr [rcx + 2032]

// CHECK:      vpdpwsuds xmm2, xmm3, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x62,0xd3,0x92,0x00,0xf8,0xff,0xff]
               vpdpwsuds xmm2, xmm3, xmmword ptr [rdx - 2048]

// CHECK:      vpdpwusd ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x65,0xd2,0xd4]
               vpdpwusd ymm2, ymm3, ymm4

// CHECK:      vpdpwusd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x61,0xd2,0xd4]
               vpdpwusd xmm2, xmm3, xmm4

// CHECK:      vpdpwusd ymm2, ymm3, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x65,0xd2,0x94,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusd ymm2, ymm3, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwusd ymm2, ymm3, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x65,0xd2,0x94,0x80,0x23,0x01,0x00,0x00]
               vpdpwusd ymm2, ymm3, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwusd ymm2, ymm3, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x65,0xd2,0x15,0x00,0x00,0x00,0x00]
               vpdpwusd ymm2, ymm3, ymmword ptr [rip]

// CHECK:      vpdpwusd ymm2, ymm3, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x65,0xd2,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwusd ymm2, ymm3, ymmword ptr [2*rbp - 1024]

// CHECK:      vpdpwusd ymm2, ymm3, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x65,0xd2,0x91,0xe0,0x0f,0x00,0x00]
               vpdpwusd ymm2, ymm3, ymmword ptr [rcx + 4064]

// CHECK:      vpdpwusd ymm2, ymm3, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x65,0xd2,0x92,0x00,0xf0,0xff,0xff]
               vpdpwusd ymm2, ymm3, ymmword ptr [rdx - 4096]

// CHECK:      vpdpwusd xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x61,0xd2,0x94,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusd xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwusd xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x61,0xd2,0x94,0x80,0x23,0x01,0x00,0x00]
               vpdpwusd xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwusd xmm2, xmm3, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x61,0xd2,0x15,0x00,0x00,0x00,0x00]
               vpdpwusd xmm2, xmm3, xmmword ptr [rip]

// CHECK:      vpdpwusd xmm2, xmm3, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x61,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwusd xmm2, xmm3, xmmword ptr [2*rbp - 512]

// CHECK:      vpdpwusd xmm2, xmm3, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x61,0xd2,0x91,0xf0,0x07,0x00,0x00]
               vpdpwusd xmm2, xmm3, xmmword ptr [rcx + 2032]

// CHECK:      vpdpwusd xmm2, xmm3, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x61,0xd2,0x92,0x00,0xf8,0xff,0xff]
               vpdpwusd xmm2, xmm3, xmmword ptr [rdx - 2048]

// CHECK:      vpdpwusds ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x65,0xd3,0xd4]
               vpdpwusds ymm2, ymm3, ymm4

// CHECK:      vpdpwusds xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x61,0xd3,0xd4]
               vpdpwusds xmm2, xmm3, xmm4

// CHECK:      vpdpwusds ymm2, ymm3, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x65,0xd3,0x94,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusds ymm2, ymm3, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwusds ymm2, ymm3, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x65,0xd3,0x94,0x80,0x23,0x01,0x00,0x00]
               vpdpwusds ymm2, ymm3, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwusds ymm2, ymm3, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x65,0xd3,0x15,0x00,0x00,0x00,0x00]
               vpdpwusds ymm2, ymm3, ymmword ptr [rip]

// CHECK:      vpdpwusds ymm2, ymm3, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x65,0xd3,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwusds ymm2, ymm3, ymmword ptr [2*rbp - 1024]

// CHECK:      vpdpwusds ymm2, ymm3, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x65,0xd3,0x91,0xe0,0x0f,0x00,0x00]
               vpdpwusds ymm2, ymm3, ymmword ptr [rcx + 4064]

// CHECK:      vpdpwusds ymm2, ymm3, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x65,0xd3,0x92,0x00,0xf0,0xff,0xff]
               vpdpwusds ymm2, ymm3, ymmword ptr [rdx - 4096]

// CHECK:      vpdpwusds xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x61,0xd3,0x94,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusds xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwusds xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x61,0xd3,0x94,0x80,0x23,0x01,0x00,0x00]
               vpdpwusds xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwusds xmm2, xmm3, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x61,0xd3,0x15,0x00,0x00,0x00,0x00]
               vpdpwusds xmm2, xmm3, xmmword ptr [rip]

// CHECK:      vpdpwusds xmm2, xmm3, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x61,0xd3,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwusds xmm2, xmm3, xmmword ptr [2*rbp - 512]

// CHECK:      vpdpwusds xmm2, xmm3, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x61,0xd3,0x91,0xf0,0x07,0x00,0x00]
               vpdpwusds xmm2, xmm3, xmmword ptr [rcx + 2032]

// CHECK:      vpdpwusds xmm2, xmm3, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x61,0xd3,0x92,0x00,0xf8,0xff,0xff]
               vpdpwusds xmm2, xmm3, xmmword ptr [rdx - 2048]

// CHECK:      vpdpwuud ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x64,0xd2,0xd4]
               vpdpwuud ymm2, ymm3, ymm4

// CHECK:      vpdpwuud xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x60,0xd2,0xd4]
               vpdpwuud xmm2, xmm3, xmm4

// CHECK:      vpdpwuud ymm2, ymm3, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x64,0xd2,0x94,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuud ymm2, ymm3, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwuud ymm2, ymm3, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x64,0xd2,0x94,0x80,0x23,0x01,0x00,0x00]
               vpdpwuud ymm2, ymm3, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwuud ymm2, ymm3, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x64,0xd2,0x15,0x00,0x00,0x00,0x00]
               vpdpwuud ymm2, ymm3, ymmword ptr [rip]

// CHECK:      vpdpwuud ymm2, ymm3, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x64,0xd2,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwuud ymm2, ymm3, ymmword ptr [2*rbp - 1024]

// CHECK:      vpdpwuud ymm2, ymm3, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x64,0xd2,0x91,0xe0,0x0f,0x00,0x00]
               vpdpwuud ymm2, ymm3, ymmword ptr [rcx + 4064]

// CHECK:      vpdpwuud ymm2, ymm3, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x64,0xd2,0x92,0x00,0xf0,0xff,0xff]
               vpdpwuud ymm2, ymm3, ymmword ptr [rdx - 4096]

// CHECK:      vpdpwuud xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x60,0xd2,0x94,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuud xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwuud xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x60,0xd2,0x94,0x80,0x23,0x01,0x00,0x00]
               vpdpwuud xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwuud xmm2, xmm3, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x60,0xd2,0x15,0x00,0x00,0x00,0x00]
               vpdpwuud xmm2, xmm3, xmmword ptr [rip]

// CHECK:      vpdpwuud xmm2, xmm3, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x60,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwuud xmm2, xmm3, xmmword ptr [2*rbp - 512]

// CHECK:      vpdpwuud xmm2, xmm3, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x60,0xd2,0x91,0xf0,0x07,0x00,0x00]
               vpdpwuud xmm2, xmm3, xmmword ptr [rcx + 2032]

// CHECK:      vpdpwuud xmm2, xmm3, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x60,0xd2,0x92,0x00,0xf8,0xff,0xff]
               vpdpwuud xmm2, xmm3, xmmword ptr [rdx - 2048]

// CHECK:      vpdpwuuds ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x64,0xd3,0xd4]
               vpdpwuuds ymm2, ymm3, ymm4

// CHECK:      vpdpwuuds xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x60,0xd3,0xd4]
               vpdpwuuds xmm2, xmm3, xmm4

// CHECK:      vpdpwuuds ymm2, ymm3, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x64,0xd3,0x94,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuuds ymm2, ymm3, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwuuds ymm2, ymm3, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x64,0xd3,0x94,0x80,0x23,0x01,0x00,0x00]
               vpdpwuuds ymm2, ymm3, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwuuds ymm2, ymm3, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x64,0xd3,0x15,0x00,0x00,0x00,0x00]
               vpdpwuuds ymm2, ymm3, ymmword ptr [rip]

// CHECK:      vpdpwuuds ymm2, ymm3, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x64,0xd3,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwuuds ymm2, ymm3, ymmword ptr [2*rbp - 1024]

// CHECK:      vpdpwuuds ymm2, ymm3, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x64,0xd3,0x91,0xe0,0x0f,0x00,0x00]
               vpdpwuuds ymm2, ymm3, ymmword ptr [rcx + 4064]

// CHECK:      vpdpwuuds ymm2, ymm3, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x64,0xd3,0x92,0x00,0xf0,0xff,0xff]
               vpdpwuuds ymm2, ymm3, ymmword ptr [rdx - 4096]

// CHECK:      vpdpwuuds xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x60,0xd3,0x94,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuuds xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwuuds xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x60,0xd3,0x94,0x80,0x23,0x01,0x00,0x00]
               vpdpwuuds xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwuuds xmm2, xmm3, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x60,0xd3,0x15,0x00,0x00,0x00,0x00]
               vpdpwuuds xmm2, xmm3, xmmword ptr [rip]

// CHECK:      vpdpwuuds xmm2, xmm3, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x60,0xd3,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwuuds xmm2, xmm3, xmmword ptr [2*rbp - 512]

// CHECK:      vpdpwuuds xmm2, xmm3, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x60,0xd3,0x91,0xf0,0x07,0x00,0x00]
               vpdpwuuds xmm2, xmm3, xmmword ptr [rcx + 2032]

// CHECK:      vpdpwuuds xmm2, xmm3, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x60,0xd3,0x92,0x00,0xf8,0xff,0xff]
               vpdpwuuds xmm2, xmm3, xmmword ptr [rdx - 2048]

