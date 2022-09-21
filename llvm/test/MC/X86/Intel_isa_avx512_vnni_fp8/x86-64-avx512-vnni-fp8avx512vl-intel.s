// REQUIRES: intel_feature_isa_avx512_vnni_fp8
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vdpbf8ps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x44,0x20,0x50,0xf0]
          vdpbf8ps ymm22, ymm23, ymm24

// CHECK: vdpbf8ps ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x44,0x27,0x50,0xf0]
          vdpbf8ps ymm22 {k7}, ymm23, ymm24

// CHECK: vdpbf8ps ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x44,0xa7,0x50,0xf0]
          vdpbf8ps ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vdpbf8ps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x44,0x00,0x50,0xf0]
          vdpbf8ps xmm22, xmm23, xmm24

// CHECK: vdpbf8ps xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x44,0x07,0x50,0xf0]
          vdpbf8ps xmm22 {k7}, xmm23, xmm24

// CHECK: vdpbf8ps xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x44,0x87,0x50,0xf0]
          vdpbf8ps xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vdpbf8ps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x44,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdpbf8ps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdpbf8ps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x44,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdpbf8ps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vdpbf8ps ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x44,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
          vdpbf8ps ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vdpbf8ps ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x44,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vdpbf8ps ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vdpbf8ps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x44,0xa7,0x50,0x71,0x7f]
          vdpbf8ps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vdpbf8ps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe5,0x44,0xb7,0x50,0x72,0x80]
          vdpbf8ps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vdpbf8ps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x44,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdpbf8ps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdpbf8ps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x44,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdpbf8ps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vdpbf8ps xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x44,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
          vdpbf8ps xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vdpbf8ps xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x44,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vdpbf8ps xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vdpbf8ps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x44,0x87,0x50,0x71,0x7f]
          vdpbf8ps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vdpbf8ps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe5,0x44,0x97,0x50,0x72,0x80]
          vdpbf8ps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vdpbhf8ps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x20,0x50,0xf0]
          vdpbhf8ps ymm22, ymm23, ymm24

// CHECK: vdpbhf8ps ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0x27,0x50,0xf0]
          vdpbhf8ps ymm22 {k7}, ymm23, ymm24

// CHECK: vdpbhf8ps ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x47,0xa7,0x50,0xf0]
          vdpbhf8ps ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vdpbhf8ps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x00,0x50,0xf0]
          vdpbhf8ps xmm22, xmm23, xmm24

// CHECK: vdpbhf8ps xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x07,0x50,0xf0]
          vdpbhf8ps xmm22 {k7}, xmm23, xmm24

// CHECK: vdpbhf8ps xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x47,0x87,0x50,0xf0]
          vdpbhf8ps xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vdpbhf8ps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdpbhf8ps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdpbhf8ps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdpbhf8ps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vdpbhf8ps ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
          vdpbhf8ps ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vdpbhf8ps ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x47,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vdpbhf8ps ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vdpbhf8ps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x47,0xa7,0x50,0x71,0x7f]
          vdpbhf8ps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vdpbhf8ps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe5,0x47,0xb7,0x50,0x72,0x80]
          vdpbhf8ps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vdpbhf8ps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x47,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdpbhf8ps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdpbhf8ps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x47,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdpbhf8ps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vdpbhf8ps xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x47,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
          vdpbhf8ps xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vdpbhf8ps xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x47,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vdpbhf8ps xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vdpbhf8ps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x47,0x87,0x50,0x71,0x7f]
          vdpbhf8ps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vdpbhf8ps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe5,0x47,0x97,0x50,0x72,0x80]
          vdpbhf8ps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vdphbf8ps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x20,0x50,0xf0]
          vdphbf8ps ymm22, ymm23, ymm24

// CHECK: vdphbf8ps ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0x27,0x50,0xf0]
          vdphbf8ps ymm22 {k7}, ymm23, ymm24

// CHECK: vdphbf8ps ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x46,0xa7,0x50,0xf0]
          vdphbf8ps ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vdphbf8ps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x50,0xf0]
          vdphbf8ps xmm22, xmm23, xmm24

// CHECK: vdphbf8ps xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x50,0xf0]
          vdphbf8ps xmm22 {k7}, xmm23, xmm24

// CHECK: vdphbf8ps xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x46,0x87,0x50,0xf0]
          vdphbf8ps xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vdphbf8ps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdphbf8ps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdphbf8ps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdphbf8ps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vdphbf8ps ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x46,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
          vdphbf8ps ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vdphbf8ps ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x46,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vdphbf8ps ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vdphbf8ps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x46,0xa7,0x50,0x71,0x7f]
          vdphbf8ps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vdphbf8ps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe5,0x46,0xb7,0x50,0x72,0x80]
          vdphbf8ps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vdphbf8ps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdphbf8ps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdphbf8ps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdphbf8ps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vdphbf8ps xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x46,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
          vdphbf8ps xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vdphbf8ps xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vdphbf8ps xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vdphbf8ps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x46,0x87,0x50,0x71,0x7f]
          vdphbf8ps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vdphbf8ps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe5,0x46,0x97,0x50,0x72,0x80]
          vdphbf8ps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vdphf8ps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x45,0x20,0x50,0xf0]
          vdphf8ps ymm22, ymm23, ymm24

// CHECK: vdphf8ps ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x45,0x27,0x50,0xf0]
          vdphf8ps ymm22 {k7}, ymm23, ymm24

// CHECK: vdphf8ps ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x85,0x45,0xa7,0x50,0xf0]
          vdphf8ps ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vdphf8ps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x45,0x00,0x50,0xf0]
          vdphf8ps xmm22, xmm23, xmm24

// CHECK: vdphf8ps xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x45,0x07,0x50,0xf0]
          vdphf8ps xmm22 {k7}, xmm23, xmm24

// CHECK: vdphf8ps xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x85,0x45,0x87,0x50,0xf0]
          vdphf8ps xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vdphf8ps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x20,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdphf8ps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdphf8ps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x27,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdphf8ps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vdphf8ps ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe5,0x45,0x30,0x50,0x35,0x00,0x00,0x00,0x00]
          vdphf8ps ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vdphf8ps ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x45,0x20,0x50,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vdphf8ps ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vdphf8ps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x45,0xa7,0x50,0x71,0x7f]
          vdphf8ps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vdphf8ps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe5,0x45,0xb7,0x50,0x72,0x80]
          vdphf8ps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vdphf8ps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x45,0x00,0x50,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdphf8ps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdphf8ps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x45,0x07,0x50,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdphf8ps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vdphf8ps xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe5,0x45,0x10,0x50,0x35,0x00,0x00,0x00,0x00]
          vdphf8ps xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vdphf8ps xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x45,0x00,0x50,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vdphf8ps xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vdphf8ps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x45,0x87,0x50,0x71,0x7f]
          vdphf8ps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vdphf8ps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe5,0x45,0x97,0x50,0x72,0x80]
          vdphf8ps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

