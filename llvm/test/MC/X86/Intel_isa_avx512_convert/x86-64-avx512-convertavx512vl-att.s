// REQUIRES: intel_feature_isa_avx512_convert
// RUN: llvm-mc -triple=x86_64-unknown-unknown -mattr=+avx512vl,+avx512convert --show-encoding < %s  | FileCheck %s

// CHECK: vcvt2ps2ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x45,0x20,0x67,0xf0]
     {evex} vcvt2ps2ph %ymm24, %ymm23, %ymm22

// CHECK: vcvt2ps2ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x45,0x27,0x67,0xf0]
     {evex} vcvt2ps2ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vcvt2ps2ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x45,0xa7,0x67,0xf0]
     {evex} vcvt2ps2ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vcvt2ps2ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x45,0x00,0x67,0xf0]
     {evex} vcvt2ps2ph %xmm24, %xmm23, %xmm22

// CHECK: vcvt2ps2ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x82,0x45,0x07,0x67,0xf0]
     {evex} vcvt2ps2ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vcvt2ps2ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x82,0x45,0x87,0x67,0xf0]
     {evex} vcvt2ps2ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvt2ps2ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x45,0x20,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vcvt2ps2ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vcvt2ps2ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x45,0x27,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vcvt2ps2ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vcvt2ps2ph  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x45,0x30,0x67,0x35,0x00,0x00,0x00,0x00]
     {evex} vcvt2ps2ph  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vcvt2ps2ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x45,0x20,0x67,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vcvt2ps2ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vcvt2ps2ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0xa7,0x67,0x71,0x7f]
     {evex} vcvt2ps2ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vcvt2ps2ph  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0xb7,0x67,0x72,0x80]
     {evex} vcvt2ps2ph  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vcvt2ps2ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x45,0x00,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vcvt2ps2ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vcvt2ps2ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x45,0x07,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vcvt2ps2ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vcvt2ps2ph  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x10,0x67,0x35,0x00,0x00,0x00,0x00]
     {evex} vcvt2ps2ph  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vcvt2ps2ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x00,0x67,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vcvt2ps2ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvt2ps2ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0x87,0x67,0x71,0x7f]
     {evex} vcvt2ps2ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvt2ps2ph  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x45,0x97,0x67,0x72,0x80]
     {evex} vcvt2ps2ph  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtbf162ph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0x67,0xf7]
     {evex} vcvtbf162ph %xmm23, %xmm22

// CHECK: vcvtbf162ph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa2,0x7e,0x0f,0x67,0xf7]
     {evex} vcvtbf162ph %xmm23, %xmm22 {%k7}

// CHECK: vcvtbf162ph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa2,0x7e,0x8f,0x67,0xf7]
     {evex} vcvtbf162ph %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtbf162ph %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x28,0x67,0xf7]
     {evex} vcvtbf162ph %ymm23, %ymm22

// CHECK: vcvtbf162ph %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa2,0x7e,0x2f,0x67,0xf7]
     {evex} vcvtbf162ph %ymm23, %ymm22 {%k7}

// CHECK: vcvtbf162ph %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa2,0x7e,0xaf,0x67,0xf7]
     {evex} vcvtbf162ph %ymm23, %ymm22 {%k7} {z}

// CHECK: vcvtbf162ph  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vcvtbf162ph  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtbf162ph  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7e,0x0f,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vcvtbf162ph  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtbf162ph  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x18,0x67,0x35,0x00,0x00,0x00,0x00]
     {evex} vcvtbf162ph  (%rip){1to8}, %xmm22

// CHECK: vcvtbf162ph  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0x67,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vcvtbf162ph  -512(,%rbp,2), %xmm22

// CHECK: vcvtbf162ph  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0x8f,0x67,0x71,0x7f]
     {evex} vcvtbf162ph  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtbf162ph  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0x9f,0x67,0x72,0x80]
     {evex} vcvtbf162ph  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtbf162ph  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x28,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vcvtbf162ph  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtbf162ph  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7e,0x2f,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vcvtbf162ph  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtbf162ph  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x38,0x67,0x35,0x00,0x00,0x00,0x00]
     {evex} vcvtbf162ph  (%rip){1to16}, %ymm22

// CHECK: vcvtbf162ph  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0x67,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vcvtbf162ph  -1024(,%rbp,2), %ymm22

// CHECK: vcvtbf162ph  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0xaf,0x67,0x71,0x7f]
     {evex} vcvtbf162ph  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtbf162ph  -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0xbf,0x67,0x72,0x80]
     {evex} vcvtbf162ph  -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vcvtneph2bf16 %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x08,0x67,0xf7]
     {evex} vcvtneph2bf16 %xmm23, %xmm22

// CHECK: vcvtneph2bf16 %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa2,0x7f,0x0f,0x67,0xf7]
     {evex} vcvtneph2bf16 %xmm23, %xmm22 {%k7}

// CHECK: vcvtneph2bf16 %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa2,0x7f,0x8f,0x67,0xf7]
     {evex} vcvtneph2bf16 %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf16 %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x28,0x67,0xf7]
     {evex} vcvtneph2bf16 %ymm23, %ymm22

// CHECK: vcvtneph2bf16 %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa2,0x7f,0x2f,0x67,0xf7]
     {evex} vcvtneph2bf16 %ymm23, %ymm22 {%k7}

// CHECK: vcvtneph2bf16 %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa2,0x7f,0xaf,0x67,0xf7]
     {evex} vcvtneph2bf16 %ymm23, %ymm22 {%k7} {z}

// CHECK: vcvtneph2bf16  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x08,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vcvtneph2bf16  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtneph2bf16  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7f,0x0f,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vcvtneph2bf16  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtneph2bf16  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x18,0x67,0x35,0x00,0x00,0x00,0x00]
     {evex} vcvtneph2bf16  (%rip){1to8}, %xmm22

// CHECK: vcvtneph2bf16  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x08,0x67,0x34,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vcvtneph2bf16  -512(,%rbp,2), %xmm22

// CHECK: vcvtneph2bf16  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7f,0x8f,0x67,0x71,0x7f]
     {evex} vcvtneph2bf16  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf16  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7f,0x9f,0x67,0x72,0x80]
     {evex} vcvtneph2bf16  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtneph2bf16  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x28,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
     {evex} vcvtneph2bf16  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtneph2bf16  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7f,0x2f,0x67,0xb4,0x80,0x23,0x01,0x00,0x00]
     {evex} vcvtneph2bf16  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtneph2bf16  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x38,0x67,0x35,0x00,0x00,0x00,0x00]
     {evex} vcvtneph2bf16  (%rip){1to16}, %ymm22

// CHECK: vcvtneph2bf16  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x28,0x67,0x34,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vcvtneph2bf16  -1024(,%rbp,2), %ymm22

// CHECK: vcvtneph2bf16  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7f,0xaf,0x67,0x71,0x7f]
     {evex} vcvtneph2bf16  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtneph2bf16  -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7f,0xbf,0x67,0x72,0x80]
     {evex} vcvtneph2bf16  -256(%rdx){1to16}, %ymm22 {%k7} {z}

