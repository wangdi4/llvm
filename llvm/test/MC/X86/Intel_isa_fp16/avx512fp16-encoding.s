// REQUIRES: intel_feature_isa_fp16
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding < %s  | FileCheck %s

// CHECK: vaddph %xmm3, %xmm2, %xmm1
// CHECK: encoding: [0x62,0xf5,0x6c,0x08,0x58,0xcb]
          vaddph %xmm3, %xmm2, %xmm1

// CHECK: vaddph %xmm23, %xmm22, %xmm21
// CHECK: encoding: [0x62,0xa5,0x4c,0x00,0x58,0xef]
          vaddph %xmm23, %xmm22, %xmm21

// CHECK: vaddph %xmm3, %xmm2, %xmm1 {%k2}
// CHECK: encoding: [0x62,0xf5,0x6c,0x0a,0x58,0xcb]
          vaddph %xmm3, %xmm2, %xmm1 {%k2}

// CHECK: vaddph %xmm23, %xmm22, %xmm21 {%k2}
// CHECK: encoding: [0x62,0xa5,0x4c,0x02,0x58,0xef]
          vaddph %xmm23, %xmm22, %xmm21 {%k2}

// CHECK: vaddph  (%rcx), %xmm2, %xmm1
// CHECK: encoding: [0x62,0xf5,0x6c,0x08,0x58,0x09]
          vaddph  (%rcx), %xmm2, %xmm1

// CHECK: vaddph  -64(%rsp), %xmm2, %xmm1
// CHECK: encoding: [0x62,0xf5,0x6c,0x08,0x58,0x4c,0x24,0xfc]
          vaddph  -64(%rsp), %xmm2, %xmm1

// CHECK: vaddph  64(%rsp), %xmm2, %xmm1
// CHECK: encoding: [0x62,0xf5,0x6c,0x08,0x58,0x4c,0x24,0x04]
          vaddph  64(%rsp), %xmm2, %xmm1

// CHECK: vaddph  268435456(%rcx,%r14,8), %xmm2, %xmm1
// CHECK: encoding: [0x62,0xb5,0x6c,0x08,0x58,0x8c,0xf1,0x00,0x00,0x00,0x10]
          vaddph  268435456(%rcx,%r14,8), %xmm2, %xmm1

// CHECK: vaddph  -536870912(%rcx,%r14,8), %xmm2, %xmm1
// CHECK: encoding: [0x62,0xb5,0x6c,0x08,0x58,0x8c,0xf1,0x00,0x00,0x00,0xe0]
          vaddph  -536870912(%rcx,%r14,8), %xmm2, %xmm1

// CHECK: vaddph  -536870910(%rcx,%r14,8), %xmm2, %xmm1
// CHECK: encoding: [0x62,0xb5,0x6c,0x08,0x58,0x8c,0xf1,0x02,0x00,0x00,0xe0]
          vaddph  -536870910(%rcx,%r14,8), %xmm2, %xmm1

// CHECK: vaddph  (%rcx), %xmm22, %xmm21
// CHECK: encoding: [0x62,0xe5,0x4c,0x00,0x58,0x29]
          vaddph  (%rcx), %xmm22, %xmm21

// CHECK: vaddph  -64(%rsp), %xmm22, %xmm21
// CHECK: encoding: [0x62,0xe5,0x4c,0x00,0x58,0x6c,0x24,0xfc]
          vaddph  -64(%rsp), %xmm22, %xmm21

// CHECK: vaddph  64(%rsp), %xmm22, %xmm21
// CHECK: encoding: [0x62,0xe5,0x4c,0x00,0x58,0x6c,0x24,0x04]
          vaddph  64(%rsp), %xmm22, %xmm21

// CHECK: vaddph  268435456(%rcx,%r14,8), %xmm22, %xmm21
// CHECK: encoding: [0x62,0xa5,0x4c,0x00,0x58,0xac,0xf1,0x00,0x00,0x00,0x10]
          vaddph  268435456(%rcx,%r14,8), %xmm22, %xmm21

// CHECK: vaddph  -536870912(%rcx,%r14,8), %xmm22, %xmm21
// CHECK: encoding: [0x62,0xa5,0x4c,0x00,0x58,0xac,0xf1,0x00,0x00,0x00,0xe0]
          vaddph  -536870912(%rcx,%r14,8), %xmm22, %xmm21

// CHECK: vaddph  -536870910(%rcx,%r14,8), %xmm22, %xmm21
// CHECK: encoding: [0x62,0xa5,0x4c,0x00,0x58,0xac,0xf1,0x02,0x00,0x00,0xe0]
          vaddph  -536870910(%rcx,%r14,8), %xmm22, %xmm21

// CHECK: vaddph  (%rcx), %xmm2, %xmm1 {%k2}
// CHECK: encoding: [0x62,0xf5,0x6c,0x0a,0x58,0x09]
          vaddph  (%rcx), %xmm2, %xmm1 {%k2}

// CHECK: vaddph  -64(%rsp), %xmm2, %xmm1 {%k2}
// CHECK: encoding: [0x62,0xf5,0x6c,0x0a,0x58,0x4c,0x24,0xfc]
          vaddph  -64(%rsp), %xmm2, %xmm1 {%k2}

// CHECK: vaddph  64(%rsp), %xmm2, %xmm1 {%k2}
// CHECK: encoding: [0x62,0xf5,0x6c,0x0a,0x58,0x4c,0x24,0x04]
          vaddph  64(%rsp), %xmm2, %xmm1 {%k2}

// CHECK: vaddph  268435456(%rcx,%r14,8), %xmm2, %xmm1 {%k2}
// CHECK: encoding: [0x62,0xb5,0x6c,0x0a,0x58,0x8c,0xf1,0x00,0x00,0x00,0x10]
          vaddph  268435456(%rcx,%r14,8), %xmm2, %xmm1 {%k2}

// CHECK: vaddph  -536870912(%rcx,%r14,8), %xmm2, %xmm1 {%k2}
// CHECK: encoding: [0x62,0xb5,0x6c,0x0a,0x58,0x8c,0xf1,0x00,0x00,0x00,0xe0]
          vaddph  -536870912(%rcx,%r14,8), %xmm2, %xmm1 {%k2}

// CHECK: vaddph  -536870910(%rcx,%r14,8), %xmm2, %xmm1 {%k2}
// CHECK: encoding: [0x62,0xb5,0x6c,0x0a,0x58,0x8c,0xf1,0x02,0x00,0x00,0xe0]
          vaddph  -536870910(%rcx,%r14,8), %xmm2, %xmm1 {%k2}

// CHECK: vaddph  (%rcx), %xmm22, %xmm21 {%k2}
// CHECK: encoding: [0x62,0xe5,0x4c,0x02,0x58,0x29]
          vaddph  (%rcx), %xmm22, %xmm21 {%k2}

// CHECK: vaddph  -64(%rsp), %xmm22, %xmm21 {%k2}
// CHECK: encoding: [0x62,0xe5,0x4c,0x02,0x58,0x6c,0x24,0xfc]
          vaddph  -64(%rsp), %xmm22, %xmm21 {%k2}

// CHECK: vaddph  64(%rsp), %xmm22, %xmm21 {%k2}
// CHECK: encoding: [0x62,0xe5,0x4c,0x02,0x58,0x6c,0x24,0x04]
          vaddph  64(%rsp), %xmm22, %xmm21 {%k2}

// CHECK: vaddph  268435456(%rcx,%r14,8), %xmm22, %xmm21 {%k2}
// CHECK: encoding: [0x62,0xa5,0x4c,0x02,0x58,0xac,0xf1,0x00,0x00,0x00,0x10]
          vaddph  268435456(%rcx,%r14,8), %xmm22, %xmm21 {%k2}

// CHECK: vaddph  -536870912(%rcx,%r14,8), %xmm22, %xmm21 {%k2}
// CHECK: encoding: [0x62,0xa5,0x4c,0x02,0x58,0xac,0xf1,0x00,0x00,0x00,0xe0]
          vaddph  -536870912(%rcx,%r14,8), %xmm22, %xmm21 {%k2}

// CHECK: vaddph  -536870910(%rcx,%r14,8), %xmm22, %xmm21 {%k2}
// CHECK: encoding: [0x62,0xa5,0x4c,0x02,0x58,0xac,0xf1,0x02,0x00,0x00,0xe0]
          vaddph  -536870910(%rcx,%r14,8), %xmm22, %xmm21 {%k2}

// CHECK: vaddph %ymm3, %ymm2, %ymm1
// CHECK: encoding: [0x62,0xf5,0x6c,0x28,0x58,0xcb]
          vaddph %ymm3, %ymm2, %ymm1

// CHECK: vaddph %ymm23, %ymm22, %ymm21
// CHECK: encoding: [0x62,0xa5,0x4c,0x20,0x58,0xef]
          vaddph %ymm23, %ymm22, %ymm21

// CHECK: vaddph %ymm3, %ymm2, %ymm1 {%k2}
// CHECK: encoding: [0x62,0xf5,0x6c,0x2a,0x58,0xcb]
          vaddph %ymm3, %ymm2, %ymm1 {%k2}

// CHECK: vaddph %ymm23, %ymm22, %ymm21 {%k2}
// CHECK: encoding: [0x62,0xa5,0x4c,0x22,0x58,0xef]
          vaddph %ymm23, %ymm22, %ymm21 {%k2}

// CHECK: vaddph  (%rcx), %ymm2, %ymm1
// CHECK: encoding: [0x62,0xf5,0x6c,0x28,0x58,0x09]
          vaddph  (%rcx), %ymm2, %ymm1

// CHECK: vaddph  -128(%rsp), %ymm2, %ymm1
// CHECK: encoding: [0x62,0xf5,0x6c,0x28,0x58,0x4c,0x24,0xfc]
          vaddph  -128(%rsp), %ymm2, %ymm1

// CHECK: vaddph  128(%rsp), %ymm2, %ymm1
// CHECK: encoding: [0x62,0xf5,0x6c,0x28,0x58,0x4c,0x24,0x04]
          vaddph  128(%rsp), %ymm2, %ymm1

// CHECK: vaddph  268435456(%rcx,%r14,8), %ymm2, %ymm1
// CHECK: encoding: [0x62,0xb5,0x6c,0x28,0x58,0x8c,0xf1,0x00,0x00,0x00,0x10]
          vaddph  268435456(%rcx,%r14,8), %ymm2, %ymm1

// CHECK: vaddph  -536870912(%rcx,%r14,8), %ymm2, %ymm1
// CHECK: encoding: [0x62,0xb5,0x6c,0x28,0x58,0x8c,0xf1,0x00,0x00,0x00,0xe0]
          vaddph  -536870912(%rcx,%r14,8), %ymm2, %ymm1

// CHECK: vaddph  -536870910(%rcx,%r14,8), %ymm2, %ymm1
// CHECK: encoding: [0x62,0xb5,0x6c,0x28,0x58,0x8c,0xf1,0x02,0x00,0x00,0xe0]
          vaddph  -536870910(%rcx,%r14,8), %ymm2, %ymm1

// CHECK: vaddph  (%rcx), %ymm22, %ymm21
// CHECK: encoding: [0x62,0xe5,0x4c,0x20,0x58,0x29]
          vaddph  (%rcx), %ymm22, %ymm21

// CHECK: vaddph  -128(%rsp), %ymm22, %ymm21
// CHECK: encoding: [0x62,0xe5,0x4c,0x20,0x58,0x6c,0x24,0xfc]
          vaddph  -128(%rsp), %ymm22, %ymm21

// CHECK: vaddph  128(%rsp), %ymm22, %ymm21
// CHECK: encoding: [0x62,0xe5,0x4c,0x20,0x58,0x6c,0x24,0x04]
          vaddph  128(%rsp), %ymm22, %ymm21

// CHECK: vaddph  268435456(%rcx,%r14,8), %ymm22, %ymm21
// CHECK: encoding: [0x62,0xa5,0x4c,0x20,0x58,0xac,0xf1,0x00,0x00,0x00,0x10]
          vaddph  268435456(%rcx,%r14,8), %ymm22, %ymm21

// CHECK: vaddph  -536870912(%rcx,%r14,8), %ymm22, %ymm21
// CHECK: encoding: [0x62,0xa5,0x4c,0x20,0x58,0xac,0xf1,0x00,0x00,0x00,0xe0]
          vaddph  -536870912(%rcx,%r14,8), %ymm22, %ymm21

// CHECK: vaddph  -536870910(%rcx,%r14,8), %ymm22, %ymm21
// CHECK: encoding: [0x62,0xa5,0x4c,0x20,0x58,0xac,0xf1,0x02,0x00,0x00,0xe0]
          vaddph  -536870910(%rcx,%r14,8), %ymm22, %ymm21

// CHECK: vaddph  (%rcx), %ymm2, %ymm1 {%k2}
// CHECK: encoding: [0x62,0xf5,0x6c,0x2a,0x58,0x09]
          vaddph  (%rcx), %ymm2, %ymm1 {%k2}

// CHECK: vaddph  -128(%rsp), %ymm2, %ymm1 {%k2}
// CHECK: encoding: [0x62,0xf5,0x6c,0x2a,0x58,0x4c,0x24,0xfc]
          vaddph  -128(%rsp), %ymm2, %ymm1 {%k2}

// CHECK: vaddph  128(%rsp), %ymm2, %ymm1 {%k2}
// CHECK: encoding: [0x62,0xf5,0x6c,0x2a,0x58,0x4c,0x24,0x04]
          vaddph  128(%rsp), %ymm2, %ymm1 {%k2}

// CHECK: vaddph  268435456(%rcx,%r14,8), %ymm2, %ymm1 {%k2}
// CHECK: encoding: [0x62,0xb5,0x6c,0x2a,0x58,0x8c,0xf1,0x00,0x00,0x00,0x10]
          vaddph  268435456(%rcx,%r14,8), %ymm2, %ymm1 {%k2}

// CHECK: vaddph  -536870912(%rcx,%r14,8), %ymm2, %ymm1 {%k2}
// CHECK: encoding: [0x62,0xb5,0x6c,0x2a,0x58,0x8c,0xf1,0x00,0x00,0x00,0xe0]
          vaddph  -536870912(%rcx,%r14,8), %ymm2, %ymm1 {%k2}

// CHECK: vaddph  -536870910(%rcx,%r14,8), %ymm2, %ymm1 {%k2}
// CHECK: encoding: [0x62,0xb5,0x6c,0x2a,0x58,0x8c,0xf1,0x02,0x00,0x00,0xe0]
          vaddph  -536870910(%rcx,%r14,8), %ymm2, %ymm1 {%k2}

// CHECK: vaddph  (%rcx), %ymm22, %ymm21 {%k2}
// CHECK: encoding: [0x62,0xe5,0x4c,0x22,0x58,0x29]
          vaddph  (%rcx), %ymm22, %ymm21 {%k2}

// CHECK: vaddph  -128(%rsp), %ymm22, %ymm21 {%k2}
// CHECK: encoding: [0x62,0xe5,0x4c,0x22,0x58,0x6c,0x24,0xfc]
          vaddph  -128(%rsp), %ymm22, %ymm21 {%k2}

// CHECK: vaddph  128(%rsp), %ymm22, %ymm21 {%k2}
// CHECK: encoding: [0x62,0xe5,0x4c,0x22,0x58,0x6c,0x24,0x04]
          vaddph  128(%rsp), %ymm22, %ymm21 {%k2}

// CHECK: vaddph  268435456(%rcx,%r14,8), %ymm22, %ymm21 {%k2}
// CHECK: encoding: [0x62,0xa5,0x4c,0x22,0x58,0xac,0xf1,0x00,0x00,0x00,0x10]
          vaddph  268435456(%rcx,%r14,8), %ymm22, %ymm21 {%k2}

// CHECK: vaddph  -536870912(%rcx,%r14,8), %ymm22, %ymm21 {%k2}
// CHECK: encoding: [0x62,0xa5,0x4c,0x22,0x58,0xac,0xf1,0x00,0x00,0x00,0xe0]
          vaddph  -536870912(%rcx,%r14,8), %ymm22, %ymm21 {%k2}

// CHECK: vaddph  -536870910(%rcx,%r14,8), %ymm22, %ymm21 {%k2}
// CHECK: encoding: [0x62,0xa5,0x4c,0x22,0x58,0xac,0xf1,0x02,0x00,0x00,0xe0]
          vaddph  -536870910(%rcx,%r14,8), %ymm22, %ymm21 {%k2}

// CHECK: vaddph %zmm3, %zmm2, %zmm1
// CHECK: encoding: [0x62,0xf5,0x6c,0x48,0x58,0xcb]
          vaddph %zmm3, %zmm2, %zmm1

// CHECK: vaddph %zmm23, %zmm22, %zmm21
// CHECK: encoding: [0x62,0xa5,0x4c,0x40,0x58,0xef]
          vaddph %zmm23, %zmm22, %zmm21

// CHECK: vaddph %zmm3, %zmm2, %zmm1 {%k2}
// CHECK: encoding: [0x62,0xf5,0x6c,0x4a,0x58,0xcb]
          vaddph %zmm3, %zmm2, %zmm1 {%k2}

// CHECK: vaddph %zmm23, %zmm22, %zmm21 {%k2}
// CHECK: encoding: [0x62,0xa5,0x4c,0x42,0x58,0xef]
          vaddph %zmm23, %zmm22, %zmm21 {%k2}

// CHECK: vaddph  (%rcx), %zmm2, %zmm1
// CHECK: encoding: [0x62,0xf5,0x6c,0x48,0x58,0x09]
          vaddph  (%rcx), %zmm2, %zmm1

// CHECK: vaddph  -256(%rsp), %zmm2, %zmm1
// CHECK: encoding: [0x62,0xf5,0x6c,0x48,0x58,0x4c,0x24,0xfc]
          vaddph  -256(%rsp), %zmm2, %zmm1

// CHECK: vaddph  256(%rsp), %zmm2, %zmm1
// CHECK: encoding: [0x62,0xf5,0x6c,0x48,0x58,0x4c,0x24,0x04]
          vaddph  256(%rsp), %zmm2, %zmm1

// CHECK: vaddph  268435456(%rcx,%r14,8), %zmm2, %zmm1
// CHECK: encoding: [0x62,0xb5,0x6c,0x48,0x58,0x8c,0xf1,0x00,0x00,0x00,0x10]
          vaddph  268435456(%rcx,%r14,8), %zmm2, %zmm1

// CHECK: vaddph  -536870912(%rcx,%r14,8), %zmm2, %zmm1
// CHECK: encoding: [0x62,0xb5,0x6c,0x48,0x58,0x8c,0xf1,0x00,0x00,0x00,0xe0]
          vaddph  -536870912(%rcx,%r14,8), %zmm2, %zmm1

// CHECK: vaddph  -536870910(%rcx,%r14,8), %zmm2, %zmm1
// CHECK: encoding: [0x62,0xb5,0x6c,0x48,0x58,0x8c,0xf1,0x02,0x00,0x00,0xe0]
          vaddph  -536870910(%rcx,%r14,8), %zmm2, %zmm1

// CHECK: vaddph  (%rcx), %zmm22, %zmm21
// CHECK: encoding: [0x62,0xe5,0x4c,0x40,0x58,0x29]
          vaddph  (%rcx), %zmm22, %zmm21

// CHECK: vaddph  -256(%rsp), %zmm22, %zmm21
// CHECK: encoding: [0x62,0xe5,0x4c,0x40,0x58,0x6c,0x24,0xfc]
          vaddph  -256(%rsp), %zmm22, %zmm21

// CHECK: vaddph  256(%rsp), %zmm22, %zmm21
// CHECK: encoding: [0x62,0xe5,0x4c,0x40,0x58,0x6c,0x24,0x04]
          vaddph  256(%rsp), %zmm22, %zmm21

// CHECK: vaddph  268435456(%rcx,%r14,8), %zmm22, %zmm21
// CHECK: encoding: [0x62,0xa5,0x4c,0x40,0x58,0xac,0xf1,0x00,0x00,0x00,0x10]
          vaddph  268435456(%rcx,%r14,8), %zmm22, %zmm21

// CHECK: vaddph  -536870912(%rcx,%r14,8), %zmm22, %zmm21
// CHECK: encoding: [0x62,0xa5,0x4c,0x40,0x58,0xac,0xf1,0x00,0x00,0x00,0xe0]
          vaddph  -536870912(%rcx,%r14,8), %zmm22, %zmm21

// CHECK: vaddph  -536870910(%rcx,%r14,8), %zmm22, %zmm21
// CHECK: encoding: [0x62,0xa5,0x4c,0x40,0x58,0xac,0xf1,0x02,0x00,0x00,0xe0]
          vaddph  -536870910(%rcx,%r14,8), %zmm22, %zmm21

// CHECK: vaddph  (%rcx), %zmm2, %zmm1 {%k2}
// CHECK: encoding: [0x62,0xf5,0x6c,0x4a,0x58,0x09]
          vaddph  (%rcx), %zmm2, %zmm1 {%k2}

// CHECK: vaddph  -256(%rsp), %zmm2, %zmm1 {%k2}
// CHECK: encoding: [0x62,0xf5,0x6c,0x4a,0x58,0x4c,0x24,0xfc]
          vaddph  -256(%rsp), %zmm2, %zmm1 {%k2}

// CHECK: vaddph  256(%rsp), %zmm2, %zmm1 {%k2}
// CHECK: encoding: [0x62,0xf5,0x6c,0x4a,0x58,0x4c,0x24,0x04]
          vaddph  256(%rsp), %zmm2, %zmm1 {%k2}

// CHECK: vaddph  268435456(%rcx,%r14,8), %zmm2, %zmm1 {%k2}
// CHECK: encoding: [0x62,0xb5,0x6c,0x4a,0x58,0x8c,0xf1,0x00,0x00,0x00,0x10]
          vaddph  268435456(%rcx,%r14,8), %zmm2, %zmm1 {%k2}

// CHECK: vaddph  -536870912(%rcx,%r14,8), %zmm2, %zmm1 {%k2}
// CHECK: encoding: [0x62,0xb5,0x6c,0x4a,0x58,0x8c,0xf1,0x00,0x00,0x00,0xe0]
          vaddph  -536870912(%rcx,%r14,8), %zmm2, %zmm1 {%k2}

// CHECK: vaddph  -536870910(%rcx,%r14,8), %zmm2, %zmm1 {%k2}
// CHECK: encoding: [0x62,0xb5,0x6c,0x4a,0x58,0x8c,0xf1,0x02,0x00,0x00,0xe0]
          vaddph  -536870910(%rcx,%r14,8), %zmm2, %zmm1 {%k2}

// CHECK: vaddph  (%rcx), %zmm22, %zmm21 {%k2}
// CHECK: encoding: [0x62,0xe5,0x4c,0x42,0x58,0x29]
          vaddph  (%rcx), %zmm22, %zmm21 {%k2}

// CHECK: vaddph  -256(%rsp), %zmm22, %zmm21 {%k2}
// CHECK: encoding: [0x62,0xe5,0x4c,0x42,0x58,0x6c,0x24,0xfc]
          vaddph  -256(%rsp), %zmm22, %zmm21 {%k2}

// CHECK: vaddph  256(%rsp), %zmm22, %zmm21 {%k2}
// CHECK: encoding: [0x62,0xe5,0x4c,0x42,0x58,0x6c,0x24,0x04]
          vaddph  256(%rsp), %zmm22, %zmm21 {%k2}

// CHECK: vaddph  268435456(%rcx,%r14,8), %zmm22, %zmm21 {%k2}
// CHECK: encoding: [0x62,0xa5,0x4c,0x42,0x58,0xac,0xf1,0x00,0x00,0x00,0x10]
          vaddph  268435456(%rcx,%r14,8), %zmm22, %zmm21 {%k2}

// CHECK: vaddph  -536870912(%rcx,%r14,8), %zmm22, %zmm21 {%k2}
// CHECK: encoding: [0x62,0xa5,0x4c,0x42,0x58,0xac,0xf1,0x00,0x00,0x00,0xe0]
          vaddph  -536870912(%rcx,%r14,8), %zmm22, %zmm21 {%k2}

// CHECK: vaddph  -536870910(%rcx,%r14,8), %zmm22, %zmm21 {%k2}
// CHECK: encoding: [0x62,0xa5,0x4c,0x42,0x58,0xac,0xf1,0x02,0x00,0x00,0xe0]
          vaddph  -536870910(%rcx,%r14,8), %zmm22, %zmm21 {%k2}

// CHECK: vaddph %xmm1, %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x0a,0x58,0xc9]
          vaddph %xmm1, %xmm1, %xmm1 {%k2}

// CHECK: vaddph %xmm1, %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x8a,0x58,0xc9]
          vaddph %xmm1, %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph %ymm23, %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xa5,0x44,0x22,0x58,0xff]
          vaddph %ymm23, %ymm23, %ymm23 {%k2}

// CHECK: vaddph %ymm23, %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xa5,0x44,0xa2,0x58,0xff]
          vaddph %ymm23, %ymm23, %ymm23 {%k2} {z}

// CHECK: vaddph 485498096, %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x0a,0x58,0x0c,0x25,0xf0,0x1c,0xf0,0x1c]
          vaddph 485498096, %xmm1, %xmm1 {%k2}

// CHECK: vaddph 485498096{1to8}, %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x1a,0x58,0x0c,0x25,0xf0,0x1c,0xf0,0x1c]
          vaddph 485498096{1to8}, %xmm1, %xmm1 {%k2}

// CHECK: vaddph (%rdx), %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x0a,0x58,0x0a]
          vaddph (%rdx), %xmm1, %xmm1 {%k2}

// CHECK: vaddph (%rdx){1to8}, %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x1a,0x58,0x0a]
          vaddph (%rdx){1to8}, %xmm1, %xmm1 {%k2}

// CHECK: vaddph 1024(%rdx), %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x0a,0x58,0x4a,0x40]
          vaddph 1024(%rdx), %xmm1, %xmm1 {%k2}

// CHECK: vaddph 128(%rdx){1to8}, %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x1a,0x58,0x4a,0x40]
          vaddph 128(%rdx){1to8}, %xmm1, %xmm1 {%k2}

// CHECK: vaddph 1024(%rdx,%rax), %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x0a,0x58,0x4c,0x02,0x40]
          vaddph 1024(%rdx,%rax), %xmm1, %xmm1 {%k2}

// CHECK: vaddph 128(%rdx,%rax){1to8}, %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x1a,0x58,0x4c,0x02,0x40]
          vaddph 128(%rdx,%rax){1to8}, %xmm1, %xmm1 {%k2}

// CHECK: vaddph 1024(%rdx,%rax,4), %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x0a,0x58,0x4c,0x82,0x40]
          vaddph 1024(%rdx,%rax,4), %xmm1, %xmm1 {%k2}

// CHECK: vaddph 128(%rdx,%rax,4){1to8}, %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x1a,0x58,0x4c,0x82,0x40]
          vaddph 128(%rdx,%rax,4){1to8}, %xmm1, %xmm1 {%k2}

// CHECK: vaddph -1024(%rdx,%rax,4), %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x0a,0x58,0x4c,0x82,0xc0]
          vaddph -1024(%rdx,%rax,4), %xmm1, %xmm1 {%k2}

// CHECK: vaddph -128(%rdx,%rax,4){1to8}, %xmm1, %xmm1 {%k2}
// CHECK: # encoding: [0x62,0xf5,0x74,0x1a,0x58,0x4c,0x82,0xc0]
          vaddph -128(%rdx,%rax,4){1to8}, %xmm1, %xmm1 {%k2}

// CHECK: vaddph 485498096, %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x8a,0x58,0x0c,0x25,0xf0,0x1c,0xf0,0x1c]
          vaddph 485498096, %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph 485498096{1to8}, %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x9a,0x58,0x0c,0x25,0xf0,0x1c,0xf0,0x1c]
          vaddph 485498096{1to8}, %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph (%rdx), %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x8a,0x58,0x0a]
          vaddph (%rdx), %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph (%rdx){1to8}, %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x9a,0x58,0x0a]
          vaddph (%rdx){1to8}, %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph 1024(%rdx), %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x8a,0x58,0x4a,0x40]
          vaddph 1024(%rdx), %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph 128(%rdx){1to8}, %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x9a,0x58,0x4a,0x40]
          vaddph 128(%rdx){1to8}, %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph 1024(%rdx,%rax), %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x8a,0x58,0x4c,0x02,0x40]
          vaddph 1024(%rdx,%rax), %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph 128(%rdx,%rax){1to8}, %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x9a,0x58,0x4c,0x02,0x40]
          vaddph 128(%rdx,%rax){1to8}, %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph 1024(%rdx,%rax,4), %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x8a,0x58,0x4c,0x82,0x40]
          vaddph 1024(%rdx,%rax,4), %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph 128(%rdx,%rax,4){1to8}, %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x9a,0x58,0x4c,0x82,0x40]
          vaddph 128(%rdx,%rax,4){1to8}, %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph -1024(%rdx,%rax,4), %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x8a,0x58,0x4c,0x82,0xc0]
          vaddph -1024(%rdx,%rax,4), %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph -128(%rdx,%rax,4){1to8}, %xmm1, %xmm1 {%k2} {z}
// CHECK: # encoding: [0x62,0xf5,0x74,0x9a,0x58,0x4c,0x82,0xc0]
          vaddph -128(%rdx,%rax,4){1to8}, %xmm1, %xmm1 {%k2} {z}

// CHECK: vaddph 485498096, %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xe5,0x44,0x22,0x58,0x3c,0x25,0xf0,0x1c,0xf0,0x1c]
          vaddph 485498096, %ymm23, %ymm23 {%k2}

// CHECK: vaddph 485498096{1to16}, %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xe5,0x44,0x32,0x58,0x3c,0x25,0xf0,0x1c,0xf0,0x1c]
          vaddph 485498096{1to16}, %ymm23, %ymm23 {%k2}

// CHECK: vaddph (%rdx), %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xe5,0x44,0x22,0x58,0x3a]
          vaddph (%rdx), %ymm23, %ymm23 {%k2}

// CHECK: vaddph (%rdx){1to16}, %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xe5,0x44,0x32,0x58,0x3a]
          vaddph (%rdx){1to16}, %ymm23, %ymm23 {%k2}

// CHECK: vaddph 2048(%rdx), %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xe5,0x44,0x22,0x58,0x7a,0x40]
          vaddph 2048(%rdx), %ymm23, %ymm23 {%k2}

// CHECK: vaddph 128(%rdx){1to16}, %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xe5,0x44,0x32,0x58,0x7a,0x40]
          vaddph 128(%rdx){1to16}, %ymm23, %ymm23 {%k2}

// CHECK: vaddph 2048(%rdx,%rax), %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xe5,0x44,0x22,0x58,0x7c,0x02,0x40]
          vaddph 2048(%rdx,%rax), %ymm23, %ymm23 {%k2}

// CHECK: vaddph 128(%rdx,%rax){1to16}, %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xe5,0x44,0x32,0x58,0x7c,0x02,0x40]
          vaddph 128(%rdx,%rax){1to16}, %ymm23, %ymm23 {%k2}

// CHECK: vaddph 2048(%rdx,%rax,4), %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xe5,0x44,0x22,0x58,0x7c,0x82,0x40]
          vaddph 2048(%rdx,%rax,4), %ymm23, %ymm23 {%k2}

// CHECK: vaddph 128(%rdx,%rax,4){1to16}, %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xe5,0x44,0x32,0x58,0x7c,0x82,0x40]
          vaddph 128(%rdx,%rax,4){1to16}, %ymm23, %ymm23 {%k2}

// CHECK: vaddph -2048(%rdx,%rax,4), %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xe5,0x44,0x22,0x58,0x7c,0x82,0xc0]
          vaddph -2048(%rdx,%rax,4), %ymm23, %ymm23 {%k2}

// CHECK: vaddph -128(%rdx,%rax,4){1to16}, %ymm23, %ymm23 {%k2}
// CHECK: # encoding: [0x62,0xe5,0x44,0x32,0x58,0x7c,0x82,0xc0]
          vaddph -128(%rdx,%rax,4){1to16}, %ymm23, %ymm23 {%k2}

// CHECK: vaddph 485498096, %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xe5,0x44,0xa2,0x58,0x3c,0x25,0xf0,0x1c,0xf0,0x1c]
          vaddph 485498096, %ymm23, %ymm23 {%k2} {z}

// CHECK: vaddph 485498096{1to16}, %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xe5,0x44,0xb2,0x58,0x3c,0x25,0xf0,0x1c,0xf0,0x1c]
          vaddph 485498096{1to16}, %ymm23, %ymm23 {%k2} {z}

// CHECK: vaddph (%rdx), %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xe5,0x44,0xa2,0x58,0x3a]
          vaddph (%rdx), %ymm23, %ymm23 {%k2} {z}

// CHECK: vaddph (%rdx){1to16}, %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xe5,0x44,0xb2,0x58,0x3a]
          vaddph (%rdx){1to16}, %ymm23, %ymm23 {%k2} {z}

// CHECK: vaddph 2048(%rdx), %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xe5,0x44,0xa2,0x58,0x7a,0x40]
          vaddph 2048(%rdx), %ymm23, %ymm23 {%k2} {z}

// CHECK: vaddph 128(%rdx){1to16}, %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xe5,0x44,0xb2,0x58,0x7a,0x40]
          vaddph 128(%rdx){1to16}, %ymm23, %ymm23 {%k2} {z}

// CHECK: vaddph 2048(%rdx,%rax), %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xe5,0x44,0xa2,0x58,0x7c,0x02,0x40]
          vaddph 2048(%rdx,%rax), %ymm23, %ymm23 {%k2} {z}

// CHECK: vaddph 128(%rdx,%rax){1to16}, %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xe5,0x44,0xb2,0x58,0x7c,0x02,0x40]
          vaddph 128(%rdx,%rax){1to16}, %ymm23, %ymm23 {%k2} {z}

// CHECK: vaddph 2048(%rdx,%rax,4), %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xe5,0x44,0xa2,0x58,0x7c,0x82,0x40]
          vaddph 2048(%rdx,%rax,4), %ymm23, %ymm23 {%k2} {z}

// CHECK: vaddph 128(%rdx,%rax,4){1to16}, %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xe5,0x44,0xb2,0x58,0x7c,0x82,0x40]
          vaddph 128(%rdx,%rax,4){1to16}, %ymm23, %ymm23 {%k2} {z}

// CHECK: vaddph -2048(%rdx,%rax,4), %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xe5,0x44,0xa2,0x58,0x7c,0x82,0xc0]
          vaddph -2048(%rdx,%rax,4), %ymm23, %ymm23 {%k2} {z}

// CHECK: vaddph -128(%rdx,%rax,4){1to16}, %ymm23, %ymm23 {%k2} {z}
// CHECK: # encoding: [0x62,0xe5,0x44,0xb2,0x58,0x7c,0x82,0xc0]
          vaddph -128(%rdx,%rax,4){1to16}, %ymm23, %ymm23 {%k2} {z}
