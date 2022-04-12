// REQUIRES: intel_feature_isa_avx512_vnni_int8
// RUN: llvm-mc -triple x86_64-unknown-unknown -mattr=+avx512vl,+avx512vnniint8 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vpdpbssd ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x47,0x20,0x50,0xf0]
     {evex} vpdpbssd ymm22, ymm23, ymm24

// CHECK: vpdpbssd ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x47,0x27,0x50,0xf0]
     {evex} vpdpbssd ymm22 {k7}, ymm23, ymm24

// CHECK: vpdpbssd ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x47,0xa7,0x50,0xf0]
     {evex} vpdpbssd ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vpdpbssd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x47,0x00,0x50,0xf0]
     {evex} vpdpbssd xmm22, xmm23, xmm24

// CHECK: vpdpbssd xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x47,0x07,0x50,0xf0]
     {evex} vpdpbssd xmm22 {k7}, xmm23, xmm24

// CHECK: vpdpbssd xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x47,0x87,0x50,0xf0]
     {evex} vpdpbssd xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vpdpbssd ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x47,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbssd ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbssd ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x47,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbssd ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbssd ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x47,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbssd ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vpdpbssd ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x47,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbssd ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vpdpbssd ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x47,0xa7,0x50,0x71,0x7f]
     {evex} vpdpbssd ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vpdpbssd ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x47,0xb7,0x50,0x72,0x80]
     {evex} vpdpbssd ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vpdpbssd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x47,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbssd xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbssd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x47,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbssd xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbssd xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x47,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbssd xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vpdpbssd xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x47,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbssd xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vpdpbssd xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x47,0x87,0x50,0x71,0x7f]
     {evex} vpdpbssd xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vpdpbssd xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x47,0x97,0x50,0x72,0x80]
     {evex} vpdpbssd xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vpdpbssds ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x47,0x20,0x51,0xf0]
     {evex} vpdpbssds ymm22, ymm23, ymm24

// CHECK: vpdpbssds ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x47,0x27,0x51,0xf0]
     {evex} vpdpbssds ymm22 {k7}, ymm23, ymm24

// CHECK: vpdpbssds ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x47,0xa7,0x51,0xf0]
     {evex} vpdpbssds ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vpdpbssds xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x47,0x00,0x51,0xf0]
     {evex} vpdpbssds xmm22, xmm23, xmm24

// CHECK: vpdpbssds xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x47,0x07,0x51,0xf0]
     {evex} vpdpbssds xmm22 {k7}, xmm23, xmm24

// CHECK: vpdpbssds xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x47,0x87,0x51,0xf0]
     {evex} vpdpbssds xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vpdpbssds ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x47,0x20,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbssds ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbssds ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x47,0x27,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbssds ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbssds ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x47,0x30,0x51,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbssds ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vpdpbssds ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x47,0x20,0x51,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbssds ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vpdpbssds ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x47,0xa7,0x51,0x71,0x7f]
     {evex} vpdpbssds ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vpdpbssds ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x47,0xb7,0x51,0x72,0x80]
     {evex} vpdpbssds ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vpdpbssds xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x47,0x00,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbssds xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbssds xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x47,0x07,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbssds xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbssds xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x47,0x10,0x51,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbssds xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vpdpbssds xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x47,0x00,0x51,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbssds xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vpdpbssds xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x47,0x87,0x51,0x71,0x7f]
     {evex} vpdpbssds xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vpdpbssds xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x47,0x97,0x51,0x72,0x80]
     {evex} vpdpbssds xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vpdpbsud ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x46,0x20,0x50,0xf0]
     {evex} vpdpbsud ymm22, ymm23, ymm24

// CHECK: vpdpbsud ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x46,0x27,0x50,0xf0]
     {evex} vpdpbsud ymm22 {k7}, ymm23, ymm24

// CHECK: vpdpbsud ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x46,0xa7,0x50,0xf0]
     {evex} vpdpbsud ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vpdpbsud xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x46,0x00,0x50,0xf0]
     {evex} vpdpbsud xmm22, xmm23, xmm24

// CHECK: vpdpbsud xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x46,0x07,0x50,0xf0]
     {evex} vpdpbsud xmm22 {k7}, xmm23, xmm24

// CHECK: vpdpbsud xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x46,0x87,0x50,0xf0]
     {evex} vpdpbsud xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vpdpbsud ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x46,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbsud ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbsud ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x46,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbsud ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbsud ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x46,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbsud ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vpdpbsud ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x46,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbsud ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vpdpbsud ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x46,0xa7,0x50,0x71,0x7f]
     {evex} vpdpbsud ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vpdpbsud ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x46,0xb7,0x50,0x72,0x80]
     {evex} vpdpbsud ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vpdpbsud xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x46,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbsud xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbsud xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x46,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbsud xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbsud xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x46,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbsud xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vpdpbsud xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x46,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbsud xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vpdpbsud xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x46,0x87,0x50,0x71,0x7f]
     {evex} vpdpbsud xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vpdpbsud xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x46,0x97,0x50,0x72,0x80]
     {evex} vpdpbsud xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vpdpbsuds ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x46,0x20,0x51,0xf0]
     {evex} vpdpbsuds ymm22, ymm23, ymm24

// CHECK: vpdpbsuds ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x46,0x27,0x51,0xf0]
     {evex} vpdpbsuds ymm22 {k7}, ymm23, ymm24

// CHECK: vpdpbsuds ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x46,0xa7,0x51,0xf0]
     {evex} vpdpbsuds ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vpdpbsuds xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x46,0x00,0x51,0xf0]
     {evex} vpdpbsuds xmm22, xmm23, xmm24

// CHECK: vpdpbsuds xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x46,0x07,0x51,0xf0]
     {evex} vpdpbsuds xmm22 {k7}, xmm23, xmm24

// CHECK: vpdpbsuds xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x46,0x87,0x51,0xf0]
     {evex} vpdpbsuds xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vpdpbsuds ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x46,0x20,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbsuds ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbsuds ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x46,0x27,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbsuds ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbsuds ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x46,0x30,0x51,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbsuds ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vpdpbsuds ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x46,0x20,0x51,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbsuds ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vpdpbsuds ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x46,0xa7,0x51,0x71,0x7f]
     {evex} vpdpbsuds ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vpdpbsuds ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x46,0xb7,0x51,0x72,0x80]
     {evex} vpdpbsuds ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vpdpbsuds xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x46,0x00,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbsuds xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbsuds xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x46,0x07,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbsuds xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbsuds xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x46,0x10,0x51,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbsuds xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vpdpbsuds xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x46,0x00,0x51,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbsuds xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vpdpbsuds xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x46,0x87,0x51,0x71,0x7f]
     {evex} vpdpbsuds xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vpdpbsuds xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x46,0x97,0x51,0x72,0x80]
     {evex} vpdpbsuds xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vpdpbuud ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0x20,0x50,0xf0]
     {evex} vpdpbuud ymm22, ymm23, ymm24

// CHECK: vpdpbuud ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0x27,0x50,0xf0]
     {evex} vpdpbuud ymm22 {k7}, ymm23, ymm24

// CHECK: vpdpbuud ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0xa7,0x50,0xf0]
     {evex} vpdpbuud ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vpdpbuud xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x00,0x50,0xf0]
     {evex} vpdpbuud xmm22, xmm23, xmm24

// CHECK: vpdpbuud xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x07,0x50,0xf0]
     {evex} vpdpbuud xmm22 {k7}, xmm23, xmm24

// CHECK: vpdpbuud xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x87,0x50,0xf0]
     {evex} vpdpbuud xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vpdpbuud ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbuud ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbuud ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbuud ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbuud ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x44,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbuud ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vpdpbuud ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x44,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbuud ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vpdpbuud ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x44,0xa7,0x50,0x71,0x7f]
     {evex} vpdpbuud ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vpdpbuud ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x44,0xb7,0x50,0x72,0x80]
     {evex} vpdpbuud ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vpdpbuud xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbuud xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbuud xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbuud xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbuud xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x44,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbuud xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vpdpbuud xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x44,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbuud xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vpdpbuud xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x44,0x87,0x50,0x71,0x7f]
     {evex} vpdpbuud xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vpdpbuud xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x44,0x97,0x50,0x72,0x80]
     {evex} vpdpbuud xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vpdpbuuds ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0x20,0x51,0xf0]
     {evex} vpdpbuuds ymm22, ymm23, ymm24

// CHECK: vpdpbuuds ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0x27,0x51,0xf0]
     {evex} vpdpbuuds ymm22 {k7}, ymm23, ymm24

// CHECK: vpdpbuuds ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0xa7,0x51,0xf0]
     {evex} vpdpbuuds ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vpdpbuuds xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x00,0x51,0xf0]
     {evex} vpdpbuuds xmm22, xmm23, xmm24

// CHECK: vpdpbuuds xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x07,0x51,0xf0]
     {evex} vpdpbuuds xmm22 {k7}, xmm23, xmm24

// CHECK: vpdpbuuds xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x87,0x51,0xf0]
     {evex} vpdpbuuds xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vpdpbuuds ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x20,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbuuds ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbuuds ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x27,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbuuds ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbuuds ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x44,0x30,0x51,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbuuds ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vpdpbuuds ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x44,0x20,0x51,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vpdpbuuds ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vpdpbuuds ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x44,0xa7,0x51,0x71,0x7f]
     {evex} vpdpbuuds ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vpdpbuuds ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x44,0xb7,0x51,0x72,0x80]
     {evex} vpdpbuuds ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vpdpbuuds xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x00,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vpdpbuuds xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vpdpbuuds xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x07,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vpdpbuuds xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vpdpbuuds xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x44,0x10,0x51,0x35,0x00,0x00,0x00,0x00]
     {evex} vpdpbuuds xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vpdpbuuds xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x44,0x00,0x51,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vpdpbuuds xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vpdpbuuds xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x44,0x87,0x51,0x71,0x7f]
     {evex} vpdpbuuds xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vpdpbuuds xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x44,0x97,0x51,0x72,0x80]
     {evex} vpdpbuuds xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

