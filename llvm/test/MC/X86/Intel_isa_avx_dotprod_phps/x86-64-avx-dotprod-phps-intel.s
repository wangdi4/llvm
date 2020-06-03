// REQUIRES: intel_feature_isa_avx_dotprod_phps
// RUN: llvm-mc -triple x86_64-unknown-unknown -mattr=+avxdotprodphps -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vdpphps ymm12, ymm13, ymm14
// CHECK: encoding: [0xc4,0x42,0x14,0x52,0xe6]
     vdpphps ymm12, ymm13, ymm14

// CHECK: vdpphps xmm12, xmm13, xmm14
// CHECK: encoding: [0xc4,0x42,0x10,0x52,0xe6]
     vdpphps xmm12, xmm13, xmm14

// CHECK: vdpphps ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x14,0x52,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vdpphps ymm12, ymm13, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdpphps ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x14,0x52,0xa4,0x80,0x23,0x01,0x00,0x00]
     vdpphps ymm12, ymm13, ymmword ptr [r8 + 4*rax + 291]

// CHECK: vdpphps ymm12, ymm13, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x14,0x52,0x25,0x00,0x00,0x00,0x00]
     vdpphps ymm12, ymm13, ymmword ptr [rip]

// CHECK: vdpphps ymm12, ymm13, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0x62,0x14,0x52,0x24,0x6d,0x00,0xfc,0xff,0xff]
     vdpphps ymm12, ymm13, ymmword ptr [2*rbp - 1024]

// CHECK: vdpphps xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0x22,0x10,0x52,0xa4,0xf5,0x00,0x00,0x00,0x10]
     vdpphps xmm12, xmm13, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK: vdpphps xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0x42,0x10,0x52,0xa4,0x80,0x23,0x01,0x00,0x00]
     vdpphps xmm12, xmm13, xmmword ptr [r8 + 4*rax + 291]

// CHECK: vdpphps xmm12, xmm13, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0x62,0x10,0x52,0x25,0x00,0x00,0x00,0x00]
     vdpphps xmm12, xmm13, xmmword ptr [rip]

// CHECK: vdpphps xmm12, xmm13, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0x62,0x10,0x52,0x24,0x6d,0x00,0xfe,0xff,0xff]
     vdpphps xmm12, xmm13, xmmword ptr [2*rbp - 512]
