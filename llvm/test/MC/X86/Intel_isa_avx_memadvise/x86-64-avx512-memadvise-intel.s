// REQUIRES: intel_feature_isa_avx_memadvise
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vmovadvisew zmmword ptr [rbp + 8*r14 + 268435456], zmm30, 123
// CHECK: encoding: [0x62,0x23,0x7e,0x48,0x11,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew zmmword ptr [rbp + 8*r14 + 268435456], zmm30, 123

// CHECK:      vmovadvisew zmmword ptr [r9], zmm30, 123
// CHECK: encoding: [0x62,0x43,0x7e,0x48,0x11,0x31,0x7b]
               vmovadvisew zmmword ptr [r9], zmm30, 123

// CHECK:      vmovadvisew zmmword ptr [rcx + 8128], zmm30, 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x11,0x71,0x7f,0x7b]
               vmovadvisew zmmword ptr [rcx + 8128], zmm30, 123

// CHECK:      vmovadvisew zmmword ptr [rdx - 8192], zmm30, 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x11,0x72,0x80,0x7b]
               vmovadvisew zmmword ptr [rdx - 8192], zmm30, 123

// CHECK:      vmovadvisew zmm30, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x23,0x7e,0x48,0x10,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew zmm30, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      vmovadvisew zmm30, zmmword ptr [r9], 123
// CHECK: encoding: [0x62,0x43,0x7e,0x48,0x10,0x31,0x7b]
               vmovadvisew zmm30, zmmword ptr [r9], 123

// CHECK:      vmovadvisew zmm30, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x10,0x71,0x7f,0x7b]
               vmovadvisew zmm30, zmmword ptr [rcx + 8128], 123

// CHECK:      vmovadvisew zmm30, zmmword ptr [rdx - 8192], 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x10,0x72,0x80,0x7b]
               vmovadvisew zmm30, zmmword ptr [rdx - 8192], 123

// CHECK:      vmovadvisew zmmword ptr [rbp + 8*r14 + 268435456], zmm30, 123
// CHECK: encoding: [0x62,0x23,0x7e,0x48,0x11,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew zmmword ptr [rbp + 8*r14 + 268435456], zmm30, 123

// CHECK:      vmovadvisew zmmword ptr [r9], zmm30, 123
// CHECK: encoding: [0x62,0x43,0x7e,0x48,0x11,0x31,0x7b]
               vmovadvisew zmmword ptr [r9], zmm30, 123

// CHECK:      vmovadvisew zmmword ptr [rcx + 8128], zmm30, 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x11,0x71,0x7f,0x7b]
               vmovadvisew zmmword ptr [rcx + 8128], zmm30, 123

// CHECK:      vmovadvisew zmmword ptr [rdx - 8192], zmm30, 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x11,0x72,0x80,0x7b]
               vmovadvisew zmmword ptr [rdx - 8192], zmm30, 123

// CHECK:      vmovadvisew zmm30, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x23,0x7e,0x48,0x10,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew zmm30, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      vmovadvisew zmm30, zmmword ptr [r9], 123
// CHECK: encoding: [0x62,0x43,0x7e,0x48,0x10,0x31,0x7b]
               vmovadvisew zmm30, zmmword ptr [r9], 123

// CHECK:      vmovadvisew zmm30, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x10,0x71,0x7f,0x7b]
               vmovadvisew zmm30, zmmword ptr [rcx + 8128], 123

// CHECK:      vmovadvisew zmm30, zmmword ptr [rdx - 8192], 123
// CHECK: encoding: [0x62,0x63,0x7e,0x48,0x10,0x72,0x80,0x7b]
               vmovadvisew zmm30, zmmword ptr [rdx - 8192], 123

// CHECK:      vmovadvisew xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x33,0x7e,0x08,0x10,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {evex} vmovadvisew xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      vmovadvisew xmm30, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x23,0x7e,0x08,0x10,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew xmm30, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      vmovadvisew xmm30, xmmword ptr [r9], 123
// CHECK: encoding: [0x62,0x43,0x7e,0x08,0x10,0x31,0x7b]
               vmovadvisew xmm30, xmmword ptr [r9], 123

// CHECK:      vmovadvisew ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x33,0x7e,0x28,0x10,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               {evex} vmovadvisew ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      vmovadvisew ymm30, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x23,0x7e,0x28,0x10,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew ymm30, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      vmovadvisew ymm30, ymmword ptr [r9], 123
// CHECK: encoding: [0x62,0x43,0x7e,0x28,0x10,0x31,0x7b]
               vmovadvisew ymm30, ymmword ptr [r9], 123

// CHECK:      vmovadvisew xmmword ptr [rbp + 8*r14 + 268435456], xmm30, 123
// CHECK: encoding: [0x62,0x23,0x7e,0x08,0x11,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew xmmword ptr [rbp + 8*r14 + 268435456], xmm30, 123

// CHECK:      vmovadvisew xmmword ptr [r9], xmm30, 123
// CHECK: encoding: [0x62,0x43,0x7e,0x08,0x11,0x31,0x7b]
               vmovadvisew xmmword ptr [r9], xmm30, 123

// CHECK:      vmovadvisew ymmword ptr [rbp + 8*r14 + 268435456], ymm30, 123
// CHECK: encoding: [0x62,0x23,0x7e,0x28,0x11,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew ymmword ptr [rbp + 8*r14 + 268435456], ymm30, 123

// CHECK:      vmovadvisew ymmword ptr [r9], ymm30, 123
// CHECK: encoding: [0x62,0x43,0x7e,0x28,0x11,0x31,0x7b]
               vmovadvisew ymmword ptr [r9], ymm30, 123
