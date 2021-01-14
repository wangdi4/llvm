// REQUIRES: intel_feature_isa_avx512_movget
// RUN: llvm-mc -triple i686-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      {evex} vmovget  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xc5,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vmovget  268435456(%esp,%esi,8), %xmm2

// CHECK:      {evex} vmovget  291(%edi,%eax,4), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xc5,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vmovget  291(%edi,%eax,4), %xmm2

// CHECK:      {evex} vmovget  (%eax), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xc5,0x10]
               {evex} vmovget  (%eax), %xmm2

// CHECK:      {evex} vmovget  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xc5,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vmovget  -512(,%ebp,2), %xmm2

// CHECK:      {evex} vmovget  2032(%ecx), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xc5,0x51,0x7f]
               {evex} vmovget  2032(%ecx), %xmm2

// CHECK:      {evex} vmovget  -2048(%edx), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xc5,0x52,0x80]
               {evex} vmovget  -2048(%edx), %xmm2

// CHECK:      {evex} vmovget  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xc5,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vmovget  268435456(%esp,%esi,8), %ymm2

// CHECK:      {evex} vmovget  291(%edi,%eax,4), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xc5,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vmovget  291(%edi,%eax,4), %ymm2

// CHECK:      {evex} vmovget  (%eax), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xc5,0x10]
               {evex} vmovget  (%eax), %ymm2

// CHECK:      {evex} vmovget  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xc5,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vmovget  -1024(,%ebp,2), %ymm2

// CHECK:      {evex} vmovget  4064(%ecx), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xc5,0x51,0x7f]
               {evex} vmovget  4064(%ecx), %ymm2

// CHECK:      {evex} vmovget  -4096(%edx), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xc5,0x52,0x80]
               {evex} vmovget  -4096(%edx), %ymm2

