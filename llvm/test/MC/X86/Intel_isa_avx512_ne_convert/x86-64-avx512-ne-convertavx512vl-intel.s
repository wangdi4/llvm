// REQUIRES: intel_feature_isa_avx_ne_convert
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      {evex} vbcstnebf162ps xmm22, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0xb1,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vbcstnebf162ps xmm22, word ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vbcstnebf162ps xmm22 {k7}, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x0f,0xb1,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vbcstnebf162ps xmm22 {k7}, word ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vbcstnebf162ps xmm22, word ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xb1,0x35,0x00,0x00,0x00,0x00]
               {evex} vbcstnebf162ps xmm22, word ptr [rip]

// CHECK:      {evex} vbcstnebf162ps xmm22, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xb1,0x34,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnebf162ps xmm22, word ptr [2*rbp - 64]

// CHECK:      {evex} vbcstnebf162ps ymm22, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x28,0xb1,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vbcstnebf162ps ymm22, word ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vbcstnebf162ps ymm22 {k7}, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x2f,0xb1,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vbcstnebf162ps ymm22 {k7}, word ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vbcstnebf162ps ymm22, word ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xb1,0x35,0x00,0x00,0x00,0x00]
               {evex} vbcstnebf162ps ymm22, word ptr [rip]

// CHECK:      {evex} vbcstnebf162ps ymm22, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xb1,0x34,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnebf162ps ymm22, word ptr [2*rbp - 64]

// CHECK:      {evex} vbcstnesh2ps xmm22, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7d,0x08,0xb1,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vbcstnesh2ps xmm22, word ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vbcstnesh2ps xmm22 {k7}, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7d,0x0f,0xb1,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vbcstnesh2ps xmm22 {k7}, word ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vbcstnesh2ps xmm22, word ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7d,0x08,0xb1,0x35,0x00,0x00,0x00,0x00]
               {evex} vbcstnesh2ps xmm22, word ptr [rip]

// CHECK:      {evex} vbcstnesh2ps xmm22, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0xe2,0x7d,0x08,0xb1,0x34,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnesh2ps xmm22, word ptr [2*rbp - 64]

// CHECK:      {evex} vbcstnesh2ps ymm22, word ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7d,0x28,0xb1,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vbcstnesh2ps ymm22, word ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vbcstnesh2ps ymm22 {k7}, word ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7d,0x2f,0xb1,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vbcstnesh2ps ymm22 {k7}, word ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vbcstnesh2ps ymm22, word ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7d,0x28,0xb1,0x35,0x00,0x00,0x00,0x00]
               {evex} vbcstnesh2ps ymm22, word ptr [rip]

// CHECK:      {evex} vbcstnesh2ps ymm22, word ptr [2*rbp - 64]
// CHECK: encoding: [0x62,0xe2,0x7d,0x28,0xb1,0x34,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnesh2ps ymm22, word ptr [2*rbp - 64]

// CHECK:      {evex} vcvtne2ps2ph ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x82,0x45,0x20,0x67,0xf0]
               {evex} vcvtne2ps2ph ymm22, ymm23, ymm24

// CHECK:      {evex} vcvtne2ps2ph xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x82,0x45,0x00,0x67,0xf0]
               {evex} vcvtne2ps2ph xmm22, xmm23, xmm24

// CHECK:      {evex} vcvtne2ps2ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x20,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtne2ps2ph ymm22, ymm23, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtne2ps2ph ymm22, ymm23, dword ptr [rip]{1to8}
// CHECK: encoding: [0x62,0xe2,0x45,0x30,0x67,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtne2ps2ph ymm22, ymm23, dword ptr [rip]{1to8}

// CHECK:      {evex} vcvtne2ps2ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x45,0x20,0x67,0x34,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtne2ps2ph ymm22, ymm23, ymmword ptr [2*rbp - 1024]

// CHECK:      {evex} vcvtne2ps2ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x45,0x00,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtne2ps2ph xmm22, xmm23, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtne2ps2ph xmm22, xmm23, dword ptr [rip]{1to4}
// CHECK: encoding: [0x62,0xe2,0x45,0x10,0x67,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtne2ps2ph xmm22, xmm23, dword ptr [rip]{1to4}

// CHECK:      {evex} vcvtne2ps2ph xmm22, xmm23, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x45,0x00,0x67,0x34,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtne2ps2ph xmm22, xmm23, xmmword ptr [2*rbp - 512]

// CHECK:      {evex} vcvtneebf162ps xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneebf162ps xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtneebf162ps xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x0f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneebf162ps xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vcvtneebf162ps xmm22, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneebf162ps xmm22, xmmword ptr [rip]

// CHECK:      {evex} vcvtneebf162ps xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xb0,0x34,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneebf162ps xmm22, xmmword ptr [2*rbp - 512]

// CHECK:      {evex} vcvtneebf162ps xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x7e,0x8f,0xb0,0x71,0x7f]
               {evex} vcvtneebf162ps xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK:      {evex} vcvtneebf162ps xmm22 {k7} {z}, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe2,0x7e,0x8f,0xb0,0x72,0x80]
               {evex} vcvtneebf162ps xmm22 {k7} {z}, xmmword ptr [rdx - 2048]

// CHECK:      {evex} vcvtneebf162ps ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x28,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneebf162ps ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtneebf162ps ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x2f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneebf162ps ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vcvtneebf162ps ymm22, ymmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneebf162ps ymm22, ymmword ptr [rip]

// CHECK:      {evex} vcvtneebf162ps ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xb0,0x34,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneebf162ps ymm22, ymmword ptr [2*rbp - 1024]

// CHECK:      {evex} vcvtneebf162ps ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x7e,0xaf,0xb0,0x71,0x7f]
               {evex} vcvtneebf162ps ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK:      {evex} vcvtneebf162ps ymm22 {k7} {z}, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe2,0x7e,0xaf,0xb0,0x72,0x80]
               {evex} vcvtneebf162ps ymm22 {k7} {z}, ymmword ptr [rdx - 4096]

// CHECK:      {evex} vcvtneeph2ps xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7d,0x08,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneeph2ps xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtneeph2ps xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7d,0x0f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneeph2ps xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vcvtneeph2ps xmm22, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7d,0x08,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneeph2ps xmm22, xmmword ptr [rip]

// CHECK:      {evex} vcvtneeph2ps xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x7d,0x08,0xb0,0x34,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneeph2ps xmm22, xmmword ptr [2*rbp - 512]

// CHECK:      {evex} vcvtneeph2ps xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x7d,0x8f,0xb0,0x71,0x7f]
               {evex} vcvtneeph2ps xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK:      {evex} vcvtneeph2ps xmm22 {k7} {z}, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe2,0x7d,0x8f,0xb0,0x72,0x80]
               {evex} vcvtneeph2ps xmm22 {k7} {z}, xmmword ptr [rdx - 2048]

// CHECK:      {evex} vcvtneeph2ps ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7d,0x28,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneeph2ps ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtneeph2ps ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7d,0x2f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneeph2ps ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vcvtneeph2ps ymm22, ymmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7d,0x28,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneeph2ps ymm22, ymmword ptr [rip]

// CHECK:      {evex} vcvtneeph2ps ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x7d,0x28,0xb0,0x34,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneeph2ps ymm22, ymmword ptr [2*rbp - 1024]

// CHECK:      {evex} vcvtneeph2ps ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x7d,0xaf,0xb0,0x71,0x7f]
               {evex} vcvtneeph2ps ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK:      {evex} vcvtneeph2ps ymm22 {k7} {z}, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe2,0x7d,0xaf,0xb0,0x72,0x80]
               {evex} vcvtneeph2ps ymm22 {k7} {z}, ymmword ptr [rdx - 4096]

// CHECK:      {evex} vcvtneobf162ps xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7f,0x08,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneobf162ps xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtneobf162ps xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7f,0x0f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneobf162ps xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vcvtneobf162ps xmm22, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7f,0x08,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneobf162ps xmm22, xmmword ptr [rip]

// CHECK:      {evex} vcvtneobf162ps xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x7f,0x08,0xb0,0x34,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneobf162ps xmm22, xmmword ptr [2*rbp - 512]

// CHECK:      {evex} vcvtneobf162ps xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x7f,0x8f,0xb0,0x71,0x7f]
               {evex} vcvtneobf162ps xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK:      {evex} vcvtneobf162ps xmm22 {k7} {z}, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe2,0x7f,0x8f,0xb0,0x72,0x80]
               {evex} vcvtneobf162ps xmm22 {k7} {z}, xmmword ptr [rdx - 2048]

// CHECK:      {evex} vcvtneobf162ps ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7f,0x28,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneobf162ps ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtneobf162ps ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7f,0x2f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneobf162ps ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vcvtneobf162ps ymm22, ymmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7f,0x28,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneobf162ps ymm22, ymmword ptr [rip]

// CHECK:      {evex} vcvtneobf162ps ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x7f,0x28,0xb0,0x34,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneobf162ps ymm22, ymmword ptr [2*rbp - 1024]

// CHECK:      {evex} vcvtneobf162ps ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x7f,0xaf,0xb0,0x71,0x7f]
               {evex} vcvtneobf162ps ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK:      {evex} vcvtneobf162ps ymm22 {k7} {z}, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe2,0x7f,0xaf,0xb0,0x72,0x80]
               {evex} vcvtneobf162ps ymm22 {k7} {z}, ymmword ptr [rdx - 4096]

// CHECK:      {evex} vcvtneoph2ps xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7c,0x08,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneoph2ps xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtneoph2ps xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7c,0x0f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneoph2ps xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vcvtneoph2ps xmm22, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7c,0x08,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneoph2ps xmm22, xmmword ptr [rip]

// CHECK:      {evex} vcvtneoph2ps xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x7c,0x08,0xb0,0x34,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneoph2ps xmm22, xmmword ptr [2*rbp - 512]

// CHECK:      {evex} vcvtneoph2ps xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x7c,0x8f,0xb0,0x71,0x7f]
               {evex} vcvtneoph2ps xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK:      {evex} vcvtneoph2ps xmm22 {k7} {z}, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe2,0x7c,0x8f,0xb0,0x72,0x80]
               {evex} vcvtneoph2ps xmm22 {k7} {z}, xmmword ptr [rdx - 2048]

// CHECK:      {evex} vcvtneoph2ps ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7c,0x28,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneoph2ps ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {evex} vcvtneoph2ps ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7c,0x2f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneoph2ps ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {evex} vcvtneoph2ps ymm22, ymmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7c,0x28,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneoph2ps ymm22, ymmword ptr [rip]

// CHECK:      {evex} vcvtneoph2ps ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x7c,0x28,0xb0,0x34,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneoph2ps ymm22, ymmword ptr [2*rbp - 1024]

// CHECK:      {evex} vcvtneoph2ps ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x7c,0xaf,0xb0,0x71,0x7f]
               {evex} vcvtneoph2ps ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK:      {evex} vcvtneoph2ps ymm22 {k7} {z}, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe2,0x7c,0xaf,0xb0,0x72,0x80]
               {evex} vcvtneoph2ps ymm22 {k7} {z}, ymmword ptr [rdx - 4096]

