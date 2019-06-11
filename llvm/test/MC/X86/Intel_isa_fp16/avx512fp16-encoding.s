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

// CHECK: vmovsh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x10,0xf0]
          vmovsh %xmm24, %xmm23, %xmm22

// CHECK: vmovsh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x10,0xf0]
          vmovsh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vmovsh %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x85,0x46,0x87,0x10,0xf0]
          vmovsh %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vmovsh  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x10,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovsh  268435456(%rbp,%r14,8), %xmm22

// CHECK: vmovsh  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x10,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovsh  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vmovsh  (%rip), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x10,0x35,0x00,0x00,0x00,0x00]
          vmovsh  (%rip), %xmm22

// CHECK: vmovsh  -64(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x10,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vmovsh  -64(,%rbp,2), %xmm22

// CHECK: vmovsh  254(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x10,0x71,0x7f]
          vmovsh  254(%rcx), %xmm22 {%k7} {z}

// CHECK: vmovsh  -256(%rdx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x10,0x72,0x80]
          vmovsh  -256(%rdx), %xmm22 {%k7} {z}

// CHECK: vmovsh  %xmm22, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x11,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovsh  %xmm22, 268435456(%rbp,%r14,8)

// CHECK: vmovsh  %xmm22, 291(%r8,%rax,4) {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x11,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovsh  %xmm22, 291(%r8,%rax,4) {%k7}

// CHECK: vmovsh  %xmm22, (%rip)
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x11,0x35,0x00,0x00,0x00,0x00]
          vmovsh  %xmm22, (%rip)

// CHECK: vmovsh  %xmm22, -64(,%rbp,2)
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x11,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vmovsh  %xmm22, -64(,%rbp,2)

// CHECK: vmovsh  %xmm22, 254(%rcx) {%k7}
// CHECK: encoding: [0x62,0xe5,0x7e,0x0f,0x11,0x71,0x7f]
          vmovsh  %xmm22, 254(%rcx) {%k7}

// CHECK: vmovsh  %xmm22, -256(%rdx) {%k7}
// CHECK: encoding: [0x62,0xe5,0x7e,0x0f,0x11,0x72,0x80]
          vmovsh  %xmm22, -256(%rdx) {%k7}

// CHECK: vmovw %r12, %xmm22
// CHECK: encoding: [0x62,0xc5,0xfd,0x08,0x6e,0xf4]
          vmovw %r12, %xmm22

// CHECK: vmovw %xmm22, %r12
// CHECK: encoding: [0x62,0xc5,0xfd,0x08,0x7e,0xf4]
          vmovw %xmm22, %r12

// CHECK: vmovw %r12d, %xmm22
// CHECK: encoding: [0x62,0xc5,0x7d,0x08,0x6e,0xf4]
          vmovw %r12d, %xmm22

// CHECK: vmovw %xmm22, %r12d
// CHECK: encoding: [0x62,0xc5,0x7d,0x08,0x7e,0xf4]
          vmovw %xmm22, %r12d

// CHECK: vmovw  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x6e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovw  268435456(%rbp,%r14,8), %xmm22

// CHECK: vmovw  291(%r8,%rax,4), %xmm22
// CHECK: encoding: [0x62,0xc5,0x7d,0x08,0x6e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovw  291(%r8,%rax,4), %xmm22

// CHECK: vmovw  (%rip), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x6e,0x35,0x00,0x00,0x00,0x00]
          vmovw  (%rip), %xmm22

// CHECK: vmovw  -64(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x6e,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vmovw  -64(,%rbp,2), %xmm22

// CHECK: vmovw  254(%rcx), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x6e,0x71,0x7f]
          vmovw  254(%rcx), %xmm22

// CHECK: vmovw  -256(%rdx), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x6e,0x72,0x80]
          vmovw  -256(%rdx), %xmm22

// CHECK: vmovw  %xmm22, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmovw  %xmm22, 268435456(%rbp,%r14,8)

// CHECK: vmovw  %xmm22, 291(%r8,%rax,4)
// CHECK: encoding: [0x62,0xc5,0x7d,0x08,0x7e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmovw  %xmm22, 291(%r8,%rax,4)

// CHECK: vmovw  %xmm22, (%rip)
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7e,0x35,0x00,0x00,0x00,0x00]
          vmovw  %xmm22, (%rip)

// CHECK: vmovw  %xmm22, -64(,%rbp,2)
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7e,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vmovw  %xmm22, -64(,%rbp,2)

// CHECK: vmovw  %xmm22, 254(%rcx)
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7e,0x71,0x7f]
          vmovw  %xmm22, 254(%rcx)

// CHECK: vmovw  %xmm22, -256(%rdx)
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7e,0x72,0x80]
          vmovw  %xmm22, -256(%rdx)

// CHECK: vcvtph2psx %ymm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x48,0x13,0xf7]
          vcvtph2psx %ymm23, %zmm22

// CHECK: vcvtph2psx {sae}, %ymm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x18,0x13,0xf7]
          vcvtph2psx {sae}, %ymm23, %zmm22

// CHECK: vcvtph2psx %ymm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa6,0x7d,0x4f,0x13,0xf7]
          vcvtph2psx %ymm23, %zmm22 {%k7}

// CHECK: vcvtph2psx {sae}, %ymm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa6,0x7d,0x9f,0x13,0xf7]
          vcvtph2psx {sae}, %ymm23, %zmm22 {%k7} {z}

// CHECK: vcvtph2psx  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x48,0x13,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2psx  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvtph2psx  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x7d,0x4f,0x13,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2psx  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvtph2psx  (%rip){1to16}, %zmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x58,0x13,0x35,0x00,0x00,0x00,0x00]
          vcvtph2psx  (%rip){1to16}, %zmm22

// CHECK: vcvtph2psx  -1024(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x48,0x13,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtph2psx  -1024(,%rbp,2), %zmm22

// CHECK: vcvtph2psx  4064(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xcf,0x13,0x71,0x7f]
          vcvtph2psx  4064(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvtph2psx  -256(%rdx){1to16}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xdf,0x13,0x72,0x80]
          vcvtph2psx  -256(%rdx){1to16}, %zmm22 {%k7} {z}

// CHECK: vcvtps2phx %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x1d,0xf7]
          vcvtps2phx %zmm23, %ymm22

// CHECK: vcvtps2phx {rn-sae}, %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x1d,0xf7]
          vcvtps2phx {rn-sae}, %zmm23, %ymm22

// CHECK: vcvtps2phx %zmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x1d,0xf7]
          vcvtps2phx %zmm23, %ymm22 {%k7}

// CHECK: vcvtps2phx {rz-sae}, %zmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xff,0x1d,0xf7]
          vcvtps2phx {rz-sae}, %zmm23, %ymm22 {%k7} {z}

// CHECK: vcvtps2phx  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x1d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtps2phx  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtps2phx  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x1d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtps2phx  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtps2phx  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x1d,0x35,0x00,0x00,0x00,0x00]
          vcvtps2phx  (%rip){1to16}, %ymm22

// CHECK: vcvtps2phx  -2048(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x1d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtps2phx  -2048(,%rbp,2), %ymm22

// CHECK: vcvtps2phx  8128(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x1d,0x71,0x7f]
          vcvtps2phx  8128(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtps2phx  -512(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x1d,0x72,0x80]
          vcvtps2phx  -512(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vcvtpd2ph %zmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x5a,0xf7]
          vcvtpd2ph %zmm23, %xmm22

// CHECK: vcvtpd2ph {rn-sae}, %zmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xfd,0x18,0x5a,0xf7]
          vcvtpd2ph {rn-sae}, %zmm23, %xmm22

// CHECK: vcvtpd2ph %zmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0xfd,0x4f,0x5a,0xf7]
          vcvtpd2ph %zmm23, %xmm22 {%k7}

// CHECK: vcvtpd2ph {rz-sae}, %zmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0xfd,0xff,0x5a,0xf7]
          vcvtpd2ph {rz-sae}, %zmm23, %xmm22 {%k7} {z}

// CHECK: vcvtpd2phz  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0xfd,0x48,0x5a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtpd2phz  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtpd2phz  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xfd,0x4f,0x5a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtpd2phz  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtpd2ph  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0xfd,0x58,0x5a,0x35,0x00,0x00,0x00,0x00]
          vcvtpd2ph  (%rip){1to8}, %xmm22

// CHECK: vcvtpd2phz  -2048(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0xfd,0x48,0x5a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtpd2phz  -2048(,%rbp,2), %xmm22

// CHECK: vcvtpd2phz  8128(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xfd,0xcf,0x5a,0x71,0x7f]
          vcvtpd2phz  8128(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtpd2ph  -1024(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xfd,0xdf,0x5a,0x72,0x80]
          vcvtpd2ph  -1024(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtph2pd %xmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x5a,0xf7]
          vcvtph2pd %xmm23, %zmm22

// CHECK: vcvtph2pd {sae}, %xmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x5a,0xf7]
          vcvtph2pd {sae}, %xmm23, %zmm22

// CHECK: vcvtph2pd %xmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x5a,0xf7]
          vcvtph2pd %xmm23, %zmm22 {%k7}

// CHECK: vcvtph2pd {sae}, %xmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x9f,0x5a,0xf7]
          vcvtph2pd {sae}, %xmm23, %zmm22 {%k7} {z}

// CHECK: vcvtph2pd  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x5a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2pd  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvtph2pd  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x5a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2pd  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvtph2pd  (%rip){1to8}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x5a,0x35,0x00,0x00,0x00,0x00]
          vcvtph2pd  (%rip){1to8}, %zmm22

// CHECK: vcvtph2pd  -512(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x5a,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2pd  -512(,%rbp,2), %zmm22

// CHECK: vcvtph2pd  2032(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x5a,0x71,0x7f]
          vcvtph2pd  2032(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvtph2pd  -256(%rdx){1to8}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x5a,0x72,0x80]
          vcvtph2pd  -256(%rdx){1to8}, %zmm22 {%k7} {z}

// CHECK: vcvtph2uw %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x7d,0xf7]
          vcvtph2uw %zmm23, %zmm22

// CHECK: vcvtph2uw {rn-sae}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x7d,0xf7]
          vcvtph2uw {rn-sae}, %zmm23, %zmm22

// CHECK: vcvtph2uw %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x7d,0xf7]
          vcvtph2uw %zmm23, %zmm22 {%k7}

// CHECK: vcvtph2uw {rz-sae}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xff,0x7d,0xf7]
          vcvtph2uw {rz-sae}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vcvtph2uw  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2uw  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvtph2uw  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2uw  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvtph2uw  (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtph2uw  (%rip){1to32}, %zmm22

// CHECK: vcvtph2uw  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x7d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtph2uw  -2048(,%rbp,2), %zmm22

// CHECK: vcvtph2uw  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x7d,0x71,0x7f]
          vcvtph2uw  8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvtph2uw  -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x7d,0x72,0x80]
          vcvtph2uw  -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vcvtph2w %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7d,0xf7]
          vcvtph2w %zmm23, %zmm22

// CHECK: vcvtph2w {rn-sae}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x7d,0xf7]
          vcvtph2w {rn-sae}, %zmm23, %zmm22

// CHECK: vcvtph2w %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x7d,0xf7]
          vcvtph2w %zmm23, %zmm22 {%k7}

// CHECK: vcvtph2w {rz-sae}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xff,0x7d,0xf7]
          vcvtph2w {rz-sae}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vcvtph2w  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2w  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvtph2w  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2w  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvtph2w  (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtph2w  (%rip){1to32}, %zmm22

// CHECK: vcvtph2w  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x7d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtph2w  -2048(,%rbp,2), %zmm22

// CHECK: vcvtph2w  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x7d,0x71,0x7f]
          vcvtph2w  8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvtph2w  -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x7d,0x72,0x80]
          vcvtph2w  -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vcvttph2uw %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x7c,0xf7]
          vcvttph2uw %zmm23, %zmm22

// CHECK: vcvttph2uw {sae}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x7c,0xf7]
          vcvttph2uw {sae}, %zmm23, %zmm22

// CHECK: vcvttph2uw %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x7c,0xf7]
          vcvttph2uw %zmm23, %zmm22 {%k7}

// CHECK: vcvttph2uw {sae}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x9f,0x7c,0xf7]
          vcvttph2uw {sae}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vcvttph2uw  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x7c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2uw  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvttph2uw  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x7c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2uw  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvttph2uw  (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x7c,0x35,0x00,0x00,0x00,0x00]
          vcvttph2uw  (%rip){1to32}, %zmm22

// CHECK: vcvttph2uw  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x7c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvttph2uw  -2048(,%rbp,2), %zmm22

// CHECK: vcvttph2uw  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x7c,0x71,0x7f]
          vcvttph2uw  8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvttph2uw  -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x7c,0x72,0x80]
          vcvttph2uw  -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vcvttph2w %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7c,0xf7]
          vcvttph2w %zmm23, %zmm22

// CHECK: vcvttph2w {sae}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x7c,0xf7]
          vcvttph2w {sae}, %zmm23, %zmm22

// CHECK: vcvttph2w %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x7c,0xf7]
          vcvttph2w %zmm23, %zmm22 {%k7}

// CHECK: vcvttph2w {sae}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0x9f,0x7c,0xf7]
          vcvttph2w {sae}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vcvttph2w  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2w  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvttph2w  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x7c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2w  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvttph2w  (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x7c,0x35,0x00,0x00,0x00,0x00]
          vcvttph2w  (%rip){1to32}, %zmm22

// CHECK: vcvttph2w  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x7c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvttph2w  -2048(,%rbp,2), %zmm22

// CHECK: vcvttph2w  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x7c,0x71,0x7f]
          vcvttph2w  8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvttph2w  -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x7c,0x72,0x80]
          vcvttph2w  -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vcvtuw2ph %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x7d,0xf7]
          vcvtuw2ph %zmm23, %zmm22

// CHECK: vcvtuw2ph {rn-sae}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x18,0x7d,0xf7]
          vcvtuw2ph {rn-sae}, %zmm23, %zmm22

// CHECK: vcvtuw2ph %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7f,0x4f,0x7d,0xf7]
          vcvtuw2ph %zmm23, %zmm22 {%k7}

// CHECK: vcvtuw2ph {rz-sae}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7f,0xff,0x7d,0xf7]
          vcvtuw2ph {rz-sae}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vcvtuw2ph  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtuw2ph  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvtuw2ph  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7f,0x4f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtuw2ph  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvtuw2ph  (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x58,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtuw2ph  (%rip){1to32}, %zmm22

// CHECK: vcvtuw2ph  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x7d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtuw2ph  -2048(,%rbp,2), %zmm22

// CHECK: vcvtuw2ph  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x7d,0x71,0x7f]
          vcvtuw2ph  8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvtuw2ph  -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0xdf,0x7d,0x72,0x80]
          vcvtuw2ph  -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vcvtw2ph %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x7d,0xf7]
          vcvtw2ph %zmm23, %zmm22

// CHECK: vcvtw2ph {rn-sae}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x18,0x7d,0xf7]
          vcvtw2ph {rn-sae}, %zmm23, %zmm22

// CHECK: vcvtw2ph %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x4f,0x7d,0xf7]
          vcvtw2ph %zmm23, %zmm22 {%k7}

// CHECK: vcvtw2ph {rz-sae}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0xff,0x7d,0xf7]
          vcvtw2ph {rz-sae}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vcvtw2ph  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtw2ph  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvtw2ph  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtw2ph  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvtw2ph  (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x58,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtw2ph  (%rip){1to32}, %zmm22

// CHECK: vcvtw2ph  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x7d,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtw2ph  -2048(,%rbp,2), %zmm22

// CHECK: vcvtw2ph  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x7d,0x71,0x7f]
          vcvtw2ph  8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvtw2ph  -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xdf,0x7d,0x72,0x80]
          vcvtw2ph  -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vcvtdq2ph %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x5b,0xf7]
          vcvtdq2ph %zmm23, %ymm22

// CHECK: vcvtdq2ph {rn-sae}, %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x5b,0xf7]
          vcvtdq2ph {rn-sae}, %zmm23, %ymm22

// CHECK: vcvtdq2ph %zmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x5b,0xf7]
          vcvtdq2ph %zmm23, %ymm22 {%k7}

// CHECK: vcvtdq2ph {rz-sae}, %zmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xff,0x5b,0xf7]
          vcvtdq2ph {rz-sae}, %zmm23, %ymm22 {%k7} {z}

// CHECK: vcvtdq2ph  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtdq2ph  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtdq2ph  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtdq2ph  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtdq2ph  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtdq2ph  (%rip){1to16}, %ymm22

// CHECK: vcvtdq2ph  -2048(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x5b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtdq2ph  -2048(,%rbp,2), %ymm22

// CHECK: vcvtdq2ph  8128(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x5b,0x71,0x7f]
          vcvtdq2ph  8128(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtdq2ph  -512(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x5b,0x72,0x80]
          vcvtdq2ph  -512(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vcvtph2dq %ymm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x5b,0xf7]
          vcvtph2dq %ymm23, %zmm22

// CHECK: vcvtph2dq {rn-sae}, %ymm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x5b,0xf7]
          vcvtph2dq {rn-sae}, %ymm23, %zmm22

// CHECK: vcvtph2dq %ymm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x5b,0xf7]
          vcvtph2dq %ymm23, %zmm22 {%k7}

// CHECK: vcvtph2dq {rz-sae}, %ymm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xff,0x5b,0xf7]
          vcvtph2dq {rz-sae}, %ymm23, %zmm22 {%k7} {z}

// CHECK: vcvtph2dq  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2dq  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvtph2dq  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2dq  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvtph2dq  (%rip){1to16}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2dq  (%rip){1to16}, %zmm22

// CHECK: vcvtph2dq  -1024(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x5b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtph2dq  -1024(,%rbp,2), %zmm22

// CHECK: vcvtph2dq  4064(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x5b,0x71,0x7f]
          vcvtph2dq  4064(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvtph2dq  -256(%rdx){1to16}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x5b,0x72,0x80]
          vcvtph2dq  -256(%rdx){1to16}, %zmm22 {%k7} {z}

// CHECK: vcvtph2udq %ymm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x79,0xf7]
          vcvtph2udq %ymm23, %zmm22

// CHECK: vcvtph2udq {rn-sae}, %ymm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x79,0xf7]
          vcvtph2udq {rn-sae}, %ymm23, %zmm22

// CHECK: vcvtph2udq %ymm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x79,0xf7]
          vcvtph2udq %ymm23, %zmm22 {%k7}

// CHECK: vcvtph2udq {rz-sae}, %ymm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xff,0x79,0xf7]
          vcvtph2udq {rz-sae}, %ymm23, %zmm22 {%k7} {z}

// CHECK: vcvtph2udq  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x79,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2udq  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvtph2udq  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x79,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2udq  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvtph2udq  (%rip){1to16}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x79,0x35,0x00,0x00,0x00,0x00]
          vcvtph2udq  (%rip){1to16}, %zmm22

// CHECK: vcvtph2udq  -1024(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x79,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtph2udq  -1024(,%rbp,2), %zmm22

// CHECK: vcvtph2udq  4064(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x79,0x71,0x7f]
          vcvtph2udq  4064(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvtph2udq  -256(%rdx){1to16}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x79,0x72,0x80]
          vcvtph2udq  -256(%rdx){1to16}, %zmm22 {%k7} {z}

// CHECK: vcvttph2dq %ymm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x5b,0xf7]
          vcvttph2dq %ymm23, %zmm22

// CHECK: vcvttph2dq {sae}, %ymm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x18,0x5b,0xf7]
          vcvttph2dq {sae}, %ymm23, %zmm22

// CHECK: vcvttph2dq %ymm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x4f,0x5b,0xf7]
          vcvttph2dq %ymm23, %zmm22 {%k7}

// CHECK: vcvttph2dq {sae}, %ymm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0x9f,0x5b,0xf7]
          vcvttph2dq {sae}, %ymm23, %zmm22 {%k7} {z}

// CHECK: vcvttph2dq  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x48,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2dq  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvttph2dq  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x4f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2dq  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvttph2dq  (%rip){1to16}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x58,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvttph2dq  (%rip){1to16}, %zmm22

// CHECK: vcvttph2dq  -1024(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x48,0x5b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvttph2dq  -1024(,%rbp,2), %zmm22

// CHECK: vcvttph2dq  4064(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xcf,0x5b,0x71,0x7f]
          vcvttph2dq  4064(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvttph2dq  -256(%rdx){1to16}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xdf,0x5b,0x72,0x80]
          vcvttph2dq  -256(%rdx){1to16}, %zmm22 {%k7} {z}

// CHECK: vcvttph2udq %ymm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x78,0xf7]
          vcvttph2udq %ymm23, %zmm22

// CHECK: vcvttph2udq {sae}, %ymm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x78,0xf7]
          vcvttph2udq {sae}, %ymm23, %zmm22

// CHECK: vcvttph2udq %ymm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x78,0xf7]
          vcvttph2udq %ymm23, %zmm22 {%k7}

// CHECK: vcvttph2udq {sae}, %ymm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x9f,0x78,0xf7]
          vcvttph2udq {sae}, %ymm23, %zmm22 {%k7} {z}

// CHECK: vcvttph2udq  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x78,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2udq  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvttph2udq  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x78,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2udq  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvttph2udq  (%rip){1to16}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x78,0x35,0x00,0x00,0x00,0x00]
          vcvttph2udq  (%rip){1to16}, %zmm22

// CHECK: vcvttph2udq  -1024(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x78,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvttph2udq  -1024(,%rbp,2), %zmm22

// CHECK: vcvttph2udq  4064(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x78,0x71,0x7f]
          vcvttph2udq  4064(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvttph2udq  -256(%rdx){1to16}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x78,0x72,0x80]
          vcvttph2udq  -256(%rdx){1to16}, %zmm22 {%k7} {z}

// CHECK: vcvtudq2ph %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x7a,0xf7]
          vcvtudq2ph %zmm23, %ymm22

// CHECK: vcvtudq2ph {rn-sae}, %zmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x18,0x7a,0xf7]
          vcvtudq2ph {rn-sae}, %zmm23, %ymm22

// CHECK: vcvtudq2ph %zmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7f,0x4f,0x7a,0xf7]
          vcvtudq2ph %zmm23, %ymm22 {%k7}

// CHECK: vcvtudq2ph {rz-sae}, %zmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7f,0xff,0x7a,0xf7]
          vcvtudq2ph {rz-sae}, %zmm23, %ymm22 {%k7} {z}

// CHECK: vcvtudq2ph  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x48,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtudq2ph  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtudq2ph  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7f,0x4f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtudq2ph  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtudq2ph  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x58,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvtudq2ph  (%rip){1to16}, %ymm22

// CHECK: vcvtudq2ph  -2048(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x48,0x7a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtudq2ph  -2048(,%rbp,2), %ymm22

// CHECK: vcvtudq2ph  8128(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0xcf,0x7a,0x71,0x7f]
          vcvtudq2ph  8128(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtudq2ph  -512(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0xdf,0x7a,0x72,0x80]
          vcvtudq2ph  -512(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vcvtph2qq %xmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7b,0xf7]
          vcvtph2qq %xmm23, %zmm22

// CHECK: vcvtph2qq {rn-sae}, %xmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x7b,0xf7]
          vcvtph2qq {rn-sae}, %xmm23, %zmm22

// CHECK: vcvtph2qq %xmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x7b,0xf7]
          vcvtph2qq %xmm23, %zmm22 {%k7}

// CHECK: vcvtph2qq {rz-sae}, %xmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xff,0x7b,0xf7]
          vcvtph2qq {rz-sae}, %xmm23, %zmm22 {%k7} {z}

// CHECK: vcvtph2qq  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2qq  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvtph2qq  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x7b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2qq  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvtph2qq  (%rip){1to8}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x7b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2qq  (%rip){1to8}, %zmm22

// CHECK: vcvtph2qq  -512(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x7b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2qq  -512(,%rbp,2), %zmm22

// CHECK: vcvtph2qq  2032(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x7b,0x71,0x7f]
          vcvtph2qq  2032(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvtph2qq  -256(%rdx){1to8}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x7b,0x72,0x80]
          vcvtph2qq  -256(%rdx){1to8}, %zmm22 {%k7} {z}

// CHECK: vcvtph2uqq %xmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x79,0xf7]
          vcvtph2uqq %xmm23, %zmm22

// CHECK: vcvtph2uqq {rn-sae}, %xmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x79,0xf7]
          vcvtph2uqq {rn-sae}, %xmm23, %zmm22

// CHECK: vcvtph2uqq %xmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x79,0xf7]
          vcvtph2uqq %xmm23, %zmm22 {%k7}

// CHECK: vcvtph2uqq {rz-sae}, %xmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xff,0x79,0xf7]
          vcvtph2uqq {rz-sae}, %xmm23, %zmm22 {%k7} {z}

// CHECK: vcvtph2uqq  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x79,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2uqq  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvtph2uqq  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x79,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2uqq  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvtph2uqq  (%rip){1to8}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x79,0x35,0x00,0x00,0x00,0x00]
          vcvtph2uqq  (%rip){1to8}, %zmm22

// CHECK: vcvtph2uqq  -512(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x79,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2uqq  -512(,%rbp,2), %zmm22

// CHECK: vcvtph2uqq  2032(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x79,0x71,0x7f]
          vcvtph2uqq  2032(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvtph2uqq  -256(%rdx){1to8}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x79,0x72,0x80]
          vcvtph2uqq  -256(%rdx){1to8}, %zmm22 {%k7} {z}

// CHECK: vcvtqq2ph %zmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x5b,0xf7]
          vcvtqq2ph %zmm23, %xmm22

// CHECK: vcvtqq2ph {rn-sae}, %zmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xfc,0x18,0x5b,0xf7]
          vcvtqq2ph {rn-sae}, %zmm23, %xmm22

// CHECK: vcvtqq2ph %zmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0xfc,0x4f,0x5b,0xf7]
          vcvtqq2ph %zmm23, %xmm22 {%k7}

// CHECK: vcvtqq2ph {rz-sae}, %zmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0xfc,0xff,0x5b,0xf7]
          vcvtqq2ph {rz-sae}, %zmm23, %xmm22 {%k7} {z}

// CHECK: vcvtqq2phz  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0xfc,0x48,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtqq2phz  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtqq2phz  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xfc,0x4f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtqq2phz  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtqq2ph  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0xfc,0x58,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtqq2ph  (%rip){1to8}, %xmm22

// CHECK: vcvtqq2phz  -2048(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0xfc,0x48,0x5b,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtqq2phz  -2048(,%rbp,2), %xmm22

// CHECK: vcvtqq2phz  8128(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xfc,0xcf,0x5b,0x71,0x7f]
          vcvtqq2phz  8128(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtqq2ph  -1024(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xfc,0xdf,0x5b,0x72,0x80]
          vcvtqq2ph  -1024(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvttph2qq %xmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7a,0xf7]
          vcvttph2qq %xmm23, %zmm22

// CHECK: vcvttph2qq {sae}, %xmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x7a,0xf7]
          vcvttph2qq {sae}, %xmm23, %zmm22

// CHECK: vcvttph2qq %xmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x7a,0xf7]
          vcvttph2qq %xmm23, %zmm22 {%k7}

// CHECK: vcvttph2qq {sae}, %xmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0x9f,0x7a,0xf7]
          vcvttph2qq {sae}, %xmm23, %zmm22 {%k7} {z}

// CHECK: vcvttph2qq  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2qq  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvttph2qq  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2qq  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvttph2qq  (%rip){1to8}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvttph2qq  (%rip){1to8}, %zmm22

// CHECK: vcvttph2qq  -512(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x7a,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2qq  -512(,%rbp,2), %zmm22

// CHECK: vcvttph2qq  2032(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x7a,0x71,0x7f]
          vcvttph2qq  2032(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvttph2qq  -256(%rdx){1to8}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x7a,0x72,0x80]
          vcvttph2qq  -256(%rdx){1to8}, %zmm22 {%k7} {z}

// CHECK: vcvttph2uqq %xmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x78,0xf7]
          vcvttph2uqq %xmm23, %zmm22

// CHECK: vcvttph2uqq {sae}, %xmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x18,0x78,0xf7]
          vcvttph2uqq {sae}, %xmm23, %zmm22

// CHECK: vcvttph2uqq %xmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x4f,0x78,0xf7]
          vcvttph2uqq %xmm23, %zmm22 {%k7}

// CHECK: vcvttph2uqq {sae}, %xmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0x9f,0x78,0xf7]
          vcvttph2uqq {sae}, %xmm23, %zmm22 {%k7} {z}

// CHECK: vcvttph2uqq  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x48,0x78,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2uqq  268435456(%rbp,%r14,8), %zmm22

// CHECK: vcvttph2uqq  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x4f,0x78,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2uqq  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vcvttph2uqq  (%rip){1to8}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x58,0x78,0x35,0x00,0x00,0x00,0x00]
          vcvttph2uqq  (%rip){1to8}, %zmm22

// CHECK: vcvttph2uqq  -512(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x48,0x78,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2uqq  -512(,%rbp,2), %zmm22

// CHECK: vcvttph2uqq  2032(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xcf,0x78,0x71,0x7f]
          vcvttph2uqq  2032(%rcx), %zmm22 {%k7} {z}

// CHECK: vcvttph2uqq  -256(%rdx){1to8}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xdf,0x78,0x72,0x80]
          vcvttph2uqq  -256(%rdx){1to8}, %zmm22 {%k7} {z}

// CHECK: vcvtuqq2ph %zmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xff,0x48,0x7a,0xf7]
          vcvtuqq2ph %zmm23, %xmm22

// CHECK: vcvtuqq2ph {rn-sae}, %zmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xff,0x18,0x7a,0xf7]
          vcvtuqq2ph {rn-sae}, %zmm23, %xmm22

// CHECK: vcvtuqq2ph %zmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0xff,0x4f,0x7a,0xf7]
          vcvtuqq2ph %zmm23, %xmm22 {%k7}

// CHECK: vcvtuqq2ph {rz-sae}, %zmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0xff,0xff,0x7a,0xf7]
          vcvtuqq2ph {rz-sae}, %zmm23, %xmm22 {%k7} {z}

// CHECK: vcvtuqq2phz  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0xff,0x48,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtuqq2phz  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtuqq2phz  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xff,0x4f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtuqq2phz  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtuqq2ph  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0xff,0x58,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvtuqq2ph  (%rip){1to8}, %xmm22

// CHECK: vcvtuqq2phz  -2048(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0xff,0x48,0x7a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vcvtuqq2phz  -2048(,%rbp,2), %xmm22

// CHECK: vcvtuqq2phz  8128(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xff,0xcf,0x7a,0x71,0x7f]
          vcvtuqq2phz  8128(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtuqq2ph  -1024(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xff,0xdf,0x7a,0x72,0x80]
          vcvtuqq2ph  -1024(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtsh2si %xmm22, %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x2d,0xce]
          vcvtsh2si %xmm22, %ecx

// CHECK: vcvtsh2si {rn-sae}, %xmm22, %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x18,0x2d,0xce]
          vcvtsh2si {rn-sae}, %xmm22, %ecx

// CHECK: vcvtsh2si {rz-sae}, %xmm22, %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x78,0x2d,0xce]
          vcvtsh2si {rz-sae}, %xmm22, %ecx

// CHECK: vcvtsh2si %xmm22, %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x2d,0xce]
          vcvtsh2si %xmm22, %r9

// CHECK: vcvtsh2si {rn-sae}, %xmm22, %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x18,0x2d,0xce]
          vcvtsh2si {rn-sae}, %xmm22, %r9

// CHECK: vcvtsh2si {rz-sae}, %xmm22, %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x78,0x2d,0xce]
          vcvtsh2si {rz-sae}, %xmm22, %r9

// CHECK: vcvtsh2si  268435456(%rbp,%r14,8), %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x2d,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvtsh2si  268435456(%rbp,%r14,8), %ecx

// CHECK: vcvtsh2si  291(%r8,%rax,4), %ecx
// CHECK: encoding: [0x62,0xd5,0x7e,0x08,0x2d,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvtsh2si  291(%r8,%rax,4), %ecx

// CHECK: vcvtsh2si  (%rip), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2d,0x0d,0x00,0x00,0x00,0x00]
          vcvtsh2si  (%rip), %ecx

// CHECK: vcvtsh2si  -64(,%rbp,2), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2d,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvtsh2si  -64(,%rbp,2), %ecx

// CHECK: vcvtsh2si  254(%rcx), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2d,0x49,0x7f]
          vcvtsh2si  254(%rcx), %ecx

// CHECK: vcvtsh2si  -256(%rdx), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2d,0x4a,0x80]
          vcvtsh2si  -256(%rdx), %ecx

// CHECK: vcvtsh2si  268435456(%rbp,%r14,8), %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x2d,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvtsh2si  268435456(%rbp,%r14,8), %r9

// CHECK: vcvtsh2si  291(%r8,%rax,4), %r9
// CHECK: encoding: [0x62,0x55,0xfe,0x08,0x2d,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvtsh2si  291(%r8,%rax,4), %r9

// CHECK: vcvtsh2si  (%rip), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2d,0x0d,0x00,0x00,0x00,0x00]
          vcvtsh2si  (%rip), %r9

// CHECK: vcvtsh2si  -64(,%rbp,2), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2d,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvtsh2si  -64(,%rbp,2), %r9

// CHECK: vcvtsh2si  254(%rcx), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2d,0x49,0x7f]
          vcvtsh2si  254(%rcx), %r9

// CHECK: vcvtsh2si  -256(%rdx), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2d,0x4a,0x80]
          vcvtsh2si  -256(%rdx), %r9

// CHECK: vcvtsh2usi %xmm22, %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x79,0xce]
          vcvtsh2usi %xmm22, %ecx

// CHECK: vcvtsh2usi {rn-sae}, %xmm22, %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x18,0x79,0xce]
          vcvtsh2usi {rn-sae}, %xmm22, %ecx

// CHECK: vcvtsh2usi {rz-sae}, %xmm22, %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x78,0x79,0xce]
          vcvtsh2usi {rz-sae}, %xmm22, %ecx

// CHECK: vcvtsh2usi %xmm22, %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x79,0xce]
          vcvtsh2usi %xmm22, %r9

// CHECK: vcvtsh2usi {rn-sae}, %xmm22, %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x18,0x79,0xce]
          vcvtsh2usi {rn-sae}, %xmm22, %r9

// CHECK: vcvtsh2usi {rz-sae}, %xmm22, %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x78,0x79,0xce]
          vcvtsh2usi {rz-sae}, %xmm22, %r9

// CHECK: vcvtsh2usi  268435456(%rbp,%r14,8), %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x79,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvtsh2usi  268435456(%rbp,%r14,8), %ecx

// CHECK: vcvtsh2usi  291(%r8,%rax,4), %ecx
// CHECK: encoding: [0x62,0xd5,0x7e,0x08,0x79,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvtsh2usi  291(%r8,%rax,4), %ecx

// CHECK: vcvtsh2usi  (%rip), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x79,0x0d,0x00,0x00,0x00,0x00]
          vcvtsh2usi  (%rip), %ecx

// CHECK: vcvtsh2usi  -64(,%rbp,2), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x79,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvtsh2usi  -64(,%rbp,2), %ecx

// CHECK: vcvtsh2usi  254(%rcx), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x79,0x49,0x7f]
          vcvtsh2usi  254(%rcx), %ecx

// CHECK: vcvtsh2usi  -256(%rdx), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x79,0x4a,0x80]
          vcvtsh2usi  -256(%rdx), %ecx

// CHECK: vcvtsh2usi  268435456(%rbp,%r14,8), %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x79,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvtsh2usi  268435456(%rbp,%r14,8), %r9

// CHECK: vcvtsh2usi  291(%r8,%rax,4), %r9
// CHECK: encoding: [0x62,0x55,0xfe,0x08,0x79,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvtsh2usi  291(%r8,%rax,4), %r9

// CHECK: vcvtsh2usi  (%rip), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x79,0x0d,0x00,0x00,0x00,0x00]
          vcvtsh2usi  (%rip), %r9

// CHECK: vcvtsh2usi  -64(,%rbp,2), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x79,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvtsh2usi  -64(,%rbp,2), %r9

// CHECK: vcvtsh2usi  254(%rcx), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x79,0x49,0x7f]
          vcvtsh2usi  254(%rcx), %r9

// CHECK: vcvtsh2usi  -256(%rdx), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x79,0x4a,0x80]
          vcvtsh2usi  -256(%rdx), %r9

// CHECK: vcvtsi2sh %r9, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0xc6,0x00,0x2a,0xf1]
          vcvtsi2sh %r9, %xmm23, %xmm22

// CHECK: vcvtsi2sh %r9, {rn-sae}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0xc6,0x10,0x2a,0xf1]
          vcvtsi2sh %r9, {rn-sae}, %xmm23, %xmm22

// CHECK: vcvtsi2sh %r9, {rz-sae}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0xc6,0x70,0x2a,0xf1]
          vcvtsi2sh %r9, {rz-sae}, %xmm23, %xmm22

// CHECK: vcvtsi2sh %ecx, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x2a,0xf1]
          vcvtsi2sh %ecx, %xmm23, %xmm22

// CHECK: vcvtsi2sh %ecx, {rn-sae}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x10,0x2a,0xf1]
          vcvtsi2sh %ecx, {rn-sae}, %xmm23, %xmm22

// CHECK: vcvtsi2sh %ecx, {rz-sae}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x70,0x2a,0xf1]
          vcvtsi2sh %ecx, {rz-sae}, %xmm23, %xmm22

// CHECK: vcvtsi2shl  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x2a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtsi2shl  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vcvtsi2shl  291(%r8,%rax,4), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0x46,0x00,0x2a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtsi2shl  291(%r8,%rax,4), %xmm23, %xmm22

// CHECK: vcvtsi2shl  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x2a,0x35,0x00,0x00,0x00,0x00]
          vcvtsi2shl  (%rip), %xmm23, %xmm22

// CHECK: vcvtsi2shl  -128(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x2a,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvtsi2shl  -128(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvtsi2shl  508(%rcx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x2a,0x71,0x7f]
          vcvtsi2shl  508(%rcx), %xmm23, %xmm22

// CHECK: vcvtsi2shl  -512(%rdx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x2a,0x72,0x80]
          vcvtsi2shl  -512(%rdx), %xmm23, %xmm22

// CHECK: vcvtsi2shq  -256(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x2a,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtsi2shq  -256(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvtsi2shq  1016(%rcx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x2a,0x71,0x7f]
          vcvtsi2shq  1016(%rcx), %xmm23, %xmm22

// CHECK: vcvtsi2shq  -1024(%rdx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x2a,0x72,0x80]
          vcvtsi2shq  -1024(%rdx), %xmm23, %xmm22

// CHECK: vcvttsh2si %xmm22, %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x2c,0xce]
          vcvttsh2si %xmm22, %ecx

// CHECK: vcvttsh2si {sae}, %xmm22, %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x18,0x2c,0xce]
          vcvttsh2si {sae}, %xmm22, %ecx

// CHECK: vcvttsh2si %xmm22, %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x2c,0xce]
          vcvttsh2si %xmm22, %r9

// CHECK: vcvttsh2si {sae}, %xmm22, %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x18,0x2c,0xce]
          vcvttsh2si {sae}, %xmm22, %r9

// CHECK: vcvttsh2si  268435456(%rbp,%r14,8), %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x2c,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvttsh2si  268435456(%rbp,%r14,8), %ecx

// CHECK: vcvttsh2si  291(%r8,%rax,4), %ecx
// CHECK: encoding: [0x62,0xd5,0x7e,0x08,0x2c,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvttsh2si  291(%r8,%rax,4), %ecx

// CHECK: vcvttsh2si  (%rip), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2c,0x0d,0x00,0x00,0x00,0x00]
          vcvttsh2si  (%rip), %ecx

// CHECK: vcvttsh2si  -64(,%rbp,2), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2c,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvttsh2si  -64(,%rbp,2), %ecx

// CHECK: vcvttsh2si  254(%rcx), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2c,0x49,0x7f]
          vcvttsh2si  254(%rcx), %ecx

// CHECK: vcvttsh2si  -256(%rdx), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x2c,0x4a,0x80]
          vcvttsh2si  -256(%rdx), %ecx

// CHECK: vcvttsh2si  268435456(%rbp,%r14,8), %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x2c,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvttsh2si  268435456(%rbp,%r14,8), %r9

// CHECK: vcvttsh2si  291(%r8,%rax,4), %r9
// CHECK: encoding: [0x62,0x55,0xfe,0x08,0x2c,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvttsh2si  291(%r8,%rax,4), %r9

// CHECK: vcvttsh2si  (%rip), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2c,0x0d,0x00,0x00,0x00,0x00]
          vcvttsh2si  (%rip), %r9

// CHECK: vcvttsh2si  -64(,%rbp,2), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2c,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvttsh2si  -64(,%rbp,2), %r9

// CHECK: vcvttsh2si  254(%rcx), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2c,0x49,0x7f]
          vcvttsh2si  254(%rcx), %r9

// CHECK: vcvttsh2si  -256(%rdx), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x2c,0x4a,0x80]
          vcvttsh2si  -256(%rdx), %r9

// CHECK: vcvttsh2usi %xmm22, %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x78,0xce]
          vcvttsh2usi %xmm22, %ecx

// CHECK: vcvttsh2usi {sae}, %xmm22, %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x18,0x78,0xce]
          vcvttsh2usi {sae}, %xmm22, %ecx

// CHECK: vcvttsh2usi %xmm22, %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x78,0xce]
          vcvttsh2usi %xmm22, %r9

// CHECK: vcvttsh2usi {sae}, %xmm22, %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x18,0x78,0xce]
          vcvttsh2usi {sae}, %xmm22, %r9

// CHECK: vcvttsh2usi  268435456(%rbp,%r14,8), %ecx
// CHECK: encoding: [0x62,0xb5,0x7e,0x08,0x78,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvttsh2usi  268435456(%rbp,%r14,8), %ecx

// CHECK: vcvttsh2usi  291(%r8,%rax,4), %ecx
// CHECK: encoding: [0x62,0xd5,0x7e,0x08,0x78,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvttsh2usi  291(%r8,%rax,4), %ecx

// CHECK: vcvttsh2usi  (%rip), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x78,0x0d,0x00,0x00,0x00,0x00]
          vcvttsh2usi  (%rip), %ecx

// CHECK: vcvttsh2usi  -64(,%rbp,2), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x78,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvttsh2usi  -64(,%rbp,2), %ecx

// CHECK: vcvttsh2usi  254(%rcx), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x78,0x49,0x7f]
          vcvttsh2usi  254(%rcx), %ecx

// CHECK: vcvttsh2usi  -256(%rdx), %ecx
// CHECK: encoding: [0x62,0xf5,0x7e,0x08,0x78,0x4a,0x80]
          vcvttsh2usi  -256(%rdx), %ecx

// CHECK: vcvttsh2usi  268435456(%rbp,%r14,8), %r9
// CHECK: encoding: [0x62,0x35,0xfe,0x08,0x78,0x8c,0xf5,0x00,0x00,0x00,0x10]
          vcvttsh2usi  268435456(%rbp,%r14,8), %r9

// CHECK: vcvttsh2usi  291(%r8,%rax,4), %r9
// CHECK: encoding: [0x62,0x55,0xfe,0x08,0x78,0x8c,0x80,0x23,0x01,0x00,0x00]
          vcvttsh2usi  291(%r8,%rax,4), %r9

// CHECK: vcvttsh2usi  (%rip), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x78,0x0d,0x00,0x00,0x00,0x00]
          vcvttsh2usi  (%rip), %r9

// CHECK: vcvttsh2usi  -64(,%rbp,2), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x78,0x0c,0x6d,0xc0,0xff,0xff,0xff]
          vcvttsh2usi  -64(,%rbp,2), %r9

// CHECK: vcvttsh2usi  254(%rcx), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x78,0x49,0x7f]
          vcvttsh2usi  254(%rcx), %r9

// CHECK: vcvttsh2usi  -256(%rdx), %r9
// CHECK: encoding: [0x62,0x75,0xfe,0x08,0x78,0x4a,0x80]
          vcvttsh2usi  -256(%rdx), %r9

// CHECK: vcvtusi2sh %r9, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0xc6,0x00,0x7b,0xf1]
          vcvtusi2sh %r9, %xmm23, %xmm22

// CHECK: vcvtusi2sh %r9, {rn-sae}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0xc6,0x10,0x7b,0xf1]
          vcvtusi2sh %r9, {rn-sae}, %xmm23, %xmm22

// CHECK: vcvtusi2sh %r9, {rz-sae}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0xc6,0x70,0x7b,0xf1]
          vcvtusi2sh %r9, {rz-sae}, %xmm23, %xmm22

// CHECK: vcvtusi2sh %ecx, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x7b,0xf1]
          vcvtusi2sh %ecx, %xmm23, %xmm22

// CHECK: vcvtusi2sh %ecx, {rn-sae}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x10,0x7b,0xf1]
          vcvtusi2sh %ecx, {rn-sae}, %xmm23, %xmm22

// CHECK: vcvtusi2sh %ecx, {rz-sae}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x70,0x7b,0xf1]
          vcvtusi2sh %ecx, {rz-sae}, %xmm23, %xmm22

// CHECK: vcvtusi2shl  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x7b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtusi2shl  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vcvtusi2shl  291(%r8,%rax,4), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xc5,0x46,0x00,0x7b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtusi2shl  291(%r8,%rax,4), %xmm23, %xmm22

// CHECK: vcvtusi2shl  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x7b,0x35,0x00,0x00,0x00,0x00]
          vcvtusi2shl  (%rip), %xmm23, %xmm22

// CHECK: vcvtusi2shl  -128(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x7b,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvtusi2shl  -128(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvtusi2shl  508(%rcx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x7b,0x71,0x7f]
          vcvtusi2shl  508(%rcx), %xmm23, %xmm22

// CHECK: vcvtusi2shl  -512(%rdx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x7b,0x72,0x80]
          vcvtusi2shl  -512(%rdx), %xmm23, %xmm22

// CHECK: vcvtusi2shq  -256(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x7b,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtusi2shq  -256(,%rbp,2), %xmm23, %xmm22

// CHECK: vcvtusi2shq  1016(%rcx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x7b,0x71,0x7f]
          vcvtusi2shq  1016(%rcx), %xmm23, %xmm22

// CHECK: vcvtusi2shq  -1024(%rdx), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0xc6,0x00,0x7b,0x72,0x80]
          vcvtusi2shq  -1024(%rdx), %xmm23, %xmm22

// CHECK: vfpclassph $123, %zmm22, %k5
// CHECK: encoding: [0x62,0xb3,0x7c,0x48,0x66,0xee,0x7b]
          vfpclassph $123, %zmm22, %k5

// CHECK: vfpclassph $123, %zmm22, %k5 {%k7}
// CHECK: encoding: [0x62,0xb3,0x7c,0x4f,0x66,0xee,0x7b]
          vfpclassph $123, %zmm22, %k5 {%k7}

// CHECK: vfpclassphz  $123, 268435456(%rbp,%r14,8), %k5
// CHECK: encoding: [0x62,0xb3,0x7c,0x48,0x66,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vfpclassphz  $123, 268435456(%rbp,%r14,8), %k5

// CHECK: vfpclassphz  $123, 291(%r8,%rax,4), %k5 {%k7}
// CHECK: encoding: [0x62,0xd3,0x7c,0x4f,0x66,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vfpclassphz  $123, 291(%r8,%rax,4), %k5 {%k7}

// CHECK: vfpclassph  $123, (%rip){1to32}, %k5
// CHECK: encoding: [0x62,0xf3,0x7c,0x58,0x66,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vfpclassph  $123, (%rip){1to32}, %k5

// CHECK: vfpclassphz  $123, -2048(,%rbp,2), %k5
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x66,0x2c,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vfpclassphz  $123, -2048(,%rbp,2), %k5

// CHECK: vfpclassphz  $123, 8128(%rcx), %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x66,0x69,0x7f,0x7b]
          vfpclassphz  $123, 8128(%rcx), %k5 {%k7}

// CHECK: vfpclassph  $123, -256(%rdx){1to32}, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x5f,0x66,0x6a,0x80,0x7b]
          vfpclassph  $123, -256(%rdx){1to32}, %k5 {%k7}

// CHECK: vfpclasssh $123, %xmm22, %k5
// CHECK: encoding: [0x62,0xb3,0x7c,0x08,0x67,0xee,0x7b]
          vfpclasssh $123, %xmm22, %k5

// CHECK: vfpclasssh $123, %xmm22, %k5 {%k7}
// CHECK: encoding: [0x62,0xb3,0x7c,0x0f,0x67,0xee,0x7b]
          vfpclasssh $123, %xmm22, %k5 {%k7}

// CHECK: vfpclasssh  $123, 268435456(%rbp,%r14,8), %k5
// CHECK: encoding: [0x62,0xb3,0x7c,0x08,0x67,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vfpclasssh  $123, 268435456(%rbp,%r14,8), %k5

// CHECK: vfpclasssh  $123, 291(%r8,%rax,4), %k5 {%k7}
// CHECK: encoding: [0x62,0xd3,0x7c,0x0f,0x67,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vfpclasssh  $123, 291(%r8,%rax,4), %k5 {%k7}

// CHECK: vfpclasssh  $123, (%rip), %k5
// CHECK: encoding: [0x62,0xf3,0x7c,0x08,0x67,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vfpclasssh  $123, (%rip), %k5

// CHECK: vfpclasssh  $123, -64(,%rbp,2), %k5
// CHECK: encoding: [0x62,0xf3,0x7c,0x08,0x67,0x2c,0x6d,0xc0,0xff,0xff,0xff,0x7b]
          vfpclasssh  $123, -64(,%rbp,2), %k5

// CHECK: vfpclasssh  $123, 254(%rcx), %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x0f,0x67,0x69,0x7f,0x7b]
          vfpclasssh  $123, 254(%rcx), %k5 {%k7}

// CHECK: vfpclasssh  $123, -256(%rdx), %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x0f,0x67,0x6a,0x80,0x7b]
          vfpclasssh  $123, -256(%rdx), %k5 {%k7}

// CHECK: vgetexpph %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x48,0x42,0xf7]
          vgetexpph %zmm23, %zmm22

// CHECK: vgetexpph {sae}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x18,0x42,0xf7]
          vgetexpph {sae}, %zmm23, %zmm22

// CHECK: vgetexpph %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa6,0x7d,0x4f,0x42,0xf7]
          vgetexpph %zmm23, %zmm22 {%k7}

// CHECK: vgetexpph {sae}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa6,0x7d,0x9f,0x42,0xf7]
          vgetexpph {sae}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vgetexpph  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x48,0x42,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vgetexpph  268435456(%rbp,%r14,8), %zmm22

// CHECK: vgetexpph  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x7d,0x4f,0x42,0xb4,0x80,0x23,0x01,0x00,0x00]
          vgetexpph  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vgetexpph  (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x58,0x42,0x35,0x00,0x00,0x00,0x00]
          vgetexpph  (%rip){1to32}, %zmm22

// CHECK: vgetexpph  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x48,0x42,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vgetexpph  -2048(,%rbp,2), %zmm22

// CHECK: vgetexpph  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xcf,0x42,0x71,0x7f]
          vgetexpph  8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vgetexpph  -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xdf,0x42,0x72,0x80]
          vgetexpph  -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vgetexpsh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x43,0xf0]
          vgetexpsh %xmm24, %xmm23, %xmm22

// CHECK: vgetexpsh {sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x43,0xf0]
          vgetexpsh {sae}, %xmm24, %xmm23, %xmm22

// CHECK: vgetexpsh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x43,0xf0]
          vgetexpsh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vgetexpsh {sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x97,0x43,0xf0]
          vgetexpsh {sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vgetexpsh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x43,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vgetexpsh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vgetexpsh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x43,0xb4,0x80,0x23,0x01,0x00,0x00]
          vgetexpsh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vgetexpsh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x43,0x35,0x00,0x00,0x00,0x00]
          vgetexpsh  (%rip), %xmm23, %xmm22

// CHECK: vgetexpsh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x43,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vgetexpsh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vgetexpsh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x43,0x71,0x7f]
          vgetexpsh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vgetexpsh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x43,0x72,0x80]
          vgetexpsh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vgetmantph $123, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x26,0xf7,0x7b]
          vgetmantph $123, %zmm23, %zmm22

// CHECK: vgetmantph $123, {sae}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x18,0x26,0xf7,0x7b]
          vgetmantph $123, {sae}, %zmm23, %zmm22

// CHECK: vgetmantph $123, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa3,0x7c,0x4f,0x26,0xf7,0x7b]
          vgetmantph $123, %zmm23, %zmm22 {%k7}

// CHECK: vgetmantph $123, {sae}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa3,0x7c,0x9f,0x26,0xf7,0x7b]
          vgetmantph $123, {sae}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vgetmantph  $123, 268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x26,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vgetmantph  $123, 268435456(%rbp,%r14,8), %zmm22

// CHECK: vgetmantph  $123, 291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x7c,0x4f,0x26,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vgetmantph  $123, 291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vgetmantph  $123, (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x58,0x26,0x35,0x00,0x00,0x00,0x00,0x7b]
          vgetmantph  $123, (%rip){1to32}, %zmm22

// CHECK: vgetmantph  $123, -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x48,0x26,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vgetmantph  $123, -2048(,%rbp,2), %zmm22

// CHECK: vgetmantph  $123, 8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0xcf,0x26,0x71,0x7f,0x7b]
          vgetmantph  $123, 8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vgetmantph  $123, -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0xdf,0x26,0x72,0x80,0x7b]
          vgetmantph  $123, -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vgetmantsh $123, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x83,0x44,0x00,0x27,0xf0,0x7b]
          vgetmantsh $123, %xmm24, %xmm23, %xmm22

// CHECK: vgetmantsh $123, {sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x83,0x44,0x10,0x27,0xf0,0x7b]
          vgetmantsh $123, {sae}, %xmm24, %xmm23, %xmm22

// CHECK: vgetmantsh $123, %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x83,0x44,0x07,0x27,0xf0,0x7b]
          vgetmantsh $123, %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vgetmantsh $123, {sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x83,0x44,0x97,0x27,0xf0,0x7b]
          vgetmantsh $123, {sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vgetmantsh  $123, 268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa3,0x44,0x00,0x27,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vgetmantsh  $123, 268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vgetmantsh  $123, 291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x44,0x07,0x27,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vgetmantsh  $123, 291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vgetmantsh  $123, (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe3,0x44,0x00,0x27,0x35,0x00,0x00,0x00,0x00,0x7b]
          vgetmantsh  $123, (%rip), %xmm23, %xmm22

// CHECK: vgetmantsh  $123, -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe3,0x44,0x00,0x27,0x34,0x6d,0xc0,0xff,0xff,0xff,0x7b]
          vgetmantsh  $123, -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vgetmantsh  $123, 254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x44,0x87,0x27,0x71,0x7f,0x7b]
          vgetmantsh  $123, 254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vgetmantsh  $123, -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x44,0x87,0x27,0x72,0x80,0x7b]
          vgetmantsh  $123, -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vrcpph %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x48,0x4c,0xf7]
          vrcpph %zmm23, %zmm22

// CHECK: vrcpph %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa6,0x7d,0x4f,0x4c,0xf7]
          vrcpph %zmm23, %zmm22 {%k7}

// CHECK: vrcpph %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa6,0x7d,0xcf,0x4c,0xf7]
          vrcpph %zmm23, %zmm22 {%k7} {z}

// CHECK: vrcpph  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x48,0x4c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrcpph  268435456(%rbp,%r14,8), %zmm22

// CHECK: vrcpph  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x7d,0x4f,0x4c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrcpph  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vrcpph  (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x58,0x4c,0x35,0x00,0x00,0x00,0x00]
          vrcpph  (%rip){1to32}, %zmm22

// CHECK: vrcpph  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x48,0x4c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vrcpph  -2048(,%rbp,2), %zmm22

// CHECK: vrcpph  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xcf,0x4c,0x71,0x7f]
          vrcpph  8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vrcpph  -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xdf,0x4c,0x72,0x80]
          vrcpph  -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vrcpsh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x4d,0xf0]
          vrcpsh %xmm24, %xmm23, %xmm22

// CHECK: vrcpsh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x4d,0xf0]
          vrcpsh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vrcpsh %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x4d,0xf0]
          vrcpsh %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vrcpsh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x4d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrcpsh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vrcpsh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x4d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrcpsh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vrcpsh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x4d,0x35,0x00,0x00,0x00,0x00]
          vrcpsh  (%rip), %xmm23, %xmm22

// CHECK: vrcpsh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x4d,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vrcpsh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vrcpsh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x4d,0x71,0x7f]
          vrcpsh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vrcpsh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x4d,0x72,0x80]
          vrcpsh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vreduceph $123, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x56,0xf7,0x7b]
          vreduceph $123, %zmm23, %zmm22

// CHECK: vreduceph $123, {sae}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x18,0x56,0xf7,0x7b]
          vreduceph $123, {sae}, %zmm23, %zmm22

// CHECK: vreduceph $123, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa3,0x7c,0x4f,0x56,0xf7,0x7b]
          vreduceph $123, %zmm23, %zmm22 {%k7}

// CHECK: vreduceph $123, {sae}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa3,0x7c,0x9f,0x56,0xf7,0x7b]
          vreduceph $123, {sae}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vreduceph  $123, 268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x56,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vreduceph  $123, 268435456(%rbp,%r14,8), %zmm22

// CHECK: vreduceph  $123, 291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x7c,0x4f,0x56,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vreduceph  $123, 291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vreduceph  $123, (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x58,0x56,0x35,0x00,0x00,0x00,0x00,0x7b]
          vreduceph  $123, (%rip){1to32}, %zmm22

// CHECK: vreduceph  $123, -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x48,0x56,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vreduceph  $123, -2048(,%rbp,2), %zmm22

// CHECK: vreduceph  $123, 8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0xcf,0x56,0x71,0x7f,0x7b]
          vreduceph  $123, 8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vreduceph  $123, -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0xdf,0x56,0x72,0x80,0x7b]
          vreduceph  $123, -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vreducesh $123, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x83,0x44,0x00,0x57,0xf0,0x7b]
          vreducesh $123, %xmm24, %xmm23, %xmm22

// CHECK: vreducesh $123, {sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x83,0x44,0x10,0x57,0xf0,0x7b]
          vreducesh $123, {sae}, %xmm24, %xmm23, %xmm22

// CHECK: vreducesh $123, %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x83,0x44,0x07,0x57,0xf0,0x7b]
          vreducesh $123, %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vreducesh $123, {sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x83,0x44,0x97,0x57,0xf0,0x7b]
          vreducesh $123, {sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vreducesh  $123, 268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa3,0x44,0x00,0x57,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vreducesh  $123, 268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vreducesh  $123, 291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x44,0x07,0x57,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vreducesh  $123, 291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vreducesh  $123, (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe3,0x44,0x00,0x57,0x35,0x00,0x00,0x00,0x00,0x7b]
          vreducesh  $123, (%rip), %xmm23, %xmm22

// CHECK: vreducesh  $123, -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe3,0x44,0x00,0x57,0x34,0x6d,0xc0,0xff,0xff,0xff,0x7b]
          vreducesh  $123, -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vreducesh  $123, 254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x44,0x87,0x57,0x71,0x7f,0x7b]
          vreducesh  $123, 254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vreducesh  $123, -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x44,0x87,0x57,0x72,0x80,0x7b]
          vreducesh  $123, -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vrndscaleph $123, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x08,0xf7,0x7b]
          vrndscaleph $123, %zmm23, %zmm22

// CHECK: vrndscaleph $123, {sae}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x18,0x08,0xf7,0x7b]
          vrndscaleph $123, {sae}, %zmm23, %zmm22

// CHECK: vrndscaleph $123, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa3,0x7c,0x4f,0x08,0xf7,0x7b]
          vrndscaleph $123, %zmm23, %zmm22 {%k7}

// CHECK: vrndscaleph $123, {sae}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa3,0x7c,0x9f,0x08,0xf7,0x7b]
          vrndscaleph $123, {sae}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vrndscaleph  $123, 268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x08,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vrndscaleph  $123, 268435456(%rbp,%r14,8), %zmm22

// CHECK: vrndscaleph  $123, 291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x7c,0x4f,0x08,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vrndscaleph  $123, 291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vrndscaleph  $123, (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x58,0x08,0x35,0x00,0x00,0x00,0x00,0x7b]
          vrndscaleph  $123, (%rip){1to32}, %zmm22

// CHECK: vrndscaleph  $123, -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x48,0x08,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vrndscaleph  $123, -2048(,%rbp,2), %zmm22

// CHECK: vrndscaleph  $123, 8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0xcf,0x08,0x71,0x7f,0x7b]
          vrndscaleph  $123, 8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vrndscaleph  $123, -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0xdf,0x08,0x72,0x80,0x7b]
          vrndscaleph  $123, -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vrndscalesh $123, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x83,0x44,0x00,0x0a,0xf0,0x7b]
          vrndscalesh $123, %xmm24, %xmm23, %xmm22

// CHECK: vrndscalesh $123, {sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x83,0x44,0x10,0x0a,0xf0,0x7b]
          vrndscalesh $123, {sae}, %xmm24, %xmm23, %xmm22

// CHECK: vrndscalesh $123, %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x83,0x44,0x07,0x0a,0xf0,0x7b]
          vrndscalesh $123, %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vrndscalesh $123, {sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x83,0x44,0x97,0x0a,0xf0,0x7b]
          vrndscalesh $123, {sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vrndscalesh  $123, 268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa3,0x44,0x00,0x0a,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vrndscalesh  $123, 268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vrndscalesh  $123, 291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x44,0x07,0x0a,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vrndscalesh  $123, 291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vrndscalesh  $123, (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe3,0x44,0x00,0x0a,0x35,0x00,0x00,0x00,0x00,0x7b]
          vrndscalesh  $123, (%rip), %xmm23, %xmm22

// CHECK: vrndscalesh  $123, -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe3,0x44,0x00,0x0a,0x34,0x6d,0xc0,0xff,0xff,0xff,0x7b]
          vrndscalesh  $123, -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vrndscalesh  $123, 254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x44,0x87,0x0a,0x71,0x7f,0x7b]
          vrndscalesh  $123, 254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vrndscalesh  $123, -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x44,0x87,0x0a,0x72,0x80,0x7b]
          vrndscalesh  $123, -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vrsqrtph %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x48,0x4e,0xf7]
          vrsqrtph %zmm23, %zmm22

// CHECK: vrsqrtph %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa6,0x7d,0x4f,0x4e,0xf7]
          vrsqrtph %zmm23, %zmm22 {%k7}

// CHECK: vrsqrtph %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa6,0x7d,0xcf,0x4e,0xf7]
          vrsqrtph %zmm23, %zmm22 {%k7} {z}

// CHECK: vrsqrtph  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x48,0x4e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrsqrtph  268435456(%rbp,%r14,8), %zmm22

// CHECK: vrsqrtph  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x7d,0x4f,0x4e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrsqrtph  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vrsqrtph  (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x58,0x4e,0x35,0x00,0x00,0x00,0x00]
          vrsqrtph  (%rip){1to32}, %zmm22

// CHECK: vrsqrtph  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x48,0x4e,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vrsqrtph  -2048(,%rbp,2), %zmm22

// CHECK: vrsqrtph  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xcf,0x4e,0x71,0x7f]
          vrsqrtph  8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vrsqrtph  -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xdf,0x4e,0x72,0x80]
          vrsqrtph  -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vrsqrtsh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x4f,0xf0]
          vrsqrtsh %xmm24, %xmm23, %xmm22

// CHECK: vrsqrtsh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x4f,0xf0]
          vrsqrtsh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vrsqrtsh %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x4f,0xf0]
          vrsqrtsh %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vrsqrtsh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x4f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrsqrtsh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vrsqrtsh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x4f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrsqrtsh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vrsqrtsh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x4f,0x35,0x00,0x00,0x00,0x00]
          vrsqrtsh  (%rip), %xmm23, %xmm22

// CHECK: vrsqrtsh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x4f,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vrsqrtsh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vrsqrtsh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x4f,0x71,0x7f]
          vrsqrtsh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vrsqrtsh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x4f,0x72,0x80]
          vrsqrtsh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vscalefph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x2c,0xf0]
          vscalefph %zmm24, %zmm23, %zmm22

// CHECK: vscalefph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x2c,0xf0]
          vscalefph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vscalefph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0x2c,0xf0]
          vscalefph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vscalefph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0x2c,0xf0]
          vscalefph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vscalefph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0x2c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vscalefph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vscalefph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0x2c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vscalefph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vscalefph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0x2c,0x35,0x00,0x00,0x00,0x00]
          vscalefph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vscalefph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0x2c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vscalefph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vscalefph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0x2c,0x71,0x7f]
          vscalefph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vscalefph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0x2c,0x72,0x80]
          vscalefph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vscalefsh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x2d,0xf0]
          vscalefsh %xmm24, %xmm23, %xmm22

// CHECK: vscalefsh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x2d,0xf0]
          vscalefsh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vscalefsh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x2d,0xf0]
          vscalefsh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vscalefsh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0x2d,0xf0]
          vscalefsh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vscalefsh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x2d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vscalefsh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vscalefsh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x2d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vscalefsh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vscalefsh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x2d,0x35,0x00,0x00,0x00,0x00]
          vscalefsh  (%rip), %xmm23, %xmm22

// CHECK: vscalefsh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x2d,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vscalefsh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vscalefsh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x2d,0x71,0x7f]
          vscalefsh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vscalefsh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x2d,0x72,0x80]
          vscalefsh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vsqrtph %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x51,0xf7]
          vsqrtph %zmm23, %zmm22

// CHECK: vsqrtph {rn-sae}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x18,0x51,0xf7]
          vsqrtph {rn-sae}, %zmm23, %zmm22

// CHECK: vsqrtph %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x4f,0x51,0xf7]
          vsqrtph %zmm23, %zmm22 {%k7}

// CHECK: vsqrtph {rz-sae}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xff,0x51,0xf7]
          vsqrtph {rz-sae}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vsqrtph  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x48,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsqrtph  268435456(%rbp,%r14,8), %zmm22

// CHECK: vsqrtph  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x4f,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsqrtph  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK: vsqrtph  (%rip){1to32}, %zmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x58,0x51,0x35,0x00,0x00,0x00,0x00]
          vsqrtph  (%rip){1to32}, %zmm22

// CHECK: vsqrtph  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x48,0x51,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vsqrtph  -2048(,%rbp,2), %zmm22

// CHECK: vsqrtph  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xcf,0x51,0x71,0x7f]
          vsqrtph  8128(%rcx), %zmm22 {%k7} {z}

// CHECK: vsqrtph  -256(%rdx){1to32}, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xdf,0x51,0x72,0x80]
          vsqrtph  -256(%rdx){1to32}, %zmm22 {%k7} {z}

// CHECK: vsqrtsh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x00,0x51,0xf0]
          vsqrtsh %xmm24, %xmm23, %xmm22

// CHECK: vsqrtsh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x85,0x46,0x10,0x51,0xf0]
          vsqrtsh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vsqrtsh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x85,0x46,0x07,0x51,0xf0]
          vsqrtsh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vsqrtsh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x85,0x46,0xf7,0x51,0xf0]
          vsqrtsh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vsqrtsh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x46,0x00,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsqrtsh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vsqrtsh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x46,0x07,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsqrtsh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vsqrtsh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x51,0x35,0x00,0x00,0x00,0x00]
          vsqrtsh  (%rip), %xmm23, %xmm22

// CHECK: vsqrtsh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe5,0x46,0x00,0x51,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vsqrtsh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vsqrtsh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x46,0x87,0x51,0x71,0x7f]
          vsqrtsh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vsqrtsh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x46,0x87,0x51,0x72,0x80]
          vsqrtsh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd132ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x98,0xf0]
          vfmadd132ph %zmm24, %zmm23, %zmm22

// CHECK: vfmadd132ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x98,0xf0]
          vfmadd132ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfmadd132ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0x98,0xf0]
          vfmadd132ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfmadd132ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0x98,0xf0]
          vfmadd132ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmadd132ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0x98,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd132ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfmadd132ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0x98,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd132ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfmadd132ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0x98,0x35,0x00,0x00,0x00,0x00]
          vfmadd132ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfmadd132ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0x98,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmadd132ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfmadd132ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0x98,0x71,0x7f]
          vfmadd132ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmadd132ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0x98,0x72,0x80]
          vfmadd132ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmadd132sh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x99,0xf0]
          vfmadd132sh %xmm24, %xmm23, %xmm22

// CHECK: vfmadd132sh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x99,0xf0]
          vfmadd132sh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vfmadd132sh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x99,0xf0]
          vfmadd132sh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmadd132sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0x99,0xf0]
          vfmadd132sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd132sh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x99,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd132sh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmadd132sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x99,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd132sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmadd132sh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x99,0x35,0x00,0x00,0x00,0x00]
          vfmadd132sh  (%rip), %xmm23, %xmm22

// CHECK: vfmadd132sh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x99,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vfmadd132sh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmadd132sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x99,0x71,0x7f]
          vfmadd132sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd132sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x99,0x72,0x80]
          vfmadd132sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd213ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xa8,0xf0]
          vfmadd213ph %zmm24, %zmm23, %zmm22

// CHECK: vfmadd213ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xa8,0xf0]
          vfmadd213ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfmadd213ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xa8,0xf0]
          vfmadd213ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfmadd213ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xa8,0xf0]
          vfmadd213ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmadd213ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xa8,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd213ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfmadd213ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xa8,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd213ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfmadd213ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xa8,0x35,0x00,0x00,0x00,0x00]
          vfmadd213ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfmadd213ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xa8,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmadd213ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfmadd213ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xa8,0x71,0x7f]
          vfmadd213ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmadd213ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xa8,0x72,0x80]
          vfmadd213ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmadd213sh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xa9,0xf0]
          vfmadd213sh %xmm24, %xmm23, %xmm22

// CHECK: vfmadd213sh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xa9,0xf0]
          vfmadd213sh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vfmadd213sh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xa9,0xf0]
          vfmadd213sh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmadd213sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xa9,0xf0]
          vfmadd213sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd213sh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xa9,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd213sh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmadd213sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xa9,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd213sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmadd213sh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xa9,0x35,0x00,0x00,0x00,0x00]
          vfmadd213sh  (%rip), %xmm23, %xmm22

// CHECK: vfmadd213sh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xa9,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vfmadd213sh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmadd213sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xa9,0x71,0x7f]
          vfmadd213sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd213sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xa9,0x72,0x80]
          vfmadd213sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd231ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xb8,0xf0]
          vfmadd231ph %zmm24, %zmm23, %zmm22

// CHECK: vfmadd231ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xb8,0xf0]
          vfmadd231ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfmadd231ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xb8,0xf0]
          vfmadd231ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfmadd231ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xb8,0xf0]
          vfmadd231ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmadd231ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xb8,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd231ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfmadd231ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xb8,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd231ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfmadd231ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xb8,0x35,0x00,0x00,0x00,0x00]
          vfmadd231ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfmadd231ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xb8,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmadd231ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfmadd231ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xb8,0x71,0x7f]
          vfmadd231ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmadd231ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xb8,0x72,0x80]
          vfmadd231ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmadd231sh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xb9,0xf0]
          vfmadd231sh %xmm24, %xmm23, %xmm22

// CHECK: vfmadd231sh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xb9,0xf0]
          vfmadd231sh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vfmadd231sh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xb9,0xf0]
          vfmadd231sh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmadd231sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xb9,0xf0]
          vfmadd231sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd231sh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xb9,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd231sh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmadd231sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xb9,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd231sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmadd231sh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xb9,0x35,0x00,0x00,0x00,0x00]
          vfmadd231sh  (%rip), %xmm23, %xmm22

// CHECK: vfmadd231sh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xb9,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vfmadd231sh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmadd231sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xb9,0x71,0x7f]
          vfmadd231sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd231sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xb9,0x72,0x80]
          vfmadd231sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddsub132ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x96,0xf0]
          vfmaddsub132ph %zmm24, %zmm23, %zmm22

// CHECK: vfmaddsub132ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x96,0xf0]
          vfmaddsub132ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfmaddsub132ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0x96,0xf0]
          vfmaddsub132ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfmaddsub132ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0x96,0xf0]
          vfmaddsub132ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmaddsub132ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0x96,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub132ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfmaddsub132ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0x96,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub132ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfmaddsub132ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0x96,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub132ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfmaddsub132ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0x96,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmaddsub132ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfmaddsub132ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0x96,0x71,0x7f]
          vfmaddsub132ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmaddsub132ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0x96,0x72,0x80]
          vfmaddsub132ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmaddsub213ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xa6,0xf0]
          vfmaddsub213ph %zmm24, %zmm23, %zmm22

// CHECK: vfmaddsub213ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xa6,0xf0]
          vfmaddsub213ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfmaddsub213ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xa6,0xf0]
          vfmaddsub213ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfmaddsub213ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xa6,0xf0]
          vfmaddsub213ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmaddsub213ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xa6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub213ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfmaddsub213ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xa6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub213ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfmaddsub213ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xa6,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub213ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfmaddsub213ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xa6,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmaddsub213ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfmaddsub213ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xa6,0x71,0x7f]
          vfmaddsub213ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmaddsub213ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xa6,0x72,0x80]
          vfmaddsub213ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmaddsub231ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xb6,0xf0]
          vfmaddsub231ph %zmm24, %zmm23, %zmm22

// CHECK: vfmaddsub231ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xb6,0xf0]
          vfmaddsub231ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfmaddsub231ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xb6,0xf0]
          vfmaddsub231ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfmaddsub231ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xb6,0xf0]
          vfmaddsub231ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmaddsub231ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xb6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub231ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfmaddsub231ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xb6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub231ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfmaddsub231ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xb6,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub231ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfmaddsub231ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xb6,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmaddsub231ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfmaddsub231ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xb6,0x71,0x7f]
          vfmaddsub231ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmaddsub231ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xb6,0x72,0x80]
          vfmaddsub231ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsub132ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x9a,0xf0]
          vfmsub132ph %zmm24, %zmm23, %zmm22

// CHECK: vfmsub132ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x9a,0xf0]
          vfmsub132ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfmsub132ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0x9a,0xf0]
          vfmsub132ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfmsub132ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0x9a,0xf0]
          vfmsub132ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsub132ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0x9a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub132ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfmsub132ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0x9a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub132ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfmsub132ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0x9a,0x35,0x00,0x00,0x00,0x00]
          vfmsub132ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfmsub132ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0x9a,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmsub132ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfmsub132ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0x9a,0x71,0x7f]
          vfmsub132ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsub132ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0x9a,0x72,0x80]
          vfmsub132ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsub132sh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x9b,0xf0]
          vfmsub132sh %xmm24, %xmm23, %xmm22

// CHECK: vfmsub132sh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x9b,0xf0]
          vfmsub132sh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vfmsub132sh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x9b,0xf0]
          vfmsub132sh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmsub132sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0x9b,0xf0]
          vfmsub132sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub132sh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x9b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub132sh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmsub132sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x9b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub132sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmsub132sh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x9b,0x35,0x00,0x00,0x00,0x00]
          vfmsub132sh  (%rip), %xmm23, %xmm22

// CHECK: vfmsub132sh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x9b,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vfmsub132sh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmsub132sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x9b,0x71,0x7f]
          vfmsub132sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub132sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x9b,0x72,0x80]
          vfmsub132sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub213ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xaa,0xf0]
          vfmsub213ph %zmm24, %zmm23, %zmm22

// CHECK: vfmsub213ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xaa,0xf0]
          vfmsub213ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfmsub213ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xaa,0xf0]
          vfmsub213ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfmsub213ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xaa,0xf0]
          vfmsub213ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsub213ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xaa,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub213ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfmsub213ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xaa,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub213ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfmsub213ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xaa,0x35,0x00,0x00,0x00,0x00]
          vfmsub213ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfmsub213ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xaa,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmsub213ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfmsub213ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xaa,0x71,0x7f]
          vfmsub213ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsub213ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xaa,0x72,0x80]
          vfmsub213ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsub213sh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xab,0xf0]
          vfmsub213sh %xmm24, %xmm23, %xmm22

// CHECK: vfmsub213sh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xab,0xf0]
          vfmsub213sh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vfmsub213sh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xab,0xf0]
          vfmsub213sh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmsub213sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xab,0xf0]
          vfmsub213sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub213sh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xab,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub213sh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmsub213sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xab,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub213sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmsub213sh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xab,0x35,0x00,0x00,0x00,0x00]
          vfmsub213sh  (%rip), %xmm23, %xmm22

// CHECK: vfmsub213sh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xab,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vfmsub213sh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmsub213sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xab,0x71,0x7f]
          vfmsub213sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub213sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xab,0x72,0x80]
          vfmsub213sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub231ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xba,0xf0]
          vfmsub231ph %zmm24, %zmm23, %zmm22

// CHECK: vfmsub231ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xba,0xf0]
          vfmsub231ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfmsub231ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xba,0xf0]
          vfmsub231ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfmsub231ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xba,0xf0]
          vfmsub231ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsub231ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xba,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub231ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfmsub231ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xba,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub231ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfmsub231ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xba,0x35,0x00,0x00,0x00,0x00]
          vfmsub231ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfmsub231ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xba,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmsub231ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfmsub231ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xba,0x71,0x7f]
          vfmsub231ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsub231ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xba,0x72,0x80]
          vfmsub231ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsub231sh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xbb,0xf0]
          vfmsub231sh %xmm24, %xmm23, %xmm22

// CHECK: vfmsub231sh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xbb,0xf0]
          vfmsub231sh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vfmsub231sh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xbb,0xf0]
          vfmsub231sh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmsub231sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xbb,0xf0]
          vfmsub231sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub231sh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xbb,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub231sh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmsub231sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xbb,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub231sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmsub231sh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xbb,0x35,0x00,0x00,0x00,0x00]
          vfmsub231sh  (%rip), %xmm23, %xmm22

// CHECK: vfmsub231sh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xbb,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vfmsub231sh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmsub231sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xbb,0x71,0x7f]
          vfmsub231sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub231sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xbb,0x72,0x80]
          vfmsub231sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsubadd132ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x97,0xf0]
          vfmsubadd132ph %zmm24, %zmm23, %zmm22

// CHECK: vfmsubadd132ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x97,0xf0]
          vfmsubadd132ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfmsubadd132ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0x97,0xf0]
          vfmsubadd132ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfmsubadd132ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0x97,0xf0]
          vfmsubadd132ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsubadd132ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0x97,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd132ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfmsubadd132ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0x97,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd132ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfmsubadd132ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0x97,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd132ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfmsubadd132ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0x97,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmsubadd132ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfmsubadd132ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0x97,0x71,0x7f]
          vfmsubadd132ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsubadd132ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0x97,0x72,0x80]
          vfmsubadd132ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsubadd213ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xa7,0xf0]
          vfmsubadd213ph %zmm24, %zmm23, %zmm22

// CHECK: vfmsubadd213ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xa7,0xf0]
          vfmsubadd213ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfmsubadd213ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xa7,0xf0]
          vfmsubadd213ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfmsubadd213ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xa7,0xf0]
          vfmsubadd213ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsubadd213ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xa7,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd213ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfmsubadd213ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xa7,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd213ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfmsubadd213ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xa7,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd213ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfmsubadd213ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xa7,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmsubadd213ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfmsubadd213ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xa7,0x71,0x7f]
          vfmsubadd213ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsubadd213ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xa7,0x72,0x80]
          vfmsubadd213ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsubadd231ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xb7,0xf0]
          vfmsubadd231ph %zmm24, %zmm23, %zmm22

// CHECK: vfmsubadd231ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xb7,0xf0]
          vfmsubadd231ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfmsubadd231ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xb7,0xf0]
          vfmsubadd231ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfmsubadd231ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xb7,0xf0]
          vfmsubadd231ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsubadd231ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xb7,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd231ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfmsubadd231ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xb7,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd231ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfmsubadd231ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xb7,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd231ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfmsubadd231ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xb7,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfmsubadd231ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfmsubadd231ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xb7,0x71,0x7f]
          vfmsubadd231ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfmsubadd231ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xb7,0x72,0x80]
          vfmsubadd231ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmadd132ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x9c,0xf0]
          vfnmadd132ph %zmm24, %zmm23, %zmm22

// CHECK: vfnmadd132ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x9c,0xf0]
          vfnmadd132ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfnmadd132ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0x9c,0xf0]
          vfnmadd132ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfnmadd132ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0x9c,0xf0]
          vfnmadd132ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmadd132ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0x9c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd132ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfnmadd132ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0x9c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd132ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfnmadd132ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0x9c,0x35,0x00,0x00,0x00,0x00]
          vfnmadd132ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfnmadd132ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0x9c,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfnmadd132ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfnmadd132ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0x9c,0x71,0x7f]
          vfnmadd132ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmadd132ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0x9c,0x72,0x80]
          vfnmadd132ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmadd132sh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x9d,0xf0]
          vfnmadd132sh %xmm24, %xmm23, %xmm22

// CHECK: vfnmadd132sh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x9d,0xf0]
          vfnmadd132sh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vfnmadd132sh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x9d,0xf0]
          vfnmadd132sh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfnmadd132sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0x9d,0xf0]
          vfnmadd132sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd132sh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x9d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd132sh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfnmadd132sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x9d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd132sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfnmadd132sh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x9d,0x35,0x00,0x00,0x00,0x00]
          vfnmadd132sh  (%rip), %xmm23, %xmm22

// CHECK: vfnmadd132sh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x9d,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vfnmadd132sh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vfnmadd132sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x9d,0x71,0x7f]
          vfnmadd132sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd132sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x9d,0x72,0x80]
          vfnmadd132sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd213ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xac,0xf0]
          vfnmadd213ph %zmm24, %zmm23, %zmm22

// CHECK: vfnmadd213ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xac,0xf0]
          vfnmadd213ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfnmadd213ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xac,0xf0]
          vfnmadd213ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfnmadd213ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xac,0xf0]
          vfnmadd213ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmadd213ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xac,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd213ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfnmadd213ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xac,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd213ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfnmadd213ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xac,0x35,0x00,0x00,0x00,0x00]
          vfnmadd213ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfnmadd213ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xac,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfnmadd213ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfnmadd213ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xac,0x71,0x7f]
          vfnmadd213ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmadd213ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xac,0x72,0x80]
          vfnmadd213ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmadd213sh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xad,0xf0]
          vfnmadd213sh %xmm24, %xmm23, %xmm22

// CHECK: vfnmadd213sh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xad,0xf0]
          vfnmadd213sh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vfnmadd213sh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xad,0xf0]
          vfnmadd213sh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfnmadd213sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xad,0xf0]
          vfnmadd213sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd213sh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xad,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd213sh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfnmadd213sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xad,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd213sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfnmadd213sh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xad,0x35,0x00,0x00,0x00,0x00]
          vfnmadd213sh  (%rip), %xmm23, %xmm22

// CHECK: vfnmadd213sh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xad,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vfnmadd213sh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vfnmadd213sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xad,0x71,0x7f]
          vfnmadd213sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd213sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xad,0x72,0x80]
          vfnmadd213sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd231ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xbc,0xf0]
          vfnmadd231ph %zmm24, %zmm23, %zmm22

// CHECK: vfnmadd231ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xbc,0xf0]
          vfnmadd231ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfnmadd231ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xbc,0xf0]
          vfnmadd231ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfnmadd231ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xbc,0xf0]
          vfnmadd231ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmadd231ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xbc,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd231ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfnmadd231ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xbc,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd231ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfnmadd231ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xbc,0x35,0x00,0x00,0x00,0x00]
          vfnmadd231ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfnmadd231ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xbc,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfnmadd231ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfnmadd231ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xbc,0x71,0x7f]
          vfnmadd231ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmadd231ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xbc,0x72,0x80]
          vfnmadd231ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmadd231sh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xbd,0xf0]
          vfnmadd231sh %xmm24, %xmm23, %xmm22

// CHECK: vfnmadd231sh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xbd,0xf0]
          vfnmadd231sh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vfnmadd231sh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xbd,0xf0]
          vfnmadd231sh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfnmadd231sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xbd,0xf0]
          vfnmadd231sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd231sh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xbd,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd231sh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfnmadd231sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xbd,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd231sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfnmadd231sh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xbd,0x35,0x00,0x00,0x00,0x00]
          vfnmadd231sh  (%rip), %xmm23, %xmm22

// CHECK: vfnmadd231sh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xbd,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vfnmadd231sh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vfnmadd231sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xbd,0x71,0x7f]
          vfnmadd231sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd231sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xbd,0x72,0x80]
          vfnmadd231sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub132ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x9e,0xf0]
          vfnmsub132ph %zmm24, %zmm23, %zmm22

// CHECK: vfnmsub132ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x9e,0xf0]
          vfnmsub132ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfnmsub132ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0x9e,0xf0]
          vfnmsub132ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfnmsub132ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0x9e,0xf0]
          vfnmsub132ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmsub132ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0x9e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub132ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfnmsub132ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0x9e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub132ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfnmsub132ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0x9e,0x35,0x00,0x00,0x00,0x00]
          vfnmsub132ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfnmsub132ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0x9e,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfnmsub132ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfnmsub132ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0x9e,0x71,0x7f]
          vfnmsub132ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmsub132ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0x9e,0x72,0x80]
          vfnmsub132ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmsub132sh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x9f,0xf0]
          vfnmsub132sh %xmm24, %xmm23, %xmm22

// CHECK: vfnmsub132sh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0x9f,0xf0]
          vfnmsub132sh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vfnmsub132sh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x9f,0xf0]
          vfnmsub132sh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfnmsub132sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0x9f,0xf0]
          vfnmsub132sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub132sh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x9f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub132sh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfnmsub132sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x9f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub132sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfnmsub132sh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x9f,0x35,0x00,0x00,0x00,0x00]
          vfnmsub132sh  (%rip), %xmm23, %xmm22

// CHECK: vfnmsub132sh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x9f,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vfnmsub132sh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vfnmsub132sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x9f,0x71,0x7f]
          vfnmsub132sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub132sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x9f,0x72,0x80]
          vfnmsub132sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub213ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xae,0xf0]
          vfnmsub213ph %zmm24, %zmm23, %zmm22

// CHECK: vfnmsub213ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xae,0xf0]
          vfnmsub213ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfnmsub213ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xae,0xf0]
          vfnmsub213ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfnmsub213ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xae,0xf0]
          vfnmsub213ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmsub213ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xae,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub213ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfnmsub213ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xae,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub213ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfnmsub213ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xae,0x35,0x00,0x00,0x00,0x00]
          vfnmsub213ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfnmsub213ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xae,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfnmsub213ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfnmsub213ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xae,0x71,0x7f]
          vfnmsub213ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmsub213ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xae,0x72,0x80]
          vfnmsub213ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmsub213sh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xaf,0xf0]
          vfnmsub213sh %xmm24, %xmm23, %xmm22

// CHECK: vfnmsub213sh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xaf,0xf0]
          vfnmsub213sh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vfnmsub213sh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xaf,0xf0]
          vfnmsub213sh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfnmsub213sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xaf,0xf0]
          vfnmsub213sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub213sh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xaf,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub213sh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfnmsub213sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xaf,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub213sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfnmsub213sh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xaf,0x35,0x00,0x00,0x00,0x00]
          vfnmsub213sh  (%rip), %xmm23, %xmm22

// CHECK: vfnmsub213sh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xaf,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vfnmsub213sh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vfnmsub213sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xaf,0x71,0x7f]
          vfnmsub213sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub213sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xaf,0x72,0x80]
          vfnmsub213sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub231ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0xbe,0xf0]
          vfnmsub231ph %zmm24, %zmm23, %zmm22

// CHECK: vfnmsub231ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xbe,0xf0]
          vfnmsub231ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK: vfnmsub231ph %zmm24, %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x47,0xbe,0xf0]
          vfnmsub231ph %zmm24, %zmm23, %zmm22 {%k7}

// CHECK: vfnmsub231ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xbe,0xf0]
          vfnmsub231ph {rz-sae}, %zmm24, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmsub231ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x40,0xbe,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub231ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK: vfnmsub231ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x47,0xbe,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub231ph  291(%r8,%rax,4), %zmm23, %zmm22 {%k7}

// CHECK: vfnmsub231ph  (%rip){1to32}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x50,0xbe,0x35,0x00,0x00,0x00,0x00]
          vfnmsub231ph  (%rip){1to32}, %zmm23, %zmm22

// CHECK: vfnmsub231ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x40,0xbe,0x34,0x6d,0x00,0xf8,0xff,0xff]
          vfnmsub231ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK: vfnmsub231ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xc7,0xbe,0x71,0x7f]
          vfnmsub231ph  8128(%rcx), %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmsub231ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xd7,0xbe,0x72,0x80]
          vfnmsub231ph  -256(%rdx){1to32}, %zmm23, %zmm22 {%k7} {z}

// CHECK: vfnmsub231sh %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xbf,0xf0]
          vfnmsub231sh %xmm24, %xmm23, %xmm22

// CHECK: vfnmsub231sh {rn-sae}, %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x10,0xbf,0xf0]
          vfnmsub231sh {rn-sae}, %xmm24, %xmm23, %xmm22

// CHECK: vfnmsub231sh %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xbf,0xf0]
          vfnmsub231sh %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfnmsub231sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xf7,0xbf,0xf0]
          vfnmsub231sh {rz-sae}, %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub231sh  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xbf,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub231sh  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfnmsub231sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xbf,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub231sh  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfnmsub231sh  (%rip), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xbf,0x35,0x00,0x00,0x00,0x00]
          vfnmsub231sh  (%rip), %xmm23, %xmm22

// CHECK: vfnmsub231sh  -64(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xbf,0x34,0x6d,0xc0,0xff,0xff,0xff]
          vfnmsub231sh  -64(,%rbp,2), %xmm23, %xmm22

// CHECK: vfnmsub231sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xbf,0x71,0x7f]
          vfnmsub231sh  254(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub231sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xbf,0x72,0x80]
          vfnmsub231sh  -256(%rdx), %xmm23, %xmm22 {%k7} {z}

