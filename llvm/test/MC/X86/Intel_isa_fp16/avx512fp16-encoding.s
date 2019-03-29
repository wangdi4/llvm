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

// CHECK: vaddsh %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x16,0x00,0x58,0xf4]
          vaddsh %xmm28, %xmm29, %xmm30

// CHECK: vaddsh {rn-sae}, %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x16,0x10,0x58,0xf4]
          vaddsh {rn-sae}, %xmm28, %xmm29, %xmm30

// CHECK: vaddsh %xmm28, %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x16,0x07,0x58,0xf4]
          vaddsh %xmm28, %xmm29, %xmm30 {%k7}

// CHECK: vaddsh {rz-sae}, %xmm28, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x16,0xf7,0x58,0xf4]
          vaddsh {rz-sae}, %xmm28, %xmm29, %xmm30 {%k7} {z}

// CHECK: vaddsh  268435456(%rbp,%r14,8), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x25,0x16,0x00,0x58,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vaddsh  268435456(%rbp,%r14,8), %xmm29, %xmm30

// CHECK: vaddsh  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x16,0x07,0x58,0xb4,0x80,0x23,0x01,0x00,0x00]
          vaddsh  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}

// CHECK: vaddsh  (%rip), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x58,0x35,0x00,0x00,0x00,0x00]
          vaddsh  (%rip), %xmm29, %xmm30

// CHECK: vaddsh  -64(,%rbp,2), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x58,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vaddsh  -64(,%rbp,2), %xmm29, %xmm30

// CHECK: vaddsh  254(%rcx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x58,0x71,0x7f]
          vaddsh  254(%rcx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vaddsh  -256(%rdx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x58,0x72,0x80]
          vaddsh  -256(%rdx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vcmpsh $123, %xmm28, %xmm29, %k5
// CHECK: encoding: [0x62,0x93,0x16,0x00,0xc2,0xec,0x7b]
          vcmpsh $123, %xmm28, %xmm29, %k5

// CHECK: vcmpsh $123, {sae}, %xmm28, %xmm29, %k5
// CHECK: encoding: [0x62,0x93,0x16,0x10,0xc2,0xec,0x7b]
          vcmpsh $123, {sae}, %xmm28, %xmm29, %k5

// CHECK: vcmpsh $123, %xmm28, %xmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0x93,0x16,0x07,0xc2,0xec,0x7b]
          vcmpsh $123, %xmm28, %xmm29, %k5 {%k7}

// CHECK: vcmpsh $123, {sae}, %xmm28, %xmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0x93,0x16,0x17,0xc2,0xec,0x7b]
          vcmpsh $123, {sae}, %xmm28, %xmm29, %k5 {%k7}

// CHECK: vcmpsh  $123, 268435456(%rbp,%r14,8), %xmm29, %k5
// CHECK: encoding: [0x62,0xb3,0x16,0x00,0xc2,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcmpsh  $123, 268435456(%rbp,%r14,8), %xmm29, %k5

// CHECK: vcmpsh  $123, 291(%r8,%rax,4), %xmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0xd3,0x16,0x07,0xc2,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcmpsh  $123, 291(%r8,%rax,4), %xmm29, %k5 {%k7}

// CHECK: vcmpsh  $123, (%rip), %xmm29, %k5
// CHECK: encoding: [0x62,0xf3,0x16,0x00,0xc2,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vcmpsh  $123, (%rip), %xmm29, %k5

// CHECK: vcmpsh  $123, -64(,%rbp,2), %xmm29, %k5
// CHECK: encoding: [0x62,0xf3,0x16,0x00,0xc2,0x2c,0x6d,0xc0,0xff,0xff,0xff,0x7b]
          vcmpsh  $123, -64(,%rbp,2), %xmm29, %k5

// CHECK: vcmpsh  $123, 254(%rcx), %xmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x16,0x07,0xc2,0x69,0x7f,0x7b]
          vcmpsh  $123, 254(%rcx), %xmm29, %k5 {%k7}

// CHECK: vcmpsh  $123, -256(%rdx), %xmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x16,0x07,0xc2,0x6a,0x80,0x7b]
          vcmpsh  $123, -256(%rdx), %xmm29, %k5 {%k7}

// CHECK: vdivsh %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x16,0x00,0x5e,0xf4]
          vdivsh %xmm28, %xmm29, %xmm30

// CHECK: vdivsh {rn-sae}, %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x16,0x10,0x5e,0xf4]
          vdivsh {rn-sae}, %xmm28, %xmm29, %xmm30

// CHECK: vdivsh %xmm28, %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x16,0x07,0x5e,0xf4]
          vdivsh %xmm28, %xmm29, %xmm30 {%k7}

// CHECK: vdivsh {rz-sae}, %xmm28, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x16,0xf7,0x5e,0xf4]
          vdivsh {rz-sae}, %xmm28, %xmm29, %xmm30 {%k7} {z}

// CHECK: vdivsh  268435456(%rbp,%r14,8), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x25,0x16,0x00,0x5e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdivsh  268435456(%rbp,%r14,8), %xmm29, %xmm30

// CHECK: vdivsh  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x16,0x07,0x5e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdivsh  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}

// CHECK: vdivsh  (%rip), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5e,0x35,0x00,0x00,0x00,0x00]
          vdivsh  (%rip), %xmm29, %xmm30

// CHECK: vdivsh  -64(,%rbp,2), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5e,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vdivsh  -64(,%rbp,2), %xmm29, %xmm30

// CHECK: vdivsh  254(%rcx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5e,0x71,0x7f]
          vdivsh  254(%rcx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vdivsh  -256(%rdx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5e,0x72,0x80]
          vdivsh  -256(%rdx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vmaxsh %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x16,0x00,0x5f,0xf4]
          vmaxsh %xmm28, %xmm29, %xmm30

// CHECK: vmaxsh {sae}, %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x16,0x10,0x5f,0xf4]
          vmaxsh {sae}, %xmm28, %xmm29, %xmm30

// CHECK: vmaxsh %xmm28, %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x16,0x07,0x5f,0xf4]
          vmaxsh %xmm28, %xmm29, %xmm30 {%k7}

// CHECK: vmaxsh {sae}, %xmm28, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x16,0x97,0x5f,0xf4]
          vmaxsh {sae}, %xmm28, %xmm29, %xmm30 {%k7} {z}

// CHECK: vmaxsh  268435456(%rbp,%r14,8), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x25,0x16,0x00,0x5f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmaxsh  268435456(%rbp,%r14,8), %xmm29, %xmm30

// CHECK: vmaxsh  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x16,0x07,0x5f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmaxsh  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}

// CHECK: vmaxsh  (%rip), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5f,0x35,0x00,0x00,0x00,0x00]
          vmaxsh  (%rip), %xmm29, %xmm30

// CHECK: vmaxsh  -64(,%rbp,2), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5f,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vmaxsh  -64(,%rbp,2), %xmm29, %xmm30

// CHECK: vmaxsh  254(%rcx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5f,0x71,0x7f]
          vmaxsh  254(%rcx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vmaxsh  -256(%rdx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5f,0x72,0x80]
          vmaxsh  -256(%rdx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vminsh %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x16,0x00,0x5d,0xf4]
          vminsh %xmm28, %xmm29, %xmm30

// CHECK: vminsh {sae}, %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x16,0x10,0x5d,0xf4]
          vminsh {sae}, %xmm28, %xmm29, %xmm30

// CHECK: vminsh %xmm28, %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x16,0x07,0x5d,0xf4]
          vminsh %xmm28, %xmm29, %xmm30 {%k7}

// CHECK: vminsh {sae}, %xmm28, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x16,0x97,0x5d,0xf4]
          vminsh {sae}, %xmm28, %xmm29, %xmm30 {%k7} {z}

// CHECK: vminsh  268435456(%rbp,%r14,8), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x25,0x16,0x00,0x5d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vminsh  268435456(%rbp,%r14,8), %xmm29, %xmm30

// CHECK: vminsh  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x16,0x07,0x5d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vminsh  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}

// CHECK: vminsh  (%rip), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5d,0x35,0x00,0x00,0x00,0x00]
          vminsh  (%rip), %xmm29, %xmm30

// CHECK: vminsh  -64(,%rbp,2), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5d,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vminsh  -64(,%rbp,2), %xmm29, %xmm30

// CHECK: vminsh  254(%rcx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5d,0x71,0x7f]
          vminsh  254(%rcx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vminsh  -256(%rdx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5d,0x72,0x80]
          vminsh  -256(%rdx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vmulsh %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x16,0x00,0x59,0xf4]
          vmulsh %xmm28, %xmm29, %xmm30

// CHECK: vmulsh {rn-sae}, %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x16,0x10,0x59,0xf4]
          vmulsh {rn-sae}, %xmm28, %xmm29, %xmm30

// CHECK: vmulsh %xmm28, %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x16,0x07,0x59,0xf4]
          vmulsh %xmm28, %xmm29, %xmm30 {%k7}

// CHECK: vmulsh {rz-sae}, %xmm28, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x16,0xf7,0x59,0xf4]
          vmulsh {rz-sae}, %xmm28, %xmm29, %xmm30 {%k7} {z}

// CHECK: vmulsh  268435456(%rbp,%r14,8), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x25,0x16,0x00,0x59,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmulsh  268435456(%rbp,%r14,8), %xmm29, %xmm30

// CHECK: vmulsh  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x16,0x07,0x59,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmulsh  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}

// CHECK: vmulsh  (%rip), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x59,0x35,0x00,0x00,0x00,0x00]
          vmulsh  (%rip), %xmm29, %xmm30

// CHECK: vmulsh  -64(,%rbp,2), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x59,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vmulsh  -64(,%rbp,2), %xmm29, %xmm30

// CHECK: vmulsh  254(%rcx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x59,0x71,0x7f]
          vmulsh  254(%rcx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vmulsh  -256(%rdx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x59,0x72,0x80]
          vmulsh  -256(%rdx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vsubsh %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x16,0x00,0x5c,0xf4]
          vsubsh %xmm28, %xmm29, %xmm30

// CHECK: vsubsh {rn-sae}, %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x16,0x10,0x5c,0xf4]
          vsubsh {rn-sae}, %xmm28, %xmm29, %xmm30

// CHECK: vsubsh %xmm28, %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x16,0x07,0x5c,0xf4]
          vsubsh %xmm28, %xmm29, %xmm30 {%k7}

// CHECK: vsubsh {rz-sae}, %xmm28, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x16,0xf7,0x5c,0xf4]
          vsubsh {rz-sae}, %xmm28, %xmm29, %xmm30 {%k7} {z}

// CHECK: vsubsh  268435456(%rbp,%r14,8), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x25,0x16,0x00,0x5c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubsh  268435456(%rbp,%r14,8), %xmm29, %xmm30

// CHECK: vsubsh  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x16,0x07,0x5c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubsh  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}

// CHECK: vsubsh  (%rip), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5c,0x35,0x00,0x00,0x00,0x00]
          vsubsh  (%rip), %xmm29, %xmm30

// CHECK: vsubsh  -64(,%rbp,2), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x16,0x00,0x5c,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vsubsh  -64(,%rbp,2), %xmm29, %xmm30

// CHECK: vsubsh  254(%rcx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5c,0x71,0x7f]
          vsubsh  254(%rcx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vsubsh  -256(%rdx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x16,0x87,0x5c,0x72,0x80]
          vsubsh  -256(%rdx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vcmpph $123, %zmm28, %zmm29, %k5
// CHECK: encoding: [0x62,0x93,0x14,0x40,0xc2,0xec,0x7b]
          vcmpph $123, %zmm28, %zmm29, %k5

// CHECK: vcmpph $123, {sae}, %zmm28, %zmm29, %k5
// CHECK: encoding: [0x62,0x93,0x14,0x10,0xc2,0xec,0x7b]
          vcmpph $123, {sae}, %zmm28, %zmm29, %k5

// CHECK: vcmpph $123, %zmm28, %zmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0x93,0x14,0x47,0xc2,0xec,0x7b]
          vcmpph $123, %zmm28, %zmm29, %k5 {%k7}

// CHECK: vcmpph $123, {sae}, %zmm28, %zmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0x93,0x14,0x17,0xc2,0xec,0x7b]
          vcmpph $123, {sae}, %zmm28, %zmm29, %k5 {%k7}

// CHECK: vcmpph  $123, 268435456(%rbp,%r14,8), %zmm29, %k5
// CHECK: encoding: [0x62,0xb3,0x14,0x40,0xc2,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcmpph  $123, 268435456(%rbp,%r14,8), %zmm29, %k5

// CHECK: vcmpph  $123, 291(%r8,%rax,4), %zmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0xd3,0x14,0x47,0xc2,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcmpph  $123, 291(%r8,%rax,4), %zmm29, %k5 {%k7}

// CHECK: vcmpph  $123, (%rip){1to32}, %zmm29, %k5
// CHECK: encoding: [0x62,0xf3,0x14,0x50,0xc2,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vcmpph  $123, (%rip){1to32}, %zmm29, %k5

// CHECK: vcmpph  $123, -2048(,%rbp,2), %zmm29, %k5
// CHECK: encoding: [0x62,0xf3,0x14,0x40,0xc2,0x2c,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcmpph  $123, -2048(,%rbp,2), %zmm29, %k5

// CHECK: vcmpph  $123, 8128(%rcx), %zmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x14,0x47,0xc2,0x69,0x7f,0x7b]
          vcmpph  $123, 8128(%rcx), %zmm29, %k5 {%k7}

// CHECK: vcmpph  $123, -256(%rdx){1to32}, %zmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x14,0x57,0xc2,0x6a,0x80,0x7b]
          vcmpph  $123, -256(%rdx){1to32}, %zmm29, %k5 {%k7}

// CHECK: vdivph %zmm28, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x05,0x14,0x40,0x5e,0xf4]
          vdivph %zmm28, %zmm29, %zmm30

// CHECK: vdivph {rn-sae}, %zmm28, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x05,0x14,0x10,0x5e,0xf4]
          vdivph {rn-sae}, %zmm28, %zmm29, %zmm30

// CHECK: vdivph %zmm28, %zmm29, %zmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x47,0x5e,0xf4]
          vdivph %zmm28, %zmm29, %zmm30 {%k7}

// CHECK: vdivph {rz-sae}, %zmm28, %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0xf7,0x5e,0xf4]
          vdivph {rz-sae}, %zmm28, %zmm29, %zmm30 {%k7} {z}

// CHECK: vdivph  268435456(%rbp,%r14,8), %zmm29, %zmm30
// CHECK: encoding: [0x62,0x25,0x14,0x40,0x5e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdivph  268435456(%rbp,%r14,8), %zmm29, %zmm30

// CHECK: vdivph  291(%r8,%rax,4), %zmm29, %zmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x47,0x5e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdivph  291(%r8,%rax,4), %zmm29, %zmm30 {%k7}

// CHECK: vdivph  (%rip){1to32}, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x65,0x14,0x50,0x5e,0x35,0x00,0x00,0x00,0x00]
          vdivph  (%rip){1to32}, %zmm29, %zmm30

// CHECK: vdivph  -2048(,%rbp,2), %zmm29, %zmm30
// CHECK: encoding: [0x62,0x65,0x14,0x40,0x5e,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vdivph  -2048(,%rbp,2), %zmm29, %zmm30

// CHECK: vdivph  8128(%rcx), %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xc7,0x5e,0x71,0x7f]
          vdivph  8128(%rcx), %zmm29, %zmm30 {%k7} {z}

// CHECK: vdivph  -256(%rdx){1to32}, %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xd7,0x5e,0x72,0x80]
          vdivph  -256(%rdx){1to32}, %zmm29, %zmm30 {%k7} {z}

// CHECK: vmaxph %zmm28, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x05,0x14,0x40,0x5f,0xf4]
          vmaxph %zmm28, %zmm29, %zmm30

// CHECK: vmaxph {sae}, %zmm28, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x05,0x14,0x10,0x5f,0xf4]
          vmaxph {sae}, %zmm28, %zmm29, %zmm30

// CHECK: vmaxph %zmm28, %zmm29, %zmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x47,0x5f,0xf4]
          vmaxph %zmm28, %zmm29, %zmm30 {%k7}

// CHECK: vmaxph {sae}, %zmm28, %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0x97,0x5f,0xf4]
          vmaxph {sae}, %zmm28, %zmm29, %zmm30 {%k7} {z}

// CHECK: vmaxph  268435456(%rbp,%r14,8), %zmm29, %zmm30
// CHECK: encoding: [0x62,0x25,0x14,0x40,0x5f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmaxph  268435456(%rbp,%r14,8), %zmm29, %zmm30

// CHECK: vmaxph  291(%r8,%rax,4), %zmm29, %zmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x47,0x5f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmaxph  291(%r8,%rax,4), %zmm29, %zmm30 {%k7}

// CHECK: vmaxph  (%rip){1to32}, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x65,0x14,0x50,0x5f,0x35,0x00,0x00,0x00,0x00]
          vmaxph  (%rip){1to32}, %zmm29, %zmm30

// CHECK: vmaxph  -2048(,%rbp,2), %zmm29, %zmm30
// CHECK: encoding: [0x62,0x65,0x14,0x40,0x5f,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vmaxph  -2048(,%rbp,2), %zmm29, %zmm30

// CHECK: vmaxph  8128(%rcx), %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xc7,0x5f,0x71,0x7f]
          vmaxph  8128(%rcx), %zmm29, %zmm30 {%k7} {z}

// CHECK: vmaxph  -256(%rdx){1to32}, %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xd7,0x5f,0x72,0x80]
          vmaxph  -256(%rdx){1to32}, %zmm29, %zmm30 {%k7} {z}

// CHECK: vminph %zmm28, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x05,0x14,0x40,0x5d,0xf4]
          vminph %zmm28, %zmm29, %zmm30

// CHECK: vminph {sae}, %zmm28, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x05,0x14,0x10,0x5d,0xf4]
          vminph {sae}, %zmm28, %zmm29, %zmm30

// CHECK: vminph %zmm28, %zmm29, %zmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x47,0x5d,0xf4]
          vminph %zmm28, %zmm29, %zmm30 {%k7}

// CHECK: vminph {sae}, %zmm28, %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0x97,0x5d,0xf4]
          vminph {sae}, %zmm28, %zmm29, %zmm30 {%k7} {z}

// CHECK: vminph  268435456(%rbp,%r14,8), %zmm29, %zmm30
// CHECK: encoding: [0x62,0x25,0x14,0x40,0x5d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vminph  268435456(%rbp,%r14,8), %zmm29, %zmm30

// CHECK: vminph  291(%r8,%rax,4), %zmm29, %zmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x47,0x5d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vminph  291(%r8,%rax,4), %zmm29, %zmm30 {%k7}

// CHECK: vminph  (%rip){1to32}, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x65,0x14,0x50,0x5d,0x35,0x00,0x00,0x00,0x00]
          vminph  (%rip){1to32}, %zmm29, %zmm30

// CHECK: vminph  -2048(,%rbp,2), %zmm29, %zmm30
// CHECK: encoding: [0x62,0x65,0x14,0x40,0x5d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vminph  -2048(,%rbp,2), %zmm29, %zmm30

// CHECK: vminph  8128(%rcx), %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xc7,0x5d,0x71,0x7f]
          vminph  8128(%rcx), %zmm29, %zmm30 {%k7} {z}

// CHECK: vminph  -256(%rdx){1to32}, %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xd7,0x5d,0x72,0x80]
          vminph  -256(%rdx){1to32}, %zmm29, %zmm30 {%k7} {z}

// CHECK: vmulph %zmm28, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x05,0x14,0x40,0x59,0xf4]
          vmulph %zmm28, %zmm29, %zmm30

// CHECK: vmulph {rn-sae}, %zmm28, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x05,0x14,0x10,0x59,0xf4]
          vmulph {rn-sae}, %zmm28, %zmm29, %zmm30

// CHECK: vmulph %zmm28, %zmm29, %zmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x47,0x59,0xf4]
          vmulph %zmm28, %zmm29, %zmm30 {%k7}

// CHECK: vmulph {rz-sae}, %zmm28, %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0xf7,0x59,0xf4]
          vmulph {rz-sae}, %zmm28, %zmm29, %zmm30 {%k7} {z}

// CHECK: vmulph  268435456(%rbp,%r14,8), %zmm29, %zmm30
// CHECK: encoding: [0x62,0x25,0x14,0x40,0x59,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmulph  268435456(%rbp,%r14,8), %zmm29, %zmm30

// CHECK: vmulph  291(%r8,%rax,4), %zmm29, %zmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x47,0x59,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmulph  291(%r8,%rax,4), %zmm29, %zmm30 {%k7}

// CHECK: vmulph  (%rip){1to32}, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x65,0x14,0x50,0x59,0x35,0x00,0x00,0x00,0x00]
          vmulph  (%rip){1to32}, %zmm29, %zmm30

// CHECK: vmulph  -2048(,%rbp,2), %zmm29, %zmm30
// CHECK: encoding: [0x62,0x65,0x14,0x40,0x59,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vmulph  -2048(,%rbp,2), %zmm29, %zmm30

// CHECK: vmulph  8128(%rcx), %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xc7,0x59,0x71,0x7f]
          vmulph  8128(%rcx), %zmm29, %zmm30 {%k7} {z}

// CHECK: vmulph  -256(%rdx){1to32}, %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xd7,0x59,0x72,0x80]
          vmulph  -256(%rdx){1to32}, %zmm29, %zmm30 {%k7} {z}

// CHECK: vsubph %zmm28, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x05,0x14,0x40,0x5c,0xf4]
          vsubph %zmm28, %zmm29, %zmm30

// CHECK: vsubph {rn-sae}, %zmm28, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x05,0x14,0x10,0x5c,0xf4]
          vsubph {rn-sae}, %zmm28, %zmm29, %zmm30

// CHECK: vsubph %zmm28, %zmm29, %zmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x47,0x5c,0xf4]
          vsubph %zmm28, %zmm29, %zmm30 {%k7}

// CHECK: vsubph {rz-sae}, %zmm28, %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0xf7,0x5c,0xf4]
          vsubph {rz-sae}, %zmm28, %zmm29, %zmm30 {%k7} {z}

// CHECK: vsubph  268435456(%rbp,%r14,8), %zmm29, %zmm30
// CHECK: encoding: [0x62,0x25,0x14,0x40,0x5c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubph  268435456(%rbp,%r14,8), %zmm29, %zmm30

// CHECK: vsubph  291(%r8,%rax,4), %zmm29, %zmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x47,0x5c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubph  291(%r8,%rax,4), %zmm29, %zmm30 {%k7}

// CHECK: vsubph  (%rip){1to32}, %zmm29, %zmm30
// CHECK: encoding: [0x62,0x65,0x14,0x50,0x5c,0x35,0x00,0x00,0x00,0x00]
          vsubph  (%rip){1to32}, %zmm29, %zmm30

// CHECK: vsubph  -2048(,%rbp,2), %zmm29, %zmm30
// CHECK: encoding: [0x62,0x65,0x14,0x40,0x5c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vsubph  -2048(,%rbp,2), %zmm29, %zmm30

// CHECK: vsubph  8128(%rcx), %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xc7,0x5c,0x71,0x7f]
          vsubph  8128(%rcx), %zmm29, %zmm30 {%k7} {z}

// CHECK: vsubph  -256(%rdx){1to32}, %zmm29, %zmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xd7,0x5c,0x72,0x80]
          vsubph  -256(%rdx){1to32}, %zmm29, %zmm30 {%k7} {z}

// CHECK: vcomish %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x7c,0x08,0x2f,0xf5]
          vcomish %xmm29, %xmm30

// CHECK: vcomish {sae}, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x7c,0x18,0x2f,0xf5]
          vcomish {sae}, %xmm29, %xmm30

// CHECK: vcomish  268435456(%rbp,%r14,8), %xmm30
// CHECK: encoding: [0x62,0x25,0x7c,0x08,0x2f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcomish  268435456(%rbp,%r14,8), %xmm30

// CHECK: vcomish  291(%r8,%rax,4), %xmm30
// CHECK: encoding: [0x62,0x45,0x7c,0x08,0x2f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcomish  291(%r8,%rax,4), %xmm30

// CHECK: vcomish  (%rip), %xmm30
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2f,0x35,0x00,0x00,0x00,0x00]
          vcomish  (%rip), %xmm30

// CHECK: vcomish  -64(,%rbp,2), %xmm30
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2f,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vcomish  -64(,%rbp,2), %xmm30

// CHECK: vcomish  254(%rcx), %xmm30
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2f,0x71,0x7f]
          vcomish  254(%rcx), %xmm30

// CHECK: vcomish  -256(%rdx), %xmm30
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2f,0x72,0x80]
          vcomish  -256(%rdx), %xmm30

// CHECK: vucomish %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x7c,0x08,0x2e,0xf5]
          vucomish %xmm29, %xmm30

// CHECK: vucomish {sae}, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x7c,0x18,0x2e,0xf5]
          vucomish {sae}, %xmm29, %xmm30

// CHECK: vucomish  268435456(%rbp,%r14,8), %xmm30
// CHECK: encoding: [0x62,0x25,0x7c,0x08,0x2e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vucomish  268435456(%rbp,%r14,8), %xmm30

// CHECK: vucomish  291(%r8,%rax,4), %xmm30
// CHECK: encoding: [0x62,0x45,0x7c,0x08,0x2e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vucomish  291(%r8,%rax,4), %xmm30

// CHECK: vucomish  (%rip), %xmm30
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2e,0x35,0x00,0x00,0x00,0x00]
          vucomish  (%rip), %xmm30

// CHECK: vucomish  -64(,%rbp,2), %xmm30
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2e,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vucomish  -64(,%rbp,2), %xmm30

// CHECK: vucomish  254(%rcx), %xmm30
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2e,0x71,0x7f]
          vucomish  254(%rcx), %xmm30

// CHECK: vucomish  -256(%rdx), %xmm30
// CHECK: encoding: [0x62,0x65,0x7c,0x08,0x2e,0x72,0x80]
          vucomish  -256(%rdx), %xmm30

