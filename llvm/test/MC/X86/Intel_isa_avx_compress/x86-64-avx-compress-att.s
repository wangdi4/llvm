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

// CHECK:      {vex} vplzcntd %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x79,0x44,0xe5]
               {vex} vplzcntd %xmm13, %xmm12

// CHECK:      {vex} vplzcntd %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x7d,0x44,0xe5]
               {vex} vplzcntd %ymm13, %ymm12

// CHECK:      {vex} vplzcntd  268435456(%rbp,%r14,8), %xmm12
// CHECK: encoding: [0xc4,0x22,0x79,0x44,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vplzcntd  268435456(%rbp,%r14,8), %xmm12

// CHECK:      {vex} vplzcntd  291(%r8,%rax,4), %xmm12
// CHECK: encoding: [0xc4,0x42,0x79,0x44,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vplzcntd  291(%r8,%rax,4), %xmm12

// CHECK:      {vex} vplzcntd  (%rip), %xmm12
// CHECK: encoding: [0xc4,0x62,0x79,0x44,0x25,0x00,0x00,0x00,0x00]
               {vex} vplzcntd  (%rip), %xmm12

// CHECK:      {vex} vplzcntd  -512(,%rbp,2), %xmm12
// CHECK: encoding: [0xc4,0x62,0x79,0x44,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vplzcntd  -512(,%rbp,2), %xmm12

// CHECK:      {vex} vplzcntd  2032(%rcx), %xmm12
// CHECK: encoding: [0xc4,0x62,0x79,0x44,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vplzcntd  2032(%rcx), %xmm12

// CHECK:      {vex} vplzcntd  -2048(%rdx), %xmm12
// CHECK: encoding: [0xc4,0x62,0x79,0x44,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vplzcntd  -2048(%rdx), %xmm12

// CHECK:      {vex} vplzcntd  268435456(%rbp,%r14,8), %ymm12
// CHECK: encoding: [0xc4,0x22,0x7d,0x44,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vplzcntd  268435456(%rbp,%r14,8), %ymm12

// CHECK:      {vex} vplzcntd  291(%r8,%rax,4), %ymm12
// CHECK: encoding: [0xc4,0x42,0x7d,0x44,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vplzcntd  291(%r8,%rax,4), %ymm12

// CHECK:      {vex} vplzcntd  (%rip), %ymm12
// CHECK: encoding: [0xc4,0x62,0x7d,0x44,0x25,0x00,0x00,0x00,0x00]
               {vex} vplzcntd  (%rip), %ymm12

// CHECK:      {vex} vplzcntd  -1024(,%rbp,2), %ymm12
// CHECK: encoding: [0xc4,0x62,0x7d,0x44,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vplzcntd  -1024(,%rbp,2), %ymm12

// CHECK:      {vex} vplzcntd  4064(%rcx), %ymm12
// CHECK: encoding: [0xc4,0x62,0x7d,0x44,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vplzcntd  4064(%rcx), %ymm12

// CHECK:      {vex} vplzcntd  -4096(%rdx), %ymm12
// CHECK: encoding: [0xc4,0x62,0x7d,0x44,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vplzcntd  -4096(%rdx), %ymm12

// CHECK:      {vex} vplzcntq %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0xf9,0x44,0xe5]
               {vex} vplzcntq %xmm13, %xmm12

// CHECK:      {vex} vplzcntq %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0xfd,0x44,0xe5]
               {vex} vplzcntq %ymm13, %ymm12

// CHECK:      {vex} vplzcntq  268435456(%rbp,%r14,8), %xmm12
// CHECK: encoding: [0xc4,0x22,0xf9,0x44,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vplzcntq  268435456(%rbp,%r14,8), %xmm12

// CHECK:      {vex} vplzcntq  291(%r8,%rax,4), %xmm12
// CHECK: encoding: [0xc4,0x42,0xf9,0x44,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vplzcntq  291(%r8,%rax,4), %xmm12

// CHECK:      {vex} vplzcntq  (%rip), %xmm12
// CHECK: encoding: [0xc4,0x62,0xf9,0x44,0x25,0x00,0x00,0x00,0x00]
               {vex} vplzcntq  (%rip), %xmm12

// CHECK:      {vex} vplzcntq  -512(,%rbp,2), %xmm12
// CHECK: encoding: [0xc4,0x62,0xf9,0x44,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vplzcntq  -512(,%rbp,2), %xmm12

// CHECK:      {vex} vplzcntq  2032(%rcx), %xmm12
// CHECK: encoding: [0xc4,0x62,0xf9,0x44,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vplzcntq  2032(%rcx), %xmm12

// CHECK:      {vex} vplzcntq  -2048(%rdx), %xmm12
// CHECK: encoding: [0xc4,0x62,0xf9,0x44,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vplzcntq  -2048(%rdx), %xmm12

// CHECK:      {vex} vplzcntq  268435456(%rbp,%r14,8), %ymm12
// CHECK: encoding: [0xc4,0x22,0xfd,0x44,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vplzcntq  268435456(%rbp,%r14,8), %ymm12

// CHECK:      {vex} vplzcntq  291(%r8,%rax,4), %ymm12
// CHECK: encoding: [0xc4,0x42,0xfd,0x44,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vplzcntq  291(%r8,%rax,4), %ymm12

// CHECK:      {vex} vplzcntq  (%rip), %ymm12
// CHECK: encoding: [0xc4,0x62,0xfd,0x44,0x25,0x00,0x00,0x00,0x00]
               {vex} vplzcntq  (%rip), %ymm12

// CHECK:      {vex} vplzcntq  -1024(,%rbp,2), %ymm12
// CHECK: encoding: [0xc4,0x62,0xfd,0x44,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vplzcntq  -1024(,%rbp,2), %ymm12

// CHECK:      {vex} vplzcntq  4064(%rcx), %ymm12
// CHECK: encoding: [0xc4,0x62,0xfd,0x44,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vplzcntq  4064(%rcx), %ymm12

// CHECK:      {vex} vplzcntq  -4096(%rdx), %ymm12
// CHECK: encoding: [0xc4,0x62,0xfd,0x44,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vplzcntq  -4096(%rdx), %ymm12

// CHECK:      vpmaskmovb  %ymm13, %ymm12, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0x22,0x1c,0x8e,0xac,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovb  %ymm13, %ymm12, 268435456(%rbp,%r14,8)

// CHECK:      vpmaskmovb  %ymm13, %ymm12, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0x42,0x1c,0x8e,0xac,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovb  %ymm13, %ymm12, 291(%r8,%rax,4)

// CHECK:      vpmaskmovb  %ymm13, %ymm12, (%rip)
// CHECK: encoding: [0xc4,0x62,0x1c,0x8e,0x2d,0x00,0x00,0x00,0x00]
               vpmaskmovb  %ymm13, %ymm12, (%rip)

// CHECK:      vpmaskmovb  %ymm13, %ymm12, -1024(,%rbp,2)
// CHECK: encoding: [0xc4,0x62,0x1c,0x8e,0x2c,0x6d,0x00,0xfc,0xff,0xff]
               vpmaskmovb  %ymm13, %ymm12, -1024(,%rbp,2)

// CHECK:      vpmaskmovb  %ymm13, %ymm12, 4064(%rcx)
// CHECK: encoding: [0xc4,0x62,0x1c,0x8e,0xa9,0xe0,0x0f,0x00,0x00]
               vpmaskmovb  %ymm13, %ymm12, 4064(%rcx)

// CHECK:      vpmaskmovb  %ymm13, %ymm12, -4096(%rdx)
// CHECK: encoding: [0xc4,0x62,0x1c,0x8e,0xaa,0x00,0xf0,0xff,0xff]
               vpmaskmovb  %ymm13, %ymm12, -4096(%rdx)

// CHECK:      vpmaskmovb  %xmm13, %xmm12, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0x22,0x18,0x8e,0xac,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovb  %xmm13, %xmm12, 268435456(%rbp,%r14,8)

// CHECK:      vpmaskmovb  %xmm13, %xmm12, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0x42,0x18,0x8e,0xac,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovb  %xmm13, %xmm12, 291(%r8,%rax,4)

// CHECK:      vpmaskmovb  %xmm13, %xmm12, (%rip)
// CHECK: encoding: [0xc4,0x62,0x18,0x8e,0x2d,0x00,0x00,0x00,0x00]
               vpmaskmovb  %xmm13, %xmm12, (%rip)

// CHECK:      vpmaskmovb  %xmm13, %xmm12, -512(,%rbp,2)
// CHECK: encoding: [0xc4,0x62,0x18,0x8e,0x2c,0x6d,0x00,0xfe,0xff,0xff]
               vpmaskmovb  %xmm13, %xmm12, -512(,%rbp,2)

// CHECK:      vpmaskmovb  %xmm13, %xmm12, 2032(%rcx)
// CHECK: encoding: [0xc4,0x62,0x18,0x8e,0xa9,0xf0,0x07,0x00,0x00]
               vpmaskmovb  %xmm13, %xmm12, 2032(%rcx)

// CHECK:      vpmaskmovb  %xmm13, %xmm12, -2048(%rdx)
// CHECK: encoding: [0xc4,0x62,0x18,0x8e,0xaa,0x00,0xf8,0xff,0xff]
               vpmaskmovb  %xmm13, %xmm12, -2048(%rdx)

// CHECK:      vpmaskmovb  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x14,0x8c,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovb  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      vpmaskmovb  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x14,0x8c,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovb  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      vpmaskmovb  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x14,0x8c,0x25,0x00,0x00,0x00,0x00]
               vpmaskmovb  (%rip), %ymm13, %ymm12

// CHECK:      vpmaskmovb  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x14,0x8c,0x24,0x6d,0x00,0xfc,0xff,0xff]
               vpmaskmovb  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      vpmaskmovb  4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x14,0x8c,0xa1,0xe0,0x0f,0x00,0x00]
               vpmaskmovb  4064(%rcx), %ymm13, %ymm12

// CHECK:      vpmaskmovb  -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x14,0x8c,0xa2,0x00,0xf0,0xff,0xff]
               vpmaskmovb  -4096(%rdx), %ymm13, %ymm12

// CHECK:      vpmaskmovb  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x10,0x8c,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovb  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      vpmaskmovb  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x10,0x8c,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovb  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      vpmaskmovb  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x10,0x8c,0x25,0x00,0x00,0x00,0x00]
               vpmaskmovb  (%rip), %xmm13, %xmm12

// CHECK:      vpmaskmovb  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x10,0x8c,0x24,0x6d,0x00,0xfe,0xff,0xff]
               vpmaskmovb  -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      vpmaskmovb  2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x10,0x8c,0xa1,0xf0,0x07,0x00,0x00]
               vpmaskmovb  2032(%rcx), %xmm13, %xmm12

// CHECK:      vpmaskmovb  -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x10,0x8c,0xa2,0x00,0xf8,0xff,0xff]
               vpmaskmovb  -2048(%rdx), %xmm13, %xmm12

// CHECK:      vpmaskmovw  %ymm13, %ymm12, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0x22,0x9c,0x8e,0xac,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovw  %ymm13, %ymm12, 268435456(%rbp,%r14,8)

// CHECK:      vpmaskmovw  %ymm13, %ymm12, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0x42,0x9c,0x8e,0xac,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovw  %ymm13, %ymm12, 291(%r8,%rax,4)

// CHECK:      vpmaskmovw  %ymm13, %ymm12, (%rip)
// CHECK: encoding: [0xc4,0x62,0x9c,0x8e,0x2d,0x00,0x00,0x00,0x00]
               vpmaskmovw  %ymm13, %ymm12, (%rip)

// CHECK:      vpmaskmovw  %ymm13, %ymm12, -1024(,%rbp,2)
// CHECK: encoding: [0xc4,0x62,0x9c,0x8e,0x2c,0x6d,0x00,0xfc,0xff,0xff]
               vpmaskmovw  %ymm13, %ymm12, -1024(,%rbp,2)

// CHECK:      vpmaskmovw  %ymm13, %ymm12, 4064(%rcx)
// CHECK: encoding: [0xc4,0x62,0x9c,0x8e,0xa9,0xe0,0x0f,0x00,0x00]
               vpmaskmovw  %ymm13, %ymm12, 4064(%rcx)

// CHECK:      vpmaskmovw  %ymm13, %ymm12, -4096(%rdx)
// CHECK: encoding: [0xc4,0x62,0x9c,0x8e,0xaa,0x00,0xf0,0xff,0xff]
               vpmaskmovw  %ymm13, %ymm12, -4096(%rdx)

// CHECK:      vpmaskmovw  %xmm13, %xmm12, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0x22,0x98,0x8e,0xac,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovw  %xmm13, %xmm12, 268435456(%rbp,%r14,8)

// CHECK:      vpmaskmovw  %xmm13, %xmm12, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0x42,0x98,0x8e,0xac,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovw  %xmm13, %xmm12, 291(%r8,%rax,4)

// CHECK:      vpmaskmovw  %xmm13, %xmm12, (%rip)
// CHECK: encoding: [0xc4,0x62,0x98,0x8e,0x2d,0x00,0x00,0x00,0x00]
               vpmaskmovw  %xmm13, %xmm12, (%rip)

// CHECK:      vpmaskmovw  %xmm13, %xmm12, -512(,%rbp,2)
// CHECK: encoding: [0xc4,0x62,0x98,0x8e,0x2c,0x6d,0x00,0xfe,0xff,0xff]
               vpmaskmovw  %xmm13, %xmm12, -512(,%rbp,2)

// CHECK:      vpmaskmovw  %xmm13, %xmm12, 2032(%rcx)
// CHECK: encoding: [0xc4,0x62,0x98,0x8e,0xa9,0xf0,0x07,0x00,0x00]
               vpmaskmovw  %xmm13, %xmm12, 2032(%rcx)

// CHECK:      vpmaskmovw  %xmm13, %xmm12, -2048(%rdx)
// CHECK: encoding: [0xc4,0x62,0x98,0x8e,0xaa,0x00,0xf8,0xff,0xff]
               vpmaskmovw  %xmm13, %xmm12, -2048(%rdx)

// CHECK:      vpmaskmovw  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x94,0x8c,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovw  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      vpmaskmovw  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x94,0x8c,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovw  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      vpmaskmovw  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x94,0x8c,0x25,0x00,0x00,0x00,0x00]
               vpmaskmovw  (%rip), %ymm13, %ymm12

// CHECK:      vpmaskmovw  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x94,0x8c,0x24,0x6d,0x00,0xfc,0xff,0xff]
               vpmaskmovw  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      vpmaskmovw  4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x94,0x8c,0xa1,0xe0,0x0f,0x00,0x00]
               vpmaskmovw  4064(%rcx), %ymm13, %ymm12

// CHECK:      vpmaskmovw  -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x94,0x8c,0xa2,0x00,0xf0,0xff,0xff]
               vpmaskmovw  -4096(%rdx), %ymm13, %ymm12

// CHECK:      vpmaskmovw  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x90,0x8c,0xa4,0xf5,0x00,0x00,0x00,0x10]
               vpmaskmovw  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      vpmaskmovw  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x90,0x8c,0xa4,0x80,0x23,0x01,0x00,0x00]
               vpmaskmovw  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      vpmaskmovw  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x90,0x8c,0x25,0x00,0x00,0x00,0x00]
               vpmaskmovw  (%rip), %xmm13, %xmm12

// CHECK:      vpmaskmovw  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x90,0x8c,0x24,0x6d,0x00,0xfe,0xff,0xff]
               vpmaskmovw  -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      vpmaskmovw  2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x90,0x8c,0xa1,0xf0,0x07,0x00,0x00]
               vpmaskmovw  2032(%rcx), %xmm13, %xmm12

// CHECK:      vpmaskmovw  -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x90,0x8c,0xa2,0x00,0xf8,0xff,0xff]
               vpmaskmovw  -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpopcntb %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x79,0x54,0xe5]
               {vex} vpopcntb %xmm13, %xmm12

// CHECK:      {vex} vpopcntb %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x7d,0x54,0xe5]
               {vex} vpopcntb %ymm13, %ymm12

// CHECK:      {vex} vpopcntb  268435456(%rbp,%r14,8), %xmm12
// CHECK: encoding: [0xc4,0x22,0x79,0x54,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntb  268435456(%rbp,%r14,8), %xmm12

// CHECK:      {vex} vpopcntb  291(%r8,%rax,4), %xmm12
// CHECK: encoding: [0xc4,0x42,0x79,0x54,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntb  291(%r8,%rax,4), %xmm12

// CHECK:      {vex} vpopcntb  (%rip), %xmm12
// CHECK: encoding: [0xc4,0x62,0x79,0x54,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntb  (%rip), %xmm12

// CHECK:      {vex} vpopcntb  -512(,%rbp,2), %xmm12
// CHECK: encoding: [0xc4,0x62,0x79,0x54,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpopcntb  -512(,%rbp,2), %xmm12

// CHECK:      {vex} vpopcntb  2032(%rcx), %xmm12
// CHECK: encoding: [0xc4,0x62,0x79,0x54,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpopcntb  2032(%rcx), %xmm12

// CHECK:      {vex} vpopcntb  -2048(%rdx), %xmm12
// CHECK: encoding: [0xc4,0x62,0x79,0x54,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpopcntb  -2048(%rdx), %xmm12

// CHECK:      {vex} vpopcntb  268435456(%rbp,%r14,8), %ymm12
// CHECK: encoding: [0xc4,0x22,0x7d,0x54,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntb  268435456(%rbp,%r14,8), %ymm12

// CHECK:      {vex} vpopcntb  291(%r8,%rax,4), %ymm12
// CHECK: encoding: [0xc4,0x42,0x7d,0x54,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntb  291(%r8,%rax,4), %ymm12

// CHECK:      {vex} vpopcntb  (%rip), %ymm12
// CHECK: encoding: [0xc4,0x62,0x7d,0x54,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntb  (%rip), %ymm12

// CHECK:      {vex} vpopcntb  -1024(,%rbp,2), %ymm12
// CHECK: encoding: [0xc4,0x62,0x7d,0x54,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpopcntb  -1024(,%rbp,2), %ymm12

// CHECK:      {vex} vpopcntb  4064(%rcx), %ymm12
// CHECK: encoding: [0xc4,0x62,0x7d,0x54,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpopcntb  4064(%rcx), %ymm12

// CHECK:      {vex} vpopcntb  -4096(%rdx), %ymm12
// CHECK: encoding: [0xc4,0x62,0x7d,0x54,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpopcntb  -4096(%rdx), %ymm12

// CHECK:      {vex} vpopcntd %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x79,0x55,0xe5]
               {vex} vpopcntd %xmm13, %xmm12

// CHECK:      {vex} vpopcntd %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x7d,0x55,0xe5]
               {vex} vpopcntd %ymm13, %ymm12

// CHECK:      {vex} vpopcntd  268435456(%rbp,%r14,8), %xmm12
// CHECK: encoding: [0xc4,0x22,0x79,0x55,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntd  268435456(%rbp,%r14,8), %xmm12

// CHECK:      {vex} vpopcntd  291(%r8,%rax,4), %xmm12
// CHECK: encoding: [0xc4,0x42,0x79,0x55,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntd  291(%r8,%rax,4), %xmm12

// CHECK:      {vex} vpopcntd  (%rip), %xmm12
// CHECK: encoding: [0xc4,0x62,0x79,0x55,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntd  (%rip), %xmm12

// CHECK:      {vex} vpopcntd  -512(,%rbp,2), %xmm12
// CHECK: encoding: [0xc4,0x62,0x79,0x55,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpopcntd  -512(,%rbp,2), %xmm12

// CHECK:      {vex} vpopcntd  2032(%rcx), %xmm12
// CHECK: encoding: [0xc4,0x62,0x79,0x55,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpopcntd  2032(%rcx), %xmm12

// CHECK:      {vex} vpopcntd  -2048(%rdx), %xmm12
// CHECK: encoding: [0xc4,0x62,0x79,0x55,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpopcntd  -2048(%rdx), %xmm12

// CHECK:      {vex} vpopcntd  268435456(%rbp,%r14,8), %ymm12
// CHECK: encoding: [0xc4,0x22,0x7d,0x55,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntd  268435456(%rbp,%r14,8), %ymm12

// CHECK:      {vex} vpopcntd  291(%r8,%rax,4), %ymm12
// CHECK: encoding: [0xc4,0x42,0x7d,0x55,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntd  291(%r8,%rax,4), %ymm12

// CHECK:      {vex} vpopcntd  (%rip), %ymm12
// CHECK: encoding: [0xc4,0x62,0x7d,0x55,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntd  (%rip), %ymm12

// CHECK:      {vex} vpopcntd  -1024(,%rbp,2), %ymm12
// CHECK: encoding: [0xc4,0x62,0x7d,0x55,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpopcntd  -1024(,%rbp,2), %ymm12

// CHECK:      {vex} vpopcntd  4064(%rcx), %ymm12
// CHECK: encoding: [0xc4,0x62,0x7d,0x55,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpopcntd  4064(%rcx), %ymm12

// CHECK:      {vex} vpopcntd  -4096(%rdx), %ymm12
// CHECK: encoding: [0xc4,0x62,0x7d,0x55,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpopcntd  -4096(%rdx), %ymm12

// CHECK:      {vex} vpopcntq %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0xf9,0x55,0xe5]
               {vex} vpopcntq %xmm13, %xmm12

// CHECK:      {vex} vpopcntq %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0xfd,0x55,0xe5]
               {vex} vpopcntq %ymm13, %ymm12

// CHECK:      {vex} vpopcntq  268435456(%rbp,%r14,8), %xmm12
// CHECK: encoding: [0xc4,0x22,0xf9,0x55,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntq  268435456(%rbp,%r14,8), %xmm12

// CHECK:      {vex} vpopcntq  291(%r8,%rax,4), %xmm12
// CHECK: encoding: [0xc4,0x42,0xf9,0x55,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntq  291(%r8,%rax,4), %xmm12

// CHECK:      {vex} vpopcntq  (%rip), %xmm12
// CHECK: encoding: [0xc4,0x62,0xf9,0x55,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntq  (%rip), %xmm12

// CHECK:      {vex} vpopcntq  -512(,%rbp,2), %xmm12
// CHECK: encoding: [0xc4,0x62,0xf9,0x55,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpopcntq  -512(,%rbp,2), %xmm12

// CHECK:      {vex} vpopcntq  2032(%rcx), %xmm12
// CHECK: encoding: [0xc4,0x62,0xf9,0x55,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpopcntq  2032(%rcx), %xmm12

// CHECK:      {vex} vpopcntq  -2048(%rdx), %xmm12
// CHECK: encoding: [0xc4,0x62,0xf9,0x55,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpopcntq  -2048(%rdx), %xmm12

// CHECK:      {vex} vpopcntq  268435456(%rbp,%r14,8), %ymm12
// CHECK: encoding: [0xc4,0x22,0xfd,0x55,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntq  268435456(%rbp,%r14,8), %ymm12

// CHECK:      {vex} vpopcntq  291(%r8,%rax,4), %ymm12
// CHECK: encoding: [0xc4,0x42,0xfd,0x55,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntq  291(%r8,%rax,4), %ymm12

// CHECK:      {vex} vpopcntq  (%rip), %ymm12
// CHECK: encoding: [0xc4,0x62,0xfd,0x55,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntq  (%rip), %ymm12

// CHECK:      {vex} vpopcntq  -1024(,%rbp,2), %ymm12
// CHECK: encoding: [0xc4,0x62,0xfd,0x55,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpopcntq  -1024(,%rbp,2), %ymm12

// CHECK:      {vex} vpopcntq  4064(%rcx), %ymm12
// CHECK: encoding: [0xc4,0x62,0xfd,0x55,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpopcntq  4064(%rcx), %ymm12

// CHECK:      {vex} vpopcntq  -4096(%rdx), %ymm12
// CHECK: encoding: [0xc4,0x62,0xfd,0x55,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpopcntq  -4096(%rdx), %ymm12

// CHECK:      {vex} vpopcntw %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0xf9,0x54,0xe5]
               {vex} vpopcntw %xmm13, %xmm12

// CHECK:      {vex} vpopcntw %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0xfd,0x54,0xe5]
               {vex} vpopcntw %ymm13, %ymm12

// CHECK:      {vex} vpopcntw  268435456(%rbp,%r14,8), %xmm12
// CHECK: encoding: [0xc4,0x22,0xf9,0x54,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntw  268435456(%rbp,%r14,8), %xmm12

// CHECK:      {vex} vpopcntw  291(%r8,%rax,4), %xmm12
// CHECK: encoding: [0xc4,0x42,0xf9,0x54,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntw  291(%r8,%rax,4), %xmm12

// CHECK:      {vex} vpopcntw  (%rip), %xmm12
// CHECK: encoding: [0xc4,0x62,0xf9,0x54,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntw  (%rip), %xmm12

// CHECK:      {vex} vpopcntw  -512(,%rbp,2), %xmm12
// CHECK: encoding: [0xc4,0x62,0xf9,0x54,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpopcntw  -512(,%rbp,2), %xmm12

// CHECK:      {vex} vpopcntw  2032(%rcx), %xmm12
// CHECK: encoding: [0xc4,0x62,0xf9,0x54,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpopcntw  2032(%rcx), %xmm12

// CHECK:      {vex} vpopcntw  -2048(%rdx), %xmm12
// CHECK: encoding: [0xc4,0x62,0xf9,0x54,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpopcntw  -2048(%rdx), %xmm12

// CHECK:      {vex} vpopcntw  268435456(%rbp,%r14,8), %ymm12
// CHECK: encoding: [0xc4,0x22,0xfd,0x54,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpopcntw  268435456(%rbp,%r14,8), %ymm12

// CHECK:      {vex} vpopcntw  291(%r8,%rax,4), %ymm12
// CHECK: encoding: [0xc4,0x42,0xfd,0x54,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpopcntw  291(%r8,%rax,4), %ymm12

// CHECK:      {vex} vpopcntw  (%rip), %ymm12
// CHECK: encoding: [0xc4,0x62,0xfd,0x54,0x25,0x00,0x00,0x00,0x00]
               {vex} vpopcntw  (%rip), %ymm12

// CHECK:      {vex} vpopcntw  -1024(,%rbp,2), %ymm12
// CHECK: encoding: [0xc4,0x62,0xfd,0x54,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpopcntw  -1024(,%rbp,2), %ymm12

// CHECK:      {vex} vpopcntw  4064(%rcx), %ymm12
// CHECK: encoding: [0xc4,0x62,0xfd,0x54,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpopcntw  4064(%rcx), %ymm12

// CHECK:      {vex} vpopcntw  -4096(%rdx), %ymm12
// CHECK: encoding: [0xc4,0x62,0xfd,0x54,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpopcntw  -4096(%rdx), %ymm12

// CHECK:      {vex} vprold $123, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0xc1,0x1d,0x72,0xcd,0x7b]
               {vex} vprold $123, %ymm13, %ymm12

// CHECK:      {vex} vprold $123, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0xc1,0x19,0x72,0xcd,0x7b]
               {vex} vprold $123, %xmm13, %xmm12

// CHECK:      {vex} vprold  $123, 268435456(%rbp,%r14,8), %xmm12
// CHECK: encoding: [0xc4,0xa1,0x19,0x72,0x8c,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprold  $123, 268435456(%rbp,%r14,8), %xmm12

// CHECK:      {vex} vprold  $123, 291(%r8,%rax,4), %xmm12
// CHECK: encoding: [0xc4,0xc1,0x19,0x72,0x8c,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprold  $123, 291(%r8,%rax,4), %xmm12

// CHECK:      {vex} vprold  $123, (%rip), %xmm12
// CHECK: encoding: [0xc5,0x99,0x72,0x0d,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprold  $123, (%rip), %xmm12

// CHECK:      {vex} vprold  $123, -512(,%rbp,2), %xmm12
// CHECK: encoding: [0xc5,0x99,0x72,0x0c,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vprold  $123, -512(,%rbp,2), %xmm12

// CHECK:      {vex} vprold  $123, 2032(%rcx), %xmm12
// CHECK: encoding: [0xc5,0x99,0x72,0x89,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vprold  $123, 2032(%rcx), %xmm12

// CHECK:      {vex} vprold  $123, -2048(%rdx), %xmm12
// CHECK: encoding: [0xc5,0x99,0x72,0x8a,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vprold  $123, -2048(%rdx), %xmm12

// CHECK:      {vex} vprold  $123, 268435456(%rbp,%r14,8), %ymm12
// CHECK: encoding: [0xc4,0xa1,0x1d,0x72,0x8c,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprold  $123, 268435456(%rbp,%r14,8), %ymm12

// CHECK:      {vex} vprold  $123, 291(%r8,%rax,4), %ymm12
// CHECK: encoding: [0xc4,0xc1,0x1d,0x72,0x8c,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprold  $123, 291(%r8,%rax,4), %ymm12

// CHECK:      {vex} vprold  $123, (%rip), %ymm12
// CHECK: encoding: [0xc5,0x9d,0x72,0x0d,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprold  $123, (%rip), %ymm12

// CHECK:      {vex} vprold  $123, -1024(,%rbp,2), %ymm12
// CHECK: encoding: [0xc5,0x9d,0x72,0x0c,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vprold  $123, -1024(,%rbp,2), %ymm12

// CHECK:      {vex} vprold  $123, 4064(%rcx), %ymm12
// CHECK: encoding: [0xc5,0x9d,0x72,0x89,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vprold  $123, 4064(%rcx), %ymm12

// CHECK:      {vex} vprold  $123, -4096(%rdx), %ymm12
// CHECK: encoding: [0xc5,0x9d,0x72,0x8a,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vprold  $123, -4096(%rdx), %ymm12

// CHECK:      {vex} vprolq $123, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0xc1,0x9d,0x72,0xcd,0x7b]
               {vex} vprolq $123, %ymm13, %ymm12

// CHECK:      {vex} vprolq $123, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0xc1,0x99,0x72,0xcd,0x7b]
               {vex} vprolq $123, %xmm13, %xmm12

// CHECK:      {vex} vprolq  $123, 268435456(%rbp,%r14,8), %xmm12
// CHECK: encoding: [0xc4,0xa1,0x99,0x72,0x8c,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprolq  $123, 268435456(%rbp,%r14,8), %xmm12

// CHECK:      {vex} vprolq  $123, 291(%r8,%rax,4), %xmm12
// CHECK: encoding: [0xc4,0xc1,0x99,0x72,0x8c,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprolq  $123, 291(%r8,%rax,4), %xmm12

// CHECK:      {vex} vprolq  $123, (%rip), %xmm12
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x0d,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprolq  $123, (%rip), %xmm12

// CHECK:      {vex} vprolq  $123, -512(,%rbp,2), %xmm12
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x0c,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vprolq  $123, -512(,%rbp,2), %xmm12

// CHECK:      {vex} vprolq  $123, 2032(%rcx), %xmm12
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x89,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vprolq  $123, 2032(%rcx), %xmm12

// CHECK:      {vex} vprolq  $123, -2048(%rdx), %xmm12
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x8a,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vprolq  $123, -2048(%rdx), %xmm12

// CHECK:      {vex} vprolq  $123, 268435456(%rbp,%r14,8), %ymm12
// CHECK: encoding: [0xc4,0xa1,0x9d,0x72,0x8c,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprolq  $123, 268435456(%rbp,%r14,8), %ymm12

// CHECK:      {vex} vprolq  $123, 291(%r8,%rax,4), %ymm12
// CHECK: encoding: [0xc4,0xc1,0x9d,0x72,0x8c,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprolq  $123, 291(%r8,%rax,4), %ymm12

// CHECK:      {vex} vprolq  $123, (%rip), %ymm12
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x0d,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprolq  $123, (%rip), %ymm12

// CHECK:      {vex} vprolq  $123, -1024(,%rbp,2), %ymm12
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x0c,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vprolq  $123, -1024(,%rbp,2), %ymm12

// CHECK:      {vex} vprolq  $123, 4064(%rcx), %ymm12
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x89,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vprolq  $123, 4064(%rcx), %ymm12

// CHECK:      {vex} vprolq  $123, -4096(%rdx), %ymm12
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x8a,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vprolq  $123, -4096(%rdx), %ymm12

// CHECK:      {vex} vprolvd %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x15,0x15,0xe6]
               {vex} vprolvd %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vprolvd %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x11,0x15,0xe6]
               {vex} vprolvd %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vprolvd  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x15,0x15,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprolvd  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vprolvd  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x15,0x15,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprolvd  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vprolvd  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x15,0x25,0x00,0x00,0x00,0x00]
               {vex} vprolvd  (%rip), %ymm13, %ymm12

// CHECK:      {vex} vprolvd  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x15,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vprolvd  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vprolvd  4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x15,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vprolvd  4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vprolvd  -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x15,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vprolvd  -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vprolvd  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x11,0x15,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprolvd  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vprolvd  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x11,0x15,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprolvd  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vprolvd  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x15,0x25,0x00,0x00,0x00,0x00]
               {vex} vprolvd  (%rip), %xmm13, %xmm12

// CHECK:      {vex} vprolvd  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x15,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vprolvd  -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vprolvd  2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x15,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vprolvd  2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vprolvd  -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x15,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vprolvd  -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vprolvq %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x95,0x15,0xe6]
               {vex} vprolvq %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vprolvq %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x91,0x15,0xe6]
               {vex} vprolvq %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vprolvq  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x95,0x15,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprolvq  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vprolvq  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x95,0x15,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprolvq  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vprolvq  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x15,0x25,0x00,0x00,0x00,0x00]
               {vex} vprolvq  (%rip), %ymm13, %ymm12

// CHECK:      {vex} vprolvq  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x15,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vprolvq  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vprolvq  4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x15,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vprolvq  4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vprolvq  -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x15,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vprolvq  -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vprolvq  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x91,0x15,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprolvq  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vprolvq  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x91,0x15,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprolvq  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vprolvq  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x15,0x25,0x00,0x00,0x00,0x00]
               {vex} vprolvq  (%rip), %xmm13, %xmm12

// CHECK:      {vex} vprolvq  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x15,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vprolvq  -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vprolvq  2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x15,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vprolvq  2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vprolvq  -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x15,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vprolvq  -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vprord $123, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0xc1,0x1d,0x72,0xc5,0x7b]
               {vex} vprord $123, %ymm13, %ymm12

// CHECK:      {vex} vprord $123, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0xc1,0x19,0x72,0xc5,0x7b]
               {vex} vprord $123, %xmm13, %xmm12

// CHECK:      {vex} vprord  $123, 268435456(%rbp,%r14,8), %xmm12
// CHECK: encoding: [0xc4,0xa1,0x19,0x72,0x84,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprord  $123, 268435456(%rbp,%r14,8), %xmm12

// CHECK:      {vex} vprord  $123, 291(%r8,%rax,4), %xmm12
// CHECK: encoding: [0xc4,0xc1,0x19,0x72,0x84,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprord  $123, 291(%r8,%rax,4), %xmm12

// CHECK:      {vex} vprord  $123, (%rip), %xmm12
// CHECK: encoding: [0xc5,0x99,0x72,0x05,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprord  $123, (%rip), %xmm12

// CHECK:      {vex} vprord  $123, -512(,%rbp,2), %xmm12
// CHECK: encoding: [0xc5,0x99,0x72,0x04,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vprord  $123, -512(,%rbp,2), %xmm12

// CHECK:      {vex} vprord  $123, 2032(%rcx), %xmm12
// CHECK: encoding: [0xc5,0x99,0x72,0x81,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vprord  $123, 2032(%rcx), %xmm12

// CHECK:      {vex} vprord  $123, -2048(%rdx), %xmm12
// CHECK: encoding: [0xc5,0x99,0x72,0x82,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vprord  $123, -2048(%rdx), %xmm12

// CHECK:      {vex} vprord  $123, 268435456(%rbp,%r14,8), %ymm12
// CHECK: encoding: [0xc4,0xa1,0x1d,0x72,0x84,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprord  $123, 268435456(%rbp,%r14,8), %ymm12

// CHECK:      {vex} vprord  $123, 291(%r8,%rax,4), %ymm12
// CHECK: encoding: [0xc4,0xc1,0x1d,0x72,0x84,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprord  $123, 291(%r8,%rax,4), %ymm12

// CHECK:      {vex} vprord  $123, (%rip), %ymm12
// CHECK: encoding: [0xc5,0x9d,0x72,0x05,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprord  $123, (%rip), %ymm12

// CHECK:      {vex} vprord  $123, -1024(,%rbp,2), %ymm12
// CHECK: encoding: [0xc5,0x9d,0x72,0x04,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vprord  $123, -1024(,%rbp,2), %ymm12

// CHECK:      {vex} vprord  $123, 4064(%rcx), %ymm12
// CHECK: encoding: [0xc5,0x9d,0x72,0x81,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vprord  $123, 4064(%rcx), %ymm12

// CHECK:      {vex} vprord  $123, -4096(%rdx), %ymm12
// CHECK: encoding: [0xc5,0x9d,0x72,0x82,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vprord  $123, -4096(%rdx), %ymm12

// CHECK:      {vex} vprorq $123, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0xc1,0x9d,0x72,0xc5,0x7b]
               {vex} vprorq $123, %ymm13, %ymm12

// CHECK:      {vex} vprorq $123, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0xc1,0x99,0x72,0xc5,0x7b]
               {vex} vprorq $123, %xmm13, %xmm12

// CHECK:      {vex} vprorq  $123, 268435456(%rbp,%r14,8), %xmm12
// CHECK: encoding: [0xc4,0xa1,0x99,0x72,0x84,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprorq  $123, 268435456(%rbp,%r14,8), %xmm12

// CHECK:      {vex} vprorq  $123, 291(%r8,%rax,4), %xmm12
// CHECK: encoding: [0xc4,0xc1,0x99,0x72,0x84,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprorq  $123, 291(%r8,%rax,4), %xmm12

// CHECK:      {vex} vprorq  $123, (%rip), %xmm12
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x05,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprorq  $123, (%rip), %xmm12

// CHECK:      {vex} vprorq  $123, -512(,%rbp,2), %xmm12
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x04,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vprorq  $123, -512(,%rbp,2), %xmm12

// CHECK:      {vex} vprorq  $123, 2032(%rcx), %xmm12
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x81,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vprorq  $123, 2032(%rcx), %xmm12

// CHECK:      {vex} vprorq  $123, -2048(%rdx), %xmm12
// CHECK: encoding: [0xc4,0xe1,0x99,0x72,0x82,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vprorq  $123, -2048(%rdx), %xmm12

// CHECK:      {vex} vprorq  $123, 268435456(%rbp,%r14,8), %ymm12
// CHECK: encoding: [0xc4,0xa1,0x9d,0x72,0x84,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vprorq  $123, 268435456(%rbp,%r14,8), %ymm12

// CHECK:      {vex} vprorq  $123, 291(%r8,%rax,4), %ymm12
// CHECK: encoding: [0xc4,0xc1,0x9d,0x72,0x84,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vprorq  $123, 291(%r8,%rax,4), %ymm12

// CHECK:      {vex} vprorq  $123, (%rip), %ymm12
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x05,0x00,0x00,0x00,0x00,0x7b]
               {vex} vprorq  $123, (%rip), %ymm12

// CHECK:      {vex} vprorq  $123, -1024(,%rbp,2), %ymm12
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x04,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vprorq  $123, -1024(,%rbp,2), %ymm12

// CHECK:      {vex} vprorq  $123, 4064(%rcx), %ymm12
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x81,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vprorq  $123, 4064(%rcx), %ymm12

// CHECK:      {vex} vprorq  $123, -4096(%rdx), %ymm12
// CHECK: encoding: [0xc4,0xe1,0x9d,0x72,0x82,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vprorq  $123, -4096(%rdx), %ymm12

// CHECK:      {vex} vprorvd %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x15,0x14,0xe6]
               {vex} vprorvd %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vprorvd %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x11,0x14,0xe6]
               {vex} vprorvd %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vprorvd  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x15,0x14,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprorvd  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vprorvd  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x15,0x14,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprorvd  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vprorvd  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x14,0x25,0x00,0x00,0x00,0x00]
               {vex} vprorvd  (%rip), %ymm13, %ymm12

// CHECK:      {vex} vprorvd  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x14,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vprorvd  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vprorvd  4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x14,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vprorvd  4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vprorvd  -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x14,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vprorvd  -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vprorvd  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x11,0x14,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprorvd  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vprorvd  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x11,0x14,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprorvd  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vprorvd  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x14,0x25,0x00,0x00,0x00,0x00]
               {vex} vprorvd  (%rip), %xmm13, %xmm12

// CHECK:      {vex} vprorvd  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x14,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vprorvd  -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vprorvd  2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x14,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vprorvd  2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vprorvd  -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x14,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vprorvd  -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vprorvq %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x95,0x14,0xe6]
               {vex} vprorvq %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vprorvq %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x91,0x14,0xe6]
               {vex} vprorvq %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vprorvq  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x95,0x14,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprorvq  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vprorvq  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x95,0x14,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprorvq  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vprorvq  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x14,0x25,0x00,0x00,0x00,0x00]
               {vex} vprorvq  (%rip), %ymm13, %ymm12

// CHECK:      {vex} vprorvq  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x14,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vprorvq  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vprorvq  4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x14,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vprorvq  4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vprorvq  -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x14,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vprorvq  -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vprorvq  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x91,0x14,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vprorvq  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vprorvq  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x91,0x14,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vprorvq  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vprorvq  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x14,0x25,0x00,0x00,0x00,0x00]
               {vex} vprorvq  (%rip), %xmm13, %xmm12

// CHECK:      {vex} vprorvq  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x14,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vprorvq  -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vprorvq  2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x14,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vprorvq  2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vprorvq  -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x14,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vprorvq  -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpshldd $123, %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x11,0x71,0xe6,0x7b]
               {vex} vpshldd $123, %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vpshldd $123, %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x15,0x71,0xe6,0x7b]
               {vex} vpshldd $123, %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vpshldd  $123, 268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x23,0x15,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshldd  $123, 268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vpshldd  $123, 291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x15,0x71,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshldd  $123, 291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vpshldd  $123, (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x15,0x71,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshldd  $123, (%rip), %ymm13, %ymm12

// CHECK:      {vex} vpshldd  $123, -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x15,0x71,0x24,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vpshldd  $123, -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vpshldd  $123, 4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x15,0x71,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vpshldd  $123, 4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vpshldd  $123, -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x15,0x71,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vpshldd  $123, -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vpshldd  $123, 268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x23,0x11,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshldd  $123, 268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vpshldd  $123, 291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x11,0x71,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshldd  $123, 291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vpshldd  $123, (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x11,0x71,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshldd  $123, (%rip), %xmm13, %xmm12

// CHECK:      {vex} vpshldd  $123, -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x11,0x71,0x24,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vpshldd  $123, -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vpshldd  $123, 2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x11,0x71,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vpshldd  $123, 2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vpshldd  $123, -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x11,0x71,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vpshldd  $123, -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpshldq $123, %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x91,0x71,0xe6,0x7b]
               {vex} vpshldq $123, %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vpshldq $123, %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x95,0x71,0xe6,0x7b]
               {vex} vpshldq $123, %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vpshldq  $123, 268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x23,0x95,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshldq  $123, 268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vpshldq  $123, 291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x95,0x71,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshldq  $123, 291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vpshldq  $123, (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x71,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshldq  $123, (%rip), %ymm13, %ymm12

// CHECK:      {vex} vpshldq  $123, -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x71,0x24,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vpshldq  $123, -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vpshldq  $123, 4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x71,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vpshldq  $123, 4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vpshldq  $123, -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x71,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vpshldq  $123, -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vpshldq  $123, 268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x23,0x91,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshldq  $123, 268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vpshldq  $123, 291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x91,0x71,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshldq  $123, 291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vpshldq  $123, (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x71,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshldq  $123, (%rip), %xmm13, %xmm12

// CHECK:      {vex} vpshldq  $123, -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x71,0x24,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vpshldq  $123, -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vpshldq  $123, 2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x71,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vpshldq  $123, 2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vpshldq  $123, -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x71,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vpshldq  $123, -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpshldvd %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x15,0x71,0xe6]
               {vex} vpshldvd %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vpshldvd %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x11,0x71,0xe6]
               {vex} vpshldvd %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vpshldvd  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x15,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshldvd  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vpshldvd  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x15,0x71,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshldvd  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vpshldvd  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x71,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshldvd  (%rip), %ymm13, %ymm12

// CHECK:      {vex} vpshldvd  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x71,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpshldvd  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vpshldvd  4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x71,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpshldvd  4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vpshldvd  -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x71,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpshldvd  -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vpshldvd  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x11,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshldvd  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vpshldvd  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x11,0x71,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshldvd  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vpshldvd  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x71,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshldvd  (%rip), %xmm13, %xmm12

// CHECK:      {vex} vpshldvd  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x71,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpshldvd  -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vpshldvd  2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x71,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpshldvd  2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vpshldvd  -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x71,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpshldvd  -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpshldvq %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x95,0x71,0xe6]
               {vex} vpshldvq %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vpshldvq %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x91,0x71,0xe6]
               {vex} vpshldvq %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vpshldvq  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x95,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshldvq  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vpshldvq  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x95,0x71,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshldvq  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vpshldvq  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x71,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshldvq  (%rip), %ymm13, %ymm12

// CHECK:      {vex} vpshldvq  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x71,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpshldvq  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vpshldvq  4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x71,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpshldvq  4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vpshldvq  -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x71,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpshldvq  -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vpshldvq  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x91,0x71,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshldvq  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vpshldvq  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x91,0x71,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshldvq  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vpshldvq  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x71,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshldvq  (%rip), %xmm13, %xmm12

// CHECK:      {vex} vpshldvq  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x71,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpshldvq  -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vpshldvq  2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x71,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpshldvq  2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vpshldvq  -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x71,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpshldvq  -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpshldvw %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x95,0x70,0xe6]
               {vex} vpshldvw %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vpshldvw %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x91,0x70,0xe6]
               {vex} vpshldvw %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vpshldvw  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x95,0x70,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshldvw  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vpshldvw  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x95,0x70,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshldvw  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vpshldvw  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x70,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshldvw  (%rip), %ymm13, %ymm12

// CHECK:      {vex} vpshldvw  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x70,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpshldvw  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vpshldvw  4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x70,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpshldvw  4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vpshldvw  -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x70,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpshldvw  -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vpshldvw  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x91,0x70,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshldvw  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vpshldvw  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x91,0x70,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshldvw  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vpshldvw  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x70,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshldvw  (%rip), %xmm13, %xmm12

// CHECK:      {vex} vpshldvw  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x70,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpshldvw  -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vpshldvw  2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x70,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpshldvw  2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vpshldvw  -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x70,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpshldvw  -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpshldw $123, %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x91,0x70,0xe6,0x7b]
               {vex} vpshldw $123, %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vpshldw $123, %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x95,0x70,0xe6,0x7b]
               {vex} vpshldw $123, %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vpshldw  $123, 268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x23,0x95,0x70,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshldw  $123, 268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vpshldw  $123, 291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x95,0x70,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshldw  $123, 291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vpshldw  $123, (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x70,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshldw  $123, (%rip), %ymm13, %ymm12

// CHECK:      {vex} vpshldw  $123, -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x70,0x24,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vpshldw  $123, -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vpshldw  $123, 4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x70,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vpshldw  $123, 4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vpshldw  $123, -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x70,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vpshldw  $123, -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vpshldw  $123, 268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x23,0x91,0x70,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshldw  $123, 268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vpshldw  $123, 291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x91,0x70,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshldw  $123, 291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vpshldw  $123, (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x70,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshldw  $123, (%rip), %xmm13, %xmm12

// CHECK:      {vex} vpshldw  $123, -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x70,0x24,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vpshldw  $123, -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vpshldw  $123, 2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x70,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vpshldw  $123, 2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vpshldw  $123, -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x70,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vpshldw  $123, -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpshrdd $123, %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x11,0x73,0xe6,0x7b]
               {vex} vpshrdd $123, %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vpshrdd $123, %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x15,0x73,0xe6,0x7b]
               {vex} vpshrdd $123, %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vpshrdd  $123, 268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x23,0x15,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshrdd  $123, 268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vpshrdd  $123, 291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x15,0x73,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshrdd  $123, 291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vpshrdd  $123, (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x15,0x73,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshrdd  $123, (%rip), %ymm13, %ymm12

// CHECK:      {vex} vpshrdd  $123, -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x15,0x73,0x24,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vpshrdd  $123, -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vpshrdd  $123, 4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x15,0x73,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vpshrdd  $123, 4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vpshrdd  $123, -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x15,0x73,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vpshrdd  $123, -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vpshrdd  $123, 268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x23,0x11,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshrdd  $123, 268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vpshrdd  $123, 291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x11,0x73,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshrdd  $123, 291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vpshrdd  $123, (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x11,0x73,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshrdd  $123, (%rip), %xmm13, %xmm12

// CHECK:      {vex} vpshrdd  $123, -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x11,0x73,0x24,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vpshrdd  $123, -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vpshrdd  $123, 2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x11,0x73,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vpshrdd  $123, 2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vpshrdd  $123, -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x11,0x73,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vpshrdd  $123, -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpshrdq $123, %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x91,0x73,0xe6,0x7b]
               {vex} vpshrdq $123, %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vpshrdq $123, %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x95,0x73,0xe6,0x7b]
               {vex} vpshrdq $123, %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vpshrdq  $123, 268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x23,0x95,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshrdq  $123, 268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vpshrdq  $123, 291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x95,0x73,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshrdq  $123, 291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vpshrdq  $123, (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x73,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshrdq  $123, (%rip), %ymm13, %ymm12

// CHECK:      {vex} vpshrdq  $123, -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x73,0x24,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vpshrdq  $123, -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vpshrdq  $123, 4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x73,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vpshrdq  $123, 4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vpshrdq  $123, -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x73,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vpshrdq  $123, -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vpshrdq  $123, 268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x23,0x91,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshrdq  $123, 268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vpshrdq  $123, 291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x91,0x73,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshrdq  $123, 291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vpshrdq  $123, (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x73,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshrdq  $123, (%rip), %xmm13, %xmm12

// CHECK:      {vex} vpshrdq  $123, -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x73,0x24,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vpshrdq  $123, -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vpshrdq  $123, 2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x73,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vpshrdq  $123, 2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vpshrdq  $123, -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x73,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vpshrdq  $123, -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvd %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x15,0x73,0xe6]
               {vex} vpshrdvd %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vpshrdvd %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x11,0x73,0xe6]
               {vex} vpshrdvd %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vpshrdvd  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x15,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshrdvd  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvd  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x15,0x73,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshrdvd  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvd  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x73,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshrdvd  (%rip), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvd  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x73,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpshrdvd  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvd  4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x73,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpshrdvd  4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvd  -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x15,0x73,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpshrdvd  -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvd  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x11,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshrdvd  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvd  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x11,0x73,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshrdvd  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvd  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x73,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshrdvd  (%rip), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvd  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x73,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpshrdvd  -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvd  2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x73,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpshrdvd  2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvd  -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x11,0x73,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpshrdvd  -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvq %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x95,0x73,0xe6]
               {vex} vpshrdvq %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vpshrdvq %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x91,0x73,0xe6]
               {vex} vpshrdvq %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vpshrdvq  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x95,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshrdvq  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvq  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x95,0x73,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshrdvq  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvq  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x73,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshrdvq  (%rip), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvq  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x73,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpshrdvq  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvq  4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x73,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpshrdvq  4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvq  -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x73,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpshrdvq  -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvq  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x91,0x73,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshrdvq  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvq  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x91,0x73,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshrdvq  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvq  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x73,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshrdvq  (%rip), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvq  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x73,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpshrdvq  -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvq  2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x73,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpshrdvq  2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvq  -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x73,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpshrdvq  -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvw %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x95,0x72,0xe6]
               {vex} vpshrdvw %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vpshrdvw %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x91,0x72,0xe6]
               {vex} vpshrdvw %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vpshrdvw  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x95,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshrdvw  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvw  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x95,0x72,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshrdvw  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvw  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x72,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshrdvw  (%rip), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvw  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x72,0x24,0x6d,0x00,0xfc,0xff,0xff]
               {vex} vpshrdvw  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvw  4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x72,0xa1,0xe0,0x0f,0x00,0x00]
               {vex} vpshrdvw  4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvw  -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x95,0x72,0xa2,0x00,0xf0,0xff,0xff]
               {vex} vpshrdvw  -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vpshrdvw  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x91,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10]
               {vex} vpshrdvw  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvw  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x91,0x72,0xa4,0x80,0x23,0x01,0x00,0x00]
               {vex} vpshrdvw  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvw  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x72,0x25,0x00,0x00,0x00,0x00]
               {vex} vpshrdvw  (%rip), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvw  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x72,0x24,0x6d,0x00,0xfe,0xff,0xff]
               {vex} vpshrdvw  -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvw  2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x72,0xa1,0xf0,0x07,0x00,0x00]
               {vex} vpshrdvw  2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vpshrdvw  -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x91,0x72,0xa2,0x00,0xf8,0xff,0xff]
               {vex} vpshrdvw  -2048(%rdx), %xmm13, %xmm12

// CHECK:      {vex} vpshrdw $123, %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x91,0x72,0xe6,0x7b]
               {vex} vpshrdw $123, %xmm14, %xmm13, %xmm12

// CHECK:      {vex} vpshrdw $123, %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x95,0x72,0xe6,0x7b]
               {vex} vpshrdw $123, %ymm14, %ymm13, %ymm12

// CHECK:      {vex} vpshrdw  $123, 268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x23,0x95,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshrdw  $123, 268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK:      {vex} vpshrdw  $123, 291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x43,0x95,0x72,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshrdw  $123, 291(%r8,%rax,4), %ymm13, %ymm12

// CHECK:      {vex} vpshrdw  $123, (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x72,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshrdw  $123, (%rip), %ymm13, %ymm12

// CHECK:      {vex} vpshrdw  $123, -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x72,0x24,0x6d,0x00,0xfc,0xff,0xff,0x7b]
               {vex} vpshrdw  $123, -1024(,%rbp,2), %ymm13, %ymm12

// CHECK:      {vex} vpshrdw  $123, 4064(%rcx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x72,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               {vex} vpshrdw  $123, 4064(%rcx), %ymm13, %ymm12

// CHECK:      {vex} vpshrdw  $123, -4096(%rdx), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x63,0x95,0x72,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               {vex} vpshrdw  $123, -4096(%rdx), %ymm13, %ymm12

// CHECK:      {vex} vpshrdw  $123, 268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x23,0x91,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {vex} vpshrdw  $123, 268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK:      {vex} vpshrdw  $123, 291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x43,0x91,0x72,0xa4,0x80,0x23,0x01,0x00,0x00,0x7b]
               {vex} vpshrdw  $123, 291(%r8,%rax,4), %xmm13, %xmm12

// CHECK:      {vex} vpshrdw  $123, (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x72,0x25,0x00,0x00,0x00,0x00,0x7b]
               {vex} vpshrdw  $123, (%rip), %xmm13, %xmm12

// CHECK:      {vex} vpshrdw  $123, -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x72,0x24,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               {vex} vpshrdw  $123, -512(,%rbp,2), %xmm13, %xmm12

// CHECK:      {vex} vpshrdw  $123, 2032(%rcx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x72,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               {vex} vpshrdw  $123, 2032(%rcx), %xmm13, %xmm12

// CHECK:      {vex} vpshrdw  $123, -2048(%rdx), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x63,0x91,0x72,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               {vex} vpshrdw  $123, -2048(%rdx), %xmm13, %xmm12

