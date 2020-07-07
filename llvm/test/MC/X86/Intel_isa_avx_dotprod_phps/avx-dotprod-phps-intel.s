// REQUIRES: intel_feature_isa_avx_dotprod_phps
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avxdotprodphps -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vdpphps ymm2, ymm3, ymm4
// CHECK: encoding: [0xc4,0xe2,0x64,0x52,0xd4]
     vdpphps ymm2, ymm3, ymm4

// CHECK: vdpphps xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x60,0x52,0xd4]
     vdpphps xmm2, xmm3, xmm4

// CHECK: vdpphps ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x64,0x52,0x94,0xf4,0x00,0x00,0x00,0x10]
     vdpphps ymm2, ymm3, ymmword ptr [esp + 8*esi + 268435456]

// CHECK: vdpphps ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x64,0x52,0x94,0x87,0x23,0x01,0x00,0x00]
     vdpphps ymm2, ymm3, ymmword ptr [edi + 4*eax + 291]

// CHECK: vdpphps ymm2, ymm3, ymmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x64,0x52,0x10]
     vdpphps ymm2, ymm3, ymmword ptr [eax]

// CHECK: vdpphps ymm2, ymm3, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x64,0x52,0x14,0x6d,0x00,0xfc,0xff,0xff]
     vdpphps ymm2, ymm3, ymmword ptr [2*ebp - 1024]

// CHECK: vdpphps xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x60,0x52,0x94,0xf4,0x00,0x00,0x00,0x10]
     vdpphps xmm2, xmm3, xmmword ptr [esp + 8*esi + 268435456]

// CHECK: vdpphps xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x60,0x52,0x94,0x87,0x23,0x01,0x00,0x00]
     vdpphps xmm2, xmm3, xmmword ptr [edi + 4*eax + 291]

// CHECK: vdpphps xmm2, xmm3, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x60,0x52,0x10]
     vdpphps xmm2, xmm3, xmmword ptr [eax]

// CHECK: vdpphps xmm2, xmm3, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x60,0x52,0x14,0x6d,0x00,0xfe,0xff,0xff]
     vdpphps xmm2, xmm3, xmmword ptr [2*ebp - 512]
