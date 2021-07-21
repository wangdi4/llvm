// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: dvpccdpwqimm xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd7,0xd4]
          dvpccdpwqimm xmm2, xmm3, xmm4

// CHECK: dvpccdpwqimm xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd7,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpccdpwqimm xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpccdpwqimm xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd7,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpccdpwqimm xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpccdpwqimm xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd7,0x10]
          dvpccdpwqimm xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpccdpwqimm xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd7,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpccdpwqimm xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpccdpwqimm xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd7,0x91,0xf0,0x07,0x00,0x00]
          dvpccdpwqimm xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpccdpwqimm xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd7,0x92,0x00,0xf8,0xff,0xff]
          dvpccdpwqimm xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpccmulwrs xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0xe1,0xa5,0xd4,0x7b]
          dvpccmulwrs xmm2, xmm3, xmm4, 123

// CHECK: dvpccmulwrs xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xe1,0xa5,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpccmulwrs xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpccmulwrs xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xe1,0xa5,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpccmulwrs xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpccmulwrs xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xe1,0xa5,0x10,0x7b]
          dvpccmulwrs xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpccmulwrs xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xe1,0xa5,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpccmulwrs xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpccmulwrs xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xe1,0xa5,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpccmulwrs xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpccmulwrs xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xe1,0xa5,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpccmulwrs xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpcdpwqimm xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd6,0xd4]
          dvpcdpwqimm xmm2, xmm3, xmm4

// CHECK: dvpcdpwqimm xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd6,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpcdpwqimm xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpcdpwqimm xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd6,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpcdpwqimm xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpcdpwqimm xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd6,0x10]
          dvpcdpwqimm xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpcdpwqimm xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd6,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpcdpwqimm xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpcdpwqimm xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd6,0x91,0xf0,0x07,0x00,0x00]
          dvpcdpwqimm xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpcdpwqimm xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd6,0x92,0x00,0xf8,0xff,0xff]
          dvpcdpwqimm xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpcdpwqre xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd6,0xd4]
          dvpcdpwqre xmm2, xmm3, xmm4

// CHECK: dvpcdpwqre xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd6,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpcdpwqre xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpcdpwqre xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd6,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpcdpwqre xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpcdpwqre xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd6,0x10]
          dvpcdpwqre xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpcdpwqre xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd6,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpcdpwqre xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpcdpwqre xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd6,0x91,0xf0,0x07,0x00,0x00]
          dvpcdpwqre xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpcdpwqre xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd6,0x92,0x00,0xf8,0xff,0xff]
          dvpcdpwqre xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpcmulwrs xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0xe0,0xa5,0xd4,0x7b]
          dvpcmulwrs xmm2, xmm3, xmm4, 123

// CHECK: dvpcmulwrs xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xe0,0xa5,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpcmulwrs xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpcmulwrs xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xe0,0xa5,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpcmulwrs xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpcmulwrs xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xe0,0xa5,0x10,0x7b]
          dvpcmulwrs xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpcmulwrs xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xe0,0xa5,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpcmulwrs xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpcmulwrs xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xe0,0xa5,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpcmulwrs xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpcmulwrs xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xe0,0xa5,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpcmulwrs xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpdpwssq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc0,0xd4]
          dvpdpwssq xmm2, xmm3, xmm4

// CHECK: dvpdpwssq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc0,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpwssq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpwssq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc0,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpwssq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpwssq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc0,0x10]
          dvpdpwssq xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpwssq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpwssq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpwssq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc0,0x91,0xf0,0x07,0x00,0x00]
          dvpdpwssq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpwssq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc0,0x92,0x00,0xf8,0xff,0xff]
          dvpdpwssq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpwsuq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xc0,0xd4]
          dvpdpwsuq xmm2, xmm3, xmm4

// CHECK: dvpdpwsuq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xc0,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpwsuq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpwsuq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xc0,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpwsuq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpwsuq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xc0,0x10]
          dvpdpwsuq xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpwsuq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xc0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpwsuq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpwsuq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xc0,0x91,0xf0,0x07,0x00,0x00]
          dvpdpwsuq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpwsuq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xc0,0x92,0x00,0xf8,0xff,0xff]
          dvpdpwsuq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpwusq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc0,0xd4]
          dvpdpwusq xmm2, xmm3, xmm4

// CHECK: dvpdpwusq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc0,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpwusq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpwusq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc0,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpwusq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpwusq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc0,0x10]
          dvpdpwusq xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpwusq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpwusq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpwusq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc0,0x91,0xf0,0x07,0x00,0x00]
          dvpdpwusq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpwusq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc0,0x92,0x00,0xf8,0xff,0xff]
          dvpdpwusq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpwuuq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc0,0xd4]
          dvpdpwuuq xmm2, xmm3, xmm4

// CHECK: dvpdpwuuq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc0,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpwuuq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpwuuq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc0,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpwuuq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpwuuq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc0,0x10]
          dvpdpwuuq xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpwuuq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpwuuq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpwuuq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc0,0x91,0xf0,0x07,0x00,0x00]
          dvpdpwuuq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpwuuq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc0,0x92,0x00,0xf8,0xff,0xff]
          dvpdpwuuq xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpnccdpwqimm xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd5,0xd4]
          dvpnccdpwqimm xmm2, xmm3, xmm4

// CHECK: dvpnccdpwqimm xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd5,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpnccdpwqimm xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpnccdpwqimm xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd5,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpnccdpwqimm xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpnccdpwqimm xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd5,0x10]
          dvpnccdpwqimm xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpnccdpwqimm xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd5,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpnccdpwqimm xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpnccdpwqimm xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd5,0x91,0xf0,0x07,0x00,0x00]
          dvpnccdpwqimm xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpnccdpwqimm xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd5,0x92,0x00,0xf8,0xff,0xff]
          dvpnccdpwqimm xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpncdpwqimm xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd4,0xd4]
          dvpncdpwqimm xmm2, xmm3, xmm4

// CHECK: dvpncdpwqimm xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd4,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpncdpwqimm xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpncdpwqimm xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd4,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpncdpwqimm xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpncdpwqimm xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd4,0x10]
          dvpncdpwqimm xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpncdpwqimm xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd4,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpncdpwqimm xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpncdpwqimm xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd4,0x91,0xf0,0x07,0x00,0x00]
          dvpncdpwqimm xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpncdpwqimm xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe2,0xd4,0x92,0x00,0xf8,0xff,0xff]
          dvpncdpwqimm xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpncdpwqre xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd4,0xd4]
          dvpncdpwqre xmm2, xmm3, xmm4

// CHECK: dvpncdpwqre xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd4,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpncdpwqre xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpncdpwqre xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd4,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpncdpwqre xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpncdpwqre xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd4,0x10]
          dvpncdpwqre xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpncdpwqre xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd4,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpncdpwqre xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpncdpwqre xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd4,0x91,0xf0,0x07,0x00,0x00]
          dvpncdpwqre xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpncdpwqre xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xd4,0x92,0x00,0xf8,0xff,0xff]
          dvpncdpwqre xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpndpwssq xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xdd,0xd4]
          dvpndpwssq xmm2, xmm3, xmm4

// CHECK: dvpndpwssq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xdd,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpndpwssq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpndpwssq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xdd,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpndpwssq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpndpwssq xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xdd,0x10]
          dvpndpwssq xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpndpwssq xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xdd,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpndpwssq xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpndpwssq xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xdd,0x91,0xf0,0x07,0x00,0x00]
          dvpndpwssq xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpndpwssq xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xdd,0x92,0x00,0xf8,0xff,0xff]
          dvpndpwssq xmm2, xmm3, xmmword ptr [edx - 2048]

