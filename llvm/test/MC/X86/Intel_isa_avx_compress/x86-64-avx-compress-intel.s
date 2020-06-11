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

