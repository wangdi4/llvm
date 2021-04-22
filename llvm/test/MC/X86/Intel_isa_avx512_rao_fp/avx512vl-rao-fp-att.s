// REQUIRES: intel_feature_isa_avx512_rao_fp
// RUN: llvm-mc -triple i686-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      {evex} vaaddpbf16  %ymm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddpbf16  %ymm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vaaddpbf16  %ymm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddpbf16  %ymm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vaaddpbf16  %ymm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0x94,0x10]
               {evex} vaaddpbf16  %ymm2, (%eax)

// CHECK:      {evex} vaaddpbf16  %ymm2, -1024(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0x94,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vaaddpbf16  %ymm2, -1024(,%ebp,2)

// CHECK:      {evex} vaaddpbf16  %ymm2, 4064(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0x94,0x51,0x7f]
               {evex} vaaddpbf16  %ymm2, 4064(%ecx) {%k7}

// CHECK:      {evex} vaaddpbf16  %ymm2, -4096(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0x94,0x52,0x80]
               {evex} vaaddpbf16  %ymm2, -4096(%edx) {%k7}

// CHECK:      {evex} vaaddpbf16  %xmm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddpbf16  %xmm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vaaddpbf16  %xmm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddpbf16  %xmm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vaaddpbf16  %xmm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0x94,0x10]
               {evex} vaaddpbf16  %xmm2, (%eax)

// CHECK:      {evex} vaaddpbf16  %xmm2, -512(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0x94,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vaaddpbf16  %xmm2, -512(,%ebp,2)

// CHECK:      {evex} vaaddpbf16  %xmm2, 2032(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0x94,0x51,0x7f]
               {evex} vaaddpbf16  %xmm2, 2032(%ecx) {%k7}

// CHECK:      {evex} vaaddpbf16  %xmm2, -2048(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0x94,0x52,0x80]
               {evex} vaaddpbf16  %xmm2, -2048(%edx) {%k7}

// CHECK:      {evex} vaaddpd  %ymm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0xfd,0x28,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddpd  %ymm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vaaddpd  %ymm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfd,0x2f,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddpd  %ymm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vaaddpd  %ymm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0xfd,0x28,0x84,0x10]
               {evex} vaaddpd  %ymm2, (%eax)

// CHECK:      {evex} vaaddpd  %ymm2, -1024(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0xfd,0x28,0x84,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vaaddpd  %ymm2, -1024(,%ebp,2)

// CHECK:      {evex} vaaddpd  %ymm2, 4064(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfd,0x2f,0x84,0x51,0x7f]
               {evex} vaaddpd  %ymm2, 4064(%ecx) {%k7}

// CHECK:      {evex} vaaddpd  %ymm2, -4096(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfd,0x2f,0x84,0x52,0x80]
               {evex} vaaddpd  %ymm2, -4096(%edx) {%k7}

// CHECK:      {evex} vaaddpd  %xmm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0xfd,0x08,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddpd  %xmm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vaaddpd  %xmm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfd,0x0f,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddpd  %xmm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vaaddpd  %xmm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0xfd,0x08,0x84,0x10]
               {evex} vaaddpd  %xmm2, (%eax)

// CHECK:      {evex} vaaddpd  %xmm2, -512(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0xfd,0x08,0x84,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vaaddpd  %xmm2, -512(,%ebp,2)

// CHECK:      {evex} vaaddpd  %xmm2, 2032(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfd,0x0f,0x84,0x51,0x7f]
               {evex} vaaddpd  %xmm2, 2032(%ecx) {%k7}

// CHECK:      {evex} vaaddpd  %xmm2, -2048(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfd,0x0f,0x84,0x52,0x80]
               {evex} vaaddpd  %xmm2, -2048(%edx) {%k7}

// CHECK:      {evex} vaaddph  %ymm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddph  %ymm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vaaddph  %ymm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddph  %ymm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vaaddph  %ymm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x94,0x10]
               {evex} vaaddph  %ymm2, (%eax)

// CHECK:      {evex} vaaddph  %ymm2, -1024(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x94,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vaaddph  %ymm2, -1024(,%ebp,2)

// CHECK:      {evex} vaaddph  %ymm2, 4064(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x94,0x51,0x7f]
               {evex} vaaddph  %ymm2, 4064(%ecx) {%k7}

// CHECK:      {evex} vaaddph  %ymm2, -4096(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x94,0x52,0x80]
               {evex} vaaddph  %ymm2, -4096(%edx) {%k7}

// CHECK:      {evex} vaaddph  %xmm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddph  %xmm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vaaddph  %xmm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddph  %xmm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vaaddph  %xmm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x94,0x10]
               {evex} vaaddph  %xmm2, (%eax)

// CHECK:      {evex} vaaddph  %xmm2, -512(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x94,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vaaddph  %xmm2, -512(,%ebp,2)

// CHECK:      {evex} vaaddph  %xmm2, 2032(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x94,0x51,0x7f]
               {evex} vaaddph  %xmm2, 2032(%ecx) {%k7}

// CHECK:      {evex} vaaddph  %xmm2, -2048(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x94,0x52,0x80]
               {evex} vaaddph  %xmm2, -2048(%edx) {%k7}

// CHECK:      {evex} vaaddps  %ymm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddps  %ymm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vaaddps  %ymm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddps  %ymm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vaaddps  %ymm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x84,0x10]
               {evex} vaaddps  %ymm2, (%eax)

// CHECK:      {evex} vaaddps  %ymm2, -1024(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x84,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vaaddps  %ymm2, -1024(,%ebp,2)

// CHECK:      {evex} vaaddps  %ymm2, 4064(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x84,0x51,0x7f]
               {evex} vaaddps  %ymm2, 4064(%ecx) {%k7}

// CHECK:      {evex} vaaddps  %ymm2, -4096(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x84,0x52,0x80]
               {evex} vaaddps  %ymm2, -4096(%edx) {%k7}

// CHECK:      {evex} vaaddps  %xmm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddps  %xmm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vaaddps  %xmm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddps  %xmm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vaaddps  %xmm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x84,0x10]
               {evex} vaaddps  %xmm2, (%eax)

// CHECK:      {evex} vaaddps  %xmm2, -512(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x84,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vaaddps  %xmm2, -512(,%ebp,2)

// CHECK:      {evex} vaaddps  %xmm2, 2032(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x84,0x51,0x7f]
               {evex} vaaddps  %xmm2, 2032(%ecx) {%k7}

// CHECK:      {evex} vaaddps  %xmm2, -2048(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x84,0x52,0x80]
               {evex} vaaddps  %xmm2, -2048(%edx) {%k7}

