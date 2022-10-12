// REQUIRES: intel_feature_isa_amx_fp8_future
// RUN: llvm-mc -triple x86_64 --show-encoding %s | FileCheck %s

// CHECK: ttdpbf8ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x58,0xfc,0xf5]
          ttdpbf8ps %tmm4, %tmm5, %tmm6

// CHECK: ttdpbf8ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x70,0xfc,0xda]
          ttdpbf8ps %tmm1, %tmm2, %tmm3

// CHECK: ttdpbhf8ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x5b,0xfc,0xf5]
          ttdpbhf8ps %tmm4, %tmm5, %tmm6

// CHECK: ttdpbhf8ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x73,0xfc,0xda]
          ttdpbhf8ps %tmm1, %tmm2, %tmm3

// CHECK: ttdphbf8ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x5a,0xfc,0xf5]
          ttdphbf8ps %tmm4, %tmm5, %tmm6

// CHECK: ttdphbf8ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x72,0xfc,0xda]
          ttdphbf8ps %tmm1, %tmm2, %tmm3

// CHECK: ttdphf8ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x59,0xfc,0xf5]
          ttdphf8ps %tmm4, %tmm5, %tmm6

// CHECK: ttdphf8ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x71,0xfc,0xda]
          ttdphf8ps %tmm1, %tmm2, %tmm3

