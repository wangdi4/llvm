// REQUIRES: intel_feature_isa_avx512_movrs
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      {vex} vmovadvisew xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x7a,0x10,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vmovadvisew xmm12, xmmword ptr [r9], 123
// CHECK: encoding: [0xc4,0x43,0x7a,0x10,0x21,0x7b]
               vmovadvisew xmm12, xmmword ptr [r9], 123

// CHECK:      {vex} vmovadvisew xmm12, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc4,0x63,0x7a,0x10,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               vmovadvisew xmm12, xmmword ptr [rcx + 2032], 123

// CHECK:      {vex} vmovadvisew xmm12, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc4,0x63,0x7a,0x10,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               vmovadvisew xmm12, xmmword ptr [rdx - 2048], 123

// CHECK:      {vex} vmovadvisew ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x7e,0x10,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vmovadvisew ymm12, ymmword ptr [r9], 123
// CHECK: encoding: [0xc4,0x43,0x7e,0x10,0x21,0x7b]
               vmovadvisew ymm12, ymmword ptr [r9], 123

// CHECK:      {vex} vmovadvisew ymm12, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc4,0x63,0x7e,0x10,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               vmovadvisew ymm12, ymmword ptr [rcx + 4064], 123

// CHECK:      {vex} vmovadvisew ymm12, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc4,0x63,0x7e,0x10,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               vmovadvisew ymm12, ymmword ptr [rdx - 4096], 123

// CHECK:      {vex} vmovadvisew xmmword ptr [rbp + 8*r14 + 268435456], xmm12, 123
// CHECK: encoding: [0xc4,0x23,0x7a,0x11,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew xmmword ptr [rbp + 8*r14 + 268435456], xmm12, 123

// CHECK:      {vex} vmovadvisew xmmword ptr [r9], xmm12, 123
// CHECK: encoding: [0xc4,0x43,0x7a,0x11,0x21,0x7b]
               vmovadvisew xmmword ptr [r9], xmm12, 123

// CHECK:      {vex} vmovadvisew xmmword ptr [rcx + 2032], xmm12, 123
// CHECK: encoding: [0xc4,0x63,0x7a,0x11,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               vmovadvisew xmmword ptr [rcx + 2032], xmm12, 123

// CHECK:      {vex} vmovadvisew xmmword ptr [rdx - 2048], xmm12, 123
// CHECK: encoding: [0xc4,0x63,0x7a,0x11,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               vmovadvisew xmmword ptr [rdx - 2048], xmm12, 123

// CHECK:      {vex} vmovadvisew ymmword ptr [rbp + 8*r14 + 268435456], ymm12, 123
// CHECK: encoding: [0xc4,0x23,0x7e,0x11,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew ymmword ptr [rbp + 8*r14 + 268435456], ymm12, 123

// CHECK:      {vex} vmovadvisew ymmword ptr [r9], ymm12, 123
// CHECK: encoding: [0xc4,0x43,0x7e,0x11,0x21,0x7b]
               vmovadvisew ymmword ptr [r9], ymm12, 123

// CHECK:      {vex} vmovadvisew ymmword ptr [rcx + 4064], ymm12, 123
// CHECK: encoding: [0xc4,0x63,0x7e,0x11,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               vmovadvisew ymmword ptr [rcx + 4064], ymm12, 123

// CHECK:      {vex} vmovadvisew ymmword ptr [rdx - 4096], ymm12, 123
// CHECK: encoding: [0xc4,0x63,0x7e,0x11,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               vmovadvisew ymmword ptr [rdx - 4096], ymm12, 123

// CHECK:      {vex} vmemadvise xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0xa1,0x7b,0x71,0x84,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmemadvise xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vmemadvise xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0xc1,0x7b,0x71,0x84,0x80,0x23,0x01,0x00,0x00,0x7b]
               vmemadvise xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vmemadvise xmmword ptr [rip], 123
// CHECK: encoding: [0xc5,0xfb,0x71,0x05,0x00,0x00,0x00,0x00,0x7b]
               vmemadvise xmmword ptr [rip], 123

// CHECK:      {vex} vmemadvise xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0xc5,0xfb,0x71,0x04,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               vmemadvise xmmword ptr [2*rbp - 512], 123

// CHECK:      {vex} vmemadvise xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc5,0xfb,0x71,0x81,0xf0,0x07,0x00,0x00,0x7b]
               vmemadvise xmmword ptr [rcx + 2032], 123

// CHECK:      {vex} vmemadvise xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc5,0xfb,0x71,0x82,0x00,0xf8,0xff,0xff,0x7b]
               vmemadvise xmmword ptr [rdx - 2048], 123

// CHECK:      {vex} vmemadvise ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0xc5,0xff,0x71,0x04,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               vmemadvise ymmword ptr [2*rbp - 1024], 123

// CHECK:      {vex} vmemadvise ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc5,0xff,0x71,0x81,0xe0,0x0f,0x00,0x00,0x7b]
               vmemadvise ymmword ptr [rcx + 4064], 123

// CHECK:      {vex} vmemadvise ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc5,0xff,0x71,0x82,0x00,0xf0,0xff,0xff,0x7b]
               vmemadvise ymmword ptr [rdx - 4096], 123

// CHECK: vmovrsb xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovrsb xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovrsb xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x0f,0x6f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovrsb xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vmovrsb xmm22, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7f,0x08,0x6f,0x35,0x00,0x00,0x00,0x00]
          vmovrsb xmm22, xmmword ptr [rip]

// CHECK: vmovrsb xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7f,0x08,0x6f,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vmovrsb xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vmovrsb xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7f,0x8f,0x6f,0x71,0x7f]
          vmovrsb xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vmovrsb xmm22 {k7} {z}, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe5,0x7f,0x8f,0x6f,0x72,0x80]
          vmovrsb xmm22 {k7} {z}, xmmword ptr [rdx - 2048]

// CHECK: vmovrsb ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7f,0x28,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovrsb ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovrsb ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7f,0x2f,0x6f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovrsb ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vmovrsb ymm22, ymmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7f,0x28,0x6f,0x35,0x00,0x00,0x00,0x00]
          vmovrsb ymm22, ymmword ptr [rip]

// CHECK: vmovrsb ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7f,0x28,0x6f,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vmovrsb ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vmovrsb ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7f,0xaf,0x6f,0x71,0x7f]
          vmovrsb ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vmovrsb ymm22 {k7} {z}, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe5,0x7f,0xaf,0x6f,0x72,0x80]
          vmovrsb ymm22 {k7} {z}, ymmword ptr [rdx - 4096]

// CHECK: vmovrsd xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovrsd xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovrsd xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x6f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovrsd xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vmovrsd xmm22, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x6f,0x35,0x00,0x00,0x00,0x00]
          vmovrsd xmm22, xmmword ptr [rip]

// CHECK: vmovrsd xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x6f,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vmovrsd xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vmovrsd xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x6f,0x71,0x7f]
          vmovrsd xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vmovrsd xmm22 {k7} {z}, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x6f,0x72,0x80]
          vmovrsd xmm22 {k7} {z}, xmmword ptr [rdx - 2048]

// CHECK: vmovrsd ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovrsd ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovrsd ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0x7e,0x2f,0x6f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovrsd ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vmovrsd ymm22, ymmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x6f,0x35,0x00,0x00,0x00,0x00]
          vmovrsd ymm22, ymmword ptr [rip]

// CHECK: vmovrsd ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x6f,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vmovrsd ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vmovrsd ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x6f,0x71,0x7f]
          vmovrsd ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vmovrsd ymm22 {k7} {z}, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x6f,0x72,0x80]
          vmovrsd ymm22 {k7} {z}, ymmword ptr [rdx - 4096]

// CHECK: vmovrsq xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfe,0x08,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovrsq xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovrsq xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfe,0x0f,0x6f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovrsq xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vmovrsq xmm22, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0xfe,0x08,0x6f,0x35,0x00,0x00,0x00,0x00]
          vmovrsq xmm22, xmmword ptr [rip]

// CHECK: vmovrsq xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xfe,0x08,0x6f,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vmovrsq xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vmovrsq xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xfe,0x8f,0x6f,0x71,0x7f]
          vmovrsq xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vmovrsq xmm22 {k7} {z}, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe5,0xfe,0x8f,0x6f,0x72,0x80]
          vmovrsq xmm22 {k7} {z}, xmmword ptr [rdx - 2048]

// CHECK: vmovrsq ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xfe,0x28,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovrsq ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovrsq ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xfe,0x2f,0x6f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovrsq ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vmovrsq ymm22, ymmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0xfe,0x28,0x6f,0x35,0x00,0x00,0x00,0x00]
          vmovrsq ymm22, ymmword ptr [rip]

// CHECK: vmovrsq ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xfe,0x28,0x6f,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vmovrsq ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vmovrsq ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xfe,0xaf,0x6f,0x71,0x7f]
          vmovrsq ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vmovrsq ymm22 {k7} {z}, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe5,0xfe,0xaf,0x6f,0x72,0x80]
          vmovrsq ymm22 {k7} {z}, ymmword ptr [rdx - 4096]

// CHECK: vmovrsw xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xff,0x08,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovrsw xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovrsw xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xff,0x0f,0x6f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovrsw xmm22 {k7}, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vmovrsw xmm22, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0xff,0x08,0x6f,0x35,0x00,0x00,0x00,0x00]
          vmovrsw xmm22, xmmword ptr [rip]

// CHECK: vmovrsw xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe5,0xff,0x08,0x6f,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vmovrsw xmm22, xmmword ptr [2*rbp - 512]

// CHECK: vmovrsw xmm22 {k7} {z}, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe5,0xff,0x8f,0x6f,0x71,0x7f]
          vmovrsw xmm22 {k7} {z}, xmmword ptr [rcx + 2032]

// CHECK: vmovrsw xmm22 {k7} {z}, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe5,0xff,0x8f,0x6f,0x72,0x80]
          vmovrsw xmm22 {k7} {z}, xmmword ptr [rdx - 2048]

// CHECK: vmovrsw ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa5,0xff,0x28,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovrsw ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vmovrsw ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc5,0xff,0x2f,0x6f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovrsw ymm22 {k7}, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vmovrsw ymm22, ymmword ptr [rip]
// CHECK: encoding: [0x62,0xe5,0xff,0x28,0x6f,0x35,0x00,0x00,0x00,0x00]
          vmovrsw ymm22, ymmword ptr [rip]

// CHECK: vmovrsw ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe5,0xff,0x28,0x6f,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vmovrsw ymm22, ymmword ptr [2*rbp - 1024]

// CHECK: vmovrsw ymm22 {k7} {z}, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe5,0xff,0xaf,0x6f,0x71,0x7f]
          vmovrsw ymm22 {k7} {z}, ymmword ptr [rcx + 4064]

// CHECK: vmovrsw ymm22 {k7} {z}, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe5,0xff,0xaf,0x6f,0x72,0x80]
          vmovrsw ymm22 {k7} {z}, ymmword ptr [rdx - 4096]
