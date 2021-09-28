// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: dvphaddlsdq xmm2, xmm3
// CHECK: encoding: [0xc4,0xe8,0x78,0xd0,0xd3]
          dvphaddlsdq xmm2, xmm3

// CHECK: dvphaddlsdq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x78,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvphaddlsdq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvphaddlsdq xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x78,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          dvphaddlsdq xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvphaddlsdq xmm2, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x78,0xd0,0x10]
          dvphaddlsdq xmm2, xmmword ptr [eax]

// CHECK: dvphaddlsdq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x78,0xd0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvphaddlsdq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: dvphaddlsdq xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x78,0xd0,0x91,0xf0,0x07,0x00,0x00]
          dvphaddlsdq xmm2, xmmword ptr [ecx + 2032]

// CHECK: dvphaddlsdq xmm2, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x78,0xd0,0x92,0x00,0xf8,0xff,0xff]
          dvphaddlsdq xmm2, xmmword ptr [edx - 2048]

// CHECK: dvphaddlsduq xmm2, xmm3
// CHECK: encoding: [0xc4,0xe8,0x79,0xd0,0xd3]
          dvphaddlsduq xmm2, xmm3

// CHECK: dvphaddlsduq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x79,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvphaddlsduq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvphaddlsduq xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x79,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          dvphaddlsduq xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvphaddlsduq xmm2, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x79,0xd0,0x10]
          dvphaddlsduq xmm2, xmmword ptr [eax]

// CHECK: dvphaddlsduq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x79,0xd0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvphaddlsduq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: dvphaddlsduq xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x79,0xd0,0x91,0xf0,0x07,0x00,0x00]
          dvphaddlsduq xmm2, xmmword ptr [ecx + 2032]

// CHECK: dvphaddlsduq xmm2, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x79,0xd0,0x92,0x00,0xf8,0xff,0xff]
          dvphaddlsduq xmm2, xmmword ptr [edx - 2048]

// CHECK: dvphaddlswq xmm2, xmm3
// CHECK: encoding: [0xc4,0xe8,0xf8,0xd0,0xd3]
          dvphaddlswq xmm2, xmm3

// CHECK: dvphaddlswq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xf8,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvphaddlswq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvphaddlswq xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xf8,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          dvphaddlswq xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvphaddlswq xmm2, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xf8,0xd0,0x10]
          dvphaddlswq xmm2, xmmword ptr [eax]

// CHECK: dvphaddlswq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xf8,0xd0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvphaddlswq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: dvphaddlswq xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xf8,0xd0,0x91,0xf0,0x07,0x00,0x00]
          dvphaddlswq xmm2, xmmword ptr [ecx + 2032]

// CHECK: dvphaddlswq xmm2, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xf8,0xd0,0x92,0x00,0xf8,0xff,0xff]
          dvphaddlswq xmm2, xmmword ptr [edx - 2048]

// CHECK: dvphaddlswuq xmm2, xmm3
// CHECK: encoding: [0xc4,0xe8,0xf9,0xd0,0xd3]
          dvphaddlswuq xmm2, xmm3

// CHECK: dvphaddlswuq xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xf9,0xd0,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvphaddlswuq xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvphaddlswuq xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xf9,0xd0,0x94,0x87,0x23,0x01,0x00,0x00]
          dvphaddlswuq xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvphaddlswuq xmm2, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xf9,0xd0,0x10]
          dvphaddlswuq xmm2, xmmword ptr [eax]

// CHECK: dvphaddlswuq xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xf9,0xd0,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvphaddlswuq xmm2, xmmword ptr [2*ebp - 512]

// CHECK: dvphaddlswuq xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xf9,0xd0,0x91,0xf0,0x07,0x00,0x00]
          dvphaddlswuq xmm2, xmmword ptr [ecx + 2032]

// CHECK: dvphaddlswuq xmm2, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xf9,0xd0,0x92,0x00,0xf8,0xff,0xff]
          dvphaddlswuq xmm2, xmmword ptr [edx - 2048]
