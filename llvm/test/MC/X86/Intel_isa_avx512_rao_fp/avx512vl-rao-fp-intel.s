// REQUIRES: intel_feature_isa_avx512_rao_fp
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      {evex} vaaddpbf16 ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddpbf16 ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      {evex} vaaddpbf16 ymmword ptr [edi + 4*eax + 291] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddpbf16 ymmword ptr [edi + 4*eax + 291] {k7}, ymm2

// CHECK:      {evex} vaaddpbf16 ymmword ptr [eax], ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0x94,0x10]
               {evex} vaaddpbf16 ymmword ptr [eax], ymm2

// CHECK:      {evex} vaaddpbf16 ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0x94,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vaaddpbf16 ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      {evex} vaaddpbf16 ymmword ptr [ecx + 4064] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0x94,0x51,0x7f]
               {evex} vaaddpbf16 ymmword ptr [ecx + 4064] {k7}, ymm2

// CHECK:      {evex} vaaddpbf16 ymmword ptr [edx - 4096] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0x94,0x52,0x80]
               {evex} vaaddpbf16 ymmword ptr [edx - 4096] {k7}, ymm2

// CHECK:      {evex} vaaddpbf16 xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddpbf16 xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vaaddpbf16 xmmword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddpbf16 xmmword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vaaddpbf16 xmmword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0x94,0x10]
               {evex} vaaddpbf16 xmmword ptr [eax], xmm2

// CHECK:      {evex} vaaddpbf16 xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0x94,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vaaddpbf16 xmmword ptr [2*ebp - 512], xmm2

// CHECK:      {evex} vaaddpbf16 xmmword ptr [ecx + 2032] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0x94,0x51,0x7f]
               {evex} vaaddpbf16 xmmword ptr [ecx + 2032] {k7}, xmm2

// CHECK:      {evex} vaaddpbf16 xmmword ptr [edx - 2048] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0x94,0x52,0x80]
               {evex} vaaddpbf16 xmmword ptr [edx - 2048] {k7}, xmm2

// CHECK:      {evex} vaaddpd ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x28,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddpd ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      {evex} vaaddpd ymmword ptr [edi + 4*eax + 291] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x2f,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddpd ymmword ptr [edi + 4*eax + 291] {k7}, ymm2

// CHECK:      {evex} vaaddpd ymmword ptr [eax], ymm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x28,0x84,0x10]
               {evex} vaaddpd ymmword ptr [eax], ymm2

// CHECK:      {evex} vaaddpd ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x28,0x84,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vaaddpd ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      {evex} vaaddpd ymmword ptr [ecx + 4064] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x2f,0x84,0x51,0x7f]
               {evex} vaaddpd ymmword ptr [ecx + 4064] {k7}, ymm2

// CHECK:      {evex} vaaddpd ymmword ptr [edx - 4096] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x2f,0x84,0x52,0x80]
               {evex} vaaddpd ymmword ptr [edx - 4096] {k7}, ymm2

// CHECK:      {evex} vaaddpd xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x08,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddpd xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vaaddpd xmmword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x0f,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddpd xmmword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vaaddpd xmmword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x08,0x84,0x10]
               {evex} vaaddpd xmmword ptr [eax], xmm2

// CHECK:      {evex} vaaddpd xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x08,0x84,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vaaddpd xmmword ptr [2*ebp - 512], xmm2

// CHECK:      {evex} vaaddpd xmmword ptr [ecx + 2032] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x0f,0x84,0x51,0x7f]
               {evex} vaaddpd xmmword ptr [ecx + 2032] {k7}, xmm2

// CHECK:      {evex} vaaddpd xmmword ptr [edx - 2048] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0xfd,0x0f,0x84,0x52,0x80]
               {evex} vaaddpd xmmword ptr [edx - 2048] {k7}, xmm2

// CHECK:      {evex} vaaddph ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddph ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      {evex} vaaddph ymmword ptr [edi + 4*eax + 291] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddph ymmword ptr [edi + 4*eax + 291] {k7}, ymm2

// CHECK:      {evex} vaaddph ymmword ptr [eax], ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x94,0x10]
               {evex} vaaddph ymmword ptr [eax], ymm2

// CHECK:      {evex} vaaddph ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x94,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vaaddph ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      {evex} vaaddph ymmword ptr [ecx + 4064] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x94,0x51,0x7f]
               {evex} vaaddph ymmword ptr [ecx + 4064] {k7}, ymm2

// CHECK:      {evex} vaaddph ymmword ptr [edx - 4096] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x94,0x52,0x80]
               {evex} vaaddph ymmword ptr [edx - 4096] {k7}, ymm2

// CHECK:      {evex} vaaddph xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddph xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vaaddph xmmword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddph xmmword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vaaddph xmmword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x94,0x10]
               {evex} vaaddph xmmword ptr [eax], xmm2

// CHECK:      {evex} vaaddph xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x94,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vaaddph xmmword ptr [2*ebp - 512], xmm2

// CHECK:      {evex} vaaddph xmmword ptr [ecx + 2032] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x94,0x51,0x7f]
               {evex} vaaddph xmmword ptr [ecx + 2032] {k7}, xmm2

// CHECK:      {evex} vaaddph xmmword ptr [edx - 2048] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x94,0x52,0x80]
               {evex} vaaddph xmmword ptr [edx - 2048] {k7}, xmm2

// CHECK:      {evex} vaaddps ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddps ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      {evex} vaaddps ymmword ptr [edi + 4*eax + 291] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddps ymmword ptr [edi + 4*eax + 291] {k7}, ymm2

// CHECK:      {evex} vaaddps ymmword ptr [eax], ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x84,0x10]
               {evex} vaaddps ymmword ptr [eax], ymm2

// CHECK:      {evex} vaaddps ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0x84,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vaaddps ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      {evex} vaaddps ymmword ptr [ecx + 4064] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x84,0x51,0x7f]
               {evex} vaaddps ymmword ptr [ecx + 4064] {k7}, ymm2

// CHECK:      {evex} vaaddps ymmword ptr [edx - 4096] {k7}, ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0x84,0x52,0x80]
               {evex} vaaddps ymmword ptr [edx - 4096] {k7}, ymm2

// CHECK:      {evex} vaaddps xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vaaddps xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      {evex} vaaddps xmmword ptr [edi + 4*eax + 291] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vaaddps xmmword ptr [edi + 4*eax + 291] {k7}, xmm2

// CHECK:      {evex} vaaddps xmmword ptr [eax], xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x84,0x10]
               {evex} vaaddps xmmword ptr [eax], xmm2

// CHECK:      {evex} vaaddps xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0x84,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vaaddps xmmword ptr [2*ebp - 512], xmm2

// CHECK:      {evex} vaaddps xmmword ptr [ecx + 2032] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x84,0x51,0x7f]
               {evex} vaaddps xmmword ptr [ecx + 2032] {k7}, xmm2

// CHECK:      {evex} vaaddps xmmword ptr [edx - 2048] {k7}, xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0x84,0x52,0x80]
               {evex} vaaddps xmmword ptr [edx - 2048] {k7}, xmm2

