// REQUIRES: intel_feature_isa_avx_bf16
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avxbf16 --show-encoding %s | FileCheck %s

// CHECK: {vex} vcvtne2ps2bf16 %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x67,0x72,0xd4]
     {vex} vcvtne2ps2bf16 %ymm4, %ymm3, %ymm2

// CHECK: {vex} vcvtne2ps2bf16 %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x63,0x72,0xd4]
     {vex} vcvtne2ps2bf16 %xmm4, %xmm3, %xmm2

// CHECK: {vex} vcvtne2ps2bf16  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x67,0x72,0x94,0xf4,0x00,0x00,0x00,0x10]
     {vex} vcvtne2ps2bf16  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: {vex} vcvtne2ps2bf16  291(%edi,%eax,4), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x67,0x72,0x94,0x87,0x23,0x01,0x00,0x00]
     {vex} vcvtne2ps2bf16  291(%edi,%eax,4), %ymm3, %ymm2

// CHECK: {vex} vcvtne2ps2bf16  (%eax), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x67,0x72,0x10]
     {vex} vcvtne2ps2bf16  (%eax), %ymm3, %ymm2

// CHECK: {vex} vcvtne2ps2bf16  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x67,0x72,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {vex} vcvtne2ps2bf16  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: {vex} vcvtne2ps2bf16  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x63,0x72,0x94,0xf4,0x00,0x00,0x00,0x10]
     {vex} vcvtne2ps2bf16  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: {vex} vcvtne2ps2bf16  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x63,0x72,0x94,0x87,0x23,0x01,0x00,0x00]
     {vex} vcvtne2ps2bf16  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK: {vex} vcvtne2ps2bf16  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x63,0x72,0x10]
     {vex} vcvtne2ps2bf16  (%eax), %xmm3, %xmm2

// CHECK: {vex} vcvtne2ps2bf16  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x63,0x72,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {vex} vcvtne2ps2bf16  -512(,%ebp,2), %xmm3, %xmm2

// CHECK: {vex} vcvtneps2bf16 %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x72,0xd3]
     {vex} vcvtneps2bf16 %xmm3, %xmm2

// CHECK: {vex} vcvtneps2bf16 %ymm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x7e,0x72,0xd3]
     {vex} vcvtneps2bf16 %ymm3, %xmm2

// CHECK: {vex} vcvtneps2bf16  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x72,0x94,0xf4,0x00,0x00,0x00,0x10]
     {vex} vcvtneps2bf16  268435456(%esp,%esi,8), %xmm2

// CHECK: {vex} vcvtneps2bf16  291(%edi,%eax,4), %xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x72,0x94,0x87,0x23,0x01,0x00,0x00]
     {vex} vcvtneps2bf16  291(%edi,%eax,4), %xmm2

// CHECK: {vex} vcvtneps2bf16  (%eax), %xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x72,0x10]
     {vex} vcvtneps2bf16  (%eax), %xmm2

// CHECK: {vex} vcvtneps2bf16  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x72,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {vex} vcvtneps2bf16  -512(,%ebp,2), %xmm2

// CHECK: {vex} vcvtneps2bf16 -1024(,%ebp,2), %xmm2 # encoding: [0xc4,0xe2,0x7a,0x72,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {vex} vcvtneps2bf16  -1024(,%ebp,2), %xmm2

// CHECK: {vex} vdpbf16ps %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x66,0x52,0xd4]
     {vex} vdpbf16ps %ymm4, %ymm3, %ymm2

// CHECK: {vex} vdpbf16ps %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x62,0x52,0xd4]
     {vex} vdpbf16ps %xmm4, %xmm3, %xmm2

// CHECK: {vex} vdpbf16ps  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x66,0x52,0x94,0xf4,0x00,0x00,0x00,0x10]
     {vex} vdpbf16ps  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK: {vex} vdpbf16ps  291(%edi,%eax,4), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x66,0x52,0x94,0x87,0x23,0x01,0x00,0x00]
     {vex} vdpbf16ps  291(%edi,%eax,4), %ymm3, %ymm2

// CHECK: {vex} vdpbf16ps  (%eax), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x66,0x52,0x10]
     {vex} vdpbf16ps  (%eax), %ymm3, %ymm2

// CHECK: {vex} vdpbf16ps  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0xc4,0xe2,0x66,0x52,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {vex} vdpbf16ps  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK: {vex} vdpbf16ps  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x62,0x52,0x94,0xf4,0x00,0x00,0x00,0x10]
     {vex} vdpbf16ps  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK: {vex} vdpbf16ps  291(%edi,%eax,4), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x62,0x52,0x94,0x87,0x23,0x01,0x00,0x00]
     {vex} vdpbf16ps  291(%edi,%eax,4), %xmm3, %xmm2

// CHECK: {vex} vdpbf16ps  (%eax), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x62,0x52,0x10]
     {vex} vdpbf16ps  (%eax), %xmm3, %xmm2

// CHECK: {vex} vdpbf16ps  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0xc4,0xe2,0x62,0x52,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {vex} vdpbf16ps  -512(,%ebp,2), %xmm3, %xmm2

