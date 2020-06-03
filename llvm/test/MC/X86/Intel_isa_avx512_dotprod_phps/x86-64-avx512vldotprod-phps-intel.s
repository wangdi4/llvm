// REQUIRES: intel_feature_isa_avx512_dotprod_phps
// RUN: llvm-mc -triple x86_64-unknown-unknown -mattr=+avx512vl,+avx512dotprodphps -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vdpphps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0x20,0x52,0xf0]
     {evex} vdpphps ymm22, ymm23, ymm24

// CHECK: vdpphps ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0x27,0x52,0xf0]
     {evex} vdpphps ymm22 {k7}, ymm23, ymm24

// CHECK: vdpphps ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x44,0xa7,0x52,0xf0]
     {evex} vdpphps ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vdpphps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x00,0x52,0xf0]
     {evex} vdpphps xmm22, xmm23, xmm24

// CHECK: vdpphps xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x07,0x52,0xf0]
     {evex} vdpphps xmm22 {k7}, xmm23, xmm24

// CHECK: vdpphps xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x44,0x87,0x52,0xf0]
     {evex} vdpphps xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vdpphps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x20,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vdpphps ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdpphps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x27,0x52,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vdpphps ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vdpphps ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x44,0x30,0x52,0x35,0x00,0x00,0x00,0x00]
     {evex} vdpphps ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vdpphps ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x44,0x20,0x52,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vdpphps ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vdpphps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x44,0xa7,0x52,0x71,0x7f]
     {evex} vdpphps ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vdpphps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x44,0xb7,0x52,0x72,0x80]
     {evex} vdpphps ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vdpphps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x44,0x00,0x52,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vdpphps xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdpphps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x44,0x07,0x52,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vdpphps xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vdpphps xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x44,0x10,0x52,0x35,0x00,0x00,0x00,0x00]
     {evex} vdpphps xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vdpphps xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x44,0x00,0x52,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vdpphps xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vdpphps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x44,0x87,0x52,0x71,0x7f]
     {evex} vdpphps xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vdpphps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x44,0x97,0x52,0x72,0x80]
     {evex} vdpphps xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

