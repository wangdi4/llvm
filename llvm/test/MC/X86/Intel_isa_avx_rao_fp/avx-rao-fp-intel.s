// REQUIRES: intel_feature_isa_avx_rao_fp
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vaaddpbf16 ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7d,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddpbf16 ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      vaaddpbf16 ymmword ptr [edi + 4*eax + 291], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7d,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddpbf16 ymmword ptr [edi + 4*eax + 291], ymm2

// CHECK:      vaaddpbf16 ymmword ptr [eax], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7d,0x94,0x10]
               vaaddpbf16 ymmword ptr [eax], ymm2

// CHECK:      vaaddpbf16 ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7d,0x94,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vaaddpbf16 ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      vaaddpbf16 ymmword ptr [ecx + 4064], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7d,0x94,0x91,0xe0,0x0f,0x00,0x00]
               vaaddpbf16 ymmword ptr [ecx + 4064], ymm2

// CHECK:      vaaddpbf16 ymmword ptr [edx - 4096], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7d,0x94,0x92,0x00,0xf0,0xff,0xff]
               vaaddpbf16 ymmword ptr [edx - 4096], ymm2

// CHECK:      vaaddpbf16 xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddpbf16 xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vaaddpbf16 xmmword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddpbf16 xmmword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vaaddpbf16 xmmword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0x94,0x10]
               vaaddpbf16 xmmword ptr [eax], xmm2

// CHECK:      vaaddpbf16 xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0x94,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vaaddpbf16 xmmword ptr [2*ebp - 512], xmm2

// CHECK:      vaaddpbf16 xmmword ptr [ecx + 2032], xmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0x94,0x91,0xf0,0x07,0x00,0x00]
               vaaddpbf16 xmmword ptr [ecx + 2032], xmm2

// CHECK:      vaaddpbf16 xmmword ptr [edx - 2048], xmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0x94,0x92,0x00,0xf8,0xff,0xff]
               vaaddpbf16 xmmword ptr [edx - 2048], xmm2

// CHECK:      vaaddpd ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfd,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddpd ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      vaaddpd ymmword ptr [edi + 4*eax + 291], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfd,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddpd ymmword ptr [edi + 4*eax + 291], ymm2

// CHECK:      vaaddpd ymmword ptr [eax], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfd,0x84,0x10]
               vaaddpd ymmword ptr [eax], ymm2

// CHECK:      vaaddpd ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfd,0x84,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vaaddpd ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      vaaddpd ymmword ptr [ecx + 4064], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfd,0x84,0x91,0xe0,0x0f,0x00,0x00]
               vaaddpd ymmword ptr [ecx + 4064], ymm2

// CHECK:      vaaddpd ymmword ptr [edx - 4096], ymm2
// CHECK: encoding: [0xc4,0xe2,0xfd,0x84,0x92,0x00,0xf0,0xff,0xff]
               vaaddpd ymmword ptr [edx - 4096], ymm2

// CHECK:      vaaddpd xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf9,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddpd xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vaaddpd xmmword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf9,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddpd xmmword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vaaddpd xmmword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf9,0x84,0x10]
               vaaddpd xmmword ptr [eax], xmm2

// CHECK:      vaaddpd xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf9,0x84,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vaaddpd xmmword ptr [2*ebp - 512], xmm2

// CHECK:      vaaddpd xmmword ptr [ecx + 2032], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf9,0x84,0x91,0xf0,0x07,0x00,0x00]
               vaaddpd xmmword ptr [ecx + 2032], xmm2

// CHECK:      vaaddpd xmmword ptr [edx - 2048], xmm2
// CHECK: encoding: [0xc4,0xe2,0xf9,0x84,0x92,0x00,0xf8,0xff,0xff]
               vaaddpd xmmword ptr [edx - 2048], xmm2

// CHECK:      vaaddph ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddph ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      vaaddph ymmword ptr [edi + 4*eax + 291], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddph ymmword ptr [edi + 4*eax + 291], ymm2

// CHECK:      vaaddph ymmword ptr [eax], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0x94,0x10]
               vaaddph ymmword ptr [eax], ymm2

// CHECK:      vaaddph ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0x94,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vaaddph ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      vaaddph ymmword ptr [ecx + 4064], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0x94,0x91,0xe0,0x0f,0x00,0x00]
               vaaddph ymmword ptr [ecx + 4064], ymm2

// CHECK:      vaaddph ymmword ptr [edx - 4096], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0x94,0x92,0x00,0xf0,0xff,0xff]
               vaaddph ymmword ptr [edx - 4096], ymm2

// CHECK:      vaaddph xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddph xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vaaddph xmmword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddph xmmword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vaaddph xmmword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x94,0x10]
               vaaddph xmmword ptr [eax], xmm2

// CHECK:      vaaddph xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x94,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vaaddph xmmword ptr [2*ebp - 512], xmm2

// CHECK:      vaaddph xmmword ptr [ecx + 2032], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x94,0x91,0xf0,0x07,0x00,0x00]
               vaaddph xmmword ptr [ecx + 2032], xmm2

// CHECK:      vaaddph xmmword ptr [edx - 2048], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x94,0x92,0x00,0xf8,0xff,0xff]
               vaaddph xmmword ptr [edx - 2048], xmm2

// CHECK:      vaaddps ymmword ptr [esp + 8*esi + 268435456], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddps ymmword ptr [esp + 8*esi + 268435456], ymm2

// CHECK:      vaaddps ymmword ptr [edi + 4*eax + 291], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddps ymmword ptr [edi + 4*eax + 291], ymm2

// CHECK:      vaaddps ymmword ptr [eax], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0x84,0x10]
               vaaddps ymmword ptr [eax], ymm2

// CHECK:      vaaddps ymmword ptr [2*ebp - 1024], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0x84,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vaaddps ymmword ptr [2*ebp - 1024], ymm2

// CHECK:      vaaddps ymmword ptr [ecx + 4064], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0x84,0x91,0xe0,0x0f,0x00,0x00]
               vaaddps ymmword ptr [ecx + 4064], ymm2

// CHECK:      vaaddps ymmword ptr [edx - 4096], ymm2
// CHECK: encoding: [0xc4,0xe2,0x7c,0x84,0x92,0x00,0xf0,0xff,0xff]
               vaaddps ymmword ptr [edx - 4096], ymm2

// CHECK:      vaaddps xmmword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddps xmmword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vaaddps xmmword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddps xmmword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vaaddps xmmword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x84,0x10]
               vaaddps xmmword ptr [eax], xmm2

// CHECK:      vaaddps xmmword ptr [2*ebp - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x84,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vaaddps xmmword ptr [2*ebp - 512], xmm2

// CHECK:      vaaddps xmmword ptr [ecx + 2032], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x84,0x91,0xf0,0x07,0x00,0x00]
               vaaddps xmmword ptr [ecx + 2032], xmm2

// CHECK:      vaaddps xmmword ptr [edx - 2048], xmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x84,0x92,0x00,0xf8,0xff,0xff]
               vaaddps xmmword ptr [edx - 2048], xmm2

// CHECK:      vaaddsbf16 word ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddsbf16 word ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vaaddsbf16 word ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddsbf16 word ptr [edi + 4*eax + 291], xmm2

// CHECK:      vaaddsbf16 word ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0x94,0x10]
               vaaddsbf16 word ptr [eax], xmm2

// CHECK:      vaaddsbf16 word ptr [2*ebp - 64], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0x94,0x14,0x6d,0xc0,0xff,0xff,0xff]
               vaaddsbf16 word ptr [2*ebp - 64], xmm2

// CHECK:      vaaddsbf16 word ptr [ecx + 254], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0x94,0x91,0xfe,0x00,0x00,0x00]
               vaaddsbf16 word ptr [ecx + 254], xmm2

// CHECK:      vaaddsbf16 word ptr [edx - 256], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0x94,0x92,0x00,0xff,0xff,0xff]
               vaaddsbf16 word ptr [edx - 256], xmm2

// CHECK:      vaaddsd qword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfb,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddsd qword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vaaddsd qword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfb,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddsd qword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vaaddsd qword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfb,0x84,0x10]
               vaaddsd qword ptr [eax], xmm2

// CHECK:      vaaddsd qword ptr [2*ebp - 256], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfb,0x84,0x14,0x6d,0x00,0xff,0xff,0xff]
               vaaddsd qword ptr [2*ebp - 256], xmm2

// CHECK:      vaaddsd qword ptr [ecx + 1016], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfb,0x84,0x91,0xf8,0x03,0x00,0x00]
               vaaddsd qword ptr [ecx + 1016], xmm2

// CHECK:      vaaddsd qword ptr [edx - 1024], xmm2
// CHECK: encoding: [0xc4,0xe2,0xfb,0x84,0x92,0x00,0xfc,0xff,0xff]
               vaaddsd qword ptr [edx - 1024], xmm2

// CHECK:      vaaddsh word ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x94,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddsh word ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vaaddsh word ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x94,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddsh word ptr [edi + 4*eax + 291], xmm2

// CHECK:      vaaddsh word ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x94,0x10]
               vaaddsh word ptr [eax], xmm2

// CHECK:      vaaddsh word ptr [2*ebp - 64], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x94,0x14,0x6d,0xc0,0xff,0xff,0xff]
               vaaddsh word ptr [2*ebp - 64], xmm2

// CHECK:      vaaddsh word ptr [ecx + 254], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x94,0x91,0xfe,0x00,0x00,0x00]
               vaaddsh word ptr [ecx + 254], xmm2

// CHECK:      vaaddsh word ptr [edx - 256], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x94,0x92,0x00,0xff,0xff,0xff]
               vaaddsh word ptr [edx - 256], xmm2

// CHECK:      vaaddss dword ptr [esp + 8*esi + 268435456], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x84,0x94,0xf4,0x00,0x00,0x00,0x10]
               vaaddss dword ptr [esp + 8*esi + 268435456], xmm2

// CHECK:      vaaddss dword ptr [edi + 4*eax + 291], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x84,0x94,0x87,0x23,0x01,0x00,0x00]
               vaaddss dword ptr [edi + 4*eax + 291], xmm2

// CHECK:      vaaddss dword ptr [eax], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x84,0x10]
               vaaddss dword ptr [eax], xmm2

// CHECK:      vaaddss dword ptr [2*ebp - 128], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x84,0x14,0x6d,0x80,0xff,0xff,0xff]
               vaaddss dword ptr [2*ebp - 128], xmm2

// CHECK:      vaaddss dword ptr [ecx + 508], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x84,0x91,0xfc,0x01,0x00,0x00]
               vaaddss dword ptr [ecx + 508], xmm2

// CHECK:      vaaddss dword ptr [edx - 512], xmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x84,0x92,0x00,0xfe,0xff,0xff]
               vaaddss dword ptr [edx - 512], xmm2

