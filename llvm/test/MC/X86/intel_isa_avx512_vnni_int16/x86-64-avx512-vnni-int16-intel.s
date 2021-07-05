// REQUIRES: intel_feature_isa_avx512_vnni_int16
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vpdpwsud zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x46,0x40,0xd2,0xf0]
               vpdpwsud zmm22, zmm23, zmm24

// CHECK:      vpdpwsud zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x46,0x47,0xd2,0xf0]
               vpdpwsud zmm22 {k7}, zmm23, zmm24

// CHECK:      vpdpwsud zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x46,0xc7,0xd2,0xf0]
               vpdpwsud zmm22 {k7} {z}, zmm23, zmm24

// CHECK:      vpdpwsud zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x46,0x40,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsud zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwsud zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x46,0x47,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwsud zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwsud zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x46,0x50,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwsud zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vpdpwsud zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x46,0x40,0xd2,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwsud zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vpdpwsud zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x46,0xc7,0xd2,0x71,0x7f]
               vpdpwsud zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vpdpwsud zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x46,0xd7,0xd2,0x72,0x80]
               vpdpwsud zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

// CHECK:      vpdpwsuds zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x46,0x40,0xd3,0xf0]
               vpdpwsuds zmm22, zmm23, zmm24

// CHECK:      vpdpwsuds zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x46,0x47,0xd3,0xf0]
               vpdpwsuds zmm22 {k7}, zmm23, zmm24

// CHECK:      vpdpwsuds zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x46,0xc7,0xd3,0xf0]
               vpdpwsuds zmm22 {k7} {z}, zmm23, zmm24

// CHECK:      vpdpwsuds zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x46,0x40,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwsuds zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwsuds zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x46,0x47,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwsuds zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwsuds zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x46,0x50,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwsuds zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vpdpwsuds zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x46,0x40,0xd3,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwsuds zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vpdpwsuds zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x46,0xc7,0xd3,0x71,0x7f]
               vpdpwsuds zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vpdpwsuds zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x46,0xd7,0xd3,0x72,0x80]
               vpdpwsuds zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

// CHECK:      vpdpwusd zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x45,0x40,0xd2,0xf0]
               vpdpwusd zmm22, zmm23, zmm24

// CHECK:      vpdpwusd zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x45,0x47,0xd2,0xf0]
               vpdpwusd zmm22 {k7}, zmm23, zmm24

// CHECK:      vpdpwusd zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x45,0xc7,0xd2,0xf0]
               vpdpwusd zmm22 {k7} {z}, zmm23, zmm24

// CHECK:      vpdpwusd zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x40,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusd zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwusd zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x45,0x47,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwusd zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwusd zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x45,0x50,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwusd zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vpdpwusd zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x45,0x40,0xd2,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwusd zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vpdpwusd zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x45,0xc7,0xd2,0x71,0x7f]
               vpdpwusd zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vpdpwusd zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x45,0xd7,0xd2,0x72,0x80]
               vpdpwusd zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

// CHECK:      vpdpwusds zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x45,0x40,0xd3,0xf0]
               vpdpwusds zmm22, zmm23, zmm24

// CHECK:      vpdpwusds zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x45,0x47,0xd3,0xf0]
               vpdpwusds zmm22 {k7}, zmm23, zmm24

// CHECK:      vpdpwusds zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x45,0xc7,0xd3,0xf0]
               vpdpwusds zmm22 {k7} {z}, zmm23, zmm24

// CHECK:      vpdpwusds zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x40,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwusds zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwusds zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x45,0x47,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwusds zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwusds zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x45,0x50,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwusds zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vpdpwusds zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x45,0x40,0xd3,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwusds zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vpdpwusds zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x45,0xc7,0xd3,0x71,0x7f]
               vpdpwusds zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vpdpwusds zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x45,0xd7,0xd3,0x72,0x80]
               vpdpwusds zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

// CHECK:      vpdpwuud zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0x40,0xd2,0xf0]
               vpdpwuud zmm22, zmm23, zmm24

// CHECK:      vpdpwuud zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0x47,0xd2,0xf0]
               vpdpwuud zmm22 {k7}, zmm23, zmm24

// CHECK:      vpdpwuud zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0xc7,0xd2,0xf0]
               vpdpwuud zmm22 {k7} {z}, zmm23, zmm24

// CHECK:      vpdpwuud zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x40,0xd2,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuud zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwuud zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x47,0xd2,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwuud zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwuud zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x44,0x50,0xd2,0x35,0x00,0x00,0x00,0x00]
               vpdpwuud zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vpdpwuud zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x44,0x40,0xd2,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwuud zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vpdpwuud zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x44,0xc7,0xd2,0x71,0x7f]
               vpdpwuud zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vpdpwuud zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x44,0xd7,0xd2,0x72,0x80]
               vpdpwuud zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

// CHECK:      vpdpwuuds zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0x40,0xd3,0xf0]
               vpdpwuuds zmm22, zmm23, zmm24

// CHECK:      vpdpwuuds zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0x47,0xd3,0xf0]
               vpdpwuuds zmm22 {k7}, zmm23, zmm24

// CHECK:      vpdpwuuds zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0xc7,0xd3,0xf0]
               vpdpwuuds zmm22 {k7} {z}, zmm23, zmm24

// CHECK:      vpdpwuuds zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x40,0xd3,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpwuuds zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpwuuds zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x47,0xd3,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpwuuds zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpwuuds zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x44,0x50,0xd3,0x35,0x00,0x00,0x00,0x00]
               vpdpwuuds zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vpdpwuuds zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x44,0x40,0xd3,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwuuds zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vpdpwuuds zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x44,0xc7,0xd3,0x71,0x7f]
               vpdpwuuds zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vpdpwuuds zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x44,0xd7,0xd3,0x72,0x80]
               vpdpwuuds zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

