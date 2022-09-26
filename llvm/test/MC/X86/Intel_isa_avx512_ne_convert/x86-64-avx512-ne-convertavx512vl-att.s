// REQUIRES: intel_feature_isa_avx512_ne_convert
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      {evex} vbcstnebf162ps  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0xb1,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vbcstnebf162ps  268435456(%rbp,%r14,8), %xmm22

// CHECK:      {evex} vbcstnebf162ps  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7e,0x0f,0xb1,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vbcstnebf162ps  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK:      {evex} vbcstnebf162ps  (%rip), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xb1,0x35,0x00,0x00,0x00,0x00]
               {evex} vbcstnebf162ps  (%rip), %xmm22

// CHECK:      {evex} vbcstnebf162ps  -64(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xb1,0x34,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnebf162ps  -64(,%rbp,2), %xmm22

// CHECK:      {evex} vbcstnebf162ps  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x28,0xb1,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vbcstnebf162ps  268435456(%rbp,%r14,8), %ymm22

// CHECK:      {evex} vbcstnebf162ps  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7e,0x2f,0xb1,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vbcstnebf162ps  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK:      {evex} vbcstnebf162ps  (%rip), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xb1,0x35,0x00,0x00,0x00,0x00]
               {evex} vbcstnebf162ps  (%rip), %ymm22

// CHECK:      {evex} vbcstnebf162ps  -64(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xb1,0x34,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnebf162ps  -64(,%rbp,2), %ymm22

// CHECK:      {evex} vbcstnesh2ps  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x08,0xb1,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vbcstnesh2ps  268435456(%rbp,%r14,8), %xmm22

// CHECK:      {evex} vbcstnesh2ps  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7d,0x0f,0xb1,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vbcstnesh2ps  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK:      {evex} vbcstnesh2ps  (%rip), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x08,0xb1,0x35,0x00,0x00,0x00,0x00]
               {evex} vbcstnesh2ps  (%rip), %xmm22

// CHECK:      {evex} vbcstnesh2ps  -64(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x08,0xb1,0x34,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnesh2ps  -64(,%rbp,2), %xmm22

// CHECK:      {evex} vbcstnesh2ps  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x28,0xb1,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vbcstnesh2ps  268435456(%rbp,%r14,8), %ymm22

// CHECK:      {evex} vbcstnesh2ps  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7d,0x2f,0xb1,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vbcstnesh2ps  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK:      {evex} vbcstnesh2ps  (%rip), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x28,0xb1,0x35,0x00,0x00,0x00,0x00]
               {evex} vbcstnesh2ps  (%rip), %ymm22

// CHECK:      {evex} vbcstnesh2ps  -64(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x28,0xb1,0x34,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnesh2ps  -64(,%rbp,2), %ymm22

// CHECK:      {evex} vcvtne2ps2ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x82,0x45,0x20,0x67,0xf0]
               {evex} vcvtne2ps2ph %ymm24, %ymm23, %ymm22

// CHECK:      {evex} vcvtne2ps2ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x82,0x45,0x00,0x67,0xf0]
               {evex} vcvtne2ps2ph %xmm24, %xmm23, %xmm22

// CHECK:      {evex} vcvtne2ps2ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa2,0x45,0x20,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtne2ps2ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK:      {evex} vcvtne2ps2ph  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x45,0x30,0x67,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtne2ps2ph  (%rip){1to8}, %ymm23, %ymm22

// CHECK:      {evex} vcvtne2ps2ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe2,0x45,0x20,0x67,0x34,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtne2ps2ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK:      {evex} vcvtne2ps2ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa2,0x45,0x00,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtne2ps2ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK:      {evex} vcvtne2ps2ph  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x10,0x67,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtne2ps2ph  (%rip){1to4}, %xmm23, %xmm22

// CHECK:      {evex} vcvtne2ps2ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x00,0x67,0x34,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtne2ps2ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK:      {evex} vcvtneebf162ps  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneebf162ps  268435456(%rbp,%r14,8), %xmm22

// CHECK:      {evex} vcvtneebf162ps  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7e,0x0f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneebf162ps  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK:      {evex} vcvtneebf162ps  (%rip), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneebf162ps  (%rip), %xmm22

// CHECK:      {evex} vcvtneebf162ps  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xb0,0x34,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneebf162ps  -512(,%rbp,2), %xmm22

// CHECK:      {evex} vcvtneebf162ps  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0x8f,0xb0,0x71,0x7f]
               {evex} vcvtneebf162ps  2032(%rcx), %xmm22 {%k7} {z}

// CHECK:      {evex} vcvtneebf162ps  -2048(%rdx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0x8f,0xb0,0x72,0x80]
               {evex} vcvtneebf162ps  -2048(%rdx), %xmm22 {%k7} {z}

// CHECK:      {evex} vcvtneebf162ps  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x28,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneebf162ps  268435456(%rbp,%r14,8), %ymm22

// CHECK:      {evex} vcvtneebf162ps  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7e,0x2f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneebf162ps  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK:      {evex} vcvtneebf162ps  (%rip), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneebf162ps  (%rip), %ymm22

// CHECK:      {evex} vcvtneebf162ps  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xb0,0x34,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneebf162ps  -1024(,%rbp,2), %ymm22

// CHECK:      {evex} vcvtneebf162ps  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0xaf,0xb0,0x71,0x7f]
               {evex} vcvtneebf162ps  4064(%rcx), %ymm22 {%k7} {z}

// CHECK:      {evex} vcvtneebf162ps  -4096(%rdx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0xaf,0xb0,0x72,0x80]
               {evex} vcvtneebf162ps  -4096(%rdx), %ymm22 {%k7} {z}

// CHECK:      {evex} vcvtneeph2ps  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x08,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneeph2ps  268435456(%rbp,%r14,8), %xmm22

// CHECK:      {evex} vcvtneeph2ps  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7d,0x0f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneeph2ps  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK:      {evex} vcvtneeph2ps  (%rip), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x08,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneeph2ps  (%rip), %xmm22

// CHECK:      {evex} vcvtneeph2ps  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x08,0xb0,0x34,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneeph2ps  -512(,%rbp,2), %xmm22

// CHECK:      {evex} vcvtneeph2ps  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7d,0x8f,0xb0,0x71,0x7f]
               {evex} vcvtneeph2ps  2032(%rcx), %xmm22 {%k7} {z}

// CHECK:      {evex} vcvtneeph2ps  -2048(%rdx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7d,0x8f,0xb0,0x72,0x80]
               {evex} vcvtneeph2ps  -2048(%rdx), %xmm22 {%k7} {z}

// CHECK:      {evex} vcvtneeph2ps  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x28,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneeph2ps  268435456(%rbp,%r14,8), %ymm22

// CHECK:      {evex} vcvtneeph2ps  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7d,0x2f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneeph2ps  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK:      {evex} vcvtneeph2ps  (%rip), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x28,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneeph2ps  (%rip), %ymm22

// CHECK:      {evex} vcvtneeph2ps  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x28,0xb0,0x34,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneeph2ps  -1024(,%rbp,2), %ymm22

// CHECK:      {evex} vcvtneeph2ps  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7d,0xaf,0xb0,0x71,0x7f]
               {evex} vcvtneeph2ps  4064(%rcx), %ymm22 {%k7} {z}

// CHECK:      {evex} vcvtneeph2ps  -4096(%rdx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7d,0xaf,0xb0,0x72,0x80]
               {evex} vcvtneeph2ps  -4096(%rdx), %ymm22 {%k7} {z}

// CHECK:      {evex} vcvtneobf162ps  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x08,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneobf162ps  268435456(%rbp,%r14,8), %xmm22

// CHECK:      {evex} vcvtneobf162ps  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7f,0x0f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneobf162ps  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK:      {evex} vcvtneobf162ps  (%rip), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x08,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneobf162ps  (%rip), %xmm22

// CHECK:      {evex} vcvtneobf162ps  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x08,0xb0,0x34,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneobf162ps  -512(,%rbp,2), %xmm22

// CHECK:      {evex} vcvtneobf162ps  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7f,0x8f,0xb0,0x71,0x7f]
               {evex} vcvtneobf162ps  2032(%rcx), %xmm22 {%k7} {z}

// CHECK:      {evex} vcvtneobf162ps  -2048(%rdx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7f,0x8f,0xb0,0x72,0x80]
               {evex} vcvtneobf162ps  -2048(%rdx), %xmm22 {%k7} {z}

// CHECK:      {evex} vcvtneobf162ps  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x28,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneobf162ps  268435456(%rbp,%r14,8), %ymm22

// CHECK:      {evex} vcvtneobf162ps  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7f,0x2f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneobf162ps  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK:      {evex} vcvtneobf162ps  (%rip), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x28,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneobf162ps  (%rip), %ymm22

// CHECK:      {evex} vcvtneobf162ps  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x28,0xb0,0x34,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneobf162ps  -1024(,%rbp,2), %ymm22

// CHECK:      {evex} vcvtneobf162ps  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7f,0xaf,0xb0,0x71,0x7f]
               {evex} vcvtneobf162ps  4064(%rcx), %ymm22 {%k7} {z}

// CHECK:      {evex} vcvtneobf162ps  -4096(%rdx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7f,0xaf,0xb0,0x72,0x80]
               {evex} vcvtneobf162ps  -4096(%rdx), %ymm22 {%k7} {z}

// CHECK:      {evex} vcvtneoph2ps  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x08,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneoph2ps  268435456(%rbp,%r14,8), %xmm22

// CHECK:      {evex} vcvtneoph2ps  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7c,0x0f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneoph2ps  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK:      {evex} vcvtneoph2ps  (%rip), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x08,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneoph2ps  (%rip), %xmm22

// CHECK:      {evex} vcvtneoph2ps  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x08,0xb0,0x34,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneoph2ps  -512(,%rbp,2), %xmm22

// CHECK:      {evex} vcvtneoph2ps  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7c,0x8f,0xb0,0x71,0x7f]
               {evex} vcvtneoph2ps  2032(%rcx), %xmm22 {%k7} {z}

// CHECK:      {evex} vcvtneoph2ps  -2048(%rdx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7c,0x8f,0xb0,0x72,0x80]
               {evex} vcvtneoph2ps  -2048(%rdx), %xmm22 {%k7} {z}

// CHECK:      {evex} vcvtneoph2ps  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x28,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneoph2ps  268435456(%rbp,%r14,8), %ymm22

// CHECK:      {evex} vcvtneoph2ps  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7c,0x2f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneoph2ps  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK:      {evex} vcvtneoph2ps  (%rip), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x28,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneoph2ps  (%rip), %ymm22

// CHECK:      {evex} vcvtneoph2ps  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x28,0xb0,0x34,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneoph2ps  -1024(,%rbp,2), %ymm22

// CHECK:      {evex} vcvtneoph2ps  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7c,0xaf,0xb0,0x71,0x7f]
               {evex} vcvtneoph2ps  4064(%rcx), %ymm22 {%k7} {z}

// CHECK:      {evex} vcvtneoph2ps  -4096(%rdx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7c,0xaf,0xb0,0x72,0x80]
               {evex} vcvtneoph2ps  -4096(%rdx), %ymm22 {%k7} {z}

