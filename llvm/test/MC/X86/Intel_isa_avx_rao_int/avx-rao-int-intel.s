// REQUIRES: intel_feature_isa_avx_rao_int
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vpaaddd ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaaddd ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      vpaaddd ymmword ptr [edi + 4*eax + 291], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaaddd ymmword ptr [edi + 4*eax + 291], ymm2

// CHECK:      vpaaddd ymmword ptr [eax], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0xfc,0x10]
               vpaaddd ymmword ptr [eax], ymm2

// CHECK:      vpaaddd ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpaaddd ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      vpaaddd ymmword ptr [ecx + 4064], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0xfc,0x91,0xe0,0x0f,0x00,0x00]
               vpaaddd ymmword ptr [ecx + 4064], ymm2

// CHECK:      vpaaddd ymmword ptr [edx - 4096], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0xfc,0x92,0x00,0xf0,0xff,0xff]
               vpaaddd ymmword ptr [edx - 4096], ymm2

// CHECK:      vpaaddd xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaaddd xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vpaaddd xmmword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaaddd xmmword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vpaaddd xmmword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0xfc,0x10]
               vpaaddd xmmword ptr [eax], xmm2

// CHECK:      vpaaddd xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpaaddd xmmword ptr [2*ebp - 512], xmm2

// CHECK:      vpaaddd xmmword ptr [ecx + 2032], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0xfc,0x91,0xf0,0x07,0x00,0x00]
               vpaaddd xmmword ptr [ecx + 2032], xmm2

// CHECK:      vpaaddd xmmword ptr [edx - 2048], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0xfc,0x92,0x00,0xf8,0xff,0xff]
               vpaaddd xmmword ptr [edx - 2048], xmm2

// CHECK:      vpaaddq ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfc,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaaddq ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      vpaaddq ymmword ptr [edi + 4*eax + 291], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfc,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaaddq ymmword ptr [edi + 4*eax + 291], ymm2

// CHECK:      vpaaddq ymmword ptr [eax], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfc,0xfc,0x10]
               vpaaddq ymmword ptr [eax], ymm2

// CHECK:      vpaaddq ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfc,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpaaddq ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      vpaaddq ymmword ptr [ecx + 4064], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfc,0xfc,0x91,0xe0,0x0f,0x00,0x00]
               vpaaddq ymmword ptr [ecx + 4064], ymm2

// CHECK:      vpaaddq ymmword ptr [edx - 4096], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfc,0xfc,0x92,0x00,0xf0,0xff,0xff]
               vpaaddq ymmword ptr [edx - 4096], ymm2

// CHECK:      vpaaddq xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf8,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaaddq xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vpaaddq xmmword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf8,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaaddq xmmword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vpaaddq xmmword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf8,0xfc,0x10]
               vpaaddq xmmword ptr [eax], xmm2

// CHECK:      vpaaddq xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf8,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpaaddq xmmword ptr [2*ebp - 512], xmm2

// CHECK:      vpaaddq xmmword ptr [ecx + 2032], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf8,0xfc,0x91,0xf0,0x07,0x00,0x00]
               vpaaddq xmmword ptr [ecx + 2032], xmm2

// CHECK:      vpaaddq xmmword ptr [edx - 2048], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf8,0xfc,0x92,0x00,0xf8,0xff,0xff]
               vpaaddq xmmword ptr [edx - 2048], xmm2

// CHECK:      vpaandd ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7d,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaandd ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      vpaandd ymmword ptr [edi + 4*eax + 291], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7d,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaandd ymmword ptr [edi + 4*eax + 291], ymm2

// CHECK:      vpaandd ymmword ptr [eax], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7d,0xfc,0x10]
               vpaandd ymmword ptr [eax], ymm2

// CHECK:      vpaandd ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7d,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpaandd ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      vpaandd ymmword ptr [ecx + 4064], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7d,0xfc,0x91,0xe0,0x0f,0x00,0x00]
               vpaandd ymmword ptr [ecx + 4064], ymm2

// CHECK:      vpaandd ymmword ptr [edx - 4096], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7d,0xfc,0x92,0x00,0xf0,0xff,0xff]
               vpaandd ymmword ptr [edx - 4096], ymm2

// CHECK:      vpaandd xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaandd xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vpaandd xmmword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaandd xmmword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vpaandd xmmword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0xfc,0x10]
               vpaandd xmmword ptr [eax], xmm2

// CHECK:      vpaandd xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpaandd xmmword ptr [2*ebp - 512], xmm2

// CHECK:      vpaandd xmmword ptr [ecx + 2032], xmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0xfc,0x91,0xf0,0x07,0x00,0x00]
               vpaandd xmmword ptr [ecx + 2032], xmm2

// CHECK:      vpaandd xmmword ptr [edx - 2048], xmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0xfc,0x92,0x00,0xf8,0xff,0xff]
               vpaandd xmmword ptr [edx - 2048], xmm2

// CHECK:      vpaandq ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfd,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaandq ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      vpaandq ymmword ptr [edi + 4*eax + 291], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfd,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaandq ymmword ptr [edi + 4*eax + 291], ymm2

// CHECK:      vpaandq ymmword ptr [eax], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfd,0xfc,0x10]
               vpaandq ymmword ptr [eax], ymm2

// CHECK:      vpaandq ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfd,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpaandq ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      vpaandq ymmword ptr [ecx + 4064], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfd,0xfc,0x91,0xe0,0x0f,0x00,0x00]
               vpaandq ymmword ptr [ecx + 4064], ymm2

// CHECK:      vpaandq ymmword ptr [edx - 4096], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfd,0xfc,0x92,0x00,0xf0,0xff,0xff]
               vpaandq ymmword ptr [edx - 4096], ymm2

// CHECK:      vpaandq xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf9,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaandq xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vpaandq xmmword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf9,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaandq xmmword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vpaandq xmmword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf9,0xfc,0x10]
               vpaandq xmmword ptr [eax], xmm2

// CHECK:      vpaandq xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf9,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpaandq xmmword ptr [2*ebp - 512], xmm2

// CHECK:      vpaandq xmmword ptr [ecx + 2032], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf9,0xfc,0x91,0xf0,0x07,0x00,0x00]
               vpaandq xmmword ptr [ecx + 2032], xmm2

// CHECK:      vpaandq xmmword ptr [edx - 2048], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf9,0xfc,0x92,0x00,0xf8,0xff,0xff]
               vpaandq xmmword ptr [edx - 2048], xmm2

// CHECK:      vpaord ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7f,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaord ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      vpaord ymmword ptr [edi + 4*eax + 291], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7f,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaord ymmword ptr [edi + 4*eax + 291], ymm2

// CHECK:      vpaord ymmword ptr [eax], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7f,0xfc,0x10]
               vpaord ymmword ptr [eax], ymm2

// CHECK:      vpaord ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7f,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpaord ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      vpaord ymmword ptr [ecx + 4064], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7f,0xfc,0x91,0xe0,0x0f,0x00,0x00]
               vpaord ymmword ptr [ecx + 4064], ymm2

// CHECK:      vpaord ymmword ptr [edx - 4096], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7f,0xfc,0x92,0x00,0xf0,0xff,0xff]
               vpaord ymmword ptr [edx - 4096], ymm2

// CHECK:      vpaord xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaord xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vpaord xmmword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaord xmmword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vpaord xmmword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0xfc,0x10]
               vpaord xmmword ptr [eax], xmm2

// CHECK:      vpaord xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpaord xmmword ptr [2*ebp - 512], xmm2

// CHECK:      vpaord xmmword ptr [ecx + 2032], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0xfc,0x91,0xf0,0x07,0x00,0x00]
               vpaord xmmword ptr [ecx + 2032], xmm2

// CHECK:      vpaord xmmword ptr [edx - 2048], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0xfc,0x92,0x00,0xf8,0xff,0xff]
               vpaord xmmword ptr [edx - 2048], xmm2

// CHECK:      vpaorq ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0xc4,0xe2,0xff,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaorq ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      vpaorq ymmword ptr [edi + 4*eax + 291], ymm2
// CHECK: encoding: [0xc4,0xe2,0xff,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaorq ymmword ptr [edi + 4*eax + 291], ymm2

// CHECK:      vpaorq ymmword ptr [eax], ymm2
// CHECK: encoding: [0xc4,0xe2,0xff,0xfc,0x10]
               vpaorq ymmword ptr [eax], ymm2

// CHECK:      vpaorq ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0xc4,0xe2,0xff,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpaorq ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      vpaorq ymmword ptr [ecx + 4064], ymm2
// CHECK: encoding: [0xc4,0xe2,0xff,0xfc,0x91,0xe0,0x0f,0x00,0x00]
               vpaorq ymmword ptr [ecx + 4064], ymm2

// CHECK:      vpaorq ymmword ptr [edx - 4096], ymm2
// CHECK: encoding: [0xc4,0xe2,0xff,0xfc,0x92,0x00,0xf0,0xff,0xff]
               vpaorq ymmword ptr [edx - 4096], ymm2

// CHECK:      vpaorq xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfb,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaorq xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vpaorq xmmword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfb,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaorq xmmword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vpaorq xmmword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfb,0xfc,0x10]
               vpaorq xmmword ptr [eax], xmm2

// CHECK:      vpaorq xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfb,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpaorq xmmword ptr [2*ebp - 512], xmm2

// CHECK:      vpaorq xmmword ptr [ecx + 2032], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfb,0xfc,0x91,0xf0,0x07,0x00,0x00]
               vpaorq xmmword ptr [ecx + 2032], xmm2

// CHECK:      vpaorq xmmword ptr [edx - 2048], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfb,0xfc,0x92,0x00,0xf8,0xff,0xff]
               vpaorq xmmword ptr [edx - 2048], xmm2

// CHECK:      vpaxord ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7e,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaxord ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      vpaxord ymmword ptr [edi + 4*eax + 291], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7e,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaxord ymmword ptr [edi + 4*eax + 291], ymm2

// CHECK:      vpaxord ymmword ptr [eax], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7e,0xfc,0x10]
               vpaxord ymmword ptr [eax], ymm2

// CHECK:      vpaxord ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7e,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpaxord ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      vpaxord ymmword ptr [ecx + 4064], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7e,0xfc,0x91,0xe0,0x0f,0x00,0x00]
               vpaxord ymmword ptr [ecx + 4064], ymm2

// CHECK:      vpaxord ymmword ptr [edx - 4096], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7e,0xfc,0x92,0x00,0xf0,0xff,0xff]
               vpaxord ymmword ptr [edx - 4096], ymm2

// CHECK:      vpaxord xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaxord xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vpaxord xmmword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaxord xmmword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vpaxord xmmword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0xfc,0x10]
               vpaxord xmmword ptr [eax], xmm2

// CHECK:      vpaxord xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpaxord xmmword ptr [2*ebp - 512], xmm2

// CHECK:      vpaxord xmmword ptr [ecx + 2032], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0xfc,0x91,0xf0,0x07,0x00,0x00]
               vpaxord xmmword ptr [ecx + 2032], xmm2

// CHECK:      vpaxord xmmword ptr [edx - 2048], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0xfc,0x92,0x00,0xf8,0xff,0xff]
               vpaxord xmmword ptr [edx - 2048], xmm2

// CHECK:      vpaxorq ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfe,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaxorq ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      vpaxorq ymmword ptr [edi + 4*eax + 291], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfe,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaxorq ymmword ptr [edi + 4*eax + 291], ymm2

// CHECK:      vpaxorq ymmword ptr [eax], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfe,0xfc,0x10]
               vpaxorq ymmword ptr [eax], ymm2

// CHECK:      vpaxorq ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfe,0xfc,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vpaxorq ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      vpaxorq ymmword ptr [ecx + 4064], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfe,0xfc,0x91,0xe0,0x0f,0x00,0x00]
               vpaxorq ymmword ptr [ecx + 4064], ymm2

// CHECK:      vpaxorq ymmword ptr [edx - 4096], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfe,0xfc,0x92,0x00,0xf0,0xff,0xff]
               vpaxorq ymmword ptr [edx - 4096], ymm2

// CHECK:      vpaxorq xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfa,0xfc,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpaxorq xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vpaxorq xmmword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfa,0xfc,0x94,0x87,0x23,0x01,0x00,0x00]
               vpaxorq xmmword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vpaxorq xmmword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfa,0xfc,0x10]
               vpaxorq xmmword ptr [eax], xmm2

// CHECK:      vpaxorq xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfa,0xfc,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vpaxorq xmmword ptr [2*ebp - 512], xmm2

// CHECK:      vpaxorq xmmword ptr [ecx + 2032], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfa,0xfc,0x91,0xf0,0x07,0x00,0x00]
               vpaxorq xmmword ptr [ecx + 2032], xmm2

// CHECK:      vpaxorq xmmword ptr [edx - 2048], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfa,0xfc,0x92,0x00,0xf8,0xff,0xff]
               vpaxorq xmmword ptr [edx - 2048], xmm2

