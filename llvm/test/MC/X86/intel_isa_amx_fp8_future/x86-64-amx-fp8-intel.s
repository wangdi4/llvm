// REQUIRES: intel_feature_isa_amx_fp8_future
// RUN: llvm-mc -triple x86_64 -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: ttdpbf8ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x58,0xfc,0xf5]
          ttdpbf8ps tmm6, tmm5, tmm4

// CHECK: ttdpbf8ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x70,0xfc,0xda]
          ttdpbf8ps tmm3, tmm2, tmm1

// CHECK: ttdpbhf8ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x5b,0xfc,0xf5]
          ttdpbhf8ps tmm6, tmm5, tmm4

// CHECK: ttdpbhf8ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x73,0xfc,0xda]
          ttdpbhf8ps tmm3, tmm2, tmm1

// CHECK: ttdphbf8ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x5a,0xfc,0xf5]
          ttdphbf8ps tmm6, tmm5, tmm4

// CHECK: ttdphbf8ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x72,0xfc,0xda]
          ttdphbf8ps tmm3, tmm2, tmm1

// CHECK: ttdphf8ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe5,0x59,0xfc,0xf5]
          ttdphf8ps tmm6, tmm5, tmm4

// CHECK: ttdphf8ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe5,0x71,0xfc,0xda]
          ttdphf8ps tmm3, tmm2, tmm1

