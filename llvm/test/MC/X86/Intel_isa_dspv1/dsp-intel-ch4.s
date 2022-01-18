// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: dvpdpbwssd xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa8,0xd4,0x7b]
          dvpdpbwssd xmm2, xmm3, xmm4, 123

// CHECK: dvpdpbwssd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa8,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpdpbwssd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpdpbwssd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa8,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpdpbwssd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpdpbwssd xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa8,0x10,0x7b]
          dvpdpbwssd xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpdpbwssd xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa8,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpdpbwssd xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpdpbwssd xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa8,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpdpbwssd xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpdpbwssd xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa8,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpdpbwssd xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpdpbwssds xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa9,0xd4,0x7b]
          dvpdpbwssds xmm2, xmm3, xmm4, 123

// CHECK: dvpdpbwssds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa9,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpdpbwssds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpdpbwssds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa9,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpdpbwssds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpdpbwssds xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa9,0x10,0x7b]
          dvpdpbwssds xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpdpbwssds xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa9,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpdpbwssds xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpdpbwssds xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa9,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpdpbwssds xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpdpbwssds xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa9,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpdpbwssds xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpdpbwsud xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa8,0xd4,0x7b]
          dvpdpbwsud xmm2, xmm3, xmm4, 123

// CHECK: dvpdpbwsud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa8,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpdpbwsud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpdpbwsud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa8,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpdpbwsud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpdpbwsud xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa8,0x10,0x7b]
          dvpdpbwsud xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpdpbwsud xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa8,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpdpbwsud xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpdpbwsud xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa8,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpdpbwsud xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpdpbwsud xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa8,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpdpbwsud xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpdpbwsuds xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa9,0xd4,0x7b]
          dvpdpbwsuds xmm2, xmm3, xmm4, 123

// CHECK: dvpdpbwsuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa9,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpdpbwsuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpdpbwsuds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa9,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpdpbwsuds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpdpbwsuds xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa9,0x10,0x7b]
          dvpdpbwsuds xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpdpbwsuds xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa9,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpdpbwsuds xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpdpbwsuds xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa9,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpdpbwsuds xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpdpbwsuds xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa9,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpdpbwsuds xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpdpbwusd xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa8,0xd4,0x7b]
          dvpdpbwusd xmm2, xmm3, xmm4, 123

// CHECK: dvpdpbwusd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa8,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpdpbwusd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpdpbwusd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa8,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpdpbwusd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpdpbwusd xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa8,0x10,0x7b]
          dvpdpbwusd xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpdpbwusd xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa8,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpdpbwusd xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpdpbwusd xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa8,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpdpbwusd xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpdpbwusd xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa8,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpdpbwusd xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpdpbwusds xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa9,0xd4,0x7b]
          dvpdpbwusds xmm2, xmm3, xmm4, 123

// CHECK: dvpdpbwusds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa9,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpdpbwusds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpdpbwusds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa9,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpdpbwusds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpdpbwusds xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa9,0x10,0x7b]
          dvpdpbwusds xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpdpbwusds xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa9,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpdpbwusds xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpdpbwusds xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa9,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpdpbwusds xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpdpbwusds xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa9,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpdpbwusds xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpdpbwuud xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa8,0xd4,0x7b]
          dvpdpbwuud xmm2, xmm3, xmm4, 123

// CHECK: dvpdpbwuud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa8,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpdpbwuud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpdpbwuud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa8,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpdpbwuud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpdpbwuud xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa8,0x10,0x7b]
          dvpdpbwuud xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpdpbwuud xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa8,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpdpbwuud xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpdpbwuud xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa8,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpdpbwuud xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpdpbwuud xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa8,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpdpbwuud xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpdpbwuuds xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa9,0xd4,0x7b]
          dvpdpbwuuds xmm2, xmm3, xmm4, 123

// CHECK: dvpdpbwuuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa9,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpdpbwuuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpdpbwuuds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa9,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpdpbwuuds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpdpbwuuds xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa9,0x10,0x7b]
          dvpdpbwuuds xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpdpbwuuds xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa9,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpdpbwuuds xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpdpbwuuds xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa9,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpdpbwuuds xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpdpbwuuds xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa9,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpdpbwuuds xmm2, xmm3, xmmword ptr [edx - 2048], 123

