// REQUIRES: intel_feature_isa_avx512_rao_int
// RUN: llvm-mc -triple i686-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      {evex} vpaaddd  %ymm2, 268435456(%esp,%esi,8)
// CHECK:  encoding: [0x62,0xf2,0x7c,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaaddd  %ymm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaaddd  %ymm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaaddd  %ymm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaaddd  %ymm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0xfc,0x10]
               {evex} vpaaddd  %ymm2, (%eax)

// CHECK:      {evex} vpaaddd  %ymm2, -1024(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaaddd  %ymm2, -1024(,%ebp,2)

// CHECK:      {evex} vpaaddd  %ymm2, 4064(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0xfc,0x51,0x7f]
               {evex} vpaaddd  %ymm2, 4064(%ecx) {%k7}

// CHECK:      {evex} vpaaddd  %ymm2, -4096(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0xfc,0x52,0x80]
               {evex} vpaaddd  %ymm2, -4096(%edx) {%k7}

// CHECK:      {evex} vpaaddd  %xmm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaaddd  %xmm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaaddd  %xmm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaaddd  %xmm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaaddd  %xmm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0xfc,0x10]
               {evex} vpaaddd  %xmm2, (%eax)

// CHECK:      {evex} vpaaddd  %xmm2, -512(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaaddd  %xmm2, -512(,%ebp,2)

// CHECK:      {evex} vpaaddd  %xmm2, 2032(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0xfc,0x51,0x7f]
               {evex} vpaaddd  %xmm2, 2032(%ecx) {%k7}

// CHECK:      {evex} vpaaddd  %xmm2, -2048(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0xfc,0x52,0x80]
               {evex} vpaaddd  %xmm2, -2048(%edx) {%k7}

// CHECK:      {evex} vpaaddq  %ymm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0xfc,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaaddq  %ymm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaaddq  %ymm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfc,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaaddq  %ymm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaaddq  %ymm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0xfc,0x28,0xfc,0x10]
               {evex} vpaaddq  %ymm2, (%eax)

// CHECK:      {evex} vpaaddq  %ymm2, -1024(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0xfc,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaaddq  %ymm2, -1024(,%ebp,2)

// CHECK:      {evex} vpaaddq  %ymm2, 4064(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfc,0x2f,0xfc,0x51,0x7f]
               {evex} vpaaddq  %ymm2, 4064(%ecx) {%k7}

// CHECK:      {evex} vpaaddq  %ymm2, -4096(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfc,0x2f,0xfc,0x52,0x80]
               {evex} vpaaddq  %ymm2, -4096(%edx) {%k7}

// CHECK:      {evex} vpaaddq  %xmm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0xfc,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaaddq  %xmm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaaddq  %xmm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfc,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaaddq  %xmm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaaddq  %xmm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0xfc,0x08,0xfc,0x10]
               {evex} vpaaddq  %xmm2, (%eax)

// CHECK:      {evex} vpaaddq  %xmm2, -512(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0xfc,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaaddq  %xmm2, -512(,%ebp,2)

// CHECK:      {evex} vpaaddq  %xmm2, 2032(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfc,0x0f,0xfc,0x51,0x7f]
               {evex} vpaaddq  %xmm2, 2032(%ecx) {%k7}

// CHECK:      {evex} vpaaddq  %xmm2, -2048(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfc,0x0f,0xfc,0x52,0x80]
               {evex} vpaaddq  %xmm2, -2048(%edx) {%k7}

// CHECK:      {evex} vpaandd  %ymm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaandd  %ymm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaandd  %ymm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaandd  %ymm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaandd  %ymm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xfc,0x10]
               {evex} vpaandd  %ymm2, (%eax)

// CHECK:      {evex} vpaandd  %ymm2, -1024(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaandd  %ymm2, -1024(,%ebp,2)

// CHECK:      {evex} vpaandd  %ymm2, 4064(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0xfc,0x51,0x7f]
               {evex} vpaandd  %ymm2, 4064(%ecx) {%k7}

// CHECK:      {evex} vpaandd  %ymm2, -4096(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0xfc,0x52,0x80]
               {evex} vpaandd  %ymm2, -4096(%edx) {%k7}

// CHECK:      {evex} vpaandd  %xmm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaandd  %xmm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaandd  %xmm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaandd  %xmm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaandd  %xmm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xfc,0x10]
               {evex} vpaandd  %xmm2, (%eax)

// CHECK:      {evex} vpaandd  %xmm2, -512(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaandd  %xmm2, -512(,%ebp,2)

// CHECK:      {evex} vpaandd  %xmm2, 2032(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0xfc,0x51,0x7f]
               {evex} vpaandd  %xmm2, 2032(%ecx) {%k7}

// CHECK:      {evex} vpaandd  %xmm2, -2048(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0xfc,0x52,0x80]
               {evex} vpaandd  %xmm2, -2048(%edx) {%k7}

// CHECK:      {evex} vpaandq  %ymm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0xfd,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaandq  %ymm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaandq  %ymm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfd,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaandq  %ymm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaandq  %ymm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0xfd,0x28,0xfc,0x10]
               {evex} vpaandq  %ymm2, (%eax)

// CHECK:      {evex} vpaandq  %ymm2, -1024(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0xfd,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaandq  %ymm2, -1024(,%ebp,2)

// CHECK:      {evex} vpaandq  %ymm2, 4064(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfd,0x2f,0xfc,0x51,0x7f]
               {evex} vpaandq  %ymm2, 4064(%ecx) {%k7}

// CHECK:      {evex} vpaandq  %ymm2, -4096(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfd,0x2f,0xfc,0x52,0x80]
               {evex} vpaandq  %ymm2, -4096(%edx) {%k7}

// CHECK:      {evex} vpaandq  %xmm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0xfd,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaandq  %xmm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaandq  %xmm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfd,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaandq  %xmm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaandq  %xmm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0xfd,0x08,0xfc,0x10]
               {evex} vpaandq  %xmm2, (%eax)

// CHECK:      {evex} vpaandq  %xmm2, -512(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0xfd,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaandq  %xmm2, -512(,%ebp,2)

// CHECK:      {evex} vpaandq  %xmm2, 2032(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfd,0x0f,0xfc,0x51,0x7f]
               {evex} vpaandq  %xmm2, 2032(%ecx) {%k7}

// CHECK:      {evex} vpaandq  %xmm2, -2048(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfd,0x0f,0xfc,0x52,0x80]
               {evex} vpaandq  %xmm2, -2048(%edx) {%k7}

// CHECK:      {evex} vpaord  %ymm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaord  %ymm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaord  %ymm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaord  %ymm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaord  %ymm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0xfc,0x10]
               {evex} vpaord  %ymm2, (%eax)

// CHECK:      {evex} vpaord  %ymm2, -1024(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaord  %ymm2, -1024(,%ebp,2)

// CHECK:      {evex} vpaord  %ymm2, 4064(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x2f,0xfc,0x51,0x7f]
               {evex} vpaord  %ymm2, 4064(%ecx) {%k7}

// CHECK:      {evex} vpaord  %ymm2, -4096(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x2f,0xfc,0x52,0x80]
               {evex} vpaord  %ymm2, -4096(%edx) {%k7}

// CHECK:      {evex} vpaord  %xmm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaord  %xmm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaord  %xmm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaord  %xmm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaord  %xmm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0xfc,0x10]
               {evex} vpaord  %xmm2, (%eax)

// CHECK:      {evex} vpaord  %xmm2, -512(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaord  %xmm2, -512(,%ebp,2)

// CHECK:      {evex} vpaord  %xmm2, 2032(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0xfc,0x51,0x7f]
               {evex} vpaord  %xmm2, 2032(%ecx) {%k7}

// CHECK:      {evex} vpaord  %xmm2, -2048(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0xfc,0x52,0x80]
               {evex} vpaord  %xmm2, -2048(%edx) {%k7}

// CHECK:      {evex} vpaorq  %ymm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0xff,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaorq  %ymm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaorq  %ymm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0xff,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaorq  %ymm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaorq  %ymm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0xff,0x28,0xfc,0x10]
               {evex} vpaorq  %ymm2, (%eax)

// CHECK:      {evex} vpaorq  %ymm2, -1024(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0xff,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaorq  %ymm2, -1024(,%ebp,2)

// CHECK:      {evex} vpaorq  %ymm2, 4064(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xff,0x2f,0xfc,0x51,0x7f]
               {evex} vpaorq  %ymm2, 4064(%ecx) {%k7}

// CHECK:      {evex} vpaorq  %ymm2, -4096(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xff,0x2f,0xfc,0x52,0x80]
               {evex} vpaorq  %ymm2, -4096(%edx) {%k7}

// CHECK:      {evex} vpaorq  %xmm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0xff,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaorq  %xmm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaorq  %xmm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0xff,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaorq  %xmm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaorq  %xmm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0xff,0x08,0xfc,0x10]
               {evex} vpaorq  %xmm2, (%eax)

// CHECK:      {evex} vpaorq  %xmm2, -512(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0xff,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaorq  %xmm2, -512(,%ebp,2)

// CHECK:      {evex} vpaorq  %xmm2, 2032(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xff,0x0f,0xfc,0x51,0x7f]
               {evex} vpaorq  %xmm2, 2032(%ecx) {%k7}

// CHECK:      {evex} vpaorq  %xmm2, -2048(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xff,0x0f,0xfc,0x52,0x80]
               {evex} vpaorq  %xmm2, -2048(%edx) {%k7}

// CHECK:      {evex} vpaxord  %ymm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaxord  %ymm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaxord  %ymm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaxord  %ymm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaxord  %ymm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xfc,0x10]
               {evex} vpaxord  %ymm2, (%eax)

// CHECK:      {evex} vpaxord  %ymm2, -1024(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaxord  %ymm2, -1024(,%ebp,2)

// CHECK:      {evex} vpaxord  %ymm2, 4064(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0xfc,0x51,0x7f]
               {evex} vpaxord  %ymm2, 4064(%ecx) {%k7}

// CHECK:      {evex} vpaxord  %ymm2, -4096(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0xfc,0x52,0x80]
               {evex} vpaxord  %ymm2, -4096(%edx) {%k7}

// CHECK:      {evex} vpaxord  %xmm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaxord  %xmm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaxord  %xmm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaxord  %xmm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaxord  %xmm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xfc,0x10]
               {evex} vpaxord  %xmm2, (%eax)

// CHECK:      {evex} vpaxord  %xmm2, -512(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaxord  %xmm2, -512(,%ebp,2)

// CHECK:      {evex} vpaxord  %xmm2, 2032(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0xfc,0x51,0x7f]
               {evex} vpaxord  %xmm2, 2032(%ecx) {%k7}

// CHECK:      {evex} vpaxord  %xmm2, -2048(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0xfc,0x52,0x80]
               {evex} vpaxord  %xmm2, -2048(%edx) {%k7}

// CHECK:      {evex} vpaxorq  %ymm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0xfe,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaxorq  %ymm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaxorq  %ymm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfe,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaxorq  %ymm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaxorq  %ymm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0xfe,0x28,0xfc,0x10]
               {evex} vpaxorq  %ymm2, (%eax)

// CHECK:      {evex} vpaxorq  %ymm2, -1024(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0xfe,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaxorq  %ymm2, -1024(,%ebp,2)

// CHECK:      {evex} vpaxorq  %ymm2, 4064(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfe,0x2f,0xfc,0x51,0x7f]
               {evex} vpaxorq  %ymm2, 4064(%ecx) {%k7}

// CHECK:      {evex} vpaxorq  %ymm2, -4096(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfe,0x2f,0xfc,0x52,0x80]
               {evex} vpaxorq  %ymm2, -4096(%edx) {%k7}

// CHECK:      {evex} vpaxorq  %xmm2, 268435456(%esp,%esi,8)
// CHECK: encoding: [0x62,0xf2,0xfe,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaxorq  %xmm2, 268435456(%esp,%esi,8)

// CHECK:      {evex} vpaxorq  %xmm2, 291(%edi,%eax,4) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfe,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaxorq  %xmm2, 291(%edi,%eax,4) {%k7}

// CHECK:      {evex} vpaxorq  %xmm2, (%eax)
// CHECK: encoding: [0x62,0xf2,0xfe,0x08,0xfc,0x10]
               {evex} vpaxorq  %xmm2, (%eax)

// CHECK:      {evex} vpaxorq  %xmm2, -512(,%ebp,2)
// CHECK: encoding: [0x62,0xf2,0xfe,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaxorq  %xmm2, -512(,%ebp,2)

// CHECK:      {evex} vpaxorq  %xmm2, 2032(%ecx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfe,0x0f,0xfc,0x51,0x7f]
               {evex} vpaxorq  %xmm2, 2032(%ecx) {%k7}

// CHECK:      {evex} vpaxorq  %xmm2, -2048(%edx) {%k7}
// CHECK: encoding: [0x62,0xf2,0xfe,0x0f,0xfc,0x52,0x80]
               {evex} vpaxorq  %xmm2, -2048(%edx) {%k7}

