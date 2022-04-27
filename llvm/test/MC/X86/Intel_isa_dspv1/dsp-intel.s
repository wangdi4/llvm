// REQUIRES: intel_feature_isa_dspv1
// RUN: llvm-mc -triple i386-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0xd4,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmm4, 123

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x10,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmmword ptr [eax], 123

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x91,0xf0,0x07,0x00,0x00,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK:      dvpcr2bfrsw xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0xe3,0xa3,0x92,0x00,0xf8,0xff,0xff,0x7b]
               dvpcr2bfrsw xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK:      dvplutsincosw xmm2, xmm3, 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0xd3,0x7b]
               dvplutsincosw xmm2, xmm3, 123

// CHECK:      dvplutsincosw xmm2, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
               dvplutsincosw xmm2, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK:      dvplutsincosw xmm2, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
               dvplutsincosw xmm2, xmmword ptr [edi + 4*eax + 291], 123

// CHECK:      dvplutsincosw xmm2, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x10,0x7b]
               dvplutsincosw xmm2, xmmword ptr [eax], 123

// CHECK:      dvplutsincosw xmm2, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               dvplutsincosw xmm2, xmmword ptr [2*ebp - 512], 123

// CHECK:      dvplutsincosw xmm2, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x91,0xf0,0x07,0x00,0x00,0x7b]
               dvplutsincosw xmm2, xmmword ptr [ecx + 2032], 123

// CHECK:      dvplutsincosw xmm2, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x78,0xa3,0x92,0x00,0xf8,0xff,0xff,0x7b]
               dvplutsincosw xmm2, xmmword ptr [edx - 2048], 123

// CHECK: dvpcaddrotsrad xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa3,0xd4,0x7b]
          dvpcaddrotsrad xmm2, xmm3, xmm4, 123

// CHECK: dvpcaddrotsrad xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa3,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpcaddrotsrad xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpcaddrotsrad xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa3,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpcaddrotsrad xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpcaddrotsrad xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa3,0x10,0x7b]
          dvpcaddrotsrad xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpcaddrotsrad xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa3,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpcaddrotsrad xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpcaddrotsrad xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa3,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpcaddrotsrad xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpcaddrotsrad xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xa3,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpcaddrotsrad xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpcaddrotsraw xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa3,0xd4,0x7b]
          dvpcaddrotsraw xmm2, xmm3, xmm4, 123

// CHECK: dvpcaddrotsraw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa3,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpcaddrotsraw xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpcaddrotsraw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa3,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpcaddrotsraw xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpcaddrotsraw xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa3,0x10,0x7b]
          dvpcaddrotsraw xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpcaddrotsraw xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa3,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpcaddrotsraw xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpcaddrotsraw xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa3,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpcaddrotsraw xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpcaddrotsraw xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa3,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpcaddrotsraw xmm2, xmm3, xmmword ptr [edx - 2048], 123

// CHECK: dvpdpint4ssd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x63,0xc3,0xd4]
          dvpdpint4ssd xmm2, xmm3, xmm4

// CHECK: dvpdpint4ssd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc3,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpint4ssd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpint4ssd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc3,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpint4ssd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpint4ssd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc3,0x10]
          dvpdpint4ssd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpint4ssd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc3,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpint4ssd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpint4ssd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc3,0x91,0xf0,0x07,0x00,0x00]
          dvpdpint4ssd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpint4ssd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x63,0xc3,0x92,0x00,0xf8,0xff,0xff]
          dvpdpint4ssd xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpint4sud xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x62,0xc3,0xd4]
          dvpdpint4sud xmm2, xmm3, xmm4

// CHECK: dvpdpint4sud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc3,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpint4sud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpint4sud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc3,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpint4sud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpint4sud xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc3,0x10]
          dvpdpint4sud xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpint4sud xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc3,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpint4sud xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpint4sud xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc3,0x91,0xf0,0x07,0x00,0x00]
          dvpdpint4sud xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpint4sud xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x62,0xc3,0x92,0x00,0xf8,0xff,0xff]
          dvpdpint4sud xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpint4usd xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x61,0xc3,0xd4]
          dvpdpint4usd xmm2, xmm3, xmm4

// CHECK: dvpdpint4usd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc3,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpint4usd xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpint4usd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc3,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpint4usd xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpint4usd xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc3,0x10]
          dvpdpint4usd xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpint4usd xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc3,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpint4usd xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpint4usd xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc3,0x91,0xf0,0x07,0x00,0x00]
          dvpdpint4usd xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpint4usd xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x61,0xc3,0x92,0x00,0xf8,0xff,0xff]
          dvpdpint4usd xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpdpint4uud xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0x60,0xc3,0xd4]
          dvpdpint4uud xmm2, xmm3, xmm4

// CHECK: dvpdpint4uud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc3,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpdpint4uud xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpdpint4uud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc3,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpdpint4uud xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpdpint4uud xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc3,0x10]
          dvpdpint4uud xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpdpint4uud xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc3,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpdpint4uud xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpdpint4uud xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc3,0x91,0xf0,0x07,0x00,0x00]
          dvpdpint4uud xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpdpint4uud xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x60,0xc3,0x92,0x00,0xf8,0xff,0xff]
          dvpdpint4uud xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpmasklddqu xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0x62,0xbc,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpmasklddqu xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpmasklddqu xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0x62,0xbc,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpmasklddqu xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpmasklddqu xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0x62,0xbc,0x10]
          dvpmasklddqu xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpmasklddqu xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0x62,0xbc,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpmasklddqu xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpmasklddqu xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0x62,0xbc,0x91,0xf0,0x07,0x00,0x00]
          dvpmasklddqu xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpmasklddqu xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0x62,0xbc,0x92,0x00,0xf8,0xff,0xff]
          dvpmasklddqu xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpmaskstdqu xmmword ptr [esp + 8*esi + 268435456], xmm2, xmm3
// CHECK: encoding: [0xc4,0xe8,0x6a,0xbe,0x9c,0xf4,0x00,0x00,0x00,0x10]
          dvpmaskstdqu xmmword ptr [esp + 8*esi + 268435456], xmm2, xmm3

// CHECK: dvpmaskstdqu xmmword ptr [edi + 4*eax + 291], xmm2, xmm3
// CHECK: encoding: [0xc4,0xe8,0x6a,0xbe,0x9c,0x87,0x23,0x01,0x00,0x00]
          dvpmaskstdqu xmmword ptr [edi + 4*eax + 291], xmm2, xmm3

// CHECK: dvpmaskstdqu xmmword ptr [eax], xmm2, xmm3
// CHECK: encoding: [0xc4,0xe8,0x6a,0xbe,0x18]
          dvpmaskstdqu xmmword ptr [eax], xmm2, xmm3

// CHECK: dvpmaskstdqu xmmword ptr [2*ebp - 512], xmm2, xmm3
// CHECK: encoding: [0xc4,0xe8,0x6a,0xbe,0x1c,0x6d,0x00,0xfe,0xff,0xff]
          dvpmaskstdqu xmmword ptr [2*ebp - 512], xmm2, xmm3

// CHECK: dvpmaskstdqu xmmword ptr [ecx + 2032], xmm2, xmm3
// CHECK: encoding: [0xc4,0xe8,0x6a,0xbe,0x99,0xf0,0x07,0x00,0x00]
          dvpmaskstdqu xmmword ptr [ecx + 2032], xmm2, xmm3

// CHECK: dvpmaskstdqu xmmword ptr [edx - 2048], xmm2, xmm3
// CHECK: encoding: [0xc4,0xe8,0x6a,0xbe,0x9a,0x00,0xf8,0xff,0xff]
          dvpmaskstdqu xmmword ptr [edx - 2048], xmm2, xmm3

// CHECK: dvpmuluwr xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc8,0xd4]
          dvpmuluwr xmm2, xmm3, xmm4

// CHECK: dvpmuluwr xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc8,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpmuluwr xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpmuluwr xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc8,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpmuluwr xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpmuluwr xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc8,0x10]
          dvpmuluwr xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpmuluwr xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc8,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpmuluwr xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpmuluwr xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc8,0x91,0xf0,0x07,0x00,0x00]
          dvpmuluwr xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpmuluwr xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xc8,0x92,0x00,0xf8,0xff,0xff]
          dvpmuluwr xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpmulwfrs xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc9,0xd4]
          dvpmulwfrs xmm2, xmm3, xmm4

// CHECK: dvpmulwfrs xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc9,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpmulwfrs xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpmulwfrs xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc9,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpmulwfrs xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpmulwfrs xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc9,0x10]
          dvpmulwfrs xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpmulwfrs xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc9,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpmulwfrs xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpmulwfrs xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc9,0x91,0xf0,0x07,0x00,0x00]
          dvpmulwfrs xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpmulwfrs xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe3,0xc9,0x92,0x00,0xf8,0xff,0xff]
          dvpmulwfrs xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpmulwr xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc8,0xd4]
          dvpmulwr xmm2, xmm3, xmm4

// CHECK: dvpmulwr xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc8,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpmulwr xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpmulwr xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc8,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpmulwr xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpmulwr xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc8,0x10]
          dvpmulwr xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpmulwr xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc8,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpmulwr xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpmulwr xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc8,0x91,0xf0,0x07,0x00,0x00]
          dvpmulwr xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpmulwr xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe1,0xc8,0x92,0x00,0xf8,0xff,0xff]
          dvpmulwr xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvpmulws xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe8,0xe0,0xcd,0xd4]
          dvpmulws xmm2, xmm3, xmm4

// CHECK: dvpmulws xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xcd,0x94,0xf4,0x00,0x00,0x00,0x10]
          dvpmulws xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: dvpmulws xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xcd,0x94,0x87,0x23,0x01,0x00,0x00]
          dvpmulws xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: dvpmulws xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xcd,0x10]
          dvpmulws xmm2, xmm3, xmmword ptr [eax]

// CHECK: dvpmulws xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xcd,0x14,0x6d,0x00,0xfe,0xff,0xff]
          dvpmulws xmm2, xmm3, xmmword ptr [2*ebp - 512]

// CHECK: dvpmulws xmm2, xmm3, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xcd,0x91,0xf0,0x07,0x00,0x00]
          dvpmulws xmm2, xmm3, xmmword ptr [ecx + 2032]

// CHECK: dvpmulws xmm2, xmm3, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe8,0xe0,0xcd,0x92,0x00,0xf8,0xff,0xff]
          dvpmulws xmm2, xmm3, xmmword ptr [edx - 2048]

// CHECK: dvptestmxcsrflgs
// CHECK: encoding: [0xc5,0xf9,0x77]
          dvptestmxcsrflgs

// CHECK: dvpunpckdq xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa7,0xd4,0x7b]
          dvpunpckdq xmm2, xmm3, xmm4, 123

// CHECK: dvpunpckdq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa7,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          dvpunpckdq xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456], 123

// CHECK: dvpunpckdq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa7,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          dvpunpckdq xmm2, xmm3, xmmword ptr [edi + 4*eax + 291], 123

// CHECK: dvpunpckdq xmm2, xmm3, xmmword ptr [eax], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa7,0x10,0x7b]
          dvpunpckdq xmm2, xmm3, xmmword ptr [eax], 123

// CHECK: dvpunpckdq xmm2, xmm3, xmmword ptr [2*ebp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa7,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          dvpunpckdq xmm2, xmm3, xmmword ptr [2*ebp - 512], 123

// CHECK: dvpunpckdq xmm2, xmm3, xmmword ptr [ecx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa7,0x91,0xf0,0x07,0x00,0x00,0x7b]
          dvpunpckdq xmm2, xmm3, xmmword ptr [ecx + 2032], 123

// CHECK: dvpunpckdq xmm2, xmm3, xmmword ptr [edx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x62,0xa7,0x92,0x00,0xf8,0xff,0xff,0x7b]
          dvpunpckdq xmm2, xmm3, xmmword ptr [edx - 2048], 123
