// REQUIRES: intel_feature_isa_avx_bf16
// RUN: llvm-mc -triple=x86_64-unknown-unknown -mattr=+avxbf16 --show-encoding < %s  | FileCheck %s

// CHECK: {vex} vcvtne2ps2bf16 %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x17,0x72,0xe6]
     {vex} vcvtne2ps2bf16 %ymm14, %ymm13, %ymm12

// CHECK: {vex} vcvtne2ps2bf16 %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x13,0x72,0xe6]
     {vex} vcvtne2ps2bf16 %xmm14, %xmm13, %xmm12

// CHECK: {vex} vcvtne2ps2bf16  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x17,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10]
     {vex} vcvtne2ps2bf16  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK: {vex} vcvtne2ps2bf16  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x17,0x72,0xa4,0x80,0x23,0x01,0x00,0x00]
     {vex} vcvtne2ps2bf16  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK: {vex} vcvtne2ps2bf16  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x17,0x72,0x25,0x00,0x00,0x00,0x00]
     {vex} vcvtne2ps2bf16  (%rip), %ymm13, %ymm12

// CHECK: {vex} vcvtne2ps2bf16  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x17,0x72,0x24,0x6d,0x00,0xfc,0xff,0xff]
     {vex} vcvtne2ps2bf16  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK: {vex} vcvtne2ps2bf16  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x13,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10]
     {vex} vcvtne2ps2bf16  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK: {vex} vcvtne2ps2bf16  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x13,0x72,0xa4,0x80,0x23,0x01,0x00,0x00]
     {vex} vcvtne2ps2bf16  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK: {vex} vcvtne2ps2bf16  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x13,0x72,0x25,0x00,0x00,0x00,0x00]
     {vex} vcvtne2ps2bf16  (%rip), %xmm13, %xmm12

// CHECK: {vex} vcvtne2ps2bf16  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x13,0x72,0x24,0x6d,0x00,0xfe,0xff,0xff]
     {vex} vcvtne2ps2bf16  -512(,%rbp,2), %xmm13, %xmm12

// CHECK: {vex} vcvtneps2bf16 %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x7a,0x72,0xe5]
     {vex} vcvtneps2bf16 %xmm13, %xmm12

// CHECK: {vex} vcvtneps2bf16 %ymm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x7e,0x72,0xe5]
     {vex} vcvtneps2bf16 %ymm13, %xmm12

// CHECK: {vex} vcvtneps2bf16  268435456(%rbp,%r14,8), %xmm12
// CHECK: encoding: [0xc4,0x22,0x7a,0x72,0xa4,0xf5,0x00,0x00,0x00,0x10]
     {vex} vcvtneps2bf16  268435456(%rbp,%r14,8), %xmm12

// CHECK: {vex} vcvtneps2bf16  291(%r8,%rax,4), %xmm12
// CHECK: encoding: [0xc4,0x42,0x7a,0x72,0xa4,0x80,0x23,0x01,0x00,0x00]
     {vex} vcvtneps2bf16  291(%r8,%rax,4), %xmm12

// CHECK: {vex} vcvtneps2bf16  (%rip), %xmm12
// CHECK: encoding: [0xc4,0x62,0x7a,0x72,0x25,0x00,0x00,0x00,0x00]
     {vex} vcvtneps2bf16  (%rip), %xmm12

// CHECK: {vex} vcvtneps2bf16  -512(,%rbp,2), %xmm12
// CHECK: encoding: [0xc4,0x62,0x7a,0x72,0x24,0x6d,0x00,0xfe,0xff,0xff]
     {vex} vcvtneps2bf16  -512(,%rbp,2), %xmm12

// CHECK:  {vex}  vcvtneps2bf16 -1024(,%rbp,2), %xmm12 # encoding: [0xc4,0x62,0x7a,0x72,0x24,0x6d,0x00,0xfc,0xff,0xff]
     {vex} vcvtneps2bf16  -1024(,%rbp,2), %xmm12

// CHECK: {vex} vdpbf16ps %ymm14, %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x16,0x52,0xe6]
     {vex} vdpbf16ps %ymm14, %ymm13, %ymm12

// CHECK: {vex} vdpbf16ps %xmm14, %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x12,0x52,0xe6]
     {vex} vdpbf16ps %xmm14, %xmm13, %xmm12

// CHECK: {vex} vdpbf16ps  268435456(%rbp,%r14,8), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x22,0x16,0x52,0xa4,0xf5,0x00,0x00,0x00,0x10]
     {vex} vdpbf16ps  268435456(%rbp,%r14,8), %ymm13, %ymm12

// CHECK: {vex} vdpbf16ps  291(%r8,%rax,4), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x42,0x16,0x52,0xa4,0x80,0x23,0x01,0x00,0x00]
     {vex} vdpbf16ps  291(%r8,%rax,4), %ymm13, %ymm12

// CHECK: {vex} vdpbf16ps  (%rip), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x16,0x52,0x25,0x00,0x00,0x00,0x00]
     {vex} vdpbf16ps  (%rip), %ymm13, %ymm12

// CHECK: {vex} vdpbf16ps  -1024(,%rbp,2), %ymm13, %ymm12
// CHECK: encoding: [0xc4,0x62,0x16,0x52,0x24,0x6d,0x00,0xfc,0xff,0xff]
     {vex} vdpbf16ps  -1024(,%rbp,2), %ymm13, %ymm12

// CHECK: {vex} vdpbf16ps  268435456(%rbp,%r14,8), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x22,0x12,0x52,0xa4,0xf5,0x00,0x00,0x00,0x10]
     {vex} vdpbf16ps  268435456(%rbp,%r14,8), %xmm13, %xmm12

// CHECK: {vex} vdpbf16ps  291(%r8,%rax,4), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x42,0x12,0x52,0xa4,0x80,0x23,0x01,0x00,0x00]
     {vex} vdpbf16ps  291(%r8,%rax,4), %xmm13, %xmm12

// CHECK: {vex} vdpbf16ps  (%rip), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x12,0x52,0x25,0x00,0x00,0x00,0x00]
     {vex} vdpbf16ps  (%rip), %xmm13, %xmm12

// CHECK: {vex} vdpbf16ps  -512(,%rbp,2), %xmm13, %xmm12
// CHECK: encoding: [0xc4,0x62,0x12,0x52,0x24,0x6d,0x00,0xfe,0xff,0xff]
     {vex} vdpbf16ps  -512(,%rbp,2), %xmm13, %xmm12

