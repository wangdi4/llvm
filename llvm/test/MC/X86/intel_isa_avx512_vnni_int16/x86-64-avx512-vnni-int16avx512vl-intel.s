// REQUIRES: intel_feature_isa_avx512_vnni_int16
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vpdpwsud ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x46,0x20,0xd2,0xf0]
               vpdpwsud ymm22, ymm23, ymm24

// CHECK:      vpdpwsud ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x46,0x27,0xd2,0xf0]
               vpdpwsud ymm22 {k7}, ymm23, ymm24

// CHECK:      vpdpwsud ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x46,0xa7,0xd2,0xf0]
               vpdpwsud ymm22 {k7} {z}, ymm23, ymm24

// CHECK:      vpdpwsud xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x46,0x00,0xd2,0xf0]
               vpdpwsud xmm22, xmm23, xmm24

// CHECK:      vpdpwsud xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x46,0x07,0xd2,0xf0]
               vpdpwsud xmm22 {k7}, xmm23, xmm24

// CHECK:      vpdpwsud xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x46,0x87,0xd2,0xf0]
               vpdpwsud xmm22 {k7} {z}, xmm23, xmm24

// CHECK:      vpdpwsud ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x46,0x20,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsud ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwsud ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x46,0x27,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwsud ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwsud ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x46,0x30,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwsud ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK:      vpdpwsud ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x46,0x20,0xd2,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwsud ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK:      vpdpwsud ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x46,0xa7,0xd2,0x71,0x7f]
               vpdpwsud ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK:      vpdpwsud ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x46,0xb7,0xd2,0x72,0x80]
               vpdpwsud ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK:      vpdpwsud xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x46,0x00,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsud xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwsud xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x46,0x07,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwsud xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwsud xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x46,0x10,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwsud xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK:      vpdpwsud xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x46,0x00,0xd2,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwsud xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK:      vpdpwsud xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x46,0x87,0xd2,0x71,0x7f]
               vpdpwsud xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK:      vpdpwsud xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x46,0x97,0xd2,0x72,0x80]
               vpdpwsud xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK:      vpdpwsuds ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x46,0x20,0xd3,0xf0]
               vpdpwsuds ymm22, ymm23, ymm24

// CHECK:      vpdpwsuds ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x46,0x27,0xd3,0xf0]
               vpdpwsuds ymm22 {k7}, ymm23, ymm24

// CHECK:      vpdpwsuds ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x46,0xa7,0xd3,0xf0]
               vpdpwsuds ymm22 {k7} {z}, ymm23, ymm24

// CHECK:      vpdpwsuds xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x46,0x00,0xd3,0xf0]
               vpdpwsuds xmm22, xmm23, xmm24

// CHECK:      vpdpwsuds xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x46,0x07,0xd3,0xf0]
               vpdpwsuds xmm22 {k7}, xmm23, xmm24

// CHECK:      vpdpwsuds xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x46,0x87,0xd3,0xf0]
               vpdpwsuds xmm22 {k7} {z}, xmm23, xmm24

// CHECK:      vpdpwsuds ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x46,0x20,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsuds ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwsuds ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x46,0x27,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwsuds ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwsuds ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x46,0x30,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwsuds ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK:      vpdpwsuds ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x46,0x20,0xd3,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwsuds ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK:      vpdpwsuds ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x46,0xa7,0xd3,0x71,0x7f]
               vpdpwsuds ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK:      vpdpwsuds ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x46,0xb7,0xd3,0x72,0x80]
               vpdpwsuds ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK:      vpdpwsuds xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x46,0x00,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsuds xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwsuds xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x46,0x07,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwsuds xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwsuds xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x46,0x10,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwsuds xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK:      vpdpwsuds xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x46,0x00,0xd3,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwsuds xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK:      vpdpwsuds xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x46,0x87,0xd3,0x71,0x7f]
               vpdpwsuds xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK:      vpdpwsuds xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x46,0x97,0xd3,0x72,0x80]
               vpdpwsuds xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK:      vpdpwusd ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x45,0x20,0xd2,0xf0]
               vpdpwusd ymm22, ymm23, ymm24

// CHECK:      vpdpwusd ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x45,0x27,0xd2,0xf0]
               vpdpwusd ymm22 {k7}, ymm23, ymm24

// CHECK:      vpdpwusd ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x45,0xa7,0xd2,0xf0]
               vpdpwusd ymm22 {k7} {z}, ymm23, ymm24

// CHECK:      vpdpwusd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x45,0x00,0xd2,0xf0]
               vpdpwusd xmm22, xmm23, xmm24

// CHECK:      vpdpwusd xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x45,0x07,0xd2,0xf0]
               vpdpwusd xmm22 {k7}, xmm23, xmm24

// CHECK:      vpdpwusd xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x45,0x87,0xd2,0xf0]
               vpdpwusd xmm22 {k7} {z}, xmm23, xmm24

// CHECK:      vpdpwusd ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x20,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusd ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwusd ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x45,0x27,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwusd ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwusd ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x45,0x30,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwusd ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK:      vpdpwusd ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x45,0x20,0xd2,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwusd ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK:      vpdpwusd ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x45,0xa7,0xd2,0x71,0x7f]
               vpdpwusd ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK:      vpdpwusd ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x45,0xb7,0xd2,0x72,0x80]
               vpdpwusd ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK:      vpdpwusd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x00,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwusd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x45,0x07,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwusd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwusd xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x45,0x10,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwusd xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK:      vpdpwusd xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x45,0x00,0xd2,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwusd xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK:      vpdpwusd xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x45,0x87,0xd2,0x71,0x7f]
               vpdpwusd xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK:      vpdpwusd xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x45,0x97,0xd2,0x72,0x80]
               vpdpwusd xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK:      vpdpwusds ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x45,0x20,0xd3,0xf0]
               vpdpwusds ymm22, ymm23, ymm24

// CHECK:      vpdpwusds ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x45,0x27,0xd3,0xf0]
               vpdpwusds ymm22 {k7}, ymm23, ymm24

// CHECK:      vpdpwusds ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x45,0xa7,0xd3,0xf0]
               vpdpwusds ymm22 {k7} {z}, ymm23, ymm24

// CHECK:      vpdpwusds xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x45,0x00,0xd3,0xf0]
               vpdpwusds xmm22, xmm23, xmm24

// CHECK:      vpdpwusds xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x45,0x07,0xd3,0xf0]
               vpdpwusds xmm22 {k7}, xmm23, xmm24

// CHECK:      vpdpwusds xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x45,0x87,0xd3,0xf0]
               vpdpwusds xmm22 {k7} {z}, xmm23, xmm24

// CHECK:      vpdpwusds ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x20,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusds ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwusds ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x45,0x27,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwusds ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwusds ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x45,0x30,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwusds ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK:      vpdpwusds ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x45,0x20,0xd3,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwusds ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK:      vpdpwusds ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x45,0xa7,0xd3,0x71,0x7f]
               vpdpwusds ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK:      vpdpwusds ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x45,0xb7,0xd3,0x72,0x80]
               vpdpwusds ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK:      vpdpwusds xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x00,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusds xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwusds xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x45,0x07,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwusds xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwusds xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x45,0x10,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwusds xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK:      vpdpwusds xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x45,0x00,0xd3,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwusds xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK:      vpdpwusds xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x45,0x87,0xd3,0x71,0x7f]
               vpdpwusds xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK:      vpdpwusds xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x45,0x97,0xd3,0x72,0x80]
               vpdpwusds xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK:      vpdpwuud ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0x20,0xd2,0xf0]
               vpdpwuud ymm22, ymm23, ymm24

// CHECK:      vpdpwuud ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0x27,0xd2,0xf0]
               vpdpwuud ymm22 {k7}, ymm23, ymm24

// CHECK:      vpdpwuud ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0xa7,0xd2,0xf0]
               vpdpwuud ymm22 {k7} {z}, ymm23, ymm24

// CHECK:      vpdpwuud xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x00,0xd2,0xf0]
               vpdpwuud xmm22, xmm23, xmm24

// CHECK:      vpdpwuud xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x07,0xd2,0xf0]
               vpdpwuud xmm22 {k7}, xmm23, xmm24

// CHECK:      vpdpwuud xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x87,0xd2,0xf0]
               vpdpwuud xmm22 {k7} {z}, xmm23, xmm24

// CHECK:      vpdpwuud ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x20,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuud ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwuud ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x27,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwuud ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwuud ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x44,0x30,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwuud ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK:      vpdpwuud ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x44,0x20,0xd2,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwuud ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK:      vpdpwuud ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x44,0xa7,0xd2,0x71,0x7f]
               vpdpwuud ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK:      vpdpwuud ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x44,0xb7,0xd2,0x72,0x80]
               vpdpwuud ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK:      vpdpwuud xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x00,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuud xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwuud xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x07,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwuud xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwuud xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x44,0x10,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwuud xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK:      vpdpwuud xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x44,0x00,0xd2,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwuud xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK:      vpdpwuud xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x44,0x87,0xd2,0x71,0x7f]
               vpdpwuud xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK:      vpdpwuud xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x44,0x97,0xd2,0x72,0x80]
               vpdpwuud xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK:      vpdpwuuds ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0x20,0xd3,0xf0]
               vpdpwuuds ymm22, ymm23, ymm24

// CHECK:      vpdpwuuds ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0x27,0xd3,0xf0]
               vpdpwuuds ymm22 {k7}, ymm23, ymm24

// CHECK:      vpdpwuuds ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0xa7,0xd3,0xf0]
               vpdpwuuds ymm22 {k7} {z}, ymm23, ymm24

// CHECK:      vpdpwuuds xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x00,0xd3,0xf0]
               vpdpwuuds xmm22, xmm23, xmm24

// CHECK:      vpdpwuuds xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x07,0xd3,0xf0]
               vpdpwuuds xmm22 {k7}, xmm23, xmm24

// CHECK:      vpdpwuuds xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x87,0xd3,0xf0]
               vpdpwuuds xmm22 {k7} {z}, xmm23, xmm24

// CHECK:      vpdpwuuds ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x20,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuuds ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwuuds ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x27,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwuuds ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwuuds ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x44,0x30,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwuuds ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK:      vpdpwuuds ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x44,0x20,0xd3,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vpdpwuuds ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK:      vpdpwuuds ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x44,0xa7,0xd3,0x71,0x7f]
               vpdpwuuds ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK:      vpdpwuuds ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x44,0xb7,0xd3,0x72,0x80]
               vpdpwuuds ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK:      vpdpwuuds xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x00,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuuds xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwuuds xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x07,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwuuds xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwuuds xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x44,0x10,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwuuds xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK:      vpdpwuuds xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x44,0x00,0xd3,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vpdpwuuds xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK:      vpdpwuuds xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x44,0x87,0xd3,0x71,0x7f]
               vpdpwuuds xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK:      vpdpwuuds xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x44,0x97,0xd3,0x72,0x80]
               vpdpwuuds xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

