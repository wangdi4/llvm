// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: dvpcr2bfrsdimm xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa5,0xd4,0x7b]
          dvpcr2bfrsdimm xmm2, xmm3, xmm4, 123

// CHECK: dvpcr2bfrsdimm xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa5,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpcr2bfrsdimm xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpcr2bfrsdimm xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa5,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpcr2bfrsdimm xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpcr2bfrsdimm xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa5,0x10,0x7b]
          dvpcr2bfrsdimm xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpcr2bfrsdimm xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa5,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpcr2bfrsdimm xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpcr2bfrsdimm xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa5,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpcr2bfrsdimm xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpcr2bfrsdimm xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa5,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpcr2bfrsdimm xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpcr2bfrsdre xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa5,0xd4,0x7b]
          dvpcr2bfrsdre xmm2, xmm3, xmm4, 123

// CHECK: dvpcr2bfrsdre xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa5,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpcr2bfrsdre xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpcr2bfrsdre xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa5,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpcr2bfrsdre xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpcr2bfrsdre xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa5,0x10,0x7b]
          dvpcr2bfrsdre xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpcr2bfrsdre xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa5,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpcr2bfrsdre xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpcr2bfrsdre xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa5,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpcr2bfrsdre xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpcr2bfrsdre xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x63,0xa5,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpcr2bfrsdre xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpdpwdssq xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa6,0xd4,0x7b]
          dvpdpwdssq xmm2, xmm3, xmm4, 123

// CHECK: dvpdpwdssq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa6,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpdpwdssq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpdpwdssq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa6,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpdpwdssq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpdpwdssq xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa6,0x10,0x7b]
          dvpdpwdssq xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpdpwdssq xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa6,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpdpwdssq xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpdpwdssq xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa6,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpdpwdssq xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpdpwdssq xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa6,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpdpwdssq xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpdpwduuq xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa6,0xd4,0x7b]
          dvpdpwduuq xmm2, xmm3, xmm4, 123

// CHECK: dvpdpwduuq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa6,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpdpwduuq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpdpwduuq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa6,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpdpwduuq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpdpwduuq xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa6,0x10,0x7b]
          dvpdpwduuq xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpdpwduuq xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa6,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpdpwduuq xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpdpwduuq xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa6,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpdpwduuq xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpdpwduuq xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa6,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpdpwduuq xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpmuluwdq xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa4,0xd4,0x7b]
          dvpmuluwdq xmm2, xmm3, xmm4, 123

// CHECK: dvpmuluwdq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa4,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpmuluwdq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpmuluwdq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa4,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpmuluwdq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpmuluwdq xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa4,0x10,0x7b]
          dvpmuluwdq xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpmuluwdq xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa4,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpmuluwdq xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpmuluwdq xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa4,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpmuluwdq xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpmuluwdq xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa4,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpmuluwdq xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpmulwdq xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa4,0xd4,0x7b]
          dvpmulwdq xmm2, xmm3, xmm4, 123

// CHECK: dvpmulwdq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa4,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpmulwdq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpmulwdq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa4,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpmulwdq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpmulwdq xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa4,0x10,0x7b]
          dvpmulwdq xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpmulwdq xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa4,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpmulwdq xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpmulwdq xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa4,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpmulwdq xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpmulwdq xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xe2,0xa4,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpmulwdq xmm2, xmm3, xmmword ptr [edx - 2048], 123

