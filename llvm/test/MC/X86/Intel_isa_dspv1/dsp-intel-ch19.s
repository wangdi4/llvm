// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: dvpalignr xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa7,0xd4,0x7b]
          dvpalignr xmm2, xmm3, xmm4, 123

// CHECK: dvpalignr xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa7,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpalignr xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpalignr xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa7,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpalignr xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpalignr xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa7,0x10,0x7b]
          dvpalignr xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpalignr xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa7,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpalignr xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpalignr xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa7,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpalignr xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpalignr xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x60,0xa7,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpalignr xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpmaxsb xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x60,0xd1,0xd4]
          dvpmaxsb xmm2, xmm3, xmm4

// CHECK: dvpmaxsb xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x60,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpmaxsb xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpmaxsb xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x60,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpmaxsb xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpmaxsb xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x60,0xd1,0x10]
          dvpmaxsb xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpmaxsb xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x60,0xd1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpmaxsb xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpmaxsb xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x60,0xd1,0x91,0xf0,0x07,0x00,0x00]
          dvpmaxsb xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpmaxsb xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x60,0xd1,0x92,0x00,0xf8,0xff,0xff]
          dvpmaxsb xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpmaxsd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x62,0xd1,0xd4]
          dvpmaxsd xmm2, xmm3, xmm4

// CHECK: dvpmaxsd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x62,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpmaxsd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpmaxsd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x62,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpmaxsd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpmaxsd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x62,0xd1,0x10]
          dvpmaxsd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpmaxsd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x62,0xd1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpmaxsd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpmaxsd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x62,0xd1,0x91,0xf0,0x07,0x00,0x00]
          dvpmaxsd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpmaxsd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x62,0xd1,0x92,0x00,0xf8,0xff,0xff]
          dvpmaxsd xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpmaxsw xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd1,0xd4]
          dvpmaxsw xmm2, xmm3, xmm4

// CHECK: dvpmaxsw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpmaxsw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpmaxsw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpmaxsw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpmaxsw xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd1,0x10]
          dvpmaxsw xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpmaxsw xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpmaxsw xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpmaxsw xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd1,0x91,0xf0,0x07,0x00,0x00]
          dvpmaxsw xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpmaxsw xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd1,0x92,0x00,0xf8,0xff,0xff]
          dvpmaxsw xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpmaxub xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x61,0xd1,0xd4]
          dvpmaxub xmm2, xmm3, xmm4

// CHECK: dvpmaxub xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x61,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpmaxub xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpmaxub xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x61,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpmaxub xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpmaxub xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x61,0xd1,0x10]
          dvpmaxub xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpmaxub xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x61,0xd1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpmaxub xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpmaxub xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x61,0xd1,0x91,0xf0,0x07,0x00,0x00]
          dvpmaxub xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpmaxub xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x61,0xd1,0x92,0x00,0xf8,0xff,0xff]
          dvpmaxub xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpmaxud xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x63,0xd1,0xd4]
          dvpmaxud xmm2, xmm3, xmm4

// CHECK: dvpmaxud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x63,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpmaxud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpmaxud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x63,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpmaxud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpmaxud xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x63,0xd1,0x10]
          dvpmaxud xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpmaxud xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x63,0xd1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpmaxud xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpmaxud xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x63,0xd1,0x91,0xf0,0x07,0x00,0x00]
          dvpmaxud xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpmaxud xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x63,0xd1,0x92,0x00,0xf8,0xff,0xff]
          dvpmaxud xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpmaxuw xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd1,0xd4]
          dvpmaxuw xmm2, xmm3, xmm4

// CHECK: dvpmaxuw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpmaxuw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpmaxuw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpmaxuw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpmaxuw xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd1,0x10]
          dvpmaxuw xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpmaxuw xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpmaxuw xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpmaxuw xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd1,0x91,0xf0,0x07,0x00,0x00]
          dvpmaxuw xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpmaxuw xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd1,0x92,0x00,0xf8,0xff,0xff]
          dvpmaxuw xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpminsb xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x60,0xd2,0xd4]
          dvpminsb xmm2, xmm3, xmm4

// CHECK: dvpminsb xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x60,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpminsb xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpminsb xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x60,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpminsb xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpminsb xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x60,0xd2,0x10]
          dvpminsb xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpminsb xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x60,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpminsb xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpminsb xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x60,0xd2,0x91,0xf0,0x07,0x00,0x00]
          dvpminsb xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpminsb xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x60,0xd2,0x92,0x00,0xf8,0xff,0xff]
          dvpminsb xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpminsd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x62,0xd2,0xd4]
          dvpminsd xmm2, xmm3, xmm4

// CHECK: dvpminsd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x62,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpminsd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpminsd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x62,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpminsd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpminsd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x62,0xd2,0x10]
          dvpminsd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpminsd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x62,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpminsd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpminsd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x62,0xd2,0x91,0xf0,0x07,0x00,0x00]
          dvpminsd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpminsd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x62,0xd2,0x92,0x00,0xf8,0xff,0xff]
          dvpminsd xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpminsw xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd2,0xd4]
          dvpminsw xmm2, xmm3, xmm4

// CHECK: dvpminsw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpminsw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpminsw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpminsw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpminsw xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd2,0x10]
          dvpminsw xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpminsw xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpminsw xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpminsw xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd2,0x91,0xf0,0x07,0x00,0x00]
          dvpminsw xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpminsw xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xd2,0x92,0x00,0xf8,0xff,0xff]
          dvpminsw xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpminub xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x61,0xd2,0xd4]
          dvpminub xmm2, xmm3, xmm4

// CHECK: dvpminub xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x61,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpminub xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpminub xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x61,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpminub xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpminub xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x61,0xd2,0x10]
          dvpminub xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpminub xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x61,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpminub xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpminub xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x61,0xd2,0x91,0xf0,0x07,0x00,0x00]
          dvpminub xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpminub xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x61,0xd2,0x92,0x00,0xf8,0xff,0xff]
          dvpminub xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpminud xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x63,0xd2,0xd4]
          dvpminud xmm2, xmm3, xmm4

// CHECK: dvpminud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x63,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpminud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpminud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x63,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpminud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpminud xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x63,0xd2,0x10]
          dvpminud xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpminud xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x63,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpminud xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpminud xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x63,0xd2,0x91,0xf0,0x07,0x00,0x00]
          dvpminud xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpminud xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x63,0xd2,0x92,0x00,0xf8,0xff,0xff]
          dvpminud xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpminuw xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd2,0xd4]
          dvpminuw xmm2, xmm3, xmm4

// CHECK: dvpminuw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpminuw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpminuw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpminuw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpminuw xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd2,0x10]
          dvpminuw xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpminuw xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpminuw xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpminuw xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd2,0x91,0xf0,0x07,0x00,0x00]
          dvpminuw xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpminuw xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xd2,0x92,0x00,0xf8,0xff,0xff]
          dvpminuw xmm2, xmm3, xmmword ptr [edx - 2048]

