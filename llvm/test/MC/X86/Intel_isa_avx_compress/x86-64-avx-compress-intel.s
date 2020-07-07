// REQUIRES: intel_feature_isa_avx_compress
// RUN: llvm-mc -triple x86_64-unknown-unknown -mattr=+avxcompress -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vpcompressb ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x0d,0x63,0xec]
               vpcompressb ymm12, ymm13, ymm14

// CHECK:      vpcompressb xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x09,0x63,0xec]
               vpcompressb xmm12, xmm13, xmm14

// CHECK:      vpcompressb xmmword ptr [rbp + 8*r14 + 268435456], xmm12, xmm13
// CHECK: encoding: [0xc4,0x22,0x11,0x63,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressb xmmword ptr [rbp + 8*r14 + 268435456], xmm12, xmm13

// CHECK:      vpcompressb xmmword ptr [r8 + 4*rax + 291], xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0x11,0x63,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressb xmmword ptr [r8 + 4*rax + 291], xmm12, xmm13

// CHECK:      vpcompressb xmmword ptr [rip], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x11,0x63,0x25,0x00,0x00,0x00,0x00]
               vpcompressb xmmword ptr [rip], xmm12, xmm13

// CHECK:      vpcompressb xmmword ptr [2*rbp - 512], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x11,0x63,0x24,0x6d,0x00,0xfe,0xff,0xff]
               vpcompressb xmmword ptr [2*rbp - 512], xmm12, xmm13

// CHECK:      vpcompressb xmmword ptr [rcx + 2032], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x11,0x63,0xa1,0xf0,0x07,0x00,0x00]
               vpcompressb xmmword ptr [rcx + 2032], xmm12, xmm13

// CHECK:      vpcompressb xmmword ptr [rdx - 2048], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x11,0x63,0xa2,0x00,0xf8,0xff,0xff]
               vpcompressb xmmword ptr [rdx - 2048], xmm12, xmm13

// CHECK:      vpcompressb ymmword ptr [rbp + 8*r14 + 268435456], ymm12, ymm13
// CHECK: encoding: [0xc4,0x22,0x15,0x63,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressb ymmword ptr [rbp + 8*r14 + 268435456], ymm12, ymm13

// CHECK:      vpcompressb ymmword ptr [r8 + 4*rax + 291], ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0x15,0x63,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressb ymmword ptr [r8 + 4*rax + 291], ymm12, ymm13

// CHECK:      vpcompressb ymmword ptr [rip], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x15,0x63,0x25,0x00,0x00,0x00,0x00]
               vpcompressb ymmword ptr [rip], ymm12, ymm13

// CHECK:      vpcompressb ymmword ptr [2*rbp - 1024], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x15,0x63,0x24,0x6d,0x00,0xfc,0xff,0xff]
               vpcompressb ymmword ptr [2*rbp - 1024], ymm12, ymm13

// CHECK:      vpcompressb ymmword ptr [rcx + 4064], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x15,0x63,0xa1,0xe0,0x0f,0x00,0x00]
               vpcompressb ymmword ptr [rcx + 4064], ymm12, ymm13

// CHECK:      vpcompressb ymmword ptr [rdx - 4096], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x15,0x63,0xa2,0x00,0xf0,0xff,0xff]
               vpcompressb ymmword ptr [rdx - 4096], ymm12, ymm13

// CHECK:      vpcompressd ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x0d,0x8b,0xec]
               vpcompressd ymm12, ymm13, ymm14

// CHECK:      vpcompressd xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x09,0x8b,0xec]
               vpcompressd xmm12, xmm13, xmm14

// CHECK:      vpcompressd xmmword ptr [rbp + 8*r14 + 268435456], xmm12, xmm13
// CHECK: encoding: [0xc4,0x22,0x11,0x8b,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressd xmmword ptr [rbp + 8*r14 + 268435456], xmm12, xmm13

// CHECK:      vpcompressd xmmword ptr [r8 + 4*rax + 291], xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0x11,0x8b,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressd xmmword ptr [r8 + 4*rax + 291], xmm12, xmm13

// CHECK:      vpcompressd xmmword ptr [rip], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x11,0x8b,0x25,0x00,0x00,0x00,0x00]
               vpcompressd xmmword ptr [rip], xmm12, xmm13

// CHECK:      vpcompressd xmmword ptr [2*rbp - 512], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x11,0x8b,0x24,0x6d,0x00,0xfe,0xff,0xff]
               vpcompressd xmmword ptr [2*rbp - 512], xmm12, xmm13

// CHECK:      vpcompressd xmmword ptr [rcx + 2032], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x11,0x8b,0xa1,0xf0,0x07,0x00,0x00]
               vpcompressd xmmword ptr [rcx + 2032], xmm12, xmm13

// CHECK:      vpcompressd xmmword ptr [rdx - 2048], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x11,0x8b,0xa2,0x00,0xf8,0xff,0xff]
               vpcompressd xmmword ptr [rdx - 2048], xmm12, xmm13

// CHECK:      vpcompressd ymmword ptr [rbp + 8*r14 + 268435456], ymm12, ymm13
// CHECK: encoding: [0xc4,0x22,0x15,0x8b,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressd ymmword ptr [rbp + 8*r14 + 268435456], ymm12, ymm13

// CHECK:      vpcompressd ymmword ptr [r8 + 4*rax + 291], ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0x15,0x8b,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressd ymmword ptr [r8 + 4*rax + 291], ymm12, ymm13

// CHECK:      vpcompressd ymmword ptr [rip], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x15,0x8b,0x25,0x00,0x00,0x00,0x00]
               vpcompressd ymmword ptr [rip], ymm12, ymm13

// CHECK:      vpcompressd ymmword ptr [2*rbp - 1024], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x15,0x8b,0x24,0x6d,0x00,0xfc,0xff,0xff]
               vpcompressd ymmword ptr [2*rbp - 1024], ymm12, ymm13

// CHECK:      vpcompressd ymmword ptr [rcx + 4064], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x15,0x8b,0xa1,0xe0,0x0f,0x00,0x00]
               vpcompressd ymmword ptr [rcx + 4064], ymm12, ymm13

// CHECK:      vpcompressd ymmword ptr [rdx - 4096], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x15,0x8b,0xa2,0x00,0xf0,0xff,0xff]
               vpcompressd ymmword ptr [rdx - 4096], ymm12, ymm13

// CHECK:      vpcompressq ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x8d,0x8b,0xec]
               vpcompressq ymm12, ymm13, ymm14

// CHECK:      vpcompressq xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x89,0x8b,0xec]
               vpcompressq xmm12, xmm13, xmm14

// CHECK:      vpcompressq xmmword ptr [rbp + 8*r14 + 268435456], xmm12, xmm13
// CHECK: encoding: [0xc4,0x22,0x91,0x8b,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressq xmmword ptr [rbp + 8*r14 + 268435456], xmm12, xmm13

// CHECK:      vpcompressq xmmword ptr [r8 + 4*rax + 291], xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0x91,0x8b,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressq xmmword ptr [r8 + 4*rax + 291], xmm12, xmm13

// CHECK:      vpcompressq xmmword ptr [rip], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x91,0x8b,0x25,0x00,0x00,0x00,0x00]
               vpcompressq xmmword ptr [rip], xmm12, xmm13

// CHECK:      vpcompressq xmmword ptr [2*rbp - 512], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x91,0x8b,0x24,0x6d,0x00,0xfe,0xff,0xff]
               vpcompressq xmmword ptr [2*rbp - 512], xmm12, xmm13

// CHECK:      vpcompressq xmmword ptr [rcx + 2032], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x91,0x8b,0xa1,0xf0,0x07,0x00,0x00]
               vpcompressq xmmword ptr [rcx + 2032], xmm12, xmm13

// CHECK:      vpcompressq xmmword ptr [rdx - 2048], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x91,0x8b,0xa2,0x00,0xf8,0xff,0xff]
               vpcompressq xmmword ptr [rdx - 2048], xmm12, xmm13

// CHECK:      vpcompressq ymmword ptr [rbp + 8*r14 + 268435456], ymm12, ymm13
// CHECK: encoding: [0xc4,0x22,0x95,0x8b,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressq ymmword ptr [rbp + 8*r14 + 268435456], ymm12, ymm13

// CHECK:      vpcompressq ymmword ptr [r8 + 4*rax + 291], ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0x95,0x8b,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressq ymmword ptr [r8 + 4*rax + 291], ymm12, ymm13

// CHECK:      vpcompressq ymmword ptr [rip], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x95,0x8b,0x25,0x00,0x00,0x00,0x00]
               vpcompressq ymmword ptr [rip], ymm12, ymm13

// CHECK:      vpcompressq ymmword ptr [2*rbp - 1024], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x95,0x8b,0x24,0x6d,0x00,0xfc,0xff,0xff]
               vpcompressq ymmword ptr [2*rbp - 1024], ymm12, ymm13

// CHECK:      vpcompressq ymmword ptr [rcx + 4064], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x95,0x8b,0xa1,0xe0,0x0f,0x00,0x00]
               vpcompressq ymmword ptr [rcx + 4064], ymm12, ymm13

// CHECK:      vpcompressq ymmword ptr [rdx - 4096], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x95,0x8b,0xa2,0x00,0xf0,0xff,0xff]
               vpcompressq ymmword ptr [rdx - 4096], ymm12, ymm13

// CHECK:      vpcompressw ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x8d,0x63,0xec]
               vpcompressw ymm12, ymm13, ymm14

// CHECK:      vpcompressw xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x89,0x63,0xec]
               vpcompressw xmm12, xmm13, xmm14

// CHECK:      vpcompressw xmmword ptr [rbp + 8*r14 + 268435456], xmm12, xmm13
// CHECK: encoding: [0xc4,0x22,0x91,0x63,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressw xmmword ptr [rbp + 8*r14 + 268435456], xmm12, xmm13

// CHECK:      vpcompressw xmmword ptr [r8 + 4*rax + 291], xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0x91,0x63,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressw xmmword ptr [r8 + 4*rax + 291], xmm12, xmm13

// CHECK:      vpcompressw xmmword ptr [rip], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x91,0x63,0x25,0x00,0x00,0x00,0x00]
               vpcompressw xmmword ptr [rip], xmm12, xmm13

// CHECK:      vpcompressw xmmword ptr [2*rbp - 512], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x91,0x63,0x24,0x6d,0x00,0xfe,0xff,0xff]
               vpcompressw xmmword ptr [2*rbp - 512], xmm12, xmm13

// CHECK:      vpcompressw xmmword ptr [rcx + 2032], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x91,0x63,0xa1,0xf0,0x07,0x00,0x00]
               vpcompressw xmmword ptr [rcx + 2032], xmm12, xmm13

// CHECK:      vpcompressw xmmword ptr [rdx - 2048], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x91,0x63,0xa2,0x00,0xf8,0xff,0xff]
               vpcompressw xmmword ptr [rdx - 2048], xmm12, xmm13

// CHECK:      vpcompressw ymmword ptr [rbp + 8*r14 + 268435456], ymm12, ymm13
// CHECK: encoding: [0xc4,0x22,0x95,0x63,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressw ymmword ptr [rbp + 8*r14 + 268435456], ymm12, ymm13

// CHECK:      vpcompressw ymmword ptr [r8 + 4*rax + 291], ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0x95,0x63,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressw ymmword ptr [r8 + 4*rax + 291], ymm12, ymm13

// CHECK:      vpcompressw ymmword ptr [rip], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x95,0x63,0x25,0x00,0x00,0x00,0x00]
               vpcompressw ymmword ptr [rip], ymm12, ymm13

// CHECK:      vpcompressw ymmword ptr [2*rbp - 1024], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x95,0x63,0x24,0x6d,0x00,0xfc,0xff,0xff]
               vpcompressw ymmword ptr [2*rbp - 1024], ymm12, ymm13

// CHECK:      vpcompressw ymmword ptr [rcx + 4064], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x95,0x63,0xa1,0xe0,0x0f,0x00,0x00]
               vpcompressw ymmword ptr [rcx + 4064], ymm12, ymm13

// CHECK:      vpcompressw ymmword ptr [rdx - 4096], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x95,0x63,0xa2,0x00,0xf0,0xff,0xff]
               vpcompressw ymmword ptr [rdx - 4096], ymm12, ymm13

// CHECK:      {vex} vplzcntd xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0x79,0x44,0xe5]
               {vex} vplzcntd xmm12, xmm13

// CHECK:      {vex} vplzcntd ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0x7d,0x44,0xe5]
               {vex} vplzcntd ymm12, ymm13

// CHECK:      {vex} vplzcntd xmm12, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x79,0x44,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vplzcntd xmm12, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vplzcntd xmm12, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x79,0x44,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vplzcntd xmm12, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vplzcntd xmm12, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x79,0x44,0x25,0x00,0x00,0x00,0x00]
               {vex} vplzcntd xmm12, xmmword ptr [rip]

// CHECK:      {vex} vplzcntd xmm12, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x79,0x44,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vplzcntd xmm12, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vplzcntd xmm12, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x79,0x44,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vplzcntd xmm12, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vplzcntd xmm12, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x79,0x44,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vplzcntd xmm12, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vplzcntd ymm12, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x7d,0x44,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vplzcntd ymm12, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vplzcntd ymm12, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x7d,0x44,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vplzcntd ymm12, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vplzcntd ymm12, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x7d,0x44,0x25,0x00,0x00,0x00,0x00]
               {vex} vplzcntd ymm12, ymmword ptr [rip]

// CHECK:      {vex} vplzcntd ymm12, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x7d,0x44,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vplzcntd ymm12, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vplzcntd ymm12, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x7d,0x44,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vplzcntd ymm12, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vplzcntd ymm12, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x7d,0x44,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vplzcntd ymm12, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vplzcntq xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0xf9,0x44,0xe5]
               {vex} vplzcntq xmm12, xmm13

// CHECK:      {vex} vplzcntq ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0xfd,0x44,0xe5]
               {vex} vplzcntq ymm12, ymm13

// CHECK:      {vex} vplzcntq xmm12, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0xf9,0x44,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vplzcntq xmm12, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vplzcntq xmm12, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0xf9,0x44,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vplzcntq xmm12, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vplzcntq xmm12, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0xf9,0x44,0x25,0x00,0x00,0x00,0x00]
               {vex} vplzcntq xmm12, xmmword ptr [rip]

// CHECK:      {vex} vplzcntq xmm12, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0xf9,0x44,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vplzcntq xmm12, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vplzcntq xmm12, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0xf9,0x44,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vplzcntq xmm12, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vplzcntq xmm12, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0xf9,0x44,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vplzcntq xmm12, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vplzcntq ymm12, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0xfd,0x44,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vplzcntq ymm12, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vplzcntq ymm12, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0xfd,0x44,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vplzcntq ymm12, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vplzcntq ymm12, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0xfd,0x44,0x25,0x00,0x00,0x00,0x00]
               {vex} vplzcntq ymm12, ymmword ptr [rip]

// CHECK:      {vex} vplzcntq ymm12, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0xfd,0x44,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vplzcntq ymm12, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vplzcntq ymm12, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0xfd,0x44,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vplzcntq ymm12, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vplzcntq ymm12, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0xfd,0x44,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vplzcntq ymm12, ymmword ptr [rdx - 4096]

// CHECK:      vpmaskmovb ymmword ptr [rbp + 8*r14 + 268435456], ymm12, ymm13
// CHECK: encoding: [0xc4,0x22,0x1c,0x8e,0xac,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovb ymmword ptr [rbp + 8*r14 + 268435456], ymm12, ymm13

// CHECK:      vpmaskmovb ymmword ptr [r8 + 4*rax + 291], ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0x1c,0x8e,0xac,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovb ymmword ptr [r8 + 4*rax + 291], ymm12, ymm13

// CHECK:      vpmaskmovb ymmword ptr [rip], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x1c,0x8e,0x2d,0x00,0x00,0x00,0x00]
               vpmaskmovb ymmword ptr [rip], ymm12, ymm13

// CHECK:      vpmaskmovb ymmword ptr [2*rbp - 1024], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x1c,0x8e,0x2c,0x6d,0x00,0xfc,0xff,0xff]
               vpmaskmovb ymmword ptr [2*rbp - 1024], ymm12, ymm13

// CHECK:      vpmaskmovb ymmword ptr [rcx + 4064], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x1c,0x8e,0xa9,0xe0,0x0f,0x00,0x00]
               vpmaskmovb ymmword ptr [rcx + 4064], ymm12, ymm13

// CHECK:      vpmaskmovb ymmword ptr [rdx - 4096], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x1c,0x8e,0xaa,0x00,0xf0,0xff,0xff]
               vpmaskmovb ymmword ptr [rdx - 4096], ymm12, ymm13

// CHECK:      vpmaskmovb xmmword ptr [rbp + 8*r14 + 268435456], xmm12, xmm13
// CHECK: encoding: [0xc4,0x22,0x18,0x8e,0xac,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovb xmmword ptr [rbp + 8*r14 + 268435456], xmm12, xmm13

// CHECK:      vpmaskmovb xmmword ptr [r8 + 4*rax + 291], xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0x18,0x8e,0xac,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovb xmmword ptr [r8 + 4*rax + 291], xmm12, xmm13

// CHECK:      vpmaskmovb xmmword ptr [rip], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x18,0x8e,0x2d,0x00,0x00,0x00,0x00]
               vpmaskmovb xmmword ptr [rip], xmm12, xmm13

// CHECK:      vpmaskmovb xmmword ptr [2*rbp - 512], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x18,0x8e,0x2c,0x6d,0x00,0xfe,0xff,0xff]
               vpmaskmovb xmmword ptr [2*rbp - 512], xmm12, xmm13

// CHECK:      vpmaskmovb xmmword ptr [rcx + 2032], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x18,0x8e,0xa9,0xf0,0x07,0x00,0x00]
               vpmaskmovb xmmword ptr [rcx + 2032], xmm12, xmm13

// CHECK:      vpmaskmovb xmmword ptr [rdx - 2048], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x18,0x8e,0xaa,0x00,0xf8,0xff,0xff]
               vpmaskmovb xmmword ptr [rdx - 2048], xmm12, xmm13

// CHECK:      vpmaskmovb ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x14,0x8c,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovb ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpmaskmovb ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x14,0x8c,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovb ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpmaskmovb ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x14,0x8c,0x25,0x00,0x00,0x00,0x00]
               vpmaskmovb ymm12, ymm13, ymmword ptr [rip]

// CHECK:      vpmaskmovb ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x14,0x8c,0x24,0x6d,0x00,0xfc,0xff,0xff]
               vpmaskmovb ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:      vpmaskmovb ymm12, ymm13, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x14,0x8c,0xa1,0xe0,0x0f,0x00,0x00]
               vpmaskmovb ymm12, ymm13, ymmword ptr [rcx + 4064]

// CHECK:      vpmaskmovb ymm12, ymm13, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x14,0x8c,0xa2,0x00,0xf0,0xff,0xff]
               vpmaskmovb ymm12, ymm13, ymmword ptr [rdx - 4096]

// CHECK:      vpmaskmovb xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x10,0x8c,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovb xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpmaskmovb xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x10,0x8c,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovb xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpmaskmovb xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x10,0x8c,0x25,0x00,0x00,0x00,0x00]
               vpmaskmovb xmm12, xmm13, xmmword ptr [rip]

// CHECK:      vpmaskmovb xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x10,0x8c,0x24,0x6d,0x00,0xfe,0xff,0xff]
               vpmaskmovb xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:      vpmaskmovb xmm12, xmm13, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x10,0x8c,0xa1,0xf0,0x07,0x00,0x00]
               vpmaskmovb xmm12, xmm13, xmmword ptr [rcx + 2032]

// CHECK:      vpmaskmovb xmm12, xmm13, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x10,0x8c,0xa2,0x00,0xf8,0xff,0xff]
               vpmaskmovb xmm12, xmm13, xmmword ptr [rdx - 2048]

// CHECK:      vpmaskmovw ymmword ptr [rbp + 8*r14 + 268435456], ymm12, ymm13
// CHECK: encoding: [0xc4,0x22,0x9c,0x8e,0xac,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovw ymmword ptr [rbp + 8*r14 + 268435456], ymm12, ymm13

// CHECK:      vpmaskmovw ymmword ptr [r8 + 4*rax + 291], ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0x9c,0x8e,0xac,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovw ymmword ptr [r8 + 4*rax + 291], ymm12, ymm13

// CHECK:      vpmaskmovw ymmword ptr [rip], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x9c,0x8e,0x2d,0x00,0x00,0x00,0x00]
               vpmaskmovw ymmword ptr [rip], ymm12, ymm13

// CHECK:      vpmaskmovw ymmword ptr [2*rbp - 1024], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x9c,0x8e,0x2c,0x6d,0x00,0xfc,0xff,0xff]
               vpmaskmovw ymmword ptr [2*rbp - 1024], ymm12, ymm13

// CHECK:      vpmaskmovw ymmword ptr [rcx + 4064], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x9c,0x8e,0xa9,0xe0,0x0f,0x00,0x00]
               vpmaskmovw ymmword ptr [rcx + 4064], ymm12, ymm13

// CHECK:      vpmaskmovw ymmword ptr [rdx - 4096], ymm12, ymm13
// CHECK: encoding: [0xc4,0x62,0x9c,0x8e,0xaa,0x00,0xf0,0xff,0xff]
               vpmaskmovw ymmword ptr [rdx - 4096], ymm12, ymm13

// CHECK:      vpmaskmovw xmmword ptr [rbp + 8*r14 + 268435456], xmm12, xmm13
// CHECK: encoding: [0xc4,0x22,0x98,0x8e,0xac,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovw xmmword ptr [rbp + 8*r14 + 268435456], xmm12, xmm13

// CHECK:      vpmaskmovw xmmword ptr [r8 + 4*rax + 291], xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0x98,0x8e,0xac,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovw xmmword ptr [r8 + 4*rax + 291], xmm12, xmm13

// CHECK:      vpmaskmovw xmmword ptr [rip], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x98,0x8e,0x2d,0x00,0x00,0x00,0x00]
               vpmaskmovw xmmword ptr [rip], xmm12, xmm13

// CHECK:      vpmaskmovw xmmword ptr [2*rbp - 512], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x98,0x8e,0x2c,0x6d,0x00,0xfe,0xff,0xff]
               vpmaskmovw xmmword ptr [2*rbp - 512], xmm12, xmm13

// CHECK:      vpmaskmovw xmmword ptr [rcx + 2032], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x98,0x8e,0xa9,0xf0,0x07,0x00,0x00]
               vpmaskmovw xmmword ptr [rcx + 2032], xmm12, xmm13

// CHECK:      vpmaskmovw xmmword ptr [rdx - 2048], xmm12, xmm13
// CHECK: encoding: [0xc4,0x62,0x98,0x8e,0xaa,0x00,0xf8,0xff,0xff]
               vpmaskmovw xmmword ptr [rdx - 2048], xmm12, xmm13

// CHECK:      vpmaskmovw ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x94,0x8c,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovw ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpmaskmovw ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x94,0x8c,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovw ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vpmaskmovw ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x94,0x8c,0x25,0x00,0x00,0x00,0x00]
               vpmaskmovw ymm12, ymm13, ymmword ptr [rip]

// CHECK:      vpmaskmovw ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x94,0x8c,0x24,0x6d,0x00,0xfc,0xff,0xff]
               vpmaskmovw ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:      vpmaskmovw ymm12, ymm13, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x94,0x8c,0xa1,0xe0,0x0f,0x00,0x00]
               vpmaskmovw ymm12, ymm13, ymmword ptr [rcx + 4064]

// CHECK:      vpmaskmovw ymm12, ymm13, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x94,0x8c,0xa2,0x00,0xf0,0xff,0xff]
               vpmaskmovw ymm12, ymm13, ymmword ptr [rdx - 4096]

// CHECK:      vpmaskmovw xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x90,0x8c,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovw xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vpmaskmovw xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x90,0x8c,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovw xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vpmaskmovw xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x90,0x8c,0x25,0x00,0x00,0x00,0x00]
               vpmaskmovw xmm12, xmm13, xmmword ptr [rip]

// CHECK:      vpmaskmovw xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x90,0x8c,0x24,0x6d,0x00,0xfe,0xff,0xff]
               vpmaskmovw xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:      vpmaskmovw xmm12, xmm13, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x90,0x8c,0xa1,0xf0,0x07,0x00,0x00]
               vpmaskmovw xmm12, xmm13, xmmword ptr [rcx + 2032]

// CHECK:      vpmaskmovw xmm12, xmm13, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x90,0x8c,0xa2,0x00,0xf8,0xff,0xff]
               vpmaskmovw xmm12, xmm13, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vpopcntb xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0x79,0x54,0xe5]
               {vex} vpopcntb xmm12, xmm13

// CHECK:      {vex} vpopcntb ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0x7d,0x54,0xe5]
               {vex} vpopcntb ymm12, ymm13

// CHECK:      {vex} vpopcntb xmm12, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x79,0x54,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntb xmm12, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpopcntb xmm12, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x79,0x54,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntb xmm12, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpopcntb xmm12, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x79,0x54,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntb xmm12, xmmword ptr [rip]

// CHECK:      {vex} vpopcntb xmm12, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x79,0x54,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpopcntb xmm12, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vpopcntb xmm12, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x79,0x54,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpopcntb xmm12, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vpopcntb xmm12, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x79,0x54,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpopcntb xmm12, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vpopcntb ymm12, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x7d,0x54,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntb ymm12, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpopcntb ymm12, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x7d,0x54,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntb ymm12, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpopcntb ymm12, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x7d,0x54,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntb ymm12, ymmword ptr [rip]

// CHECK:      {vex} vpopcntb ymm12, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x7d,0x54,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpopcntb ymm12, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vpopcntb ymm12, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x7d,0x54,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpopcntb ymm12, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vpopcntb ymm12, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x7d,0x54,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpopcntb ymm12, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vpopcntd xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0x79,0x55,0xe5]
               {vex} vpopcntd xmm12, xmm13

// CHECK:      {vex} vpopcntd ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0x7d,0x55,0xe5]
               {vex} vpopcntd ymm12, ymm13

// CHECK:      {vex} vpopcntd xmm12, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x79,0x55,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntd xmm12, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpopcntd xmm12, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x79,0x55,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntd xmm12, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpopcntd xmm12, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x79,0x55,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntd xmm12, xmmword ptr [rip]

// CHECK:      {vex} vpopcntd xmm12, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x79,0x55,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpopcntd xmm12, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vpopcntd xmm12, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x79,0x55,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpopcntd xmm12, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vpopcntd xmm12, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x79,0x55,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpopcntd xmm12, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vpopcntd ymm12, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x7d,0x55,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntd ymm12, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpopcntd ymm12, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x7d,0x55,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntd ymm12, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpopcntd ymm12, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x7d,0x55,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntd ymm12, ymmword ptr [rip]

// CHECK:      {vex} vpopcntd ymm12, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x7d,0x55,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpopcntd ymm12, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vpopcntd ymm12, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x7d,0x55,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpopcntd ymm12, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vpopcntd ymm12, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x7d,0x55,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpopcntd ymm12, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vpopcntq xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0xf9,0x55,0xe5]
               {vex} vpopcntq xmm12, xmm13

// CHECK:      {vex} vpopcntq ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0xfd,0x55,0xe5]
               {vex} vpopcntq ymm12, ymm13

// CHECK:      {vex} vpopcntq xmm12, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0xf9,0x55,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntq xmm12, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpopcntq xmm12, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0xf9,0x55,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntq xmm12, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpopcntq xmm12, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0xf9,0x55,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntq xmm12, xmmword ptr [rip]

// CHECK:      {vex} vpopcntq xmm12, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0xf9,0x55,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpopcntq xmm12, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vpopcntq xmm12, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0xf9,0x55,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpopcntq xmm12, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vpopcntq xmm12, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0xf9,0x55,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpopcntq xmm12, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vpopcntq ymm12, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0xfd,0x55,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntq ymm12, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpopcntq ymm12, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0xfd,0x55,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntq ymm12, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpopcntq ymm12, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0xfd,0x55,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntq ymm12, ymmword ptr [rip]

// CHECK:      {vex} vpopcntq ymm12, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0xfd,0x55,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpopcntq ymm12, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vpopcntq ymm12, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0xfd,0x55,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpopcntq ymm12, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vpopcntq ymm12, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0xfd,0x55,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpopcntq ymm12, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vpopcntw xmm12, xmm13
// CHECK: encoding: [0xc4,0x42,0xf9,0x54,0xe5]
               {vex} vpopcntw xmm12, xmm13

// CHECK:      {vex} vpopcntw ymm12, ymm13
// CHECK: encoding: [0xc4,0x42,0xfd,0x54,0xe5]
               {vex} vpopcntw ymm12, ymm13

// CHECK:      {vex} vpopcntw xmm12, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0xf9,0x54,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntw xmm12, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpopcntw xmm12, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0xf9,0x54,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntw xmm12, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpopcntw xmm12, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0xf9,0x54,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntw xmm12, xmmword ptr [rip]

// CHECK:      {vex} vpopcntw xmm12, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0xf9,0x54,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpopcntw xmm12, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vpopcntw xmm12, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0xf9,0x54,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpopcntw xmm12, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vpopcntw xmm12, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0xf9,0x54,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpopcntw xmm12, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vpopcntw ymm12, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0xfd,0x54,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntw ymm12, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpopcntw ymm12, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0xfd,0x54,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntw ymm12, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpopcntw ymm12, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0xfd,0x54,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntw ymm12, ymmword ptr [rip]

// CHECK:      {vex} vpopcntw ymm12, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0xfd,0x54,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpopcntw ymm12, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vpopcntw ymm12, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0xfd,0x54,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpopcntw ymm12, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vpopcntw ymm12, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0xfd,0x54,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpopcntw ymm12, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vprold ymm12, ymm13, 123
// CHECK: encoding: [0xc4,0xc1,0x1d,0x72,0xcd,0x7b]
               {vex} vprold ymm12, ymm13, 123

// CHECK:      {vex} vprold xmm12, xmm13, 123
// CHECK: encoding: [0xc4,0xc1,0x19,0x72,0xcd,0x7b]
               {vex} vprold xmm12, xmm13, 123

// CHECK:      {vex} vprold xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0xa1,0x19,0x72,0x8c,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprold xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vprold xmm12, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0xc1,0x19,0x72,0x8c,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprold xmm12, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vprold xmm12, xmmword ptr [rip], 123
// CHECK: encoding: [0xc5,0x99,0x72,0x0d,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprold xmm12, xmmword ptr [rip], 123

// CHECK:      {vex} vprold xmm12, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0xc5,0x99,0x72,0x0c,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vprold xmm12, xmmword ptr [2*rbp - 512], 123

// CHECK:      {vex} vprold xmm12, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc5,0x99,0x72,0x89,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vprold xmm12, xmmword ptr [rcx + 2032], 123

// CHECK:      {vex} vprold xmm12, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc5,0x99,0x72,0x8a,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vprold xmm12, xmmword ptr [rdx - 2048], 123

// CHECK:      {vex} vprold ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0xa1,0x1d,0x72,0x8c,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprold ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vprold ymm12, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0xc1,0x1d,0x72,0x8c,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprold ymm12, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vprold ymm12, ymmword ptr [rip], 123
// CHECK: encoding: [0xc5,0x9d,0x72,0x0d,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprold ymm12, ymmword ptr [rip], 123

// CHECK:      {vex} vprold ymm12, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0xc5,0x9d,0x72,0x0c,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vprold ymm12, ymmword ptr [2*rbp - 1024], 123

// CHECK:      {vex} vprold ymm12, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc5,0x9d,0x72,0x89,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vprold ymm12, ymmword ptr [rcx + 4064], 123

// CHECK:      {vex} vprold ymm12, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc5,0x9d,0x72,0x8a,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vprold ymm12, ymmword ptr [rdx - 4096], 123

// CHECK:      {vex} vprolq ymm12, ymm13, 123
// CHECK: encoding: [0xc4,0xc1,0x9d,0x72,0xcd,0x7b]
               {vex} vprolq ymm12, ymm13, 123

// CHECK:      {vex} vprolq xmm12, xmm13, 123
// CHECK: encoding: [0xc4,0xc1,0x99,0x72,0xcd,0x7b]
               {vex} vprolq xmm12, xmm13, 123

// CHECK:      {vex} vprolq xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0xa1,0x99,0x72,0x8c,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprolq xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vprolq xmm12, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0xc1,0x99,0x72,0x8c,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprolq xmm12, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vprolq xmm12, xmmword ptr [rip], 123
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x0d,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprolq xmm12, xmmword ptr [rip], 123

// CHECK:      {vex} vprolq xmm12, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x0c,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vprolq xmm12, xmmword ptr [2*rbp - 512], 123

// CHECK:      {vex} vprolq xmm12, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x89,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vprolq xmm12, xmmword ptr [rcx + 2032], 123

// CHECK:      {vex} vprolq xmm12, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x8a,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vprolq xmm12, xmmword ptr [rdx - 2048], 123

// CHECK:      {vex} vprolq ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0xa1,0x9d,0x72,0x8c,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprolq ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vprolq ymm12, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0xc1,0x9d,0x72,0x8c,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprolq ymm12, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vprolq ymm12, ymmword ptr [rip], 123
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x0d,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprolq ymm12, ymmword ptr [rip], 123

// CHECK:      {vex} vprolq ymm12, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x0c,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vprolq ymm12, ymmword ptr [2*rbp - 1024], 123

// CHECK:      {vex} vprolq ymm12, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x89,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vprolq ymm12, ymmword ptr [rcx + 4064], 123

// CHECK:      {vex} vprolq ymm12, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x8a,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vprolq ymm12, ymmword ptr [rdx - 4096], 123

// CHECK:      {vex} vprolvd ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x15,0x15,0xe6]
               {vex} vprolvd ymm12, ymm13, ymm14

// CHECK:      {vex} vprolvd xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x11,0x15,0xe6]
               {vex} vprolvd xmm12, xmm13, xmm14

// CHECK:      {vex} vprolvd ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x15,0x15,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprolvd ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vprolvd ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x15,0x15,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprolvd ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vprolvd ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x15,0x15,0x25,0x00,0x00,0x00,0x00]
               {vex} vprolvd ymm12, ymm13, ymmword ptr [rip]

// CHECK:      {vex} vprolvd ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x15,0x15,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vprolvd ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vprolvd ymm12, ymm13, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x15,0x15,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vprolvd ymm12, ymm13, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vprolvd ymm12, ymm13, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x15,0x15,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vprolvd ymm12, ymm13, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vprolvd xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x11,0x15,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprolvd xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vprolvd xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x11,0x15,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprolvd xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vprolvd xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x11,0x15,0x25,0x00,0x00,0x00,0x00]
               {vex} vprolvd xmm12, xmm13, xmmword ptr [rip]

// CHECK:      {vex} vprolvd xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x11,0x15,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vprolvd xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vprolvd xmm12, xmm13, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x11,0x15,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vprolvd xmm12, xmm13, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vprolvd xmm12, xmm13, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x11,0x15,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vprolvd xmm12, xmm13, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vprolvq ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x95,0x15,0xe6]
               {vex} vprolvq ymm12, ymm13, ymm14

// CHECK:      {vex} vprolvq xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x91,0x15,0xe6]
               {vex} vprolvq xmm12, xmm13, xmm14

// CHECK:      {vex} vprolvq ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x95,0x15,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprolvq ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vprolvq ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x95,0x15,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprolvq ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vprolvq ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x95,0x15,0x25,0x00,0x00,0x00,0x00]
               {vex} vprolvq ymm12, ymm13, ymmword ptr [rip]

// CHECK:      {vex} vprolvq ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x95,0x15,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vprolvq ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vprolvq ymm12, ymm13, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x95,0x15,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vprolvq ymm12, ymm13, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vprolvq ymm12, ymm13, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x95,0x15,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vprolvq ymm12, ymm13, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vprolvq xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x91,0x15,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprolvq xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vprolvq xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x91,0x15,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprolvq xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vprolvq xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x91,0x15,0x25,0x00,0x00,0x00,0x00]
               {vex} vprolvq xmm12, xmm13, xmmword ptr [rip]

// CHECK:      {vex} vprolvq xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x91,0x15,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vprolvq xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vprolvq xmm12, xmm13, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x91,0x15,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vprolvq xmm12, xmm13, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vprolvq xmm12, xmm13, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x91,0x15,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vprolvq xmm12, xmm13, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vprord ymm12, ymm13, 123
// CHECK: encoding: [0xc4,0xc1,0x1d,0x72,0xc5,0x7b]
               {vex} vprord ymm12, ymm13, 123

// CHECK:      {vex} vprord xmm12, xmm13, 123
// CHECK: encoding: [0xc4,0xc1,0x19,0x72,0xc5,0x7b]
               {vex} vprord xmm12, xmm13, 123

// CHECK:      {vex} vprord xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0xa1,0x19,0x72,0x84,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprord xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vprord xmm12, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0xc1,0x19,0x72,0x84,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprord xmm12, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vprord xmm12, xmmword ptr [rip], 123
// CHECK: encoding: [0xc5,0x99,0x72,0x05,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprord xmm12, xmmword ptr [rip], 123

// CHECK:      {vex} vprord xmm12, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0xc5,0x99,0x72,0x04,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vprord xmm12, xmmword ptr [2*rbp - 512], 123

// CHECK:      {vex} vprord xmm12, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc5,0x99,0x72,0x81,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vprord xmm12, xmmword ptr [rcx + 2032], 123

// CHECK:      {vex} vprord xmm12, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc5,0x99,0x72,0x82,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vprord xmm12, xmmword ptr [rdx - 2048], 123

// CHECK:      {vex} vprord ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0xa1,0x1d,0x72,0x84,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprord ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vprord ymm12, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0xc1,0x1d,0x72,0x84,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprord ymm12, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vprord ymm12, ymmword ptr [rip], 123
// CHECK: encoding: [0xc5,0x9d,0x72,0x05,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprord ymm12, ymmword ptr [rip], 123

// CHECK:      {vex} vprord ymm12, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0xc5,0x9d,0x72,0x04,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vprord ymm12, ymmword ptr [2*rbp - 1024], 123

// CHECK:      {vex} vprord ymm12, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc5,0x9d,0x72,0x81,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vprord ymm12, ymmword ptr [rcx + 4064], 123

// CHECK:      {vex} vprord ymm12, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc5,0x9d,0x72,0x82,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vprord ymm12, ymmword ptr [rdx - 4096], 123

// CHECK:      {vex} vprorq ymm12, ymm13, 123
// CHECK: encoding: [0xc4,0xc1,0x9d,0x72,0xc5,0x7b]
               {vex} vprorq ymm12, ymm13, 123

// CHECK:      {vex} vprorq xmm12, xmm13, 123
// CHECK: encoding: [0xc4,0xc1,0x99,0x72,0xc5,0x7b]
               {vex} vprorq xmm12, xmm13, 123

// CHECK:      {vex} vprorq xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0xa1,0x99,0x72,0x84,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprorq xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vprorq xmm12, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0xc1,0x99,0x72,0x84,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprorq xmm12, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vprorq xmm12, xmmword ptr [rip], 123
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x05,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprorq xmm12, xmmword ptr [rip], 123

// CHECK:      {vex} vprorq xmm12, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x04,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vprorq xmm12, xmmword ptr [2*rbp - 512], 123

// CHECK:      {vex} vprorq xmm12, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x81,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vprorq xmm12, xmmword ptr [rcx + 2032], 123

// CHECK:      {vex} vprorq xmm12, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x82,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vprorq xmm12, xmmword ptr [rdx - 2048], 123

// CHECK:      {vex} vprorq ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0xa1,0x9d,0x72,0x84,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprorq ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vprorq ymm12, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0xc1,0x9d,0x72,0x84,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprorq ymm12, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vprorq ymm12, ymmword ptr [rip], 123
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x05,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprorq ymm12, ymmword ptr [rip], 123

// CHECK:      {vex} vprorq ymm12, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x04,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vprorq ymm12, ymmword ptr [2*rbp - 1024], 123

// CHECK:      {vex} vprorq ymm12, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x81,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vprorq ymm12, ymmword ptr [rcx + 4064], 123

// CHECK:      {vex} vprorq ymm12, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x82,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vprorq ymm12, ymmword ptr [rdx - 4096], 123

// CHECK:      {vex} vprorvd ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x15,0x14,0xe6]
               {vex} vprorvd ymm12, ymm13, ymm14

// CHECK:      {vex} vprorvd xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x11,0x14,0xe6]
               {vex} vprorvd xmm12, xmm13, xmm14

// CHECK:      {vex} vprorvd ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x15,0x14,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprorvd ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vprorvd ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x15,0x14,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprorvd ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vprorvd ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x15,0x14,0x25,0x00,0x00,0x00,0x00]
               {vex} vprorvd ymm12, ymm13, ymmword ptr [rip]

// CHECK:      {vex} vprorvd ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x15,0x14,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vprorvd ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vprorvd ymm12, ymm13, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x15,0x14,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vprorvd ymm12, ymm13, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vprorvd ymm12, ymm13, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x15,0x14,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vprorvd ymm12, ymm13, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vprorvd xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x11,0x14,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprorvd xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vprorvd xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x11,0x14,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprorvd xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vprorvd xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x11,0x14,0x25,0x00,0x00,0x00,0x00]
               {vex} vprorvd xmm12, xmm13, xmmword ptr [rip]

// CHECK:      {vex} vprorvd xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x11,0x14,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vprorvd xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vprorvd xmm12, xmm13, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x11,0x14,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vprorvd xmm12, xmm13, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vprorvd xmm12, xmm13, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x11,0x14,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vprorvd xmm12, xmm13, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vprorvq ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x95,0x14,0xe6]
               {vex} vprorvq ymm12, ymm13, ymm14

// CHECK:      {vex} vprorvq xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x91,0x14,0xe6]
               {vex} vprorvq xmm12, xmm13, xmm14

// CHECK:      {vex} vprorvq ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x95,0x14,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprorvq ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vprorvq ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x95,0x14,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprorvq ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vprorvq ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x95,0x14,0x25,0x00,0x00,0x00,0x00]
               {vex} vprorvq ymm12, ymm13, ymmword ptr [rip]

// CHECK:      {vex} vprorvq ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x95,0x14,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vprorvq ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vprorvq ymm12, ymm13, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x95,0x14,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vprorvq ymm12, ymm13, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vprorvq ymm12, ymm13, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x95,0x14,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vprorvq ymm12, ymm13, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vprorvq xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x91,0x14,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprorvq xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vprorvq xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x91,0x14,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprorvq xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vprorvq xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x91,0x14,0x25,0x00,0x00,0x00,0x00]
               {vex} vprorvq xmm12, xmm13, xmmword ptr [rip]

// CHECK:      {vex} vprorvq xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x91,0x14,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vprorvq xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vprorvq xmm12, xmm13, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x91,0x14,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vprorvq xmm12, xmm13, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vprorvq xmm12, xmm13, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x91,0x14,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vprorvq xmm12, xmm13, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vpshldd xmm12, xmm13, xmm14, 123
// CHECK: encoding: [0xc4,0x43,0x11,0x71,0xe6,0x7b]
               {vex} vpshldd xmm12, xmm13, xmm14, 123

// CHECK:      {vex} vpshldd ymm12, ymm13, ymm14, 123
// CHECK: encoding: [0xc4,0x43,0x15,0x71,0xe6,0x7b]
               {vex} vpshldd ymm12, ymm13, ymm14, 123

// CHECK:      {vex} vpshldd ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x15,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshldd ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vpshldd ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0x43,0x15,0x71,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshldd ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vpshldd ymm12, ymm13, ymmword ptr [rip], 123
// CHECK: encoding: [0xc4,0x63,0x15,0x71,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshldd ymm12, ymm13, ymmword ptr [rip], 123

// CHECK:      {vex} vpshldd ymm12, ymm13, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0xc4,0x63,0x15,0x71,0x24,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vpshldd ymm12, ymm13, ymmword ptr [2*rbp - 1024], 123

// CHECK:      {vex} vpshldd ymm12, ymm13, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc4,0x63,0x15,0x71,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vpshldd ymm12, ymm13, ymmword ptr [rcx + 4064], 123

// CHECK:      {vex} vpshldd ymm12, ymm13, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc4,0x63,0x15,0x71,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vpshldd ymm12, ymm13, ymmword ptr [rdx - 4096], 123

// CHECK:      {vex} vpshldd xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x11,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshldd xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vpshldd xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0x43,0x11,0x71,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshldd xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vpshldd xmm12, xmm13, xmmword ptr [rip], 123
// CHECK: encoding: [0xc4,0x63,0x11,0x71,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshldd xmm12, xmm13, xmmword ptr [rip], 123

// CHECK:      {vex} vpshldd xmm12, xmm13, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0xc4,0x63,0x11,0x71,0x24,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vpshldd xmm12, xmm13, xmmword ptr [2*rbp - 512], 123

// CHECK:      {vex} vpshldd xmm12, xmm13, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc4,0x63,0x11,0x71,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vpshldd xmm12, xmm13, xmmword ptr [rcx + 2032], 123

// CHECK:      {vex} vpshldd xmm12, xmm13, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc4,0x63,0x11,0x71,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vpshldd xmm12, xmm13, xmmword ptr [rdx - 2048], 123

// CHECK:      {vex} vpshldq xmm12, xmm13, xmm14, 123
// CHECK: encoding: [0xc4,0x43,0x91,0x71,0xe6,0x7b]
               {vex} vpshldq xmm12, xmm13, xmm14, 123

// CHECK:      {vex} vpshldq ymm12, ymm13, ymm14, 123
// CHECK: encoding: [0xc4,0x43,0x95,0x71,0xe6,0x7b]
               {vex} vpshldq ymm12, ymm13, ymm14, 123

// CHECK:      {vex} vpshldq ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x95,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshldq ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vpshldq ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0x43,0x95,0x71,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshldq ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vpshldq ymm12, ymm13, ymmword ptr [rip], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x71,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshldq ymm12, ymm13, ymmword ptr [rip], 123

// CHECK:      {vex} vpshldq ymm12, ymm13, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x71,0x24,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vpshldq ymm12, ymm13, ymmword ptr [2*rbp - 1024], 123

// CHECK:      {vex} vpshldq ymm12, ymm13, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x71,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vpshldq ymm12, ymm13, ymmword ptr [rcx + 4064], 123

// CHECK:      {vex} vpshldq ymm12, ymm13, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x71,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vpshldq ymm12, ymm13, ymmword ptr [rdx - 4096], 123

// CHECK:      {vex} vpshldq xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x91,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshldq xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vpshldq xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0x43,0x91,0x71,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshldq xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vpshldq xmm12, xmm13, xmmword ptr [rip], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x71,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshldq xmm12, xmm13, xmmword ptr [rip], 123

// CHECK:      {vex} vpshldq xmm12, xmm13, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x71,0x24,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vpshldq xmm12, xmm13, xmmword ptr [2*rbp - 512], 123

// CHECK:      {vex} vpshldq xmm12, xmm13, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x71,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vpshldq xmm12, xmm13, xmmword ptr [rcx + 2032], 123

// CHECK:      {vex} vpshldq xmm12, xmm13, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x71,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vpshldq xmm12, xmm13, xmmword ptr [rdx - 2048], 123

// CHECK:      {vex} vpshldvd ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x15,0x71,0xe6]
               {vex} vpshldvd ymm12, ymm13, ymm14

// CHECK:      {vex} vpshldvd xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x11,0x71,0xe6]
               {vex} vpshldvd xmm12, xmm13, xmm14

// CHECK:      {vex} vpshldvd ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x15,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshldvd ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpshldvd ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x15,0x71,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshldvd ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpshldvd ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x15,0x71,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshldvd ymm12, ymm13, ymmword ptr [rip]

// CHECK:      {vex} vpshldvd ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x15,0x71,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpshldvd ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vpshldvd ymm12, ymm13, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x15,0x71,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpshldvd ymm12, ymm13, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vpshldvd ymm12, ymm13, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x15,0x71,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpshldvd ymm12, ymm13, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vpshldvd xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x11,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshldvd xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpshldvd xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x11,0x71,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshldvd xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpshldvd xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x11,0x71,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshldvd xmm12, xmm13, xmmword ptr [rip]

// CHECK:      {vex} vpshldvd xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x11,0x71,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpshldvd xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vpshldvd xmm12, xmm13, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x11,0x71,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpshldvd xmm12, xmm13, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vpshldvd xmm12, xmm13, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x11,0x71,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpshldvd xmm12, xmm13, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vpshldvq ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x95,0x71,0xe6]
               {vex} vpshldvq ymm12, ymm13, ymm14

// CHECK:      {vex} vpshldvq xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x91,0x71,0xe6]
               {vex} vpshldvq xmm12, xmm13, xmm14

// CHECK:      {vex} vpshldvq ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x95,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshldvq ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpshldvq ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x95,0x71,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshldvq ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpshldvq ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x95,0x71,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshldvq ymm12, ymm13, ymmword ptr [rip]

// CHECK:      {vex} vpshldvq ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x95,0x71,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpshldvq ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vpshldvq ymm12, ymm13, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x95,0x71,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpshldvq ymm12, ymm13, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vpshldvq ymm12, ymm13, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x95,0x71,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpshldvq ymm12, ymm13, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vpshldvq xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x91,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshldvq xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpshldvq xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x91,0x71,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshldvq xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpshldvq xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x91,0x71,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshldvq xmm12, xmm13, xmmword ptr [rip]

// CHECK:      {vex} vpshldvq xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x91,0x71,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpshldvq xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vpshldvq xmm12, xmm13, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x91,0x71,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpshldvq xmm12, xmm13, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vpshldvq xmm12, xmm13, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x91,0x71,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpshldvq xmm12, xmm13, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vpshldvw ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x95,0x70,0xe6]
               {vex} vpshldvw ymm12, ymm13, ymm14

// CHECK:      {vex} vpshldvw xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x91,0x70,0xe6]
               {vex} vpshldvw xmm12, xmm13, xmm14

// CHECK:      {vex} vpshldvw ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x95,0x70,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshldvw ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpshldvw ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x95,0x70,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshldvw ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpshldvw ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x95,0x70,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshldvw ymm12, ymm13, ymmword ptr [rip]

// CHECK:      {vex} vpshldvw ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x95,0x70,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpshldvw ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vpshldvw ymm12, ymm13, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x95,0x70,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpshldvw ymm12, ymm13, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vpshldvw ymm12, ymm13, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x95,0x70,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpshldvw ymm12, ymm13, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vpshldvw xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x91,0x70,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshldvw xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpshldvw xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x91,0x70,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshldvw xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpshldvw xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x91,0x70,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshldvw xmm12, xmm13, xmmword ptr [rip]

// CHECK:      {vex} vpshldvw xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x91,0x70,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpshldvw xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vpshldvw xmm12, xmm13, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x91,0x70,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpshldvw xmm12, xmm13, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vpshldvw xmm12, xmm13, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x91,0x70,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpshldvw xmm12, xmm13, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vpshldw xmm12, xmm13, xmm14, 123
// CHECK: encoding: [0xc4,0x43,0x91,0x70,0xe6,0x7b]
               {vex} vpshldw xmm12, xmm13, xmm14, 123

// CHECK:      {vex} vpshldw ymm12, ymm13, ymm14, 123
// CHECK: encoding: [0xc4,0x43,0x95,0x70,0xe6,0x7b]
               {vex} vpshldw ymm12, ymm13, ymm14, 123

// CHECK:      {vex} vpshldw ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x95,0x70,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshldw ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vpshldw ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0x43,0x95,0x70,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshldw ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vpshldw ymm12, ymm13, ymmword ptr [rip], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x70,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshldw ymm12, ymm13, ymmword ptr [rip], 123

// CHECK:      {vex} vpshldw ymm12, ymm13, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x70,0x24,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vpshldw ymm12, ymm13, ymmword ptr [2*rbp - 1024], 123

// CHECK:      {vex} vpshldw ymm12, ymm13, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x70,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vpshldw ymm12, ymm13, ymmword ptr [rcx + 4064], 123

// CHECK:      {vex} vpshldw ymm12, ymm13, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x70,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vpshldw ymm12, ymm13, ymmword ptr [rdx - 4096], 123

// CHECK:      {vex} vpshldw xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x91,0x70,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshldw xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vpshldw xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0x43,0x91,0x70,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshldw xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vpshldw xmm12, xmm13, xmmword ptr [rip], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x70,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshldw xmm12, xmm13, xmmword ptr [rip], 123

// CHECK:      {vex} vpshldw xmm12, xmm13, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x70,0x24,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vpshldw xmm12, xmm13, xmmword ptr [2*rbp - 512], 123

// CHECK:      {vex} vpshldw xmm12, xmm13, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x70,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vpshldw xmm12, xmm13, xmmword ptr [rcx + 2032], 123

// CHECK:      {vex} vpshldw xmm12, xmm13, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x70,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vpshldw xmm12, xmm13, xmmword ptr [rdx - 2048], 123

// CHECK:      {vex} vpshrdd xmm12, xmm13, xmm14, 123
// CHECK: encoding: [0xc4,0x43,0x11,0x73,0xe6,0x7b]
               {vex} vpshrdd xmm12, xmm13, xmm14, 123

// CHECK:      {vex} vpshrdd ymm12, ymm13, ymm14, 123
// CHECK: encoding: [0xc4,0x43,0x15,0x73,0xe6,0x7b]
               {vex} vpshrdd ymm12, ymm13, ymm14, 123

// CHECK:      {vex} vpshrdd ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x15,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshrdd ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vpshrdd ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0x43,0x15,0x73,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshrdd ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vpshrdd ymm12, ymm13, ymmword ptr [rip], 123
// CHECK: encoding: [0xc4,0x63,0x15,0x73,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshrdd ymm12, ymm13, ymmword ptr [rip], 123

// CHECK:      {vex} vpshrdd ymm12, ymm13, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0xc4,0x63,0x15,0x73,0x24,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vpshrdd ymm12, ymm13, ymmword ptr [2*rbp - 1024], 123

// CHECK:      {vex} vpshrdd ymm12, ymm13, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc4,0x63,0x15,0x73,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vpshrdd ymm12, ymm13, ymmword ptr [rcx + 4064], 123

// CHECK:      {vex} vpshrdd ymm12, ymm13, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc4,0x63,0x15,0x73,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vpshrdd ymm12, ymm13, ymmword ptr [rdx - 4096], 123

// CHECK:      {vex} vpshrdd xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x11,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshrdd xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vpshrdd xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0x43,0x11,0x73,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshrdd xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vpshrdd xmm12, xmm13, xmmword ptr [rip], 123
// CHECK: encoding: [0xc4,0x63,0x11,0x73,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshrdd xmm12, xmm13, xmmword ptr [rip], 123

// CHECK:      {vex} vpshrdd xmm12, xmm13, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0xc4,0x63,0x11,0x73,0x24,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vpshrdd xmm12, xmm13, xmmword ptr [2*rbp - 512], 123

// CHECK:      {vex} vpshrdd xmm12, xmm13, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc4,0x63,0x11,0x73,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vpshrdd xmm12, xmm13, xmmword ptr [rcx + 2032], 123

// CHECK:      {vex} vpshrdd xmm12, xmm13, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc4,0x63,0x11,0x73,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vpshrdd xmm12, xmm13, xmmword ptr [rdx - 2048], 123

// CHECK:      {vex} vpshrdq xmm12, xmm13, xmm14, 123
// CHECK: encoding: [0xc4,0x43,0x91,0x73,0xe6,0x7b]
               {vex} vpshrdq xmm12, xmm13, xmm14, 123

// CHECK:      {vex} vpshrdq ymm12, ymm13, ymm14, 123
// CHECK: encoding: [0xc4,0x43,0x95,0x73,0xe6,0x7b]
               {vex} vpshrdq ymm12, ymm13, ymm14, 123

// CHECK:      {vex} vpshrdq ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x95,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshrdq ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vpshrdq ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0x43,0x95,0x73,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshrdq ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vpshrdq ymm12, ymm13, ymmword ptr [rip], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x73,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshrdq ymm12, ymm13, ymmword ptr [rip], 123

// CHECK:      {vex} vpshrdq ymm12, ymm13, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x73,0x24,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vpshrdq ymm12, ymm13, ymmword ptr [2*rbp - 1024], 123

// CHECK:      {vex} vpshrdq ymm12, ymm13, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x73,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vpshrdq ymm12, ymm13, ymmword ptr [rcx + 4064], 123

// CHECK:      {vex} vpshrdq ymm12, ymm13, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x73,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vpshrdq ymm12, ymm13, ymmword ptr [rdx - 4096], 123

// CHECK:      {vex} vpshrdq xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x91,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshrdq xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vpshrdq xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0x43,0x91,0x73,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshrdq xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vpshrdq xmm12, xmm13, xmmword ptr [rip], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x73,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshrdq xmm12, xmm13, xmmword ptr [rip], 123

// CHECK:      {vex} vpshrdq xmm12, xmm13, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x73,0x24,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vpshrdq xmm12, xmm13, xmmword ptr [2*rbp - 512], 123

// CHECK:      {vex} vpshrdq xmm12, xmm13, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x73,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vpshrdq xmm12, xmm13, xmmword ptr [rcx + 2032], 123

// CHECK:      {vex} vpshrdq xmm12, xmm13, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x73,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vpshrdq xmm12, xmm13, xmmword ptr [rdx - 2048], 123

// CHECK:      {vex} vpshrdvd ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x15,0x73,0xe6]
               {vex} vpshrdvd ymm12, ymm13, ymm14

// CHECK:      {vex} vpshrdvd xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x11,0x73,0xe6]
               {vex} vpshrdvd xmm12, xmm13, xmm14

// CHECK:      {vex} vpshrdvd ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x15,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshrdvd ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpshrdvd ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x15,0x73,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshrdvd ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpshrdvd ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x15,0x73,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshrdvd ymm12, ymm13, ymmword ptr [rip]

// CHECK:      {vex} vpshrdvd ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x15,0x73,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpshrdvd ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vpshrdvd ymm12, ymm13, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x15,0x73,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpshrdvd ymm12, ymm13, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vpshrdvd ymm12, ymm13, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x15,0x73,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpshrdvd ymm12, ymm13, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vpshrdvd xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x11,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshrdvd xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpshrdvd xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x11,0x73,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshrdvd xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpshrdvd xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x11,0x73,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshrdvd xmm12, xmm13, xmmword ptr [rip]

// CHECK:      {vex} vpshrdvd xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x11,0x73,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpshrdvd xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vpshrdvd xmm12, xmm13, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x11,0x73,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpshrdvd xmm12, xmm13, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vpshrdvd xmm12, xmm13, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x11,0x73,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpshrdvd xmm12, xmm13, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vpshrdvq ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x95,0x73,0xe6]
               {vex} vpshrdvq ymm12, ymm13, ymm14

// CHECK:      {vex} vpshrdvq xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x91,0x73,0xe6]
               {vex} vpshrdvq xmm12, xmm13, xmm14

// CHECK:      {vex} vpshrdvq ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x95,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshrdvq ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpshrdvq ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x95,0x73,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshrdvq ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpshrdvq ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x95,0x73,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshrdvq ymm12, ymm13, ymmword ptr [rip]

// CHECK:      {vex} vpshrdvq ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x95,0x73,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpshrdvq ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vpshrdvq ymm12, ymm13, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x95,0x73,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpshrdvq ymm12, ymm13, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vpshrdvq ymm12, ymm13, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x95,0x73,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpshrdvq ymm12, ymm13, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vpshrdvq xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x91,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshrdvq xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpshrdvq xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x91,0x73,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshrdvq xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpshrdvq xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x91,0x73,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshrdvq xmm12, xmm13, xmmword ptr [rip]

// CHECK:      {vex} vpshrdvq xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x91,0x73,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpshrdvq xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vpshrdvq xmm12, xmm13, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x91,0x73,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpshrdvq xmm12, xmm13, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vpshrdvq xmm12, xmm13, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x91,0x73,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpshrdvq xmm12, xmm13, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vpshrdvw ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x95,0x72,0xe6]
               {vex} vpshrdvw ymm12, ymm13, ymm14

// CHECK:      {vex} vpshrdvw xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x91,0x72,0xe6]
               {vex} vpshrdvw xmm12, xmm13, xmm14

// CHECK:      {vex} vpshrdvw ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x95,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshrdvw ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpshrdvw ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x95,0x72,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshrdvw ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpshrdvw ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x95,0x72,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshrdvw ymm12, ymm13, ymmword ptr [rip]

// CHECK:      {vex} vpshrdvw ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x95,0x72,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpshrdvw ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK:      {vex} vpshrdvw ymm12, ymm13, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0x62,0x95,0x72,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpshrdvw ymm12, ymm13, ymmword ptr [rcx + 4064]

// CHECK:      {vex} vpshrdvw ymm12, ymm13, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0x62,0x95,0x72,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpshrdvw ymm12, ymm13, ymmword ptr [rdx - 4096]

// CHECK:      {vex} vpshrdvw xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x91,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshrdvw xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      {vex} vpshrdvw xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x91,0x72,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshrdvw xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      {vex} vpshrdvw xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x91,0x72,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshrdvw xmm12, xmm13, xmmword ptr [rip]

// CHECK:      {vex} vpshrdvw xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x91,0x72,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpshrdvw xmm12, xmm13, xmmword ptr [2*rbp - 512]

// CHECK:      {vex} vpshrdvw xmm12, xmm13, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0x62,0x91,0x72,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpshrdvw xmm12, xmm13, xmmword ptr [rcx + 2032]

// CHECK:      {vex} vpshrdvw xmm12, xmm13, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0x62,0x91,0x72,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpshrdvw xmm12, xmm13, xmmword ptr [rdx - 2048]

// CHECK:      {vex} vpshrdw xmm12, xmm13, xmm14, 123
// CHECK: encoding: [0xc4,0x43,0x91,0x72,0xe6,0x7b]
               {vex} vpshrdw xmm12, xmm13, xmm14, 123

// CHECK:      {vex} vpshrdw ymm12, ymm13, ymm14, 123
// CHECK: encoding: [0xc4,0x43,0x95,0x72,0xe6,0x7b]
               {vex} vpshrdw ymm12, ymm13, ymm14, 123

// CHECK:      {vex} vpshrdw ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x95,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshrdw ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vpshrdw ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0x43,0x95,0x72,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshrdw ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vpshrdw ymm12, ymm13, ymmword ptr [rip], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x72,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshrdw ymm12, ymm13, ymmword ptr [rip], 123

// CHECK:      {vex} vpshrdw ymm12, ymm13, ymmword ptr [2*rbp - 1024], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x72,0x24,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vpshrdw ymm12, ymm13, ymmword ptr [2*rbp - 1024], 123

// CHECK:      {vex} vpshrdw ymm12, ymm13, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x72,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vpshrdw ymm12, ymm13, ymmword ptr [rcx + 4064], 123

// CHECK:      {vex} vpshrdw ymm12, ymm13, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc4,0x63,0x95,0x72,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vpshrdw ymm12, ymm13, ymmword ptr [rdx - 4096], 123

// CHECK:      {vex} vpshrdw xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x91,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshrdw xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      {vex} vpshrdw xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0x43,0x91,0x72,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshrdw xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      {vex} vpshrdw xmm12, xmm13, xmmword ptr [rip], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x72,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshrdw xmm12, xmm13, xmmword ptr [rip], 123

// CHECK:      {vex} vpshrdw xmm12, xmm13, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x72,0x24,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vpshrdw xmm12, xmm13, xmmword ptr [2*rbp - 512], 123

// CHECK:      {vex} vpshrdw xmm12, xmm13, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x72,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vpshrdw xmm12, xmm13, xmmword ptr [rcx + 2032], 123

// CHECK:      {vex} vpshrdw xmm12, xmm13, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc4,0x63,0x91,0x72,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vpshrdw xmm12, xmm13, xmmword ptr [rdx - 2048], 123

