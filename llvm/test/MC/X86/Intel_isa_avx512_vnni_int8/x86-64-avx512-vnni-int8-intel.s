// REQUIRES: intel_feature_isa_avx512_vnni_int8
// RUN: llvm-mc -triple x86_64-unknown-unknown -mattr=+avx512vnniint8 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vpdpbssd zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x47,0x40,0x50,0xf0]
               vpdpbssd zmm22, zmm23, zmm24

// CHECK:      vpdpbssd zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x47,0x47,0x50,0xf0]
               vpdpbssd zmm22 {k7}, zmm23, zmm24

// CHECK:      vpdpbssd zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x47,0xc7,0x50,0xf0]
               vpdpbssd zmm22 {k7} {z}, zmm23, zmm24

// CHECK:      vpdpbssd zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x47,0x40,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpbssd zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpbssd zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x47,0x47,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpbssd zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpbssd zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x47,0x50,0x50,0x35,0x00,0x00,0x00,0x00]
               vpdpbssd zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vpdpbssd zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x47,0x40,0x50,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbssd zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vpdpbssd zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x47,0xc7,0x50,0x71,0x7f]
               vpdpbssd zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vpdpbssd zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x47,0xd7,0x50,0x72,0x80]
               vpdpbssd zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

// CHECK:      vpdpbssds zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x47,0x40,0x51,0xf0]
               vpdpbssds zmm22, zmm23, zmm24

// CHECK:      vpdpbssds zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x47,0x47,0x51,0xf0]
               vpdpbssds zmm22 {k7}, zmm23, zmm24

// CHECK:      vpdpbssds zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x47,0xc7,0x51,0xf0]
               vpdpbssds zmm22 {k7} {z}, zmm23, zmm24

// CHECK:      vpdpbssds zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x47,0x40,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpbssds zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpbssds zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x47,0x47,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpbssds zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpbssds zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x47,0x50,0x51,0x35,0x00,0x00,0x00,0x00]
               vpdpbssds zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vpdpbssds zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x47,0x40,0x51,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbssds zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vpdpbssds zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x47,0xc7,0x51,0x71,0x7f]
               vpdpbssds zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vpdpbssds zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x47,0xd7,0x51,0x72,0x80]
               vpdpbssds zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

// CHECK:      vpdpbsud zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x46,0x40,0x50,0xf0]
               vpdpbsud zmm22, zmm23, zmm24

// CHECK:      vpdpbsud zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x46,0x47,0x50,0xf0]
               vpdpbsud zmm22 {k7}, zmm23, zmm24

// CHECK:      vpdpbsud zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x46,0xc7,0x50,0xf0]
               vpdpbsud zmm22 {k7} {z}, zmm23, zmm24

// CHECK:      vpdpbsud zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x46,0x40,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpbsud zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpbsud zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x46,0x47,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpbsud zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpbsud zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x46,0x50,0x50,0x35,0x00,0x00,0x00,0x00]
               vpdpbsud zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vpdpbsud zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x46,0x40,0x50,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbsud zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vpdpbsud zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x46,0xc7,0x50,0x71,0x7f]
               vpdpbsud zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vpdpbsud zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x46,0xd7,0x50,0x72,0x80]
               vpdpbsud zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

// CHECK:      vpdpbsuds zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x46,0x40,0x51,0xf0]
               vpdpbsuds zmm22, zmm23, zmm24

// CHECK:      vpdpbsuds zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x46,0x47,0x51,0xf0]
               vpdpbsuds zmm22 {k7}, zmm23, zmm24

// CHECK:      vpdpbsuds zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x46,0xc7,0x51,0xf0]
               vpdpbsuds zmm22 {k7} {z}, zmm23, zmm24

// CHECK:      vpdpbsuds zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x46,0x40,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpbsuds zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpbsuds zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x46,0x47,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpbsuds zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpbsuds zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x46,0x50,0x51,0x35,0x00,0x00,0x00,0x00]
               vpdpbsuds zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vpdpbsuds zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x46,0x40,0x51,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbsuds zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vpdpbsuds zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x46,0xc7,0x51,0x71,0x7f]
               vpdpbsuds zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vpdpbsuds zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x46,0xd7,0x51,0x72,0x80]
               vpdpbsuds zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

// CHECK:      vpdpbuud zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0x40,0x50,0xf0]
               vpdpbuud zmm22, zmm23, zmm24

// CHECK:      vpdpbuud zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0x47,0x50,0xf0]
               vpdpbuud zmm22 {k7}, zmm23, zmm24

// CHECK:      vpdpbuud zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0xc7,0x50,0xf0]
               vpdpbuud zmm22 {k7} {z}, zmm23, zmm24

// CHECK:      vpdpbuud zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x40,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpbuud zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpbuud zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x47,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpbuud zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpbuud zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x44,0x50,0x50,0x35,0x00,0x00,0x00,0x00]
               vpdpbuud zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vpdpbuud zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x44,0x40,0x50,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbuud zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vpdpbuud zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x44,0xc7,0x50,0x71,0x7f]
               vpdpbuud zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vpdpbuud zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x44,0xd7,0x50,0x72,0x80]
               vpdpbuud zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

// CHECK:      vpdpbuuds zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0x40,0x51,0xf0]
               vpdpbuuds zmm22, zmm23, zmm24

// CHECK:      vpdpbuuds zmm22 {k7}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0x47,0x51,0xf0]
               vpdpbuuds zmm22 {k7}, zmm23, zmm24

// CHECK:      vpdpbuuds zmm22 {k7} {z}, zmm23, zmm24
// CHECK: encoding: [0x62,0x82,0x44,0xc7,0x51,0xf0]
               vpdpbuuds zmm22 {k7} {z}, zmm23, zmm24

// CHECK:      vpdpbuuds zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x40,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vpdpbuuds zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpdpbuuds zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x47,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
               vpdpbuuds zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpdpbuuds zmm22, zmm23, dword ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x44,0x50,0x51,0x35,0x00,0x00,0x00,0x00]
               vpdpbuuds zmm22, zmm23, dword ptr [rip]{1to16}

// CHECK:      vpdpbuuds zmm22, zmm23, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x44,0x40,0x51,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vpdpbuuds zmm22, zmm23, zmmword ptr [2*rbp - 2048]

// CHECK:      vpdpbuuds zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x44,0xc7,0x51,0x71,0x7f]
               vpdpbuuds zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128]

// CHECK:      vpdpbuuds zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}
// CHECK: encoding: [0x62,0xe2,0x44,0xd7,0x51,0x72,0x80]
               vpdpbuuds zmm22 {k7} {z}, zmm23, dword ptr [rdx - 512]{1to16}

