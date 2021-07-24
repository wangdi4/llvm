// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: dvpslsd xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0x7a,0xa2,0xd3,0x7b]
          dvpslsd xmm2, xmm3, 123

// CHECK: dvpslsd xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x7a,0xa2,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpslsd xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpslsd xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x7a,0xa2,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpslsd xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpslsd xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x7a,0xa2,0x10,0x7b]
          dvpslsd xmm2, xmmword ptr [eax], 123

// CHECK: dvpslsd xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x7a,0xa2,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpslsd xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpslsd xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x7a,0xa2,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpslsd xmm2, xmmword ptr [ecx + 2032], 123

// CHECK: dvpslsd xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x7a,0xa2,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpslsd xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpslsud xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0x79,0xa2,0xd3,0x7b]
          dvpslsud xmm2, xmm3, 123

// CHECK: dvpslsud xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x79,0xa2,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpslsud xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpslsud xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x79,0xa2,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpslsud xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpslsud xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x79,0xa2,0x10,0x7b]
          dvpslsud xmm2, xmmword ptr [eax], 123

// CHECK: dvpslsud xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x79,0xa2,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpslsud xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpslsud xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x79,0xa2,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpslsud xmm2, xmmword ptr [ecx + 2032], 123

// CHECK: dvpslsud xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x79,0xa2,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpslsud xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpslvsd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x62,0xe2,0xd4]
          dvpslvsd xmm2, xmm3, xmm4

// CHECK: dvpslvsd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x62,0xe2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpslvsd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpslvsd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x62,0xe2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpslvsd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpslvsd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x62,0xe2,0x10]
          dvpslvsd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpslvsd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x62,0xe2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpslvsd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpslvsd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x62,0xe2,0x91,0xf0,0x07,0x00,0x00]
          dvpslvsd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpslvsd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x62,0xe2,0x92,0x00,0xf8,0xff,0xff]
          dvpslvsd xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpslvsud xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x61,0xe2,0xd4]
          dvpslvsud xmm2, xmm3, xmm4

// CHECK: dvpslvsud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x61,0xe2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpslvsud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpslvsud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x61,0xe2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpslvsud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpslvsud xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x61,0xe2,0x10]
          dvpslvsud xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpslvsud xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x61,0xe2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpslvsud xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpslvsud xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x61,0xe2,0x91,0xf0,0x07,0x00,0x00]
          dvpslvsud xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpslvsud xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x61,0xe2,0x92,0x00,0xf8,0xff,0xff]
          dvpslvsud xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpsrard xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0xa2,0xd3,0x7b]
          dvpsrard xmm2, xmm3, 123

// CHECK: dvpsrard xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0xa2,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpsrard xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpsrard xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0xa2,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpsrard xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpsrard xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0xa2,0x10,0x7b]
          dvpsrard xmm2, xmmword ptr [eax], 123

// CHECK: dvpsrard xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0xa2,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpsrard xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpsrard xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0xa2,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpsrard xmm2, xmmword ptr [ecx + 2032], 123

// CHECK: dvpsrard xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0xa2,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpsrard xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpsravrd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x63,0xe2,0xd4]
          dvpsravrd xmm2, xmm3, xmm4

// CHECK: dvpsravrd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x63,0xe2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpsravrd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpsravrd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x63,0xe2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpsravrd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpsravrd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x63,0xe2,0x10]
          dvpsravrd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpsravrd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x63,0xe2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpsravrd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpsravrd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x63,0xe2,0x91,0xf0,0x07,0x00,0x00]
          dvpsravrd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpsravrd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x63,0xe2,0x92,0x00,0xf8,0xff,0xff]
          dvpsravrd xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpsrrud xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa2,0xd3,0x7b]
          dvpsrrud xmm2, xmm3, 123

// CHECK: dvpsrrud xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa2,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpsrrud xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpsrrud xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa2,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpsrrud xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpsrrud xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa2,0x10,0x7b]
          dvpsrrud xmm2, xmmword ptr [eax], 123

// CHECK: dvpsrrud xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa2,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpsrrud xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpsrrud xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa2,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpsrrud xmm2, xmmword ptr [ecx + 2032], 123

// CHECK: dvpsrrud xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa2,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpsrrud xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpsrvrud xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x60,0xe2,0xd4]
          dvpsrvrud xmm2, xmm3, xmm4

// CHECK: dvpsrvrud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x60,0xe2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpsrvrud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpsrvrud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x60,0xe2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpsrvrud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpsrvrud xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x60,0xe2,0x10]
          dvpsrvrud xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpsrvrud xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x60,0xe2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpsrvrud xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpsrvrud xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x60,0xe2,0x91,0xf0,0x07,0x00,0x00]
          dvpsrvrud xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpsrvrud xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x60,0xe2,0x92,0x00,0xf8,0xff,0xff]
          dvpsrvrud xmm2, xmm3, xmmword ptr [edx - 2048]

