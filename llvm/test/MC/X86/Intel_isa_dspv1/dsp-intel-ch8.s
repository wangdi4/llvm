// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: dvpdpbssd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x63,0xc1,0xd4]
          dvpdpbssd xmm2, xmm3, xmm4

// CHECK: dvpdpbssd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpbssd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpbssd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpbssd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpbssd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc1,0x10]
          dvpdpbssd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpbssd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpbssd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpbssd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc1,0x91,0xf0,0x07,0x00,0x00]
          dvpdpbssd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpbssd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc1,0x92,0x00,0xf8,0xff,0xff]
          dvpdpbssd xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpbssds xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x63,0xc2,0xd4]
          dvpdpbssds xmm2, xmm3, xmm4

// CHECK: dvpdpbssds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpbssds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpbssds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpbssds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpbssds xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc2,0x10]
          dvpdpbssds xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpbssds xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpbssds xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpbssds xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc2,0x91,0xf0,0x07,0x00,0x00]
          dvpdpbssds xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpbssds xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc2,0x92,0x00,0xf8,0xff,0xff]
          dvpdpbssds xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpbsud xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x62,0xc1,0xd4]
          dvpdpbsud xmm2, xmm3, xmm4

// CHECK: dvpdpbsud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpbsud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpbsud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpbsud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpbsud xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc1,0x10]
          dvpdpbsud xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpbsud xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpbsud xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpbsud xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc1,0x91,0xf0,0x07,0x00,0x00]
          dvpdpbsud xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpbsud xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc1,0x92,0x00,0xf8,0xff,0xff]
          dvpdpbsud xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpbsuds xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x62,0xc2,0xd4]
          dvpdpbsuds xmm2, xmm3, xmm4

// CHECK: dvpdpbsuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpbsuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpbsuds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpbsuds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpbsuds xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc2,0x10]
          dvpdpbsuds xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpbsuds xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpbsuds xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpbsuds xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc2,0x91,0xf0,0x07,0x00,0x00]
          dvpdpbsuds xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpbsuds xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc2,0x92,0x00,0xf8,0xff,0xff]
          dvpdpbsuds xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpbusd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x61,0xc1,0xd4]
          dvpdpbusd xmm2, xmm3, xmm4

// CHECK: dvpdpbusd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpbusd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpbusd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpbusd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpbusd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc1,0x10]
          dvpdpbusd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpbusd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpbusd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpbusd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc1,0x91,0xf0,0x07,0x00,0x00]
          dvpdpbusd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpbusd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc1,0x92,0x00,0xf8,0xff,0xff]
          dvpdpbusd xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpbusds xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x61,0xc2,0xd4]
          dvpdpbusds xmm2, xmm3, xmm4

// CHECK: dvpdpbusds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpbusds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpbusds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpbusds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpbusds xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc2,0x10]
          dvpdpbusds xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpbusds xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpbusds xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpbusds xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc2,0x91,0xf0,0x07,0x00,0x00]
          dvpdpbusds xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpbusds xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc2,0x92,0x00,0xf8,0xff,0xff]
          dvpdpbusds xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpbuud xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x60,0xc1,0xd4]
          dvpdpbuud xmm2, xmm3, xmm4

// CHECK: dvpdpbuud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc1,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpbuud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpbuud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc1,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpbuud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpbuud xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc1,0x10]
          dvpdpbuud xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpbuud xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc1,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpbuud xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpbuud xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc1,0x91,0xf0,0x07,0x00,0x00]
          dvpdpbuud xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpbuud xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc1,0x92,0x00,0xf8,0xff,0xff]
          dvpdpbuud xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpbuuds xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x60,0xc2,0xd4]
          dvpdpbuuds xmm2, xmm3, xmm4

// CHECK: dvpdpbuuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc2,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpbuuds xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpbuuds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc2,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpbuuds xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpbuuds xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc2,0x10]
          dvpdpbuuds xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpbuuds xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc2,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpbuuds xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpbuuds xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc2,0x91,0xf0,0x07,0x00,0x00]
          dvpdpbuuds xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpbuuds xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc2,0x92,0x00,0xf8,0xff,0xff]
          dvpdpbuuds xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpndpbssd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xde,0xd4]
          dvpndpbssd xmm2, xmm3, xmm4

// CHECK: dvpndpbssd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xde,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpndpbssd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpndpbssd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xde,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpndpbssd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpndpbssd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xde,0x10]
          dvpndpbssd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpndpbssd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xde,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpndpbssd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpndpbssd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xde,0x91,0xf0,0x07,0x00,0x00]
          dvpndpbssd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpndpbssd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xde,0x92,0x00,0xf8,0xff,0xff]
          dvpndpbssd xmm2, xmm3, xmmword ptr [edx - 2048]

