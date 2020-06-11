// REQUIRES: intel_feature_isa_avx_compress
// RUN: llvm-mc -triple x86_64-unknown-unknown -mattr=+avxcompress --show-encoding < %s  | FileCheck %s

// CHECK:      vpcompressb %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x0d,0x63,0xec]
               vpcompressb %ymm14, %ymm13, %ymm12

// CHECK:      vpcompressb %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x09,0x63,0xec]
               vpcompressb %xmm14, %xmm13, %xmm12

// CHECK:      vpcompressb  %xmm13, %xmm12, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0x22,0x11,0x63,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressb  %xmm13, %xmm12, 268435456(%rbp,%r14,8)

// CHECK:      vpcompressb  %xmm13, %xmm12, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0x42,0x11,0x63,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressb  %xmm13, %xmm12, 291(%r8,%rax,4)

// CHECK:      vpcompressb  %xmm13, %xmm12, (%rip)
// CHECK: encoding: [0xc4,0x62,0x11,0x63,0x25,0x00,0x00,0x00,0x00]
               vpcompressb  %xmm13, %xmm12, (%rip)

// CHECK:      vpcompressb  %xmm13, %xmm12, -512(,%rbp,2)
// CHECK: encoding: [0xc4,0x62,0x11,0x63,0x24,0x6d,0x00,0xfe,0xff,0xff]
               vpcompressb  %xmm13, %xmm12, -512(,%rbp,2)
// CHECK:      vpcompressb  %xmm13, %xmm12, 2032(%rcx)
// CHECK: encoding: [0xc4,0x62,0x11,0x63,0xa1,0xf0,0x07,0x00,0x00]
               vpcompressb  %xmm13, %xmm12, 2032(%rcx)

// CHECK:      vpcompressb  %xmm13, %xmm12, -2048(%rdx)
// CHECK: encoding: [0xc4,0x62,0x11,0x63,0xa2,0x00,0xf8,0xff,0xff]
               vpcompressb  %xmm13, %xmm12, -2048(%rdx)

// CHECK:      vpcompressb  %ymm13, %ymm12, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0x22,0x15,0x63,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressb  %ymm13, %ymm12, 268435456(%rbp,%r14,8)

// CHECK:      vpcompressb  %ymm13, %ymm12, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0x42,0x15,0x63,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressb  %ymm13, %ymm12, 291(%r8,%rax,4)

// CHECK:      vpcompressb  %ymm13, %ymm12, (%rip)
// CHECK: encoding: [0xc4,0x62,0x15,0x63,0x25,0x00,0x00,0x00,0x00]
               vpcompressb  %ymm13, %ymm12, (%rip)

// CHECK:      vpcompressb  %ymm13, %ymm12, -1024(,%rbp,2)
// CHECK: encoding: [0xc4,0x62,0x15,0x63,0x24,0x6d,0x00,0xfc,0xff,0xff]
               vpcompressb  %ymm13, %ymm12, -1024(,%rbp,2)

// CHECK:      vpcompressb  %ymm13, %ymm12, 4064(%rcx)
// CHECK: encoding: [0xc4,0x62,0x15,0x63,0xa1,0xe0,0x0f,0x00,0x00]
               vpcompressb  %ymm13, %ymm12, 4064(%rcx)

// CHECK:      vpcompressb  %ymm13, %ymm12, -4096(%rdx)
// CHECK: encoding: [0xc4,0x62,0x15,0x63,0xa2,0x00,0xf0,0xff,0xff]
               vpcompressb  %ymm13, %ymm12, -4096(%rdx)

// CHECK:      vpcompressd %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x0d,0x8b,0xec]
               vpcompressd %ymm14, %ymm13, %ymm12

// CHECK:      vpcompressd %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x09,0x8b,0xec]
               vpcompressd %xmm14, %xmm13, %xmm12

// CHECK:      vpcompressd  %xmm13, %xmm12, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0x22,0x11,0x8b,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressd  %xmm13, %xmm12, 268435456(%rbp,%r14,8)

// CHECK:      vpcompressd  %xmm13, %xmm12, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0x42,0x11,0x8b,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressd  %xmm13, %xmm12, 291(%r8,%rax,4)

// CHECK:      vpcompressd  %xmm13, %xmm12, (%rip)
// CHECK: encoding: [0xc4,0x62,0x11,0x8b,0x25,0x00,0x00,0x00,0x00]
               vpcompressd  %xmm13, %xmm12, (%rip)

// CHECK:      vpcompressd  %xmm13, %xmm12, -512(,%rbp,2)
// CHECK: encoding: [0xc4,0x62,0x11,0x8b,0x24,0x6d,0x00,0xfe,0xff,0xff]
               vpcompressd  %xmm13, %xmm12, -512(,%rbp,2)

// CHECK:      vpcompressd  %xmm13, %xmm12, 2032(%rcx)
// CHECK: encoding: [0xc4,0x62,0x11,0x8b,0xa1,0xf0,0x07,0x00,0x00]
               vpcompressd  %xmm13, %xmm12, 2032(%rcx)

// CHECK:      vpcompressd  %xmm13, %xmm12, -2048(%rdx)
// CHECK: encoding: [0xc4,0x62,0x11,0x8b,0xa2,0x00,0xf8,0xff,0xff]
               vpcompressd  %xmm13, %xmm12, -2048(%rdx)

// CHECK:      vpcompressd  %ymm13, %ymm12, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0x22,0x15,0x8b,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressd  %ymm13, %ymm12, 268435456(%rbp,%r14,8)

// CHECK:      vpcompressd  %ymm13, %ymm12, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0x42,0x15,0x8b,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressd  %ymm13, %ymm12, 291(%r8,%rax,4)

// CHECK:      vpcompressd  %ymm13, %ymm12, (%rip)
// CHECK: encoding: [0xc4,0x62,0x15,0x8b,0x25,0x00,0x00,0x00,0x00]
               vpcompressd  %ymm13, %ymm12, (%rip)

// CHECK:      vpcompressd  %ymm13, %ymm12, -1024(,%rbp,2)
// CHECK: encoding: [0xc4,0x62,0x15,0x8b,0x24,0x6d,0x00,0xfc,0xff,0xff]
               vpcompressd  %ymm13, %ymm12, -1024(,%rbp,2)

// CHECK:      vpcompressd  %ymm13, %ymm12, 4064(%rcx)
// CHECK: encoding: [0xc4,0x62,0x15,0x8b,0xa1,0xe0,0x0f,0x00,0x00]
               vpcompressd  %ymm13, %ymm12, 4064(%rcx)

// CHECK:      vpcompressd  %ymm13, %ymm12, -4096(%rdx)
// CHECK: encoding: [0xc4,0x62,0x15,0x8b,0xa2,0x00,0xf0,0xff,0xff]
               vpcompressd  %ymm13, %ymm12, -4096(%rdx)

// CHECK:      vpcompressq %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x8d,0x8b,0xec]
               vpcompressq %ymm14, %ymm13, %ymm12

// CHECK:      vpcompressq %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x89,0x8b,0xec]
               vpcompressq %xmm14, %xmm13, %xmm12

// CHECK:      vpcompressq  %xmm13, %xmm12, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0x22,0x91,0x8b,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressq  %xmm13, %xmm12, 268435456(%rbp,%r14,8)

// CHECK:      vpcompressq  %xmm13, %xmm12, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0x42,0x91,0x8b,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressq  %xmm13, %xmm12, 291(%r8,%rax,4)

// CHECK:      vpcompressq  %xmm13, %xmm12, (%rip)
// CHECK: encoding: [0xc4,0x62,0x91,0x8b,0x25,0x00,0x00,0x00,0x00]
               vpcompressq  %xmm13, %xmm12, (%rip)

// CHECK:      vpcompressq  %xmm13, %xmm12, -512(,%rbp,2)
// CHECK: encoding: [0xc4,0x62,0x91,0x8b,0x24,0x6d,0x00,0xfe,0xff,0xff]
               vpcompressq  %xmm13, %xmm12, -512(,%rbp,2)

// CHECK:      vpcompressq  %xmm13, %xmm12, 2032(%rcx)
// CHECK: encoding: [0xc4,0x62,0x91,0x8b,0xa1,0xf0,0x07,0x00,0x00]
               vpcompressq  %xmm13, %xmm12, 2032(%rcx)

// CHECK:      vpcompressq  %xmm13, %xmm12, -2048(%rdx)
// CHECK: encoding: [0xc4,0x62,0x91,0x8b,0xa2,0x00,0xf8,0xff,0xff]
               vpcompressq  %xmm13, %xmm12, -2048(%rdx)

// CHECK:      vpcompressq  %ymm13, %ymm12, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0x22,0x95,0x8b,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressq  %ymm13, %ymm12, 268435456(%rbp,%r14,8)

// CHECK:      vpcompressq  %ymm13, %ymm12, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0x42,0x95,0x8b,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressq  %ymm13, %ymm12, 291(%r8,%rax,4)

// CHECK:      vpcompressq  %ymm13, %ymm12, (%rip)
// CHECK: encoding: [0xc4,0x62,0x95,0x8b,0x25,0x00,0x00,0x00,0x00]
               vpcompressq  %ymm13, %ymm12, (%rip)

// CHECK:      vpcompressq  %ymm13, %ymm12, -1024(,%rbp,2)
// CHECK: encoding: [0xc4,0x62,0x95,0x8b,0x24,0x6d,0x00,0xfc,0xff,0xff]
               vpcompressq  %ymm13, %ymm12, -1024(,%rbp,2)

// CHECK:      vpcompressq  %ymm13, %ymm12, 4064(%rcx)
// CHECK: encoding: [0xc4,0x62,0x95,0x8b,0xa1,0xe0,0x0f,0x00,0x00]
               vpcompressq  %ymm13, %ymm12, 4064(%rcx)

// CHECK:      vpcompressq  %ymm13, %ymm12, -4096(%rdx)
// CHECK: encoding: [0xc4,0x62,0x95,0x8b,0xa2,0x00,0xf0,0xff,0xff]
               vpcompressq  %ymm13, %ymm12, -4096(%rdx)

// CHECK:      vpcompressw %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x8d,0x63,0xec]
               vpcompressw %ymm14, %ymm13, %ymm12

// CHECK:      vpcompressw %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x89,0x63,0xec]
               vpcompressw %xmm14, %xmm13, %xmm12

// CHECK:      vpcompressw  %xmm13, %xmm12, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0x22,0x91,0x63,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressw  %xmm13, %xmm12, 268435456(%rbp,%r14,8)

// CHECK:      vpcompressw  %xmm13, %xmm12, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0x42,0x91,0x63,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressw  %xmm13, %xmm12, 291(%r8,%rax,4)

// CHECK:      vpcompressw  %xmm13, %xmm12, (%rip)
// CHECK: encoding: [0xc4,0x62,0x91,0x63,0x25,0x00,0x00,0x00,0x00]
               vpcompressw  %xmm13, %xmm12, (%rip)

// CHECK:      vpcompressw  %xmm13, %xmm12, -512(,%rbp,2)
// CHECK: encoding: [0xc4,0x62,0x91,0x63,0x24,0x6d,0x00,0xfe,0xff,0xff]
               vpcompressw  %xmm13, %xmm12, -512(,%rbp,2)

// CHECK:      vpcompressw  %xmm13, %xmm12, 2032(%rcx)
// CHECK: encoding: [0xc4,0x62,0x91,0x63,0xa1,0xf0,0x07,0x00,0x00]
               vpcompressw  %xmm13, %xmm12, 2032(%rcx)

// CHECK:      vpcompressw  %xmm13, %xmm12, -2048(%rdx)
// CHECK: encoding: [0xc4,0x62,0x91,0x63,0xa2,0x00,0xf8,0xff,0xff]
               vpcompressw  %xmm13, %xmm12, -2048(%rdx)

// CHECK:      vpcompressw  %ymm13, %ymm12, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0x22,0x95,0x63,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpcompressw  %ymm13, %ymm12, 268435456(%rbp,%r14,8)

// CHECK:      vpcompressw  %ymm13, %ymm12, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0x42,0x95,0x63,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpcompressw  %ymm13, %ymm12, 291(%r8,%rax,4)

// CHECK:      vpcompressw  %ymm13, %ymm12, (%rip)
// CHECK: encoding: [0xc4,0x62,0x95,0x63,0x25,0x00,0x00,0x00,0x00]
               vpcompressw  %ymm13, %ymm12, (%rip)

// CHECK:      vpcompressw  %ymm13, %ymm12, -1024(,%rbp,2)
// CHECK: encoding: [0xc4,0x62,0x95,0x63,0x24,0x6d,0x00,0xfc,0xff,0xff]
               vpcompressw  %ymm13, %ymm12, -1024(,%rbp,2)

// CHECK:      vpcompressw  %ymm13, %ymm12, 4064(%rcx)
// CHECK: encoding: [0xc4,0x62,0x95,0x63,0xa1,0xe0,0x0f,0x00,0x00]
               vpcompressw  %ymm13, %ymm12, 4064(%rcx)

// CHECK:      vpcompressw  %ymm13, %ymm12, -4096(%rdx)
// CHECK: encoding: [0xc4,0x62,0x95,0x63,0xa2,0x00,0xf0,0xff,0xff]
               vpcompressw  %ymm13, %ymm12, -4096(%rdx)

