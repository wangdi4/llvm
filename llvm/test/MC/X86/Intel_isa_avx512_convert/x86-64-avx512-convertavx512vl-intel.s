// REQUIRES: intel_feature_isa_avx512_convert
// RUN: llvm-mc -triple x86_64-unknown-unknown -mattr=+avx512vl,+avx512convert -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vcvt2ps2ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x45,0x20,0x67,0xf0]
     {evex} vcvt2ps2ph ymm22, ymm23, ymm24

// CHECK: vcvt2ps2ph ymm22 {k7}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x45,0x27,0x67,0xf0]
     {evex} vcvt2ps2ph ymm22 {k7}, ymm23, ymm24

// CHECK: vcvt2ps2ph ymm22 {k7} {z}, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x45,0xa7,0x67,0xf0]
     {evex} vcvt2ps2ph ymm22 {k7} {z}, ymm23, ymm24

// CHECK: vcvt2ps2ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x45,0x00,0x67,0xf0]
     {evex} vcvt2ps2ph xmm22, xmm23, xmm24

// CHECK: vcvt2ps2ph xmm22 {k7}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x45,0x07,0x67,0xf0]
     {evex} vcvt2ps2ph xmm22 {k7}, xmm23, xmm24

// CHECK: vcvt2ps2ph xmm22 {k7} {z}, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x45,0x87,0x67,0xf0]
     {evex} vcvt2ps2ph xmm22 {k7} {z}, xmm23, xmm24

// CHECK: vcvt2ps2ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x20,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vcvt2ps2ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvt2ps2ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x45,0x27,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vcvt2ps2ph ymm22 {k7}, ymm23, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvt2ps2ph ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x45,0x30,0x67,0x35,0x00,0x00,0x00,0x00]
     {evex} vcvt2ps2ph ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK: vcvt2ps2ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x45,0x20,0x67,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vcvt2ps2ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK: vcvt2ps2ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x45,0xa7,0x67,0x71,0x7f]
     {evex} vcvt2ps2ph ymm22 {k7} {z}, ymm23, ymmword ptr [rcx + 4064]

// CHECK: vcvt2ps2ph ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}
// CHECK: encoding: [0x62,0xe2,0x45,0xb7,0x67,0x72,0x80]
     {evex} vcvt2ps2ph ymm22 {k7} {z}, ymm23, dword ptr [rdx - 512]{1to8}

// CHECK: vcvt2ps2ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x00,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vcvt2ps2ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvt2ps2ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x45,0x07,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vcvt2ps2ph xmm22 {k7}, xmm23, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvt2ps2ph xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x45,0x10,0x67,0x35,0x00,0x00,0x00,0x00]
     {evex} vcvt2ps2ph xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK: vcvt2ps2ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x45,0x00,0x67,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vcvt2ps2ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK: vcvt2ps2ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x45,0x87,0x67,0x71,0x7f]
     {evex} vcvt2ps2ph xmm22 {k7} {z}, xmm23, xmmword ptr [rcx + 2032]

// CHECK: vcvt2ps2ph xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}
// CHECK: encoding: [0x62,0xe2,0x45,0x97,0x67,0x72,0x80]
     {evex} vcvt2ps2ph xmm22 {k7} {z}, xmm23, dword ptr [rdx - 512]{1to4}

// CHECK: vcvtbf162ph xmm22, xmm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0x67,0xf7]
     {evex} vcvtbf162ph xmm22, xmm23

// CHECK: vcvtbf162ph xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x0f,0x67,0xf7]
     {evex} vcvtbf162ph xmm22 {k7}, xmm23

// CHECK: vcvtbf162ph xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x8f,0x67,0xf7]
     {evex} vcvtbf162ph xmm22 {k7} {z}, xmm23

// CHECK: vcvtbf162ph ymm22, ymm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x28,0x67,0xf7]
     {evex} vcvtbf162ph ymm22, ymm23

// CHECK: vcvtbf162ph ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa2,0x7e,0x2f,0x67,0xf7]
     {evex} vcvtbf162ph ymm22 {k7}, ymm23

// CHECK: vcvtbf162ph ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa2,0x7e,0xaf,0x67,0xf7]
     {evex} vcvtbf162ph ymm22 {k7} {z}, ymm23

// CHECK: vcvtbf162ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vcvtbf162ph xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbf162ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x0f,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vcvtbf162ph xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbf162ph xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x7e,0x18,0x67,0x35,0x00,0x00,0x00,0x00]
     {evex} vcvtbf162ph xmm22, word ptr [rip]{1to8}

// CHECK: vcvtbf162ph xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0x67,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vcvtbf162ph xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtbf162ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x7e,0x8f,0x67,0x71,0x7f]
     {evex} vcvtbf162ph xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtbf162ph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe2,0x7e,0x9f,0x67,0x72,0x80]
     {evex} vcvtbf162ph xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtbf162ph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x28,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vcvtbf162ph ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtbf162ph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x2f,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vcvtbf162ph ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtbf162ph ymm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x7e,0x38,0x67,0x35,0x00,0x00,0x00,0x00]
     {evex} vcvtbf162ph ymm22, word ptr [rip]{1to16}

// CHECK: vcvtbf162ph ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0x67,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vcvtbf162ph ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtbf162ph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x7e,0xaf,0x67,0x71,0x7f]
     {evex} vcvtbf162ph ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtbf162ph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe2,0x7e,0xbf,0x67,0x72,0x80]
     {evex} vcvtbf162ph ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}

// CHECK: vcvtneph2bf16 xmm22, xmm23
// CHECK: encoding: [0x62,0xa2,0x7f,0x08,0x67,0xf7]
     {evex} vcvtneph2bf16 xmm22, xmm23

// CHECK: vcvtneph2bf16 xmm22 {k7}, xmm23
// CHECK: encoding: [0x62,0xa2,0x7f,0x0f,0x67,0xf7]
     {evex} vcvtneph2bf16 xmm22 {k7}, xmm23

// CHECK: vcvtneph2bf16 xmm22 {k7} {z}, xmm23
// CHECK: encoding: [0x62,0xa2,0x7f,0x8f,0x67,0xf7]
     {evex} vcvtneph2bf16 xmm22 {k7} {z}, xmm23

// CHECK: vcvtneph2bf16 ymm22, ymm23
// CHECK: encoding: [0x62,0xa2,0x7f,0x28,0x67,0xf7]
     {evex} vcvtneph2bf16 ymm22, ymm23

// CHECK: vcvtneph2bf16 ymm22 {k7}, ymm23
// CHECK: encoding: [0x62,0xa2,0x7f,0x2f,0x67,0xf7]
     {evex} vcvtneph2bf16 ymm22 {k7}, ymm23

// CHECK: vcvtneph2bf16 ymm22 {k7} {z}, ymm23
// CHECK: encoding: [0x62,0xa2,0x7f,0xaf,0x67,0xf7]
     {evex} vcvtneph2bf16 ymm22 {k7} {z}, ymm23

// CHECK: vcvtneph2bf16 xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7f,0x08,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vcvtneph2bf16 xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtneph2bf16 xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7f,0x0f,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vcvtneph2bf16 xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtneph2bf16 xmm22, word ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x7f,0x18,0x67,0x35,0x00,0x00,0x00,0x00]
     {evex} vcvtneph2bf16 xmm22, word ptr [rip]{1to8}

// CHECK: vcvtneph2bf16 xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x7f,0x08,0x67,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vcvtneph2bf16 xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vcvtneph2bf16 xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x7f,0x8f,0x67,0x71,0x7f]
     {evex} vcvtneph2bf16 xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vcvtneph2bf16 xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}
// CHECK: encoding: [0x62,0xe2,0x7f,0x9f,0x67,0x72,0x80]
     {evex} vcvtneph2bf16 xmm22 {k7} {z}, word ptr [rdx - 256]{1to8}

// CHECK: vcvtneph2bf16 ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7f,0x28,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vcvtneph2bf16 ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vcvtneph2bf16 ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7f,0x2f,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vcvtneph2bf16 ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vcvtneph2bf16 ymm22, word ptr [rip]{1to16}
// CHECK: encoding: [0x62,0xe2,0x7f,0x38,0x67,0x35,0x00,0x00,0x00,0x00]
     {evex} vcvtneph2bf16 ymm22, word ptr [rip]{1to16}

// CHECK: vcvtneph2bf16 ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x7f,0x28,0x67,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vcvtneph2bf16 ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vcvtneph2bf16 ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x7f,0xaf,0x67,0x71,0x7f]
     {evex} vcvtneph2bf16 ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vcvtneph2bf16 ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}
// CHECK: encoding: [0x62,0xe2,0x7f,0xbf,0x67,0x72,0x80]
     {evex} vcvtneph2bf16 ymm22 {k7} {z}, word ptr [rdx - 256]{1to16}

