// REQUIRES: intel_feature_isa_avx512_rao_int
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      {evex} vpaaddd ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaaddd ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      {evex} vpaaddd ymmword ptr [edi + 4*eax + 291] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaaddd ymmword ptr [edi + 4*eax + 291] {k7}, ymm2

// CHECK:      {evex} vpaaddd ymmword ptr [eax], ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0xfc,0x10]
               {evex} vpaaddd ymmword ptr [eax], ymm2

// CHECK:      {evex} vpaaddd ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaaddd ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      {evex} vpaaddd ymmword ptr [ecx + 4064] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0xfc,0x51,0x7f]
               {evex} vpaaddd ymmword ptr [ecx + 4064] {k7}, ymm2

// CHECK:      {evex} vpaaddd ymmword ptr [edx - 4096] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0xfc,0x52,0x80]
               {evex} vpaaddd ymmword ptr [edx - 4096] {k7}, ymm2

// CHECK:      {evex} vpaaddd xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaaddd xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vpaaddd xmmword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaaddd xmmword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vpaaddd xmmword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0xfc,0x10]
               {evex} vpaaddd xmmword ptr [eax], xmm2

// CHECK:      {evex} vpaaddd xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaaddd xmmword ptr [2*ebp - 512], xmm2

// CHECK:      {evex} vpaaddd xmmword ptr [ecx + 2032] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0xfc,0x51,0x7f]
               {evex} vpaaddd xmmword ptr [ecx + 2032] {k7}, xmm2

// CHECK:      {evex} vpaaddd xmmword ptr [edx - 2048] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0xfc,0x52,0x80]
               {evex} vpaaddd xmmword ptr [edx - 2048] {k7}, xmm2

// CHECK:      {evex} vpaaddq ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaaddq ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      {evex} vpaaddq ymmword ptr [edi + 4*eax + 291] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaaddq ymmword ptr [edi + 4*eax + 291] {k7}, ymm2

// CHECK:      {evex} vpaaddq ymmword ptr [eax], ymm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x28,0xfc,0x10]
               {evex} vpaaddq ymmword ptr [eax], ymm2

// CHECK:      {evex} vpaaddq ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaaddq ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      {evex} vpaaddq ymmword ptr [ecx + 4064] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x2f,0xfc,0x51,0x7f]
               {evex} vpaaddq ymmword ptr [ecx + 4064] {k7}, ymm2

// CHECK:      {evex} vpaaddq ymmword ptr [edx - 4096] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x2f,0xfc,0x52,0x80]
               {evex} vpaaddq ymmword ptr [edx - 4096] {k7}, ymm2

// CHECK:      {evex} vpaaddq xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaaddq xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vpaaddq xmmword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaaddq xmmword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vpaaddq xmmword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x08,0xfc,0x10]
               {evex} vpaaddq xmmword ptr [eax], xmm2

// CHECK:      {evex} vpaaddq xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaaddq xmmword ptr [2*ebp - 512], xmm2

// CHECK:      {evex} vpaaddq xmmword ptr [ecx + 2032] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x0f,0xfc,0x51,0x7f]
               {evex} vpaaddq xmmword ptr [ecx + 2032] {k7}, xmm2

// CHECK:      {evex} vpaaddq xmmword ptr [edx - 2048] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xfc,0x0f,0xfc,0x52,0x80]
               {evex} vpaaddq xmmword ptr [edx - 2048] {k7}, xmm2

// CHECK:      {evex} vpaandd ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaandd ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      {evex} vpaandd ymmword ptr [edi + 4*eax + 291] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaandd ymmword ptr [edi + 4*eax + 291] {k7}, ymm2

// CHECK:      {evex} vpaandd ymmword ptr [eax], ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xfc,0x10]
               {evex} vpaandd ymmword ptr [eax], ymm2

// CHECK:      {evex} vpaandd ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaandd ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      {evex} vpaandd ymmword ptr [ecx + 4064] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0xfc,0x51,0x7f]
               {evex} vpaandd ymmword ptr [ecx + 4064] {k7}, ymm2

// CHECK:      {evex} vpaandd ymmword ptr [edx - 4096] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0xfc,0x52,0x80]
               {evex} vpaandd ymmword ptr [edx - 4096] {k7}, ymm2

// CHECK:      {evex} vpaandd xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaandd xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vpaandd xmmword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaandd xmmword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vpaandd xmmword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xfc,0x10]
               {evex} vpaandd xmmword ptr [eax], xmm2

// CHECK:      {evex} vpaandd xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaandd xmmword ptr [2*ebp - 512], xmm2

// CHECK:      {evex} vpaandd xmmword ptr [ecx + 2032] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0xfc,0x51,0x7f]
               {evex} vpaandd xmmword ptr [ecx + 2032] {k7}, xmm2

// CHECK:      {evex} vpaandd xmmword ptr [edx - 2048] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0xfc,0x52,0x80]
               {evex} vpaandd xmmword ptr [edx - 2048] {k7}, xmm2

// CHECK:      {evex} vpaandq ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaandq ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      {evex} vpaandq ymmword ptr [edi + 4*eax + 291] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaandq ymmword ptr [edi + 4*eax + 291] {k7}, ymm2

// CHECK:      {evex} vpaandq ymmword ptr [eax], ymm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x28,0xfc,0x10]
               {evex} vpaandq ymmword ptr [eax], ymm2

// CHECK:      {evex} vpaandq ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaandq ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      {evex} vpaandq ymmword ptr [ecx + 4064] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x2f,0xfc,0x51,0x7f]
               {evex} vpaandq ymmword ptr [ecx + 4064] {k7}, ymm2

// CHECK:      {evex} vpaandq ymmword ptr [edx - 4096] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x2f,0xfc,0x52,0x80]
               {evex} vpaandq ymmword ptr [edx - 4096] {k7}, ymm2

// CHECK:      {evex} vpaandq xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaandq xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vpaandq xmmword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaandq xmmword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vpaandq xmmword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x08,0xfc,0x10]
               {evex} vpaandq xmmword ptr [eax], xmm2

// CHECK:      {evex} vpaandq xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaandq xmmword ptr [2*ebp - 512], xmm2

// CHECK:      {evex} vpaandq xmmword ptr [ecx + 2032] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x0f,0xfc,0x51,0x7f]
               {evex} vpaandq xmmword ptr [ecx + 2032] {k7}, xmm2

// CHECK:      {evex} vpaandq xmmword ptr [edx - 2048] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x0f,0xfc,0x52,0x80]
               {evex} vpaandq xmmword ptr [edx - 2048] {k7}, xmm2

// CHECK:      {evex} vpaord ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaord ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      {evex} vpaord ymmword ptr [edi + 4*eax + 291] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaord ymmword ptr [edi + 4*eax + 291] {k7}, ymm2

// CHECK:      {evex} vpaord ymmword ptr [eax], ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0xfc,0x10]
               {evex} vpaord ymmword ptr [eax], ymm2

// CHECK:      {evex} vpaord ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaord ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      {evex} vpaord ymmword ptr [ecx + 4064] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x2f,0xfc,0x51,0x7f]
               {evex} vpaord ymmword ptr [ecx + 4064] {k7}, ymm2

// CHECK:      {evex} vpaord ymmword ptr [edx - 4096] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x2f,0xfc,0x52,0x80]
               {evex} vpaord ymmword ptr [edx - 4096] {k7}, ymm2

// CHECK:      {evex} vpaord xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaord xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vpaord xmmword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaord xmmword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vpaord xmmword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0xfc,0x10]
               {evex} vpaord xmmword ptr [eax], xmm2

// CHECK:      {evex} vpaord xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaord xmmword ptr [2*ebp - 512], xmm2

// CHECK:      {evex} vpaord xmmword ptr [ecx + 2032] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0xfc,0x51,0x7f]
               {evex} vpaord xmmword ptr [ecx + 2032] {k7}, xmm2

// CHECK:      {evex} vpaord xmmword ptr [edx - 2048] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0xfc,0x52,0x80]
               {evex} vpaord xmmword ptr [edx - 2048] {k7}, xmm2

// CHECK:      {evex} vpaorq ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0x62,0xf2,0xff,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaorq ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      {evex} vpaorq ymmword ptr [edi + 4*eax + 291] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xff,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaorq ymmword ptr [edi + 4*eax + 291] {k7}, ymm2

// CHECK:      {evex} vpaorq ymmword ptr [eax], ymm2
// CHECK: encoding: [0x62,0xf2,0xff,0x28,0xfc,0x10]
               {evex} vpaorq ymmword ptr [eax], ymm2

// CHECK:      {evex} vpaorq ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0x62,0xf2,0xff,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaorq ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      {evex} vpaorq ymmword ptr [ecx + 4064] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xff,0x2f,0xfc,0x51,0x7f]
               {evex} vpaorq ymmword ptr [ecx + 4064] {k7}, ymm2

// CHECK:      {evex} vpaorq ymmword ptr [edx - 4096] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xff,0x2f,0xfc,0x52,0x80]
               {evex} vpaorq ymmword ptr [edx - 4096] {k7}, ymm2

// CHECK:      {evex} vpaorq xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaorq xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vpaorq xmmword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaorq xmmword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vpaorq xmmword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x08,0xfc,0x10]
               {evex} vpaorq xmmword ptr [eax], xmm2

// CHECK:      {evex} vpaorq xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaorq xmmword ptr [2*ebp - 512], xmm2

// CHECK:      {evex} vpaorq xmmword ptr [ecx + 2032] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x0f,0xfc,0x51,0x7f]
               {evex} vpaorq xmmword ptr [ecx + 2032] {k7}, xmm2

// CHECK:      {evex} vpaorq xmmword ptr [edx - 2048] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xff,0x0f,0xfc,0x52,0x80]
               {evex} vpaorq xmmword ptr [edx - 2048] {k7}, xmm2

// CHECK:      {evex} vpaxord ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaxord ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      {evex} vpaxord ymmword ptr [edi + 4*eax + 291] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaxord ymmword ptr [edi + 4*eax + 291] {k7}, ymm2

// CHECK:      {evex} vpaxord ymmword ptr [eax], ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xfc,0x10]
               {evex} vpaxord ymmword ptr [eax], ymm2

// CHECK:      {evex} vpaxord ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaxord ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      {evex} vpaxord ymmword ptr [ecx + 4064] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0xfc,0x51,0x7f]
               {evex} vpaxord ymmword ptr [ecx + 4064] {k7}, ymm2

// CHECK:      {evex} vpaxord ymmword ptr [edx - 4096] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0xfc,0x52,0x80]
               {evex} vpaxord ymmword ptr [edx - 4096] {k7}, ymm2

// CHECK:      {evex} vpaxord xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaxord xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vpaxord xmmword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaxord xmmword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vpaxord xmmword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xfc,0x10]
               {evex} vpaxord xmmword ptr [eax], xmm2

// CHECK:      {evex} vpaxord xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaxord xmmword ptr [2*ebp - 512], xmm2

// CHECK:      {evex} vpaxord xmmword ptr [ecx + 2032] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0xfc,0x51,0x7f]
               {evex} vpaxord xmmword ptr [ecx + 2032] {k7}, xmm2

// CHECK:      {evex} vpaxord xmmword ptr [edx - 2048] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0xfc,0x52,0x80]
               {evex} vpaxord xmmword ptr [edx - 2048] {k7}, xmm2

// CHECK:      {evex} vpaxorq ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x28,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaxorq ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      {evex} vpaxorq ymmword ptr [edi + 4*eax + 291] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x2f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaxorq ymmword ptr [edi + 4*eax + 291] {k7}, ymm2

// CHECK:      {evex} vpaxorq ymmword ptr [eax], ymm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x28,0xfc,0x10]
               {evex} vpaxorq ymmword ptr [eax], ymm2

// CHECK:      {evex} vpaxorq ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x28,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vpaxorq ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      {evex} vpaxorq ymmword ptr [ecx + 4064] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x2f,0xfc,0x51,0x7f]
               {evex} vpaxorq ymmword ptr [ecx + 4064] {k7}, ymm2

// CHECK:      {evex} vpaxorq ymmword ptr [edx - 4096] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x2f,0xfc,0x52,0x80]
               {evex} vpaxorq ymmword ptr [edx - 4096] {k7}, ymm2

// CHECK:      {evex} vpaxorq xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x08,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vpaxorq xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vpaxorq xmmword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x0f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vpaxorq xmmword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vpaxorq xmmword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x08,0xfc,0x10]
               {evex} vpaxorq xmmword ptr [eax], xmm2

// CHECK:      {evex} vpaxorq xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x08,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vpaxorq xmmword ptr [2*ebp - 512], xmm2

// CHECK:      {evex} vpaxorq xmmword ptr [ecx + 2032] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x0f,0xfc,0x51,0x7f]
               {evex} vpaxorq xmmword ptr [ecx + 2032] {k7}, xmm2

// CHECK:      {evex} vpaxorq xmmword ptr [edx - 2048] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xfe,0x0f,0xfc,0x52,0x80]
               {evex} vpaxorq xmmword ptr [edx - 2048] {k7}, xmm2

