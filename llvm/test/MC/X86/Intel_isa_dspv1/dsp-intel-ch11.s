// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: dvpsadaccubws xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe3,0xdf,0xd4]
          dvpsadaccubws xmm2, xmm3, xmm4

// CHECK: dvpsadaccubws xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xdf,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpsadaccubws xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpsadaccubws xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xdf,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpsadaccubws xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpsadaccubws xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xdf,0x10]
          dvpsadaccubws xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpsadaccubws xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xdf,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpsadaccubws xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpsadaccubws xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xdf,0x91,0xf0,0x07,0x00,0x00]
          dvpsadaccubws xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpsadaccubws xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xdf,0x92,0x00,0xf8,0xff,0xff]
          dvpsadaccubws xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpslrsqd xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa1,0xd3,0x7b]
          dvpslrsqd xmm2, xmm3, 123

// CHECK: dvpslrsqd xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa1,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpslrsqd xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpslrsqd xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa1,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpslrsqd xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpslrsqd xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa1,0x10,0x7b]
          dvpslrsqd xmm2, xmmword ptr [eax], 123

// CHECK: dvpslrsqd xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa1,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpslrsqd xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpslrsqd xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa1,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpslrsqd xmm2, xmmword ptr [ecx + 2032], 123

// CHECK: dvpslrsqd xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa1,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpslrsqd xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpslrsqw xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa0,0xd3,0x7b]
          dvpslrsqw xmm2, xmm3, 123

// CHECK: dvpslrsqw xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa0,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpslrsqw xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpslrsqw xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa0,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpslrsqw xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpslrsqw xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa0,0x10,0x7b]
          dvpslrsqw xmm2, xmmword ptr [eax], 123

// CHECK: dvpslrsqw xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa0,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpslrsqw xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpslrsqw xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa0,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpslrsqw xmm2, xmmword ptr [ecx + 2032], 123

// CHECK: dvpslrsqw xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xfa,0xa0,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpslrsqw xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpslrsuqd xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa1,0xd3,0x7b]
          dvpslrsuqd xmm2, xmm3, 123

// CHECK: dvpslrsuqd xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa1,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpslrsuqd xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpslrsuqd xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa1,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpslrsuqd xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpslrsuqd xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa1,0x10,0x7b]
          dvpslrsuqd xmm2, xmmword ptr [eax], 123

// CHECK: dvpslrsuqd xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa1,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpslrsuqd xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpslrsuqd xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa1,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpslrsuqd xmm2, xmmword ptr [ecx + 2032], 123

// CHECK: dvpslrsuqd xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa1,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpslrsuqd xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpslrsuqw xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa0,0xd3,0x7b]
          dvpslrsuqw xmm2, xmm3, 123

// CHECK: dvpslrsuqw xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa0,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpslrsuqw xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpslrsuqw xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa0,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpslrsuqw xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpslrsuqw xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa0,0x10,0x7b]
          dvpslrsuqw xmm2, xmmword ptr [eax], 123

// CHECK: dvpslrsuqw xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa0,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpslrsuqw xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpslrsuqw xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa0,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpslrsuqw xmm2, xmmword ptr [ecx + 2032], 123

// CHECK: dvpslrsuqw xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xf9,0xa0,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpslrsuqw xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpslvrsqd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe1,0xd4]
          dvpslvrsqd xmm2, xmm3, xmm4

// CHECK: dvpslvrsqd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpslvrsqd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpslvrsqd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpslvrsqd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpslvrsqd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe1,0x10]
          dvpslvrsqd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpslvrsqd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpslvrsqd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpslvrsqd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe1,0x91,0xf0,0x07,0x00,0x00]
          dvpslvrsqd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpslvrsqd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe1,0x92,0x00,0xf8,0xff,0xff]
          dvpslvrsqd xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpslvrsqw xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe0,0xd4]
          dvpslvrsqw xmm2, xmm3, xmm4

// CHECK: dvpslvrsqw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe0,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpslvrsqw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpslvrsqw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe0,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpslvrsqw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpslvrsqw xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe0,0x10]
          dvpslvrsqw xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpslvrsqw xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpslvrsqw xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpslvrsqw xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe0,0x91,0xf0,0x07,0x00,0x00]
          dvpslvrsqw xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpslvrsqw xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xe0,0x92,0x00,0xf8,0xff,0xff]
          dvpslvrsqw xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpslvrsuqd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe1,0xd4]
          dvpslvrsuqd xmm2, xmm3, xmm4

// CHECK: dvpslvrsuqd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpslvrsuqd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpslvrsuqd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpslvrsuqd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpslvrsuqd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe1,0x10]
          dvpslvrsuqd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpslvrsuqd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpslvrsuqd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpslvrsuqd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe1,0x91,0xf0,0x07,0x00,0x00]
          dvpslvrsuqd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpslvrsuqd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe1,0x92,0x00,0xf8,0xff,0xff]
          dvpslvrsuqd xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpslvrsuqw xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe0,0xd4]
          dvpslvrsuqw xmm2, xmm3, xmm4

// CHECK: dvpslvrsuqw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe0,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpslvrsuqw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpslvrsuqw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe0,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpslvrsuqw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpslvrsuqw xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe0,0x10]
          dvpslvrsuqw xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpslvrsuqw xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpslvrsuqw xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpslvrsuqw xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe0,0x91,0xf0,0x07,0x00,0x00]
          dvpslvrsuqw xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpslvrsuqw xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xe0,0x92,0x00,0xf8,0xff,0xff]
          dvpslvrsuqw xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpsrarsqd xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa1,0xd3,0x7b]
          dvpsrarsqd xmm2, xmm3, 123

// CHECK: dvpsrarsqd xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa1,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpsrarsqd xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpsrarsqd xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa1,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpsrarsqd xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpsrarsqd xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa1,0x10,0x7b]
          dvpsrarsqd xmm2, xmmword ptr [eax], 123

// CHECK: dvpsrarsqd xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa1,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpsrarsqd xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpsrarsqd xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa1,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpsrarsqd xmm2, xmmword ptr [ecx + 2032], 123

// CHECK: dvpsrarsqd xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa1,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpsrarsqd xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpsrarsqw xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa0,0xd3,0x7b]
          dvpsrarsqw xmm2, xmm3, 123

// CHECK: dvpsrarsqw xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa0,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpsrarsqw xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpsrarsqw xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa0,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpsrarsqw xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpsrarsqw xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa0,0x10,0x7b]
          dvpsrarsqw xmm2, xmmword ptr [eax], 123

// CHECK: dvpsrarsqw xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa0,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpsrarsqw xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpsrarsqw xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa0,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpsrarsqw xmm2, xmmword ptr [ecx + 2032], 123

// CHECK: dvpsrarsqw xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xfb,0xa0,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpsrarsqw xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpsravrsqd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe1,0xd4]
          dvpsravrsqd xmm2, xmm3, xmm4

// CHECK: dvpsravrsqd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpsravrsqd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpsravrsqd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpsravrsqd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpsravrsqd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe1,0x10]
          dvpsravrsqd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpsravrsqd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpsravrsqd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpsravrsqd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe1,0x91,0xf0,0x07,0x00,0x00]
          dvpsravrsqd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpsravrsqd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe1,0x92,0x00,0xf8,0xff,0xff]
          dvpsravrsqd xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpsravrsqw xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe0,0xd4]
          dvpsravrsqw xmm2, xmm3, xmm4

// CHECK: dvpsravrsqw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe0,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpsravrsqw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpsravrsqw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe0,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpsravrsqw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpsravrsqw xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe0,0x10]
          dvpsravrsqw xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpsravrsqw xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpsravrsqw xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpsravrsqw xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe0,0x91,0xf0,0x07,0x00,0x00]
          dvpsravrsqw xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpsravrsqw xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xe0,0x92,0x00,0xf8,0xff,0xff]
          dvpsravrsqw xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpsrrsuqd xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa1,0xd3,0x7b]
          dvpsrrsuqd xmm2, xmm3, 123

// CHECK: dvpsrrsuqd xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa1,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpsrrsuqd xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpsrrsuqd xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa1,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpsrrsuqd xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpsrrsuqd xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa1,0x10,0x7b]
          dvpsrrsuqd xmm2, xmmword ptr [eax], 123

// CHECK: dvpsrrsuqd xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa1,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpsrrsuqd xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpsrrsuqd xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa1,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpsrrsuqd xmm2, xmmword ptr [ecx + 2032], 123

// CHECK: dvpsrrsuqd xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa1,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpsrrsuqd xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpsrrsuqw xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa0,0xd3,0x7b]
          dvpsrrsuqw xmm2, xmm3, 123

// CHECK: dvpsrrsuqw xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa0,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpsrrsuqw xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpsrrsuqw xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa0,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpsrrsuqw xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpsrrsuqw xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa0,0x10,0x7b]
          dvpsrrsuqw xmm2, xmmword ptr [eax], 123

// CHECK: dvpsrrsuqw xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa0,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpsrrsuqw xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpsrrsuqw xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa0,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpsrrsuqw xmm2, xmmword ptr [ecx + 2032], 123

// CHECK: dvpsrrsuqw xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xf8,0xa0,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpsrrsuqw xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpsrvrsuqd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe1,0xd4]
          dvpsrvrsuqd xmm2, xmm3, xmm4

// CHECK: dvpsrvrsuqd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpsrvrsuqd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpsrvrsuqd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpsrvrsuqd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpsrvrsuqd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe1,0x10]
          dvpsrvrsuqd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpsrvrsuqd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpsrvrsuqd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpsrvrsuqd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe1,0x91,0xf0,0x07,0x00,0x00]
          dvpsrvrsuqd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpsrvrsuqd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe1,0x92,0x00,0xf8,0xff,0xff]
          dvpsrvrsuqd xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpsrvrsuqw xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe0,0xd4]
          dvpsrvrsuqw xmm2, xmm3, xmm4

// CHECK: dvpsrvrsuqw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe0,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpsrvrsuqw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpsrvrsuqw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe0,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpsrvrsuqw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpsrvrsuqw xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe0,0x10]
          dvpsrvrsuqw xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpsrvrsuqw xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpsrvrsuqw xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpsrvrsuqw xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe0,0x91,0xf0,0x07,0x00,0x00]
          dvpsrvrsuqw xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpsrvrsuqw xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xe0,0x92,0x00,0xf8,0xff,0xff]
          dvpsrvrsuqw xmm2, xmm3, xmmword ptr [edx - 2048]

