// REQUIRES: intel_feature_isa_avx512_movzxc
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vmovd   xmm21, xmm22
// CHECK: encoding: [0x62,0xa1,0x7e,0x08,0x7e,0xee]
          vmovd   xmm21, xmm22

// CHECK: vmovd   xmm21, xmm22
// CHECK: encoding: [0x62,0xa1,0x7d,0x08,0xd6,0xee]
          vmovd.s   xmm21, xmm22

// CHECK: vmovw   xmm21, xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x6e,0xee]
          vmovw   xmm21, xmm22

// CHECK: vmovw   xmm21, xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x7e,0xee]
          vmovw.s   xmm21, xmm22
