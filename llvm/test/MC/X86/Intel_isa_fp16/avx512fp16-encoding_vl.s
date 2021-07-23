// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding < %s  | FileCheck %s

// CHECK: vcmpph $123, %ymm28, %ymm29, %k5
// CHECK: encoding: [0x62,0x93,0x14,0x20,0xc2,0xec,0x7b]
          vcmpph $123, %ymm28, %ymm29, %k5

// CHECK: vcmpph $123, %ymm28, %ymm29, %k5 {%k7}
// CHECK: encoding: [0x62,0x93,0x14,0x27,0xc2,0xec,0x7b]
          vcmpph $123, %ymm28, %ymm29, %k5 {%k7}

// CHECK: vcmpph $123, %xmm28, %xmm29, %k5
// CHECK: encoding: [0x62,0x93,0x14,0x00,0xc2,0xec,0x7b]
          vcmpph $123, %xmm28, %xmm29, %k5

// CHECK: vcmpph $123, %xmm28, %xmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0x93,0x14,0x07,0xc2,0xec,0x7b]
          vcmpph $123, %xmm28, %xmm29, %k5 {%k7}

// CHECK: vcmpph  $123, 268435456(%rbp,%r14,8), %xmm29, %k5
// CHECK: encoding: [0x62,0xb3,0x14,0x00,0xc2,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcmpph  $123, 268435456(%rbp,%r14,8), %xmm29, %k5

// CHECK: vcmpph  $123, 291(%r8,%rax,4), %xmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0xd3,0x14,0x07,0xc2,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcmpph  $123, 291(%r8,%rax,4), %xmm29, %k5 {%k7}

// CHECK: vcmpph  $123, (%rip){1to8}, %xmm29, %k5
// CHECK: encoding: [0x62,0xf3,0x14,0x10,0xc2,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vcmpph  $123, (%rip){1to8}, %xmm29, %k5

// CHECK: vcmpph  $123, -512(,%rbp,2), %xmm29, %k5
// CHECK: encoding: [0x62,0xf3,0x14,0x00,0xc2,0x2c,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vcmpph  $123, -512(,%rbp,2), %xmm29, %k5

// CHECK: vcmpph  $123, 2032(%rcx), %xmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x14,0x07,0xc2,0x69,0x7f,0x7b]
          vcmpph  $123, 2032(%rcx), %xmm29, %k5 {%k7}

// CHECK: vcmpph  $123, -256(%rdx){1to8}, %xmm29, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x14,0x17,0xc2,0x6a,0x80,0x7b]
          vcmpph  $123, -256(%rdx){1to8}, %xmm29, %k5 {%k7}

// CHECK: vcmpph  $123, 268435456(%rbp,%r14,8), %ymm29, %k5
// CHECK: encoding: [0x62,0xb3,0x14,0x20,0xc2,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vcmpph  $123, 268435456(%rbp,%r14,8), %ymm29, %k5

// CHECK: vcmpph  $123, 291(%r8,%rax,4), %ymm29, %k5 {%k7}
// CHECK: encoding: [0x62,0xd3,0x14,0x27,0xc2,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vcmpph  $123, 291(%r8,%rax,4), %ymm29, %k5 {%k7}

// CHECK: vcmpph  $123, (%rip){1to16}, %ymm29, %k5
// CHECK: encoding: [0x62,0xf3,0x14,0x30,0xc2,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vcmpph  $123, (%rip){1to16}, %ymm29, %k5

// CHECK: vcmpph  $123, -1024(,%rbp,2), %ymm29, %k5
// CHECK: encoding: [0x62,0xf3,0x14,0x20,0xc2,0x2c,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vcmpph  $123, -1024(,%rbp,2), %ymm29, %k5

// CHECK: vcmpph  $123, 4064(%rcx), %ymm29, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x14,0x27,0xc2,0x69,0x7f,0x7b]
          vcmpph  $123, 4064(%rcx), %ymm29, %k5 {%k7}

// CHECK: vcmpph  $123, -256(%rdx){1to16}, %ymm29, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x14,0x37,0xc2,0x6a,0x80,0x7b]
          vcmpph  $123, -256(%rdx){1to16}, %ymm29, %k5 {%k7}

// CHECK: vdivph %ymm28, %ymm29, %ymm30
// CHECK: encoding: [0x62,0x05,0x14,0x20,0x5e,0xf4]
          vdivph %ymm28, %ymm29, %ymm30

// CHECK: vdivph %ymm28, %ymm29, %ymm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x27,0x5e,0xf4]
          vdivph %ymm28, %ymm29, %ymm30 {%k7}

// CHECK: vdivph %ymm28, %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0xa7,0x5e,0xf4]
          vdivph %ymm28, %ymm29, %ymm30 {%k7} {z}

// CHECK: vdivph %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x14,0x00,0x5e,0xf4]
          vdivph %xmm28, %xmm29, %xmm30

// CHECK: vdivph %xmm28, %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x07,0x5e,0xf4]
          vdivph %xmm28, %xmm29, %xmm30 {%k7}

// CHECK: vdivph %xmm28, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0x87,0x5e,0xf4]
          vdivph %xmm28, %xmm29, %xmm30 {%k7} {z}

// CHECK: vdivph  268435456(%rbp,%r14,8), %ymm29, %ymm30
// CHECK: encoding: [0x62,0x25,0x14,0x20,0x5e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdivph  268435456(%rbp,%r14,8), %ymm29, %ymm30

// CHECK: vdivph  291(%r8,%rax,4), %ymm29, %ymm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x27,0x5e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdivph  291(%r8,%rax,4), %ymm29, %ymm30 {%k7}

// CHECK: vdivph  (%rip){1to16}, %ymm29, %ymm30
// CHECK: encoding: [0x62,0x65,0x14,0x30,0x5e,0x35,0x00,0x00,0x00,0x00]
          vdivph  (%rip){1to16}, %ymm29, %ymm30

// CHECK: vdivph  -1024(,%rbp,2), %ymm29, %ymm30
// CHECK: encoding: [0x62,0x65,0x14,0x20,0x5e,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vdivph  -1024(,%rbp,2), %ymm29, %ymm30

// CHECK: vdivph  4064(%rcx), %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xa7,0x5e,0x71,0x7f]
          vdivph  4064(%rcx), %ymm29, %ymm30 {%k7} {z}

// CHECK: vdivph  -256(%rdx){1to16}, %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xb7,0x5e,0x72,0x80]
          vdivph  -256(%rdx){1to16}, %ymm29, %ymm30 {%k7} {z}

// CHECK: vdivph  268435456(%rbp,%r14,8), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x25,0x14,0x00,0x5e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vdivph  268435456(%rbp,%r14,8), %xmm29, %xmm30

// CHECK: vdivph  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x07,0x5e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vdivph  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}

// CHECK: vdivph  (%rip){1to8}, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x14,0x10,0x5e,0x35,0x00,0x00,0x00,0x00]
          vdivph  (%rip){1to8}, %xmm29, %xmm30

// CHECK: vdivph  -512(,%rbp,2), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x14,0x00,0x5e,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vdivph  -512(,%rbp,2), %xmm29, %xmm30

// CHECK: vdivph  2032(%rcx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0x87,0x5e,0x71,0x7f]
          vdivph  2032(%rcx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vdivph  -256(%rdx){1to8}, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0x97,0x5e,0x72,0x80]
          vdivph  -256(%rdx){1to8}, %xmm29, %xmm30 {%k7} {z}

// CHECK: vmaxph %ymm28, %ymm29, %ymm30
// CHECK: encoding: [0x62,0x05,0x14,0x20,0x5f,0xf4]
          vmaxph %ymm28, %ymm29, %ymm30

// CHECK: vmaxph %ymm28, %ymm29, %ymm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x27,0x5f,0xf4]
          vmaxph %ymm28, %ymm29, %ymm30 {%k7}

// CHECK: vmaxph %ymm28, %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0xa7,0x5f,0xf4]
          vmaxph %ymm28, %ymm29, %ymm30 {%k7} {z}

// CHECK: vmaxph %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x14,0x00,0x5f,0xf4]
          vmaxph %xmm28, %xmm29, %xmm30

// CHECK: vmaxph %xmm28, %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x07,0x5f,0xf4]
          vmaxph %xmm28, %xmm29, %xmm30 {%k7}

// CHECK: vmaxph %xmm28, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0x87,0x5f,0xf4]
          vmaxph %xmm28, %xmm29, %xmm30 {%k7} {z}

// CHECK: vmaxph  268435456(%rbp,%r14,8), %ymm29, %ymm30
// CHECK: encoding: [0x62,0x25,0x14,0x20,0x5f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmaxph  268435456(%rbp,%r14,8), %ymm29, %ymm30

// CHECK: vmaxph  291(%r8,%rax,4), %ymm29, %ymm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x27,0x5f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmaxph  291(%r8,%rax,4), %ymm29, %ymm30 {%k7}

// CHECK: vmaxph  (%rip){1to16}, %ymm29, %ymm30
// CHECK: encoding: [0x62,0x65,0x14,0x30,0x5f,0x35,0x00,0x00,0x00,0x00]
          vmaxph  (%rip){1to16}, %ymm29, %ymm30

// CHECK: vmaxph  -1024(,%rbp,2), %ymm29, %ymm30
// CHECK: encoding: [0x62,0x65,0x14,0x20,0x5f,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vmaxph  -1024(,%rbp,2), %ymm29, %ymm30

// CHECK: vmaxph  4064(%rcx), %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xa7,0x5f,0x71,0x7f]
          vmaxph  4064(%rcx), %ymm29, %ymm30 {%k7} {z}

// CHECK: vmaxph  -256(%rdx){1to16}, %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xb7,0x5f,0x72,0x80]
          vmaxph  -256(%rdx){1to16}, %ymm29, %ymm30 {%k7} {z}

// CHECK: vmaxph  268435456(%rbp,%r14,8), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x25,0x14,0x00,0x5f,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmaxph  268435456(%rbp,%r14,8), %xmm29, %xmm30

// CHECK: vmaxph  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x07,0x5f,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmaxph  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}

// CHECK: vmaxph  (%rip){1to8}, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x14,0x10,0x5f,0x35,0x00,0x00,0x00,0x00]
          vmaxph  (%rip){1to8}, %xmm29, %xmm30

// CHECK: vmaxph  -512(,%rbp,2), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x14,0x00,0x5f,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vmaxph  -512(,%rbp,2), %xmm29, %xmm30

// CHECK: vmaxph  2032(%rcx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0x87,0x5f,0x71,0x7f]
          vmaxph  2032(%rcx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vmaxph  -256(%rdx){1to8}, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0x97,0x5f,0x72,0x80]
          vmaxph  -256(%rdx){1to8}, %xmm29, %xmm30 {%k7} {z}

// CHECK: vminph %ymm28, %ymm29, %ymm30
// CHECK: encoding: [0x62,0x05,0x14,0x20,0x5d,0xf4]
          vminph %ymm28, %ymm29, %ymm30

// CHECK: vminph %ymm28, %ymm29, %ymm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x27,0x5d,0xf4]
          vminph %ymm28, %ymm29, %ymm30 {%k7}

// CHECK: vminph %ymm28, %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0xa7,0x5d,0xf4]
          vminph %ymm28, %ymm29, %ymm30 {%k7} {z}

// CHECK: vminph %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x14,0x00,0x5d,0xf4]
          vminph %xmm28, %xmm29, %xmm30

// CHECK: vminph %xmm28, %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x07,0x5d,0xf4]
          vminph %xmm28, %xmm29, %xmm30 {%k7}

// CHECK: vminph %xmm28, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0x87,0x5d,0xf4]
          vminph %xmm28, %xmm29, %xmm30 {%k7} {z}

// CHECK: vminph  268435456(%rbp,%r14,8), %ymm29, %ymm30
// CHECK: encoding: [0x62,0x25,0x14,0x20,0x5d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vminph  268435456(%rbp,%r14,8), %ymm29, %ymm30

// CHECK: vminph  291(%r8,%rax,4), %ymm29, %ymm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x27,0x5d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vminph  291(%r8,%rax,4), %ymm29, %ymm30 {%k7}

// CHECK: vminph  (%rip){1to16}, %ymm29, %ymm30
// CHECK: encoding: [0x62,0x65,0x14,0x30,0x5d,0x35,0x00,0x00,0x00,0x00]
          vminph  (%rip){1to16}, %ymm29, %ymm30

// CHECK: vminph  -1024(,%rbp,2), %ymm29, %ymm30
// CHECK: encoding: [0x62,0x65,0x14,0x20,0x5d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vminph  -1024(,%rbp,2), %ymm29, %ymm30

// CHECK: vminph  4064(%rcx), %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xa7,0x5d,0x71,0x7f]
          vminph  4064(%rcx), %ymm29, %ymm30 {%k7} {z}

// CHECK: vminph  -256(%rdx){1to16}, %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xb7,0x5d,0x72,0x80]
          vminph  -256(%rdx){1to16}, %ymm29, %ymm30 {%k7} {z}

// CHECK: vminph  268435456(%rbp,%r14,8), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x25,0x14,0x00,0x5d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vminph  268435456(%rbp,%r14,8), %xmm29, %xmm30

// CHECK: vminph  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x07,0x5d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vminph  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}

// CHECK: vminph  (%rip){1to8}, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x14,0x10,0x5d,0x35,0x00,0x00,0x00,0x00]
          vminph  (%rip){1to8}, %xmm29, %xmm30

// CHECK: vminph  -512(,%rbp,2), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x14,0x00,0x5d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vminph  -512(,%rbp,2), %xmm29, %xmm30

// CHECK: vminph  2032(%rcx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0x87,0x5d,0x71,0x7f]
          vminph  2032(%rcx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vminph  -256(%rdx){1to8}, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0x97,0x5d,0x72,0x80]
          vminph  -256(%rdx){1to8}, %xmm29, %xmm30 {%k7} {z}

// CHECK: vmulph %ymm28, %ymm29, %ymm30
// CHECK: encoding: [0x62,0x05,0x14,0x20,0x59,0xf4]
          vmulph %ymm28, %ymm29, %ymm30

// CHECK: vmulph %ymm28, %ymm29, %ymm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x27,0x59,0xf4]
          vmulph %ymm28, %ymm29, %ymm30 {%k7}

// CHECK: vmulph %ymm28, %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0xa7,0x59,0xf4]
          vmulph %ymm28, %ymm29, %ymm30 {%k7} {z}

// CHECK: vmulph %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x14,0x00,0x59,0xf4]
          vmulph %xmm28, %xmm29, %xmm30

// CHECK: vmulph %xmm28, %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x07,0x59,0xf4]
          vmulph %xmm28, %xmm29, %xmm30 {%k7}

// CHECK: vmulph %xmm28, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0x87,0x59,0xf4]
          vmulph %xmm28, %xmm29, %xmm30 {%k7} {z}

// CHECK: vmulph  268435456(%rbp,%r14,8), %ymm29, %ymm30
// CHECK: encoding: [0x62,0x25,0x14,0x20,0x59,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmulph  268435456(%rbp,%r14,8), %ymm29, %ymm30

// CHECK: vmulph  291(%r8,%rax,4), %ymm29, %ymm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x27,0x59,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmulph  291(%r8,%rax,4), %ymm29, %ymm30 {%k7}

// CHECK: vmulph  (%rip){1to16}, %ymm29, %ymm30
// CHECK: encoding: [0x62,0x65,0x14,0x30,0x59,0x35,0x00,0x00,0x00,0x00]
          vmulph  (%rip){1to16}, %ymm29, %ymm30

// CHECK: vmulph  -1024(,%rbp,2), %ymm29, %ymm30
// CHECK: encoding: [0x62,0x65,0x14,0x20,0x59,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vmulph  -1024(,%rbp,2), %ymm29, %ymm30

// CHECK: vmulph  4064(%rcx), %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xa7,0x59,0x71,0x7f]
          vmulph  4064(%rcx), %ymm29, %ymm30 {%k7} {z}

// CHECK: vmulph  -256(%rdx){1to16}, %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xb7,0x59,0x72,0x80]
          vmulph  -256(%rdx){1to16}, %ymm29, %ymm30 {%k7} {z}

// CHECK: vmulph  268435456(%rbp,%r14,8), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x25,0x14,0x00,0x59,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vmulph  268435456(%rbp,%r14,8), %xmm29, %xmm30

// CHECK: vmulph  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x07,0x59,0xb4,0x80,0x23,0x01,0x00,0x00]
          vmulph  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}

// CHECK: vmulph  (%rip){1to8}, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x14,0x10,0x59,0x35,0x00,0x00,0x00,0x00]
          vmulph  (%rip){1to8}, %xmm29, %xmm30

// CHECK: vmulph  -512(,%rbp,2), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x14,0x00,0x59,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vmulph  -512(,%rbp,2), %xmm29, %xmm30

// CHECK: vmulph  2032(%rcx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0x87,0x59,0x71,0x7f]
          vmulph  2032(%rcx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vmulph  -256(%rdx){1to8}, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0x97,0x59,0x72,0x80]
          vmulph  -256(%rdx){1to8}, %xmm29, %xmm30 {%k7} {z}

// CHECK: vsubph %ymm28, %ymm29, %ymm30
// CHECK: encoding: [0x62,0x05,0x14,0x20,0x5c,0xf4]
          vsubph %ymm28, %ymm29, %ymm30

// CHECK: vsubph %ymm28, %ymm29, %ymm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x27,0x5c,0xf4]
          vsubph %ymm28, %ymm29, %ymm30 {%k7}

// CHECK: vsubph %ymm28, %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0xa7,0x5c,0xf4]
          vsubph %ymm28, %ymm29, %ymm30 {%k7} {z}

// CHECK: vsubph %xmm28, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x05,0x14,0x00,0x5c,0xf4]
          vsubph %xmm28, %xmm29, %xmm30

// CHECK: vsubph %xmm28, %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x05,0x14,0x07,0x5c,0xf4]
          vsubph %xmm28, %xmm29, %xmm30 {%k7}

// CHECK: vsubph %xmm28, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x05,0x14,0x87,0x5c,0xf4]
          vsubph %xmm28, %xmm29, %xmm30 {%k7} {z}

// CHECK: vsubph  268435456(%rbp,%r14,8), %ymm29, %ymm30
// CHECK: encoding: [0x62,0x25,0x14,0x20,0x5c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubph  268435456(%rbp,%r14,8), %ymm29, %ymm30

// CHECK: vsubph  291(%r8,%rax,4), %ymm29, %ymm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x27,0x5c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubph  291(%r8,%rax,4), %ymm29, %ymm30 {%k7}

// CHECK: vsubph  (%rip){1to16}, %ymm29, %ymm30
// CHECK: encoding: [0x62,0x65,0x14,0x30,0x5c,0x35,0x00,0x00,0x00,0x00]
          vsubph  (%rip){1to16}, %ymm29, %ymm30

// CHECK: vsubph  -1024(,%rbp,2), %ymm29, %ymm30
// CHECK: encoding: [0x62,0x65,0x14,0x20,0x5c,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vsubph  -1024(,%rbp,2), %ymm29, %ymm30

// CHECK: vsubph  4064(%rcx), %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xa7,0x5c,0x71,0x7f]
          vsubph  4064(%rcx), %ymm29, %ymm30 {%k7} {z}

// CHECK: vsubph  -256(%rdx){1to16}, %ymm29, %ymm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0xb7,0x5c,0x72,0x80]
          vsubph  -256(%rdx){1to16}, %ymm29, %ymm30 {%k7} {z}

// CHECK: vsubph  268435456(%rbp,%r14,8), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x25,0x14,0x00,0x5c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsubph  268435456(%rbp,%r14,8), %xmm29, %xmm30

// CHECK: vsubph  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}
// CHECK: encoding: [0x62,0x45,0x14,0x07,0x5c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsubph  291(%r8,%rax,4), %xmm29, %xmm30 {%k7}

// CHECK: vsubph  (%rip){1to8}, %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x14,0x10,0x5c,0x35,0x00,0x00,0x00,0x00]
          vsubph  (%rip){1to8}, %xmm29, %xmm30

// CHECK: vsubph  -512(,%rbp,2), %xmm29, %xmm30
// CHECK: encoding: [0x62,0x65,0x14,0x00,0x5c,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vsubph  -512(,%rbp,2), %xmm29, %xmm30

// CHECK: vsubph  2032(%rcx), %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0x87,0x5c,0x71,0x7f]
          vsubph  2032(%rcx), %xmm29, %xmm30 {%k7} {z}

// CHECK: vsubph  -256(%rdx){1to8}, %xmm29, %xmm30 {%k7} {z}
// CHECK: encoding: [0x62,0x65,0x14,0x97,0x5c,0x72,0x80]
          vsubph  -256(%rdx){1to8}, %xmm29, %xmm30 {%k7} {z}

// CHECK: vcvtph2psx %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x13,0xf7]
          vcvtph2psx %xmm23, %xmm22

// CHECK: vcvtph2psx %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa6,0x7d,0x0f,0x13,0xf7]
          vcvtph2psx %xmm23, %xmm22 {%k7}

// CHECK: vcvtph2psx %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa6,0x7d,0x8f,0x13,0xf7]
          vcvtph2psx %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtph2psx %xmm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x13,0xf7]
          vcvtph2psx %xmm23, %ymm22

// CHECK: vcvtph2psx %xmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa6,0x7d,0x2f,0x13,0xf7]
          vcvtph2psx %xmm23, %ymm22 {%k7}

// CHECK: vcvtph2psx %xmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa6,0x7d,0xaf,0x13,0xf7]
          vcvtph2psx %xmm23, %ymm22 {%k7} {z}

// CHECK: vcvtph2psx  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x13,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2psx  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtph2psx  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x7d,0x0f,0x13,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2psx  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtph2psx  (%rip){1to4}, %xmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x18,0x13,0x35,0x00,0x00,0x00,0x00]
          vcvtph2psx  (%rip){1to4}, %xmm22

// CHECK: vcvtph2psx  -256(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x08,0x13,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtph2psx  -256(,%rbp,2), %xmm22

// CHECK: vcvtph2psx  1016(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0x8f,0x13,0x71,0x7f]
          vcvtph2psx  1016(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtph2psx  -256(%rdx){1to4}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0x9f,0x13,0x72,0x80]
          vcvtph2psx  -256(%rdx){1to4}, %xmm22 {%k7} {z}

// CHECK: vcvtph2psx  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x13,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2psx  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtph2psx  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x7d,0x2f,0x13,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2psx  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtph2psx  (%rip){1to8}, %ymm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x38,0x13,0x35,0x00,0x00,0x00,0x00]
          vcvtph2psx  (%rip){1to8}, %ymm22

// CHECK: vcvtph2psx  -512(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x28,0x13,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2psx  -512(,%rbp,2), %ymm22

// CHECK: vcvtph2psx  2032(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xaf,0x13,0x71,0x7f]
          vcvtph2psx  2032(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtph2psx  -256(%rdx){1to8}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xbf,0x13,0x72,0x80]
          vcvtph2psx  -256(%rdx){1to8}, %ymm22 {%k7} {z}

// CHECK: vcvtps2phx %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x1d,0xf7]
          vcvtps2phx %xmm23, %xmm22

// CHECK: vcvtps2phx %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x1d,0xf7]
          vcvtps2phx %xmm23, %xmm22 {%k7}

// CHECK: vcvtps2phx %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x1d,0xf7]
          vcvtps2phx %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtps2phx %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x1d,0xf7]
          vcvtps2phx %ymm23, %xmm22

// CHECK: vcvtps2phx %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x1d,0xf7]
          vcvtps2phx %ymm23, %xmm22 {%k7}

// CHECK: vcvtps2phx %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x1d,0xf7]
          vcvtps2phx %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtps2phxx  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x1d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtps2phxx  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtps2phxx  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x1d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtps2phxx  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtps2phx  (%rip){1to4}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x1d,0x35,0x00,0x00,0x00,0x00]
          vcvtps2phx  (%rip){1to4}, %xmm22

// CHECK: vcvtps2phxx  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x1d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtps2phxx  -512(,%rbp,2), %xmm22

// CHECK: vcvtps2phxx  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x1d,0x71,0x7f]
          vcvtps2phxx  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtps2phx  -512(%rdx){1to4}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x1d,0x72,0x80]
          vcvtps2phx  -512(%rdx){1to4}, %xmm22 {%k7} {z}

// CHECK: vcvtps2phx  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x1d,0x35,0x00,0x00,0x00,0x00]
          vcvtps2phx  (%rip){1to8}, %xmm22

// CHECK: vcvtps2phxy  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x1d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtps2phxy  -1024(,%rbp,2), %xmm22

// CHECK: vcvtps2phxy  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x1d,0x71,0x7f]
          vcvtps2phxy  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtps2phx  -512(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x1d,0x72,0x80]
          vcvtps2phx  -512(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtpd2ph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xfd,0x08,0x5a,0xf7]
          vcvtpd2ph %xmm23, %xmm22

// CHECK: vcvtpd2ph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0xfd,0x0f,0x5a,0xf7]
          vcvtpd2ph %xmm23, %xmm22 {%k7}

// CHECK: vcvtpd2ph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0xfd,0x8f,0x5a,0xf7]
          vcvtpd2ph %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtpd2ph %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xfd,0x28,0x5a,0xf7]
          vcvtpd2ph %ymm23, %xmm22

// CHECK: vcvtpd2ph %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0xfd,0x2f,0x5a,0xf7]
          vcvtpd2ph %ymm23, %xmm22 {%k7}

// CHECK: vcvtpd2ph %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0xfd,0xaf,0x5a,0xf7]
          vcvtpd2ph %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtpd2phx  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0xfd,0x08,0x5a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtpd2phx  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtpd2phx  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xfd,0x0f,0x5a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtpd2phx  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtpd2ph  (%rip){1to2}, %xmm22
// CHECK: encoding: [0x62,0xe5,0xfd,0x18,0x5a,0x35,0x00,0x00,0x00,0x00]
          vcvtpd2ph  (%rip){1to2}, %xmm22

// CHECK: vcvtpd2phx  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0xfd,0x08,0x5a,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtpd2phx  -512(,%rbp,2), %xmm22

// CHECK: vcvtpd2phx  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xfd,0x8f,0x5a,0x71,0x7f]
          vcvtpd2phx  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtpd2ph  -1024(%rdx){1to2}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xfd,0x9f,0x5a,0x72,0x80]
          vcvtpd2ph  -1024(%rdx){1to2}, %xmm22 {%k7} {z}

// CHECK: vcvtpd2ph  (%rip){1to4}, %xmm22
// CHECK: encoding: [0x62,0xe5,0xfd,0x38,0x5a,0x35,0x00,0x00,0x00,0x00]
          vcvtpd2ph  (%rip){1to4}, %xmm22

// CHECK: vcvtpd2phy  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0xfd,0x28,0x5a,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtpd2phy  -1024(,%rbp,2), %xmm22

// CHECK: vcvtpd2phy  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xfd,0xaf,0x5a,0x71,0x7f]
          vcvtpd2phy  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtpd2ph  -1024(%rdx){1to4}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xfd,0xbf,0x5a,0x72,0x80]
          vcvtpd2ph  -1024(%rdx){1to4}, %xmm22 {%k7} {z}

// CHECK: vcvtph2pd %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x5a,0xf7]
          vcvtph2pd %xmm23, %xmm22

// CHECK: vcvtph2pd %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x5a,0xf7]
          vcvtph2pd %xmm23, %xmm22 {%k7}

// CHECK: vcvtph2pd %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x5a,0xf7]
          vcvtph2pd %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtph2pd %xmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x5a,0xf7]
          vcvtph2pd %xmm23, %ymm22

// CHECK: vcvtph2pd %xmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x5a,0xf7]
          vcvtph2pd %xmm23, %ymm22 {%k7}

// CHECK: vcvtph2pd %xmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x5a,0xf7]
          vcvtph2pd %xmm23, %ymm22 {%k7} {z}

// CHECK: vcvtph2pd  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x5a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2pd  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtph2pd  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x5a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2pd  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtph2pd  (%rip){1to2}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x5a,0x35,0x00,0x00,0x00,0x00]
          vcvtph2pd  (%rip){1to2}, %xmm22

// CHECK: vcvtph2pd  -128(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x5a,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvtph2pd  -128(,%rbp,2), %xmm22

// CHECK: vcvtph2pd  508(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x5a,0x71,0x7f]
          vcvtph2pd  508(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtph2pd  -256(%rdx){1to2}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x5a,0x72,0x80]
          vcvtph2pd  -256(%rdx){1to2}, %xmm22 {%k7} {z}

// CHECK: vcvtph2pd  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x5a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2pd  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtph2pd  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x2f,0x5a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2pd  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtph2pd  (%rip){1to4}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x5a,0x35,0x00,0x00,0x00,0x00]
          vcvtph2pd  (%rip){1to4}, %ymm22

// CHECK: vcvtph2pd  -256(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x5a,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtph2pd  -256(,%rbp,2), %ymm22

// CHECK: vcvtph2pd  1016(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x5a,0x71,0x7f]
          vcvtph2pd  1016(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtph2pd  -256(%rdx){1to4}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x5a,0x72,0x80]
          vcvtph2pd  -256(%rdx){1to4}, %ymm22 {%k7} {z}

// CHECK: vcvtph2uw %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x7d,0xf7]
          vcvtph2uw %xmm23, %xmm22

// CHECK: vcvtph2uw %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x7d,0xf7]
          vcvtph2uw %xmm23, %xmm22 {%k7}

// CHECK: vcvtph2uw %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x7d,0xf7]
          vcvtph2uw %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtph2uw %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x7d,0xf7]
          vcvtph2uw %ymm23, %ymm22

// CHECK: vcvtph2uw %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x7d,0xf7]
          vcvtph2uw %ymm23, %ymm22 {%k7}

// CHECK: vcvtph2uw %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x7d,0xf7]
          vcvtph2uw %ymm23, %ymm22 {%k7} {z}

// CHECK: vcvtph2uw  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2uw  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtph2uw  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2uw  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtph2uw  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtph2uw  (%rip){1to8}, %xmm22

// CHECK: vcvtph2uw  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x7d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2uw  -512(,%rbp,2), %xmm22

// CHECK: vcvtph2uw  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x7d,0x71,0x7f]
          vcvtph2uw  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtph2uw  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x7d,0x72,0x80]
          vcvtph2uw  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtph2uw  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2uw  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtph2uw  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x2f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2uw  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtph2uw  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtph2uw  (%rip){1to16}, %ymm22

// CHECK: vcvtph2uw  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x7d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtph2uw  -1024(,%rbp,2), %ymm22

// CHECK: vcvtph2uw  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x7d,0x71,0x7f]
          vcvtph2uw  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtph2uw  -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x7d,0x72,0x80]
          vcvtph2uw  -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vcvtph2w %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7d,0xf7]
          vcvtph2w %xmm23, %xmm22

// CHECK: vcvtph2w %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x7d,0xf7]
          vcvtph2w %xmm23, %xmm22 {%k7}

// CHECK: vcvtph2w %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x7d,0xf7]
          vcvtph2w %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtph2w %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7d,0xf7]
          vcvtph2w %ymm23, %ymm22

// CHECK: vcvtph2w %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x7d,0xf7]
          vcvtph2w %ymm23, %ymm22 {%k7}

// CHECK: vcvtph2w %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x7d,0xf7]
          vcvtph2w %ymm23, %ymm22 {%k7} {z}

// CHECK: vcvtph2w  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2w  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtph2w  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2w  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtph2w  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtph2w  (%rip){1to8}, %xmm22

// CHECK: vcvtph2w  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2w  -512(,%rbp,2), %xmm22

// CHECK: vcvtph2w  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x7d,0x71,0x7f]
          vcvtph2w  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtph2w  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x7d,0x72,0x80]
          vcvtph2w  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtph2w  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2w  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtph2w  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2w  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtph2w  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtph2w  (%rip){1to16}, %ymm22

// CHECK: vcvtph2w  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x7d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtph2w  -1024(,%rbp,2), %ymm22

// CHECK: vcvtph2w  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x7d,0x71,0x7f]
          vcvtph2w  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtph2w  -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x7d,0x72,0x80]
          vcvtph2w  -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vcvttph2uw %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x7c,0xf7]
          vcvttph2uw %xmm23, %xmm22

// CHECK: vcvttph2uw %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x7c,0xf7]
          vcvttph2uw %xmm23, %xmm22 {%k7}

// CHECK: vcvttph2uw %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x7c,0xf7]
          vcvttph2uw %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvttph2uw %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x7c,0xf7]
          vcvttph2uw %ymm23, %ymm22

// CHECK: vcvttph2uw %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x7c,0xf7]
          vcvttph2uw %ymm23, %ymm22 {%k7}

// CHECK: vcvttph2uw %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x7c,0xf7]
          vcvttph2uw %ymm23, %ymm22 {%k7} {z}

// CHECK: vcvttph2uw  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x7c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2uw  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvttph2uw  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x7c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2uw  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvttph2uw  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x7c,0x35,0x00,0x00,0x00,0x00]
          vcvttph2uw  (%rip){1to8}, %xmm22

// CHECK: vcvttph2uw  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x7c,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2uw  -512(,%rbp,2), %xmm22

// CHECK: vcvttph2uw  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x7c,0x71,0x7f]
          vcvttph2uw  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvttph2uw  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x7c,0x72,0x80]
          vcvttph2uw  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvttph2uw  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x7c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2uw  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvttph2uw  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x2f,0x7c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2uw  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvttph2uw  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x7c,0x35,0x00,0x00,0x00,0x00]
          vcvttph2uw  (%rip){1to16}, %ymm22

// CHECK: vcvttph2uw  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x7c,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvttph2uw  -1024(,%rbp,2), %ymm22

// CHECK: vcvttph2uw  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x7c,0x71,0x7f]
          vcvttph2uw  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvttph2uw  -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x7c,0x72,0x80]
          vcvttph2uw  -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vcvttph2w %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7c,0xf7]
          vcvttph2w %xmm23, %xmm22

// CHECK: vcvttph2w %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x7c,0xf7]
          vcvttph2w %xmm23, %xmm22 {%k7}

// CHECK: vcvttph2w %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x7c,0xf7]
          vcvttph2w %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvttph2w %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7c,0xf7]
          vcvttph2w %ymm23, %ymm22

// CHECK: vcvttph2w %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x7c,0xf7]
          vcvttph2w %ymm23, %ymm22 {%k7}

// CHECK: vcvttph2w %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x7c,0xf7]
          vcvttph2w %ymm23, %ymm22 {%k7} {z}

// CHECK: vcvttph2w  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2w  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvttph2w  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x7c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2w  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvttph2w  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x7c,0x35,0x00,0x00,0x00,0x00]
          vcvttph2w  (%rip){1to8}, %xmm22

// CHECK: vcvttph2w  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7c,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2w  -512(,%rbp,2), %xmm22

// CHECK: vcvttph2w  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x7c,0x71,0x7f]
          vcvttph2w  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvttph2w  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x7c,0x72,0x80]
          vcvttph2w  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvttph2w  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2w  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvttph2w  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x7c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2w  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvttph2w  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x7c,0x35,0x00,0x00,0x00,0x00]
          vcvttph2w  (%rip){1to16}, %ymm22

// CHECK: vcvttph2w  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x7c,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvttph2w  -1024(,%rbp,2), %ymm22

// CHECK: vcvttph2w  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x7c,0x71,0x7f]
          vcvttph2w  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvttph2w  -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x7c,0x72,0x80]
          vcvttph2w  -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vcvtuw2ph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x7d,0xf7]
          vcvtuw2ph %xmm23, %xmm22

// CHECK: vcvtuw2ph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7f,0x0f,0x7d,0xf7]
          vcvtuw2ph %xmm23, %xmm22 {%k7}

// CHECK: vcvtuw2ph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7f,0x8f,0x7d,0xf7]
          vcvtuw2ph %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtuw2ph %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x28,0x7d,0xf7]
          vcvtuw2ph %ymm23, %ymm22

// CHECK: vcvtuw2ph %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7f,0x2f,0x7d,0xf7]
          vcvtuw2ph %ymm23, %ymm22 {%k7}

// CHECK: vcvtuw2ph %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7f,0xaf,0x7d,0xf7]
          vcvtuw2ph %ymm23, %ymm22 {%k7} {z}

// CHECK: vcvtuw2ph  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtuw2ph  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtuw2ph  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7f,0x0f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtuw2ph  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtuw2ph  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x18,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtuw2ph  (%rip){1to8}, %xmm22

// CHECK: vcvtuw2ph  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x08,0x7d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtuw2ph  -512(,%rbp,2), %xmm22

// CHECK: vcvtuw2ph  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0x8f,0x7d,0x71,0x7f]
          vcvtuw2ph  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtuw2ph  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0x9f,0x7d,0x72,0x80]
          vcvtuw2ph  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtuw2ph  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x28,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtuw2ph  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtuw2ph  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7f,0x2f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtuw2ph  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtuw2ph  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x38,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtuw2ph  (%rip){1to16}, %ymm22

// CHECK: vcvtuw2ph  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x28,0x7d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtuw2ph  -1024(,%rbp,2), %ymm22

// CHECK: vcvtuw2ph  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0xaf,0x7d,0x71,0x7f]
          vcvtuw2ph  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtuw2ph  -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0xbf,0x7d,0x72,0x80]
          vcvtuw2ph  -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vcvtw2ph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x7d,0xf7]
          vcvtw2ph %xmm23, %xmm22

// CHECK: vcvtw2ph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x0f,0x7d,0xf7]
          vcvtw2ph %xmm23, %xmm22 {%k7}

// CHECK: vcvtw2ph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0x8f,0x7d,0xf7]
          vcvtw2ph %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtw2ph %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x7d,0xf7]
          vcvtw2ph %ymm23, %ymm22

// CHECK: vcvtw2ph %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x2f,0x7d,0xf7]
          vcvtw2ph %ymm23, %ymm22 {%k7}

// CHECK: vcvtw2ph %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0xaf,0x7d,0xf7]
          vcvtw2ph %ymm23, %ymm22 {%k7} {z}

// CHECK: vcvtw2ph  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtw2ph  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtw2ph  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtw2ph  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtw2ph  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x18,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtw2ph  (%rip){1to8}, %xmm22

// CHECK: vcvtw2ph  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x7d,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtw2ph  -512(,%rbp,2), %xmm22

// CHECK: vcvtw2ph  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x7d,0x71,0x7f]
          vcvtw2ph  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtw2ph  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x9f,0x7d,0x72,0x80]
          vcvtw2ph  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtw2ph  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x7d,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtw2ph  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtw2ph  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x2f,0x7d,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtw2ph  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtw2ph  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x38,0x7d,0x35,0x00,0x00,0x00,0x00]
          vcvtw2ph  (%rip){1to16}, %ymm22

// CHECK: vcvtw2ph  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x7d,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtw2ph  -1024(,%rbp,2), %ymm22

// CHECK: vcvtw2ph  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x7d,0x71,0x7f]
          vcvtw2ph  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtw2ph  -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xbf,0x7d,0x72,0x80]
          vcvtw2ph  -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vcvtdq2ph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x5b,0xf7]
          vcvtdq2ph %xmm23, %xmm22

// CHECK: vcvtdq2ph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x5b,0xf7]
          vcvtdq2ph %xmm23, %xmm22 {%k7}

// CHECK: vcvtdq2ph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x5b,0xf7]
          vcvtdq2ph %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtdq2ph %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x5b,0xf7]
          vcvtdq2ph %ymm23, %xmm22

// CHECK: vcvtdq2ph %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x5b,0xf7]
          vcvtdq2ph %ymm23, %xmm22 {%k7}

// CHECK: vcvtdq2ph %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x5b,0xf7]
          vcvtdq2ph %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtdq2phx  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtdq2phx  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtdq2phx  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtdq2phx  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtdq2ph  (%rip){1to4}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtdq2ph  (%rip){1to4}, %xmm22

// CHECK: vcvtdq2phx  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x5b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtdq2phx  -512(,%rbp,2), %xmm22

// CHECK: vcvtdq2phx  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x5b,0x71,0x7f]
          vcvtdq2phx  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtdq2ph  -512(%rdx){1to4}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x5b,0x72,0x80]
          vcvtdq2ph  -512(%rdx){1to4}, %xmm22 {%k7} {z}

// CHECK: vcvtdq2ph  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtdq2ph  (%rip){1to8}, %xmm22

// CHECK: vcvtdq2phy  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x5b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtdq2phy  -1024(,%rbp,2), %xmm22

// CHECK: vcvtdq2phy  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x5b,0x71,0x7f]
          vcvtdq2phy  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtdq2ph  -512(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x5b,0x72,0x80]
          vcvtdq2ph  -512(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtph2dq %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x5b,0xf7]
          vcvtph2dq %xmm23, %xmm22

// CHECK: vcvtph2dq %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x5b,0xf7]
          vcvtph2dq %xmm23, %xmm22 {%k7}

// CHECK: vcvtph2dq %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x5b,0xf7]
          vcvtph2dq %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtph2dq %xmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x5b,0xf7]
          vcvtph2dq %xmm23, %ymm22

// CHECK: vcvtph2dq %xmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x5b,0xf7]
          vcvtph2dq %xmm23, %ymm22 {%k7}

// CHECK: vcvtph2dq %xmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x5b,0xf7]
          vcvtph2dq %xmm23, %ymm22 {%k7} {z}

// CHECK: vcvtph2dq  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2dq  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtph2dq  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2dq  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtph2dq  (%rip){1to4}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2dq  (%rip){1to4}, %xmm22

// CHECK: vcvtph2dq  -256(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x5b,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtph2dq  -256(,%rbp,2), %xmm22

// CHECK: vcvtph2dq  1016(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x5b,0x71,0x7f]
          vcvtph2dq  1016(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtph2dq  -256(%rdx){1to4}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x5b,0x72,0x80]
          vcvtph2dq  -256(%rdx){1to4}, %xmm22 {%k7} {z}

// CHECK: vcvtph2dq  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2dq  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtph2dq  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2dq  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtph2dq  (%rip){1to8}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2dq  (%rip){1to8}, %ymm22

// CHECK: vcvtph2dq  -512(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x5b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2dq  -512(,%rbp,2), %ymm22

// CHECK: vcvtph2dq  2032(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x5b,0x71,0x7f]
          vcvtph2dq  2032(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtph2dq  -256(%rdx){1to8}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x5b,0x72,0x80]
          vcvtph2dq  -256(%rdx){1to8}, %ymm22 {%k7} {z}

// CHECK: vcvtph2udq %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x79,0xf7]
          vcvtph2udq %xmm23, %xmm22

// CHECK: vcvtph2udq %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x79,0xf7]
          vcvtph2udq %xmm23, %xmm22 {%k7}

// CHECK: vcvtph2udq %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x79,0xf7]
          vcvtph2udq %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtph2udq %xmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x79,0xf7]
          vcvtph2udq %xmm23, %ymm22

// CHECK: vcvtph2udq %xmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x79,0xf7]
          vcvtph2udq %xmm23, %ymm22 {%k7}

// CHECK: vcvtph2udq %xmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x79,0xf7]
          vcvtph2udq %xmm23, %ymm22 {%k7} {z}

// CHECK: vcvtph2udq  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x79,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2udq  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtph2udq  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x79,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2udq  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtph2udq  (%rip){1to4}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x79,0x35,0x00,0x00,0x00,0x00]
          vcvtph2udq  (%rip){1to4}, %xmm22

// CHECK: vcvtph2udq  -256(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x79,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtph2udq  -256(,%rbp,2), %xmm22

// CHECK: vcvtph2udq  1016(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x79,0x71,0x7f]
          vcvtph2udq  1016(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtph2udq  -256(%rdx){1to4}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x79,0x72,0x80]
          vcvtph2udq  -256(%rdx){1to4}, %xmm22 {%k7} {z}

// CHECK: vcvtph2udq  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x79,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2udq  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtph2udq  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x2f,0x79,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2udq  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtph2udq  (%rip){1to8}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x79,0x35,0x00,0x00,0x00,0x00]
          vcvtph2udq  (%rip){1to8}, %ymm22

// CHECK: vcvtph2udq  -512(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x79,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtph2udq  -512(,%rbp,2), %ymm22

// CHECK: vcvtph2udq  2032(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x79,0x71,0x7f]
          vcvtph2udq  2032(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtph2udq  -256(%rdx){1to8}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x79,0x72,0x80]
          vcvtph2udq  -256(%rdx){1to8}, %ymm22 {%k7} {z}

// CHECK: vcvttph2dq %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x5b,0xf7]
          vcvttph2dq %xmm23, %xmm22

// CHECK: vcvttph2dq %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x0f,0x5b,0xf7]
          vcvttph2dq %xmm23, %xmm22 {%k7}

// CHECK: vcvttph2dq %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0x8f,0x5b,0xf7]
          vcvttph2dq %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvttph2dq %xmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x5b,0xf7]
          vcvttph2dq %xmm23, %ymm22

// CHECK: vcvttph2dq %xmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7e,0x2f,0x5b,0xf7]
          vcvttph2dq %xmm23, %ymm22 {%k7}

// CHECK: vcvttph2dq %xmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7e,0xaf,0x5b,0xf7]
          vcvttph2dq %xmm23, %ymm22 {%k7} {z}

// CHECK: vcvttph2dq  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x08,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2dq  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvttph2dq  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x0f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2dq  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvttph2dq  (%rip){1to4}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x18,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvttph2dq  (%rip){1to4}, %xmm22

// CHECK: vcvttph2dq  -256(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x08,0x5b,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvttph2dq  -256(,%rbp,2), %xmm22

// CHECK: vcvttph2dq  1016(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x8f,0x5b,0x71,0x7f]
          vcvttph2dq  1016(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvttph2dq  -256(%rdx){1to4}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0x9f,0x5b,0x72,0x80]
          vcvttph2dq  -256(%rdx){1to4}, %xmm22 {%k7} {z}

// CHECK: vcvttph2dq  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7e,0x28,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2dq  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvttph2dq  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7e,0x2f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2dq  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvttph2dq  (%rip){1to8}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x38,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvttph2dq  (%rip){1to8}, %ymm22

// CHECK: vcvttph2dq  -512(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7e,0x28,0x5b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2dq  -512(,%rbp,2), %ymm22

// CHECK: vcvttph2dq  2032(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xaf,0x5b,0x71,0x7f]
          vcvttph2dq  2032(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvttph2dq  -256(%rdx){1to8}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7e,0xbf,0x5b,0x72,0x80]
          vcvttph2dq  -256(%rdx){1to8}, %ymm22 {%k7} {z}

// CHECK: vcvttph2udq %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x78,0xf7]
          vcvttph2udq %xmm23, %xmm22

// CHECK: vcvttph2udq %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x78,0xf7]
          vcvttph2udq %xmm23, %xmm22 {%k7}

// CHECK: vcvttph2udq %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x78,0xf7]
          vcvttph2udq %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvttph2udq %xmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x78,0xf7]
          vcvttph2udq %xmm23, %ymm22

// CHECK: vcvttph2udq %xmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x78,0xf7]
          vcvttph2udq %xmm23, %ymm22 {%k7}

// CHECK: vcvttph2udq %xmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x78,0xf7]
          vcvttph2udq %xmm23, %ymm22 {%k7} {z}

// CHECK: vcvttph2udq  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x78,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2udq  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvttph2udq  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x78,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2udq  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvttph2udq  (%rip){1to4}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x78,0x35,0x00,0x00,0x00,0x00]
          vcvttph2udq  (%rip){1to4}, %xmm22

// CHECK: vcvttph2udq  -256(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x78,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvttph2udq  -256(,%rbp,2), %xmm22

// CHECK: vcvttph2udq  1016(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x78,0x71,0x7f]
          vcvttph2udq  1016(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvttph2udq  -256(%rdx){1to4}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x78,0x72,0x80]
          vcvttph2udq  -256(%rdx){1to4}, %xmm22 {%k7} {z}

// CHECK: vcvttph2udq  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x78,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2udq  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvttph2udq  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x2f,0x78,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2udq  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvttph2udq  (%rip){1to8}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x78,0x35,0x00,0x00,0x00,0x00]
          vcvttph2udq  (%rip){1to8}, %ymm22

// CHECK: vcvttph2udq  -512(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x78,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvttph2udq  -512(,%rbp,2), %ymm22

// CHECK: vcvttph2udq  2032(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x78,0x71,0x7f]
          vcvttph2udq  2032(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvttph2udq  -256(%rdx){1to8}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x78,0x72,0x80]
          vcvttph2udq  -256(%rdx){1to8}, %ymm22 {%k7} {z}

// CHECK: vcvtudq2ph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x7a,0xf7]
          vcvtudq2ph %xmm23, %xmm22

// CHECK: vcvtudq2ph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7f,0x0f,0x7a,0xf7]
          vcvtudq2ph %xmm23, %xmm22 {%k7}

// CHECK: vcvtudq2ph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7f,0x8f,0x7a,0xf7]
          vcvtudq2ph %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtudq2ph %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x28,0x7a,0xf7]
          vcvtudq2ph %ymm23, %xmm22

// CHECK: vcvtudq2ph %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7f,0x2f,0x7a,0xf7]
          vcvtudq2ph %ymm23, %xmm22 {%k7}

// CHECK: vcvtudq2ph %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7f,0xaf,0x7a,0xf7]
          vcvtudq2ph %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtudq2phx  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7f,0x08,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtudq2phx  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtudq2phx  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7f,0x0f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtudq2phx  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtudq2ph  (%rip){1to4}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x18,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvtudq2ph  (%rip){1to4}, %xmm22

// CHECK: vcvtudq2phx  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x08,0x7a,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtudq2phx  -512(,%rbp,2), %xmm22

// CHECK: vcvtudq2phx  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0x8f,0x7a,0x71,0x7f]
          vcvtudq2phx  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtudq2ph  -512(%rdx){1to4}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0x9f,0x7a,0x72,0x80]
          vcvtudq2ph  -512(%rdx){1to4}, %xmm22 {%k7} {z}

// CHECK: vcvtudq2ph  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x38,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvtudq2ph  (%rip){1to8}, %xmm22

// CHECK: vcvtudq2phy  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7f,0x28,0x7a,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtudq2phy  -1024(,%rbp,2), %xmm22

// CHECK: vcvtudq2phy  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0xaf,0x7a,0x71,0x7f]
          vcvtudq2phy  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtudq2ph  -512(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7f,0xbf,0x7a,0x72,0x80]
          vcvtudq2ph  -512(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vcvtph2qq %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7b,0xf7]
          vcvtph2qq %xmm23, %xmm22

// CHECK: vcvtph2qq %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x7b,0xf7]
          vcvtph2qq %xmm23, %xmm22 {%k7}

// CHECK: vcvtph2qq %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x7b,0xf7]
          vcvtph2qq %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtph2qq %xmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7b,0xf7]
          vcvtph2qq %xmm23, %ymm22

// CHECK: vcvtph2qq %xmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x7b,0xf7]
          vcvtph2qq %xmm23, %ymm22 {%k7}

// CHECK: vcvtph2qq %xmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x7b,0xf7]
          vcvtph2qq %xmm23, %ymm22 {%k7} {z}

// CHECK: vcvtph2qq  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2qq  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtph2qq  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x7b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2qq  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtph2qq  (%rip){1to2}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x7b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2qq  (%rip){1to2}, %xmm22

// CHECK: vcvtph2qq  -128(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7b,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvtph2qq  -128(,%rbp,2), %xmm22

// CHECK: vcvtph2qq  508(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x7b,0x71,0x7f]
          vcvtph2qq  508(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtph2qq  -256(%rdx){1to2}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x7b,0x72,0x80]
          vcvtph2qq  -256(%rdx){1to2}, %xmm22 {%k7} {z}

// CHECK: vcvtph2qq  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2qq  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtph2qq  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x7b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2qq  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtph2qq  (%rip){1to4}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x7b,0x35,0x00,0x00,0x00,0x00]
          vcvtph2qq  (%rip){1to4}, %ymm22

// CHECK: vcvtph2qq  -256(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x7b,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtph2qq  -256(,%rbp,2), %ymm22

// CHECK: vcvtph2qq  1016(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x7b,0x71,0x7f]
          vcvtph2qq  1016(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtph2qq  -256(%rdx){1to4}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x7b,0x72,0x80]
          vcvtph2qq  -256(%rdx){1to4}, %ymm22 {%k7} {z}

// CHECK: vcvtph2uqq %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x79,0xf7]
          vcvtph2uqq %xmm23, %xmm22

// CHECK: vcvtph2uqq %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x79,0xf7]
          vcvtph2uqq %xmm23, %xmm22 {%k7}

// CHECK: vcvtph2uqq %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x79,0xf7]
          vcvtph2uqq %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtph2uqq %xmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x79,0xf7]
          vcvtph2uqq %xmm23, %ymm22

// CHECK: vcvtph2uqq %xmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x79,0xf7]
          vcvtph2uqq %xmm23, %ymm22 {%k7}

// CHECK: vcvtph2uqq %xmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x79,0xf7]
          vcvtph2uqq %xmm23, %ymm22 {%k7} {z}

// CHECK: vcvtph2uqq  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x79,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2uqq  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtph2uqq  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x79,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2uqq  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtph2uqq  (%rip){1to2}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x79,0x35,0x00,0x00,0x00,0x00]
          vcvtph2uqq  (%rip){1to2}, %xmm22

// CHECK: vcvtph2uqq  -128(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x79,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvtph2uqq  -128(,%rbp,2), %xmm22

// CHECK: vcvtph2uqq  508(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x79,0x71,0x7f]
          vcvtph2uqq  508(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtph2uqq  -256(%rdx){1to2}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x79,0x72,0x80]
          vcvtph2uqq  -256(%rdx){1to2}, %xmm22 {%k7} {z}

// CHECK: vcvtph2uqq  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x79,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtph2uqq  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvtph2uqq  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x79,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtph2uqq  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvtph2uqq  (%rip){1to4}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x79,0x35,0x00,0x00,0x00,0x00]
          vcvtph2uqq  (%rip){1to4}, %ymm22

// CHECK: vcvtph2uqq  -256(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x79,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvtph2uqq  -256(,%rbp,2), %ymm22

// CHECK: vcvtph2uqq  1016(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x79,0x71,0x7f]
          vcvtph2uqq  1016(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvtph2uqq  -256(%rdx){1to4}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x79,0x72,0x80]
          vcvtph2uqq  -256(%rdx){1to4}, %ymm22 {%k7} {z}

// CHECK: vcvtqq2ph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xfc,0x08,0x5b,0xf7]
          vcvtqq2ph %xmm23, %xmm22

// CHECK: vcvtqq2ph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0xfc,0x0f,0x5b,0xf7]
          vcvtqq2ph %xmm23, %xmm22 {%k7}

// CHECK: vcvtqq2ph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0xfc,0x8f,0x5b,0xf7]
          vcvtqq2ph %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtqq2ph %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xfc,0x28,0x5b,0xf7]
          vcvtqq2ph %ymm23, %xmm22

// CHECK: vcvtqq2ph %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0xfc,0x2f,0x5b,0xf7]
          vcvtqq2ph %ymm23, %xmm22 {%k7}

// CHECK: vcvtqq2ph %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0xfc,0xaf,0x5b,0xf7]
          vcvtqq2ph %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtqq2phx  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0xfc,0x08,0x5b,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtqq2phx  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtqq2phx  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xfc,0x0f,0x5b,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtqq2phx  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtqq2ph  (%rip){1to2}, %xmm22
// CHECK: encoding: [0x62,0xe5,0xfc,0x18,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtqq2ph  (%rip){1to2}, %xmm22

// CHECK: vcvtqq2phx  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0xfc,0x08,0x5b,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtqq2phx  -512(,%rbp,2), %xmm22

// CHECK: vcvtqq2phx  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xfc,0x8f,0x5b,0x71,0x7f]
          vcvtqq2phx  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtqq2ph  -1024(%rdx){1to2}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xfc,0x9f,0x5b,0x72,0x80]
          vcvtqq2ph  -1024(%rdx){1to2}, %xmm22 {%k7} {z}

// CHECK: vcvtqq2ph  (%rip){1to4}, %xmm22
// CHECK: encoding: [0x62,0xe5,0xfc,0x38,0x5b,0x35,0x00,0x00,0x00,0x00]
          vcvtqq2ph  (%rip){1to4}, %xmm22

// CHECK: vcvtqq2phy  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0xfc,0x28,0x5b,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtqq2phy  -1024(,%rbp,2), %xmm22

// CHECK: vcvtqq2phy  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xfc,0xaf,0x5b,0x71,0x7f]
          vcvtqq2phy  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtqq2ph  -1024(%rdx){1to4}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xfc,0xbf,0x5b,0x72,0x80]
          vcvtqq2ph  -1024(%rdx){1to4}, %xmm22 {%k7} {z}

// CHECK: vcvttph2qq %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7a,0xf7]
          vcvttph2qq %xmm23, %xmm22

// CHECK: vcvttph2qq %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x7a,0xf7]
          vcvttph2qq %xmm23, %xmm22 {%k7}

// CHECK: vcvttph2qq %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x7a,0xf7]
          vcvttph2qq %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvttph2qq %xmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7a,0xf7]
          vcvttph2qq %xmm23, %ymm22

// CHECK: vcvttph2qq %xmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x7a,0xf7]
          vcvttph2qq %xmm23, %ymm22 {%k7}

// CHECK: vcvttph2qq %xmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x7a,0xf7]
          vcvttph2qq %xmm23, %ymm22 {%k7} {z}

// CHECK: vcvttph2qq  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2qq  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvttph2qq  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2qq  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvttph2qq  (%rip){1to2}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvttph2qq  (%rip){1to2}, %xmm22

// CHECK: vcvttph2qq  -128(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x7a,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvttph2qq  -128(,%rbp,2), %xmm22

// CHECK: vcvttph2qq  508(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x7a,0x71,0x7f]
          vcvttph2qq  508(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvttph2qq  -256(%rdx){1to2}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x7a,0x72,0x80]
          vcvttph2qq  -256(%rdx){1to2}, %xmm22 {%k7} {z}

// CHECK: vcvttph2qq  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2qq  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvttph2qq  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2qq  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvttph2qq  (%rip){1to4}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvttph2qq  (%rip){1to4}, %ymm22

// CHECK: vcvttph2qq  -256(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x7a,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvttph2qq  -256(,%rbp,2), %ymm22

// CHECK: vcvttph2qq  1016(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x7a,0x71,0x7f]
          vcvttph2qq  1016(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvttph2qq  -256(%rdx){1to4}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x7a,0x72,0x80]
          vcvttph2qq  -256(%rdx){1to4}, %ymm22 {%k7} {z}

// CHECK: vcvttph2uqq %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x78,0xf7]
          vcvttph2uqq %xmm23, %xmm22

// CHECK: vcvttph2uqq %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x0f,0x78,0xf7]
          vcvttph2uqq %xmm23, %xmm22 {%k7}

// CHECK: vcvttph2uqq %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0x8f,0x78,0xf7]
          vcvttph2uqq %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvttph2uqq %xmm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x78,0xf7]
          vcvttph2uqq %xmm23, %ymm22

// CHECK: vcvttph2uqq %xmm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7d,0x2f,0x78,0xf7]
          vcvttph2uqq %xmm23, %ymm22 {%k7}

// CHECK: vcvttph2uqq %xmm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7d,0xaf,0x78,0xf7]
          vcvttph2uqq %xmm23, %ymm22 {%k7} {z}

// CHECK: vcvttph2uqq  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x08,0x78,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2uqq  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvttph2uqq  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x0f,0x78,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2uqq  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvttph2uqq  (%rip){1to2}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x18,0x78,0x35,0x00,0x00,0x00,0x00]
          vcvttph2uqq  (%rip){1to2}, %xmm22

// CHECK: vcvttph2uqq  -128(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x08,0x78,0x34,0x6d,0x80,0xff,0xff,0xff]
          vcvttph2uqq  -128(,%rbp,2), %xmm22

// CHECK: vcvttph2uqq  508(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x8f,0x78,0x71,0x7f]
          vcvttph2uqq  508(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvttph2uqq  -256(%rdx){1to2}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0x9f,0x78,0x72,0x80]
          vcvttph2uqq  -256(%rdx){1to2}, %xmm22 {%k7} {z}

// CHECK: vcvttph2uqq  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7d,0x28,0x78,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvttph2uqq  268435456(%rbp,%r14,8), %ymm22

// CHECK: vcvttph2uqq  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7d,0x2f,0x78,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvttph2uqq  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vcvttph2uqq  (%rip){1to4}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x38,0x78,0x35,0x00,0x00,0x00,0x00]
          vcvttph2uqq  (%rip){1to4}, %ymm22

// CHECK: vcvttph2uqq  -256(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7d,0x28,0x78,0x34,0x6d,0x00,0xff,0xff,0xff]
          vcvttph2uqq  -256(,%rbp,2), %ymm22

// CHECK: vcvttph2uqq  1016(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xaf,0x78,0x71,0x7f]
          vcvttph2uqq  1016(%rcx), %ymm22 {%k7} {z}

// CHECK: vcvttph2uqq  -256(%rdx){1to4}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7d,0xbf,0x78,0x72,0x80]
          vcvttph2uqq  -256(%rdx){1to4}, %ymm22 {%k7} {z}

// CHECK: vcvtuqq2ph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xff,0x08,0x7a,0xf7]
          vcvtuqq2ph %xmm23, %xmm22

// CHECK: vcvtuqq2ph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0xff,0x0f,0x7a,0xf7]
          vcvtuqq2ph %xmm23, %xmm22 {%k7}

// CHECK: vcvtuqq2ph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0xff,0x8f,0x7a,0xf7]
          vcvtuqq2ph %xmm23, %xmm22 {%k7} {z}

// CHECK: vcvtuqq2ph %ymm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0xff,0x28,0x7a,0xf7]
          vcvtuqq2ph %ymm23, %xmm22

// CHECK: vcvtuqq2ph %ymm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0xff,0x2f,0x7a,0xf7]
          vcvtuqq2ph %ymm23, %xmm22 {%k7}

// CHECK: vcvtuqq2ph %ymm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0xff,0xaf,0x7a,0xf7]
          vcvtuqq2ph %ymm23, %xmm22 {%k7} {z}

// CHECK: vcvtuqq2phx  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0xff,0x08,0x7a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vcvtuqq2phx  268435456(%rbp,%r14,8), %xmm22

// CHECK: vcvtuqq2phx  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0xff,0x0f,0x7a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vcvtuqq2phx  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vcvtuqq2ph  (%rip){1to2}, %xmm22
// CHECK: encoding: [0x62,0xe5,0xff,0x18,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvtuqq2ph  (%rip){1to2}, %xmm22

// CHECK: vcvtuqq2phx  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0xff,0x08,0x7a,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vcvtuqq2phx  -512(,%rbp,2), %xmm22

// CHECK: vcvtuqq2phx  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xff,0x8f,0x7a,0x71,0x7f]
          vcvtuqq2phx  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtuqq2ph  -1024(%rdx){1to2}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xff,0x9f,0x7a,0x72,0x80]
          vcvtuqq2ph  -1024(%rdx){1to2}, %xmm22 {%k7} {z}

// CHECK: vcvtuqq2ph  (%rip){1to4}, %xmm22
// CHECK: encoding: [0x62,0xe5,0xff,0x38,0x7a,0x35,0x00,0x00,0x00,0x00]
          vcvtuqq2ph  (%rip){1to4}, %xmm22

// CHECK: vcvtuqq2phy  -1024(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0xff,0x28,0x7a,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vcvtuqq2phy  -1024(,%rbp,2), %xmm22

// CHECK: vcvtuqq2phy  4064(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xff,0xaf,0x7a,0x71,0x7f]
          vcvtuqq2phy  4064(%rcx), %xmm22 {%k7} {z}

// CHECK: vcvtuqq2ph  -1024(%rdx){1to4}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0xff,0xbf,0x7a,0x72,0x80]
          vcvtuqq2ph  -1024(%rdx){1to4}, %xmm22 {%k7} {z}

// CHECK: vfpclassph $123, %xmm22, %k5
// CHECK: encoding: [0x62,0xb3,0x7c,0x08,0x66,0xee,0x7b]
          vfpclassph $123, %xmm22, %k5

// CHECK: vfpclassph $123, %xmm22, %k5 {%k7}
// CHECK: encoding: [0x62,0xb3,0x7c,0x0f,0x66,0xee,0x7b]
          vfpclassph $123, %xmm22, %k5 {%k7}

// CHECK: vfpclassph $123, %ymm22, %k5
// CHECK: encoding: [0x62,0xb3,0x7c,0x28,0x66,0xee,0x7b]
          vfpclassph $123, %ymm22, %k5

// CHECK: vfpclassph $123, %ymm22, %k5 {%k7}
// CHECK: encoding: [0x62,0xb3,0x7c,0x2f,0x66,0xee,0x7b]
          vfpclassph $123, %ymm22, %k5 {%k7}

// CHECK: vfpclassphx  $123, 268435456(%rbp,%r14,8), %k5
// CHECK: encoding: [0x62,0xb3,0x7c,0x08,0x66,0xac,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vfpclassphx  $123, 268435456(%rbp,%r14,8), %k5

// CHECK: vfpclassphx  $123, 291(%r8,%rax,4), %k5 {%k7}
// CHECK: encoding: [0x62,0xd3,0x7c,0x0f,0x66,0xac,0x80,0x23,0x01,0x00,0x00,0x7b]
          vfpclassphx  $123, 291(%r8,%rax,4), %k5 {%k7}

// CHECK: vfpclassph  $123, (%rip){1to8}, %k5
// CHECK: encoding: [0x62,0xf3,0x7c,0x18,0x66,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vfpclassph  $123, (%rip){1to8}, %k5

// CHECK: vfpclassphx  $123, -512(,%rbp,2), %k5
// CHECK: encoding: [0x62,0xf3,0x7c,0x08,0x66,0x2c,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vfpclassphx  $123, -512(,%rbp,2), %k5

// CHECK: vfpclassphx  $123, 2032(%rcx), %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x0f,0x66,0x69,0x7f,0x7b]
          vfpclassphx  $123, 2032(%rcx), %k5 {%k7}

// CHECK: vfpclassph  $123, -256(%rdx){1to8}, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x1f,0x66,0x6a,0x80,0x7b]
          vfpclassph  $123, -256(%rdx){1to8}, %k5 {%k7}

// CHECK: vfpclassph  $123, (%rip){1to16}, %k5
// CHECK: encoding: [0x62,0xf3,0x7c,0x38,0x66,0x2d,0x00,0x00,0x00,0x00,0x7b]
          vfpclassph  $123, (%rip){1to16}, %k5

// CHECK: vfpclassphy  $123, -1024(,%rbp,2), %k5
// CHECK: encoding: [0x62,0xf3,0x7c,0x28,0x66,0x2c,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vfpclassphy  $123, -1024(,%rbp,2), %k5

// CHECK: vfpclassphy  $123, 4064(%rcx), %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x2f,0x66,0x69,0x7f,0x7b]
          vfpclassphy  $123, 4064(%rcx), %k5 {%k7}

// CHECK: vfpclassph  $123, -256(%rdx){1to16}, %k5 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x3f,0x66,0x6a,0x80,0x7b]
          vfpclassph  $123, -256(%rdx){1to16}, %k5 {%k7}

// CHECK: vgetexpph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x42,0xf7]
          vgetexpph %xmm23, %xmm22

// CHECK: vgetexpph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa6,0x7d,0x0f,0x42,0xf7]
          vgetexpph %xmm23, %xmm22 {%k7}

// CHECK: vgetexpph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa6,0x7d,0x8f,0x42,0xf7]
          vgetexpph %xmm23, %xmm22 {%k7} {z}

// CHECK: vgetexpph %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x42,0xf7]
          vgetexpph %ymm23, %ymm22

// CHECK: vgetexpph %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa6,0x7d,0x2f,0x42,0xf7]
          vgetexpph %ymm23, %ymm22 {%k7}

// CHECK: vgetexpph %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa6,0x7d,0xaf,0x42,0xf7]
          vgetexpph %ymm23, %ymm22 {%k7} {z}

// CHECK: vgetexpph  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x42,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vgetexpph  268435456(%rbp,%r14,8), %xmm22

// CHECK: vgetexpph  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x7d,0x0f,0x42,0xb4,0x80,0x23,0x01,0x00,0x00]
          vgetexpph  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vgetexpph  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x18,0x42,0x35,0x00,0x00,0x00,0x00]
          vgetexpph  (%rip){1to8}, %xmm22

// CHECK: vgetexpph  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x08,0x42,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vgetexpph  -512(,%rbp,2), %xmm22

// CHECK: vgetexpph  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0x8f,0x42,0x71,0x7f]
          vgetexpph  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vgetexpph  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0x9f,0x42,0x72,0x80]
          vgetexpph  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vgetexpph  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x42,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vgetexpph  268435456(%rbp,%r14,8), %ymm22

// CHECK: vgetexpph  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x7d,0x2f,0x42,0xb4,0x80,0x23,0x01,0x00,0x00]
          vgetexpph  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vgetexpph  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x38,0x42,0x35,0x00,0x00,0x00,0x00]
          vgetexpph  (%rip){1to16}, %ymm22

// CHECK: vgetexpph  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x28,0x42,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vgetexpph  -1024(,%rbp,2), %ymm22

// CHECK: vgetexpph  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xaf,0x42,0x71,0x7f]
          vgetexpph  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vgetexpph  -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xbf,0x42,0x72,0x80]
          vgetexpph  -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vgetmantph $123, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x28,0x26,0xf7,0x7b]
          vgetmantph $123, %ymm23, %ymm22

// CHECK: vgetmantph $123, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa3,0x7c,0x2f,0x26,0xf7,0x7b]
          vgetmantph $123, %ymm23, %ymm22 {%k7}

// CHECK: vgetmantph $123, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa3,0x7c,0xaf,0x26,0xf7,0x7b]
          vgetmantph $123, %ymm23, %ymm22 {%k7} {z}

// CHECK: vgetmantph $123, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x08,0x26,0xf7,0x7b]
          vgetmantph $123, %xmm23, %xmm22

// CHECK: vgetmantph $123, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa3,0x7c,0x0f,0x26,0xf7,0x7b]
          vgetmantph $123, %xmm23, %xmm22 {%k7}

// CHECK: vgetmantph $123, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa3,0x7c,0x8f,0x26,0xf7,0x7b]
          vgetmantph $123, %xmm23, %xmm22 {%k7} {z}

// CHECK: vgetmantph  $123, 268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x08,0x26,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vgetmantph  $123, 268435456(%rbp,%r14,8), %xmm22

// CHECK: vgetmantph  $123, 291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x7c,0x0f,0x26,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vgetmantph  $123, 291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vgetmantph  $123, (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x18,0x26,0x35,0x00,0x00,0x00,0x00,0x7b]
          vgetmantph  $123, (%rip){1to8}, %xmm22

// CHECK: vgetmantph  $123, -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x08,0x26,0x34,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vgetmantph  $123, -512(,%rbp,2), %xmm22

// CHECK: vgetmantph  $123, 2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0x8f,0x26,0x71,0x7f,0x7b]
          vgetmantph  $123, 2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vgetmantph  $123, -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0x9f,0x26,0x72,0x80,0x7b]
          vgetmantph  $123, -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vgetmantph  $123, 268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x28,0x26,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vgetmantph  $123, 268435456(%rbp,%r14,8), %ymm22

// CHECK: vgetmantph  $123, 291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x7c,0x2f,0x26,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vgetmantph  $123, 291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vgetmantph  $123, (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x38,0x26,0x35,0x00,0x00,0x00,0x00,0x7b]
          vgetmantph  $123, (%rip){1to16}, %ymm22

// CHECK: vgetmantph  $123, -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x28,0x26,0x34,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vgetmantph  $123, -1024(,%rbp,2), %ymm22

// CHECK: vgetmantph  $123, 4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0xaf,0x26,0x71,0x7f,0x7b]
          vgetmantph  $123, 4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vgetmantph  $123, -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0xbf,0x26,0x72,0x80,0x7b]
          vgetmantph  $123, -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vrcpph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x4c,0xf7]
          vrcpph %xmm23, %xmm22

// CHECK: vrcpph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa6,0x7d,0x0f,0x4c,0xf7]
          vrcpph %xmm23, %xmm22 {%k7}

// CHECK: vrcpph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa6,0x7d,0x8f,0x4c,0xf7]
          vrcpph %xmm23, %xmm22 {%k7} {z}

// CHECK: vrcpph %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x4c,0xf7]
          vrcpph %ymm23, %ymm22

// CHECK: vrcpph %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa6,0x7d,0x2f,0x4c,0xf7]
          vrcpph %ymm23, %ymm22 {%k7}

// CHECK: vrcpph %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa6,0x7d,0xaf,0x4c,0xf7]
          vrcpph %ymm23, %ymm22 {%k7} {z}

// CHECK: vrcpph  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x4c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrcpph  268435456(%rbp,%r14,8), %xmm22

// CHECK: vrcpph  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x7d,0x0f,0x4c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrcpph  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vrcpph  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x18,0x4c,0x35,0x00,0x00,0x00,0x00]
          vrcpph  (%rip){1to8}, %xmm22

// CHECK: vrcpph  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x08,0x4c,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vrcpph  -512(,%rbp,2), %xmm22

// CHECK: vrcpph  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0x8f,0x4c,0x71,0x7f]
          vrcpph  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vrcpph  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0x9f,0x4c,0x72,0x80]
          vrcpph  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vrcpph  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x4c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrcpph  268435456(%rbp,%r14,8), %ymm22

// CHECK: vrcpph  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x7d,0x2f,0x4c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrcpph  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vrcpph  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x38,0x4c,0x35,0x00,0x00,0x00,0x00]
          vrcpph  (%rip){1to16}, %ymm22

// CHECK: vrcpph  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x28,0x4c,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vrcpph  -1024(,%rbp,2), %ymm22

// CHECK: vrcpph  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xaf,0x4c,0x71,0x7f]
          vrcpph  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vrcpph  -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xbf,0x4c,0x72,0x80]
          vrcpph  -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vreduceph $123, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x28,0x56,0xf7,0x7b]
          vreduceph $123, %ymm23, %ymm22

// CHECK: vreduceph $123, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa3,0x7c,0x2f,0x56,0xf7,0x7b]
          vreduceph $123, %ymm23, %ymm22 {%k7}

// CHECK: vreduceph $123, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa3,0x7c,0xaf,0x56,0xf7,0x7b]
          vreduceph $123, %ymm23, %ymm22 {%k7} {z}

// CHECK: vreduceph $123, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x08,0x56,0xf7,0x7b]
          vreduceph $123, %xmm23, %xmm22

// CHECK: vreduceph $123, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa3,0x7c,0x0f,0x56,0xf7,0x7b]
          vreduceph $123, %xmm23, %xmm22 {%k7}

// CHECK: vreduceph $123, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa3,0x7c,0x8f,0x56,0xf7,0x7b]
          vreduceph $123, %xmm23, %xmm22 {%k7} {z}

// CHECK: vreduceph  $123, 268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x08,0x56,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vreduceph  $123, 268435456(%rbp,%r14,8), %xmm22

// CHECK: vreduceph  $123, 291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x7c,0x0f,0x56,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vreduceph  $123, 291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vreduceph  $123, (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x18,0x56,0x35,0x00,0x00,0x00,0x00,0x7b]
          vreduceph  $123, (%rip){1to8}, %xmm22

// CHECK: vreduceph  $123, -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x08,0x56,0x34,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vreduceph  $123, -512(,%rbp,2), %xmm22

// CHECK: vreduceph  $123, 2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0x8f,0x56,0x71,0x7f,0x7b]
          vreduceph  $123, 2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vreduceph  $123, -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0x9f,0x56,0x72,0x80,0x7b]
          vreduceph  $123, -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vreduceph  $123, 268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x28,0x56,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vreduceph  $123, 268435456(%rbp,%r14,8), %ymm22

// CHECK: vreduceph  $123, 291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x7c,0x2f,0x56,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vreduceph  $123, 291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vreduceph  $123, (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x38,0x56,0x35,0x00,0x00,0x00,0x00,0x7b]
          vreduceph  $123, (%rip){1to16}, %ymm22

// CHECK: vreduceph  $123, -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x28,0x56,0x34,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vreduceph  $123, -1024(,%rbp,2), %ymm22

// CHECK: vreduceph  $123, 4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0xaf,0x56,0x71,0x7f,0x7b]
          vreduceph  $123, 4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vreduceph  $123, -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0xbf,0x56,0x72,0x80,0x7b]
          vreduceph  $123, -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vrndscaleph $123, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x28,0x08,0xf7,0x7b]
          vrndscaleph $123, %ymm23, %ymm22

// CHECK: vrndscaleph $123, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa3,0x7c,0x2f,0x08,0xf7,0x7b]
          vrndscaleph $123, %ymm23, %ymm22 {%k7}

// CHECK: vrndscaleph $123, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa3,0x7c,0xaf,0x08,0xf7,0x7b]
          vrndscaleph $123, %ymm23, %ymm22 {%k7} {z}

// CHECK: vrndscaleph $123, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x08,0x08,0xf7,0x7b]
          vrndscaleph $123, %xmm23, %xmm22

// CHECK: vrndscaleph $123, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa3,0x7c,0x0f,0x08,0xf7,0x7b]
          vrndscaleph $123, %xmm23, %xmm22 {%k7}

// CHECK: vrndscaleph $123, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa3,0x7c,0x8f,0x08,0xf7,0x7b]
          vrndscaleph $123, %xmm23, %xmm22 {%k7} {z}

// CHECK: vrndscaleph  $123, 268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x08,0x08,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vrndscaleph  $123, 268435456(%rbp,%r14,8), %xmm22

// CHECK: vrndscaleph  $123, 291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x7c,0x0f,0x08,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vrndscaleph  $123, 291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vrndscaleph  $123, (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x18,0x08,0x35,0x00,0x00,0x00,0x00,0x7b]
          vrndscaleph  $123, (%rip){1to8}, %xmm22

// CHECK: vrndscaleph  $123, -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x08,0x08,0x34,0x6d,0x00,0xfe,0xff,0xff,0x7b]
          vrndscaleph  $123, -512(,%rbp,2), %xmm22

// CHECK: vrndscaleph  $123, 2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0x8f,0x08,0x71,0x7f,0x7b]
          vrndscaleph  $123, 2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vrndscaleph  $123, -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0x9f,0x08,0x72,0x80,0x7b]
          vrndscaleph  $123, -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vrndscaleph  $123, 268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa3,0x7c,0x28,0x08,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
          vrndscaleph  $123, 268435456(%rbp,%r14,8), %ymm22

// CHECK: vrndscaleph  $123, 291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc3,0x7c,0x2f,0x08,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
          vrndscaleph  $123, 291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vrndscaleph  $123, (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x38,0x08,0x35,0x00,0x00,0x00,0x00,0x7b]
          vrndscaleph  $123, (%rip){1to16}, %ymm22

// CHECK: vrndscaleph  $123, -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x28,0x08,0x34,0x6d,0x00,0xfc,0xff,0xff,0x7b]
          vrndscaleph  $123, -1024(,%rbp,2), %ymm22

// CHECK: vrndscaleph  $123, 4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0xaf,0x08,0x71,0x7f,0x7b]
          vrndscaleph  $123, 4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vrndscaleph  $123, -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe3,0x7c,0xbf,0x08,0x72,0x80,0x7b]
          vrndscaleph  $123, -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vrsqrtph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x4e,0xf7]
          vrsqrtph %xmm23, %xmm22

// CHECK: vrsqrtph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa6,0x7d,0x0f,0x4e,0xf7]
          vrsqrtph %xmm23, %xmm22 {%k7}

// CHECK: vrsqrtph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa6,0x7d,0x8f,0x4e,0xf7]
          vrsqrtph %xmm23, %xmm22 {%k7} {z}

// CHECK: vrsqrtph %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x4e,0xf7]
          vrsqrtph %ymm23, %ymm22

// CHECK: vrsqrtph %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa6,0x7d,0x2f,0x4e,0xf7]
          vrsqrtph %ymm23, %ymm22 {%k7}

// CHECK: vrsqrtph %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa6,0x7d,0xaf,0x4e,0xf7]
          vrsqrtph %ymm23, %ymm22 {%k7} {z}

// CHECK: vrsqrtph  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x08,0x4e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrsqrtph  268435456(%rbp,%r14,8), %xmm22

// CHECK: vrsqrtph  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x7d,0x0f,0x4e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrsqrtph  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vrsqrtph  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x18,0x4e,0x35,0x00,0x00,0x00,0x00]
          vrsqrtph  (%rip){1to8}, %xmm22

// CHECK: vrsqrtph  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x08,0x4e,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vrsqrtph  -512(,%rbp,2), %xmm22

// CHECK: vrsqrtph  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0x8f,0x4e,0x71,0x7f]
          vrsqrtph  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vrsqrtph  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0x9f,0x4e,0x72,0x80]
          vrsqrtph  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vrsqrtph  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa6,0x7d,0x28,0x4e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vrsqrtph  268435456(%rbp,%r14,8), %ymm22

// CHECK: vrsqrtph  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x7d,0x2f,0x4e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vrsqrtph  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vrsqrtph  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x38,0x4e,0x35,0x00,0x00,0x00,0x00]
          vrsqrtph  (%rip){1to16}, %ymm22

// CHECK: vrsqrtph  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe6,0x7d,0x28,0x4e,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vrsqrtph  -1024(,%rbp,2), %ymm22

// CHECK: vrsqrtph  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xaf,0x4e,0x71,0x7f]
          vrsqrtph  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vrsqrtph  -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x7d,0xbf,0x4e,0x72,0x80]
          vrsqrtph  -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vscalefph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x2c,0xf0]
          vscalefph %ymm24, %ymm23, %ymm22

// CHECK: vscalefph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x2c,0xf0]
          vscalefph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vscalefph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x2c,0xf0]
          vscalefph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vscalefph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x2c,0xf0]
          vscalefph %xmm24, %xmm23, %xmm22

// CHECK: vscalefph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x2c,0xf0]
          vscalefph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vscalefph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x2c,0xf0]
          vscalefph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vscalefph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x2c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vscalefph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vscalefph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x2c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vscalefph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vscalefph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x2c,0x35,0x00,0x00,0x00,0x00]
          vscalefph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vscalefph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x2c,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vscalefph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vscalefph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x2c,0x71,0x7f]
          vscalefph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vscalefph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x2c,0x72,0x80]
          vscalefph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vscalefph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x2c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vscalefph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vscalefph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x2c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vscalefph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vscalefph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x2c,0x35,0x00,0x00,0x00,0x00]
          vscalefph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vscalefph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x2c,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vscalefph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vscalefph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x2c,0x71,0x7f]
          vscalefph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vscalefph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x2c,0x72,0x80]
          vscalefph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vsqrtph %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x51,0xf7]
          vsqrtph %xmm23, %xmm22

// CHECK: vsqrtph %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x0f,0x51,0xf7]
          vsqrtph %xmm23, %xmm22 {%k7}

// CHECK: vsqrtph %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0x8f,0x51,0xf7]
          vsqrtph %xmm23, %xmm22 {%k7} {z}

// CHECK: vsqrtph %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x51,0xf7]
          vsqrtph %ymm23, %ymm22

// CHECK: vsqrtph %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xa5,0x7c,0x2f,0x51,0xf7]
          vsqrtph %ymm23, %ymm22 {%k7}

// CHECK: vsqrtph %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xa5,0x7c,0xaf,0x51,0xf7]
          vsqrtph %ymm23, %ymm22 {%k7} {z}

// CHECK: vsqrtph  268435456(%rbp,%r14,8), %xmm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x08,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsqrtph  268435456(%rbp,%r14,8), %xmm22

// CHECK: vsqrtph  291(%r8,%rax,4), %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x0f,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsqrtph  291(%r8,%rax,4), %xmm22 {%k7}

// CHECK: vsqrtph  (%rip){1to8}, %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x18,0x51,0x35,0x00,0x00,0x00,0x00]
          vsqrtph  (%rip){1to8}, %xmm22

// CHECK: vsqrtph  -512(,%rbp,2), %xmm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x08,0x51,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vsqrtph  -512(,%rbp,2), %xmm22

// CHECK: vsqrtph  2032(%rcx), %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x8f,0x51,0x71,0x7f]
          vsqrtph  2032(%rcx), %xmm22 {%k7} {z}

// CHECK: vsqrtph  -256(%rdx){1to8}, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0x9f,0x51,0x72,0x80]
          vsqrtph  -256(%rdx){1to8}, %xmm22 {%k7} {z}

// CHECK: vsqrtph  268435456(%rbp,%r14,8), %ymm22
// CHECK: encoding: [0x62,0xa5,0x7c,0x28,0x51,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vsqrtph  268435456(%rbp,%r14,8), %ymm22

// CHECK: vsqrtph  291(%r8,%rax,4), %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc5,0x7c,0x2f,0x51,0xb4,0x80,0x23,0x01,0x00,0x00]
          vsqrtph  291(%r8,%rax,4), %ymm22 {%k7}

// CHECK: vsqrtph  (%rip){1to16}, %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x38,0x51,0x35,0x00,0x00,0x00,0x00]
          vsqrtph  (%rip){1to16}, %ymm22

// CHECK: vsqrtph  -1024(,%rbp,2), %ymm22
// CHECK: encoding: [0x62,0xe5,0x7c,0x28,0x51,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vsqrtph  -1024(,%rbp,2), %ymm22

// CHECK: vsqrtph  4064(%rcx), %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xaf,0x51,0x71,0x7f]
          vsqrtph  4064(%rcx), %ymm22 {%k7} {z}

// CHECK: vsqrtph  -256(%rdx){1to16}, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe5,0x7c,0xbf,0x51,0x72,0x80]
          vsqrtph  -256(%rdx){1to16}, %ymm22 {%k7} {z}

// CHECK: vfmadd132ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x98,0xf0]
          vfmadd132ph %ymm24, %ymm23, %ymm22

// CHECK: vfmadd132ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x98,0xf0]
          vfmadd132ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmadd132ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x98,0xf0]
          vfmadd132ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmadd132ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x98,0xf0]
          vfmadd132ph %xmm24, %xmm23, %xmm22

// CHECK: vfmadd132ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x98,0xf0]
          vfmadd132ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmadd132ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x98,0xf0]
          vfmadd132ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd132ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x98,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd132ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmadd132ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x98,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd132ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmadd132ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x98,0x35,0x00,0x00,0x00,0x00]
          vfmadd132ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfmadd132ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x98,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmadd132ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmadd132ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x98,0x71,0x7f]
          vfmadd132ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmadd132ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x98,0x72,0x80]
          vfmadd132ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmadd132ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x98,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd132ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmadd132ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x98,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd132ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmadd132ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x98,0x35,0x00,0x00,0x00,0x00]
          vfmadd132ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfmadd132ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x98,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmadd132ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmadd132ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x98,0x71,0x7f]
          vfmadd132ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd132ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x98,0x72,0x80]
          vfmadd132ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd213ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xa8,0xf0]
          vfmadd213ph %ymm24, %ymm23, %ymm22

// CHECK: vfmadd213ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xa8,0xf0]
          vfmadd213ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmadd213ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xa8,0xf0]
          vfmadd213ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmadd213ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xa8,0xf0]
          vfmadd213ph %xmm24, %xmm23, %xmm22

// CHECK: vfmadd213ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xa8,0xf0]
          vfmadd213ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmadd213ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xa8,0xf0]
          vfmadd213ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd213ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xa8,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd213ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmadd213ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xa8,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd213ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmadd213ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xa8,0x35,0x00,0x00,0x00,0x00]
          vfmadd213ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfmadd213ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xa8,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmadd213ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmadd213ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xa8,0x71,0x7f]
          vfmadd213ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmadd213ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xa8,0x72,0x80]
          vfmadd213ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmadd213ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xa8,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd213ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmadd213ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xa8,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd213ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmadd213ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xa8,0x35,0x00,0x00,0x00,0x00]
          vfmadd213ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfmadd213ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xa8,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmadd213ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmadd213ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xa8,0x71,0x7f]
          vfmadd213ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd213ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xa8,0x72,0x80]
          vfmadd213ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd231ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xb8,0xf0]
          vfmadd231ph %ymm24, %ymm23, %ymm22

// CHECK: vfmadd231ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xb8,0xf0]
          vfmadd231ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmadd231ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xb8,0xf0]
          vfmadd231ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmadd231ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xb8,0xf0]
          vfmadd231ph %xmm24, %xmm23, %xmm22

// CHECK: vfmadd231ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xb8,0xf0]
          vfmadd231ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmadd231ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xb8,0xf0]
          vfmadd231ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd231ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xb8,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd231ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmadd231ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xb8,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd231ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmadd231ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xb8,0x35,0x00,0x00,0x00,0x00]
          vfmadd231ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfmadd231ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xb8,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmadd231ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmadd231ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xb8,0x71,0x7f]
          vfmadd231ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmadd231ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xb8,0x72,0x80]
          vfmadd231ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmadd231ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xb8,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmadd231ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmadd231ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xb8,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmadd231ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmadd231ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xb8,0x35,0x00,0x00,0x00,0x00]
          vfmadd231ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfmadd231ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xb8,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmadd231ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmadd231ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xb8,0x71,0x7f]
          vfmadd231ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmadd231ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xb8,0x72,0x80]
          vfmadd231ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddsub132ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x96,0xf0]
          vfmaddsub132ph %ymm24, %ymm23, %ymm22

// CHECK: vfmaddsub132ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x96,0xf0]
          vfmaddsub132ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmaddsub132ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x96,0xf0]
          vfmaddsub132ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmaddsub132ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x96,0xf0]
          vfmaddsub132ph %xmm24, %xmm23, %xmm22

// CHECK: vfmaddsub132ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x96,0xf0]
          vfmaddsub132ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmaddsub132ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x96,0xf0]
          vfmaddsub132ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddsub132ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x96,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub132ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmaddsub132ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x96,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub132ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmaddsub132ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x96,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub132ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfmaddsub132ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x96,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmaddsub132ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmaddsub132ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x96,0x71,0x7f]
          vfmaddsub132ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmaddsub132ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x96,0x72,0x80]
          vfmaddsub132ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmaddsub132ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x96,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub132ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmaddsub132ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x96,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub132ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmaddsub132ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x96,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub132ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfmaddsub132ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x96,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmaddsub132ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmaddsub132ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x96,0x71,0x7f]
          vfmaddsub132ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddsub132ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x96,0x72,0x80]
          vfmaddsub132ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddsub213ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xa6,0xf0]
          vfmaddsub213ph %ymm24, %ymm23, %ymm22

// CHECK: vfmaddsub213ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xa6,0xf0]
          vfmaddsub213ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmaddsub213ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xa6,0xf0]
          vfmaddsub213ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmaddsub213ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xa6,0xf0]
          vfmaddsub213ph %xmm24, %xmm23, %xmm22

// CHECK: vfmaddsub213ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xa6,0xf0]
          vfmaddsub213ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmaddsub213ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xa6,0xf0]
          vfmaddsub213ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddsub213ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xa6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub213ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmaddsub213ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xa6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub213ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmaddsub213ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xa6,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub213ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfmaddsub213ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xa6,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmaddsub213ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmaddsub213ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xa6,0x71,0x7f]
          vfmaddsub213ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmaddsub213ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xa6,0x72,0x80]
          vfmaddsub213ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmaddsub213ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xa6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub213ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmaddsub213ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xa6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub213ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmaddsub213ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xa6,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub213ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfmaddsub213ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xa6,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmaddsub213ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmaddsub213ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xa6,0x71,0x7f]
          vfmaddsub213ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddsub213ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xa6,0x72,0x80]
          vfmaddsub213ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddsub231ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xb6,0xf0]
          vfmaddsub231ph %ymm24, %ymm23, %ymm22

// CHECK: vfmaddsub231ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xb6,0xf0]
          vfmaddsub231ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmaddsub231ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xb6,0xf0]
          vfmaddsub231ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmaddsub231ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xb6,0xf0]
          vfmaddsub231ph %xmm24, %xmm23, %xmm22

// CHECK: vfmaddsub231ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xb6,0xf0]
          vfmaddsub231ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmaddsub231ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xb6,0xf0]
          vfmaddsub231ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddsub231ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xb6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub231ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmaddsub231ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xb6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub231ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmaddsub231ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xb6,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub231ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfmaddsub231ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xb6,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmaddsub231ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmaddsub231ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xb6,0x71,0x7f]
          vfmaddsub231ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmaddsub231ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xb6,0x72,0x80]
          vfmaddsub231ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmaddsub231ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xb6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddsub231ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmaddsub231ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xb6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddsub231ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmaddsub231ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xb6,0x35,0x00,0x00,0x00,0x00]
          vfmaddsub231ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfmaddsub231ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xb6,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmaddsub231ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmaddsub231ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xb6,0x71,0x7f]
          vfmaddsub231ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddsub231ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xb6,0x72,0x80]
          vfmaddsub231ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub132ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x9a,0xf0]
          vfmsub132ph %ymm24, %ymm23, %ymm22

// CHECK: vfmsub132ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x9a,0xf0]
          vfmsub132ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmsub132ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x9a,0xf0]
          vfmsub132ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsub132ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x9a,0xf0]
          vfmsub132ph %xmm24, %xmm23, %xmm22

// CHECK: vfmsub132ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x9a,0xf0]
          vfmsub132ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmsub132ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x9a,0xf0]
          vfmsub132ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub132ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x9a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub132ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmsub132ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x9a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub132ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmsub132ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x9a,0x35,0x00,0x00,0x00,0x00]
          vfmsub132ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfmsub132ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x9a,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmsub132ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmsub132ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x9a,0x71,0x7f]
          vfmsub132ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsub132ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x9a,0x72,0x80]
          vfmsub132ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsub132ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x9a,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub132ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmsub132ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x9a,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub132ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmsub132ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x9a,0x35,0x00,0x00,0x00,0x00]
          vfmsub132ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfmsub132ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x9a,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmsub132ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmsub132ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x9a,0x71,0x7f]
          vfmsub132ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub132ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x9a,0x72,0x80]
          vfmsub132ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub213ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xaa,0xf0]
          vfmsub213ph %ymm24, %ymm23, %ymm22

// CHECK: vfmsub213ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xaa,0xf0]
          vfmsub213ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmsub213ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xaa,0xf0]
          vfmsub213ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsub213ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xaa,0xf0]
          vfmsub213ph %xmm24, %xmm23, %xmm22

// CHECK: vfmsub213ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xaa,0xf0]
          vfmsub213ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmsub213ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xaa,0xf0]
          vfmsub213ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub213ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xaa,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub213ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmsub213ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xaa,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub213ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmsub213ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xaa,0x35,0x00,0x00,0x00,0x00]
          vfmsub213ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfmsub213ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xaa,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmsub213ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmsub213ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xaa,0x71,0x7f]
          vfmsub213ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsub213ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xaa,0x72,0x80]
          vfmsub213ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsub213ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xaa,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub213ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmsub213ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xaa,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub213ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmsub213ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xaa,0x35,0x00,0x00,0x00,0x00]
          vfmsub213ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfmsub213ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xaa,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmsub213ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmsub213ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xaa,0x71,0x7f]
          vfmsub213ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub213ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xaa,0x72,0x80]
          vfmsub213ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub231ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xba,0xf0]
          vfmsub231ph %ymm24, %ymm23, %ymm22

// CHECK: vfmsub231ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xba,0xf0]
          vfmsub231ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmsub231ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xba,0xf0]
          vfmsub231ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsub231ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xba,0xf0]
          vfmsub231ph %xmm24, %xmm23, %xmm22

// CHECK: vfmsub231ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xba,0xf0]
          vfmsub231ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmsub231ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xba,0xf0]
          vfmsub231ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub231ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xba,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub231ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmsub231ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xba,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub231ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmsub231ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xba,0x35,0x00,0x00,0x00,0x00]
          vfmsub231ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfmsub231ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xba,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmsub231ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmsub231ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xba,0x71,0x7f]
          vfmsub231ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsub231ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xba,0x72,0x80]
          vfmsub231ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsub231ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xba,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsub231ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmsub231ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xba,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsub231ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmsub231ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xba,0x35,0x00,0x00,0x00,0x00]
          vfmsub231ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfmsub231ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xba,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmsub231ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmsub231ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xba,0x71,0x7f]
          vfmsub231ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsub231ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xba,0x72,0x80]
          vfmsub231ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsubadd132ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x97,0xf0]
          vfmsubadd132ph %ymm24, %ymm23, %ymm22

// CHECK: vfmsubadd132ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x97,0xf0]
          vfmsubadd132ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmsubadd132ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x97,0xf0]
          vfmsubadd132ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsubadd132ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x97,0xf0]
          vfmsubadd132ph %xmm24, %xmm23, %xmm22

// CHECK: vfmsubadd132ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x97,0xf0]
          vfmsubadd132ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmsubadd132ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x97,0xf0]
          vfmsubadd132ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsubadd132ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x97,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd132ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmsubadd132ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x97,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd132ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmsubadd132ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x97,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd132ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfmsubadd132ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x97,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmsubadd132ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmsubadd132ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x97,0x71,0x7f]
          vfmsubadd132ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsubadd132ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x97,0x72,0x80]
          vfmsubadd132ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsubadd132ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x97,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd132ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmsubadd132ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x97,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd132ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmsubadd132ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x97,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd132ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfmsubadd132ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x97,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmsubadd132ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmsubadd132ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x97,0x71,0x7f]
          vfmsubadd132ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsubadd132ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x97,0x72,0x80]
          vfmsubadd132ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsubadd213ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xa7,0xf0]
          vfmsubadd213ph %ymm24, %ymm23, %ymm22

// CHECK: vfmsubadd213ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xa7,0xf0]
          vfmsubadd213ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmsubadd213ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xa7,0xf0]
          vfmsubadd213ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsubadd213ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xa7,0xf0]
          vfmsubadd213ph %xmm24, %xmm23, %xmm22

// CHECK: vfmsubadd213ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xa7,0xf0]
          vfmsubadd213ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmsubadd213ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xa7,0xf0]
          vfmsubadd213ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsubadd213ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xa7,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd213ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmsubadd213ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xa7,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd213ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmsubadd213ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xa7,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd213ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfmsubadd213ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xa7,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmsubadd213ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmsubadd213ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xa7,0x71,0x7f]
          vfmsubadd213ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsubadd213ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xa7,0x72,0x80]
          vfmsubadd213ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsubadd213ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xa7,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd213ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmsubadd213ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xa7,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd213ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmsubadd213ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xa7,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd213ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfmsubadd213ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xa7,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmsubadd213ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmsubadd213ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xa7,0x71,0x7f]
          vfmsubadd213ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsubadd213ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xa7,0x72,0x80]
          vfmsubadd213ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsubadd231ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xb7,0xf0]
          vfmsubadd231ph %ymm24, %ymm23, %ymm22

// CHECK: vfmsubadd231ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xb7,0xf0]
          vfmsubadd231ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmsubadd231ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xb7,0xf0]
          vfmsubadd231ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsubadd231ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xb7,0xf0]
          vfmsubadd231ph %xmm24, %xmm23, %xmm22

// CHECK: vfmsubadd231ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xb7,0xf0]
          vfmsubadd231ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmsubadd231ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xb7,0xf0]
          vfmsubadd231ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsubadd231ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xb7,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd231ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmsubadd231ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xb7,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd231ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmsubadd231ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xb7,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd231ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfmsubadd231ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xb7,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmsubadd231ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmsubadd231ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xb7,0x71,0x7f]
          vfmsubadd231ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsubadd231ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xb7,0x72,0x80]
          vfmsubadd231ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmsubadd231ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xb7,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmsubadd231ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmsubadd231ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xb7,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmsubadd231ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmsubadd231ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xb7,0x35,0x00,0x00,0x00,0x00]
          vfmsubadd231ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfmsubadd231ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xb7,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmsubadd231ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmsubadd231ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xb7,0x71,0x7f]
          vfmsubadd231ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmsubadd231ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xb7,0x72,0x80]
          vfmsubadd231ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd132ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x9c,0xf0]
          vfnmadd132ph %ymm24, %ymm23, %ymm22

// CHECK: vfnmadd132ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x9c,0xf0]
          vfnmadd132ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfnmadd132ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x9c,0xf0]
          vfnmadd132ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmadd132ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x9c,0xf0]
          vfnmadd132ph %xmm24, %xmm23, %xmm22

// CHECK: vfnmadd132ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x9c,0xf0]
          vfnmadd132ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfnmadd132ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x9c,0xf0]
          vfnmadd132ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd132ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x9c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd132ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfnmadd132ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x9c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd132ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfnmadd132ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x9c,0x35,0x00,0x00,0x00,0x00]
          vfnmadd132ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfnmadd132ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x9c,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfnmadd132ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfnmadd132ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x9c,0x71,0x7f]
          vfnmadd132ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmadd132ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x9c,0x72,0x80]
          vfnmadd132ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmadd132ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x9c,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd132ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfnmadd132ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x9c,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd132ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfnmadd132ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x9c,0x35,0x00,0x00,0x00,0x00]
          vfnmadd132ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfnmadd132ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x9c,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfnmadd132ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfnmadd132ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x9c,0x71,0x7f]
          vfnmadd132ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd132ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x9c,0x72,0x80]
          vfnmadd132ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd213ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xac,0xf0]
          vfnmadd213ph %ymm24, %ymm23, %ymm22

// CHECK: vfnmadd213ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xac,0xf0]
          vfnmadd213ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfnmadd213ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xac,0xf0]
          vfnmadd213ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmadd213ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xac,0xf0]
          vfnmadd213ph %xmm24, %xmm23, %xmm22

// CHECK: vfnmadd213ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xac,0xf0]
          vfnmadd213ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfnmadd213ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xac,0xf0]
          vfnmadd213ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd213ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xac,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd213ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfnmadd213ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xac,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd213ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfnmadd213ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xac,0x35,0x00,0x00,0x00,0x00]
          vfnmadd213ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfnmadd213ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xac,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfnmadd213ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfnmadd213ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xac,0x71,0x7f]
          vfnmadd213ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmadd213ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xac,0x72,0x80]
          vfnmadd213ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmadd213ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xac,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd213ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfnmadd213ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xac,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd213ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfnmadd213ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xac,0x35,0x00,0x00,0x00,0x00]
          vfnmadd213ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfnmadd213ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xac,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfnmadd213ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfnmadd213ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xac,0x71,0x7f]
          vfnmadd213ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd213ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xac,0x72,0x80]
          vfnmadd213ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd231ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xbc,0xf0]
          vfnmadd231ph %ymm24, %ymm23, %ymm22

// CHECK: vfnmadd231ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xbc,0xf0]
          vfnmadd231ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfnmadd231ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xbc,0xf0]
          vfnmadd231ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmadd231ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xbc,0xf0]
          vfnmadd231ph %xmm24, %xmm23, %xmm22

// CHECK: vfnmadd231ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xbc,0xf0]
          vfnmadd231ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfnmadd231ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xbc,0xf0]
          vfnmadd231ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd231ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xbc,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd231ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfnmadd231ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xbc,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd231ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfnmadd231ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xbc,0x35,0x00,0x00,0x00,0x00]
          vfnmadd231ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfnmadd231ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xbc,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfnmadd231ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfnmadd231ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xbc,0x71,0x7f]
          vfnmadd231ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmadd231ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xbc,0x72,0x80]
          vfnmadd231ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmadd231ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xbc,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmadd231ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfnmadd231ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xbc,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmadd231ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfnmadd231ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xbc,0x35,0x00,0x00,0x00,0x00]
          vfnmadd231ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfnmadd231ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xbc,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfnmadd231ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfnmadd231ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xbc,0x71,0x7f]
          vfnmadd231ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmadd231ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xbc,0x72,0x80]
          vfnmadd231ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub132ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x9e,0xf0]
          vfnmsub132ph %ymm24, %ymm23, %ymm22

// CHECK: vfnmsub132ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0x9e,0xf0]
          vfnmsub132ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfnmsub132ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0x9e,0xf0]
          vfnmsub132ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmsub132ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x9e,0xf0]
          vfnmsub132ph %xmm24, %xmm23, %xmm22

// CHECK: vfnmsub132ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0x9e,0xf0]
          vfnmsub132ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfnmsub132ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0x9e,0xf0]
          vfnmsub132ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub132ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0x9e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub132ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfnmsub132ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0x9e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub132ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfnmsub132ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0x9e,0x35,0x00,0x00,0x00,0x00]
          vfnmsub132ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfnmsub132ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0x9e,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfnmsub132ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfnmsub132ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0x9e,0x71,0x7f]
          vfnmsub132ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmsub132ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0x9e,0x72,0x80]
          vfnmsub132ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmsub132ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0x9e,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub132ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfnmsub132ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0x9e,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub132ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfnmsub132ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0x9e,0x35,0x00,0x00,0x00,0x00]
          vfnmsub132ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfnmsub132ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0x9e,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfnmsub132ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfnmsub132ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0x9e,0x71,0x7f]
          vfnmsub132ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub132ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0x9e,0x72,0x80]
          vfnmsub132ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub213ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xae,0xf0]
          vfnmsub213ph %ymm24, %ymm23, %ymm22

// CHECK: vfnmsub213ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xae,0xf0]
          vfnmsub213ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfnmsub213ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xae,0xf0]
          vfnmsub213ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmsub213ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xae,0xf0]
          vfnmsub213ph %xmm24, %xmm23, %xmm22

// CHECK: vfnmsub213ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xae,0xf0]
          vfnmsub213ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfnmsub213ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xae,0xf0]
          vfnmsub213ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub213ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xae,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub213ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfnmsub213ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xae,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub213ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfnmsub213ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xae,0x35,0x00,0x00,0x00,0x00]
          vfnmsub213ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfnmsub213ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xae,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfnmsub213ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfnmsub213ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xae,0x71,0x7f]
          vfnmsub213ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmsub213ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xae,0x72,0x80]
          vfnmsub213ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmsub213ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xae,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub213ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfnmsub213ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xae,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub213ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfnmsub213ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xae,0x35,0x00,0x00,0x00,0x00]
          vfnmsub213ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfnmsub213ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xae,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfnmsub213ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfnmsub213ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xae,0x71,0x7f]
          vfnmsub213ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub213ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xae,0x72,0x80]
          vfnmsub213ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub231ph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0xbe,0xf0]
          vfnmsub231ph %ymm24, %ymm23, %ymm22

// CHECK: vfnmsub231ph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x27,0xbe,0xf0]
          vfnmsub231ph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfnmsub231ph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0xa7,0xbe,0xf0]
          vfnmsub231ph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmsub231ph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0xbe,0xf0]
          vfnmsub231ph %xmm24, %xmm23, %xmm22

// CHECK: vfnmsub231ph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x45,0x07,0xbe,0xf0]
          vfnmsub231ph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfnmsub231ph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x45,0x87,0xbe,0xf0]
          vfnmsub231ph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub231ph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x45,0x20,0xbe,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub231ph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfnmsub231ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x27,0xbe,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub231ph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfnmsub231ph  (%rip){1to16}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x30,0xbe,0x35,0x00,0x00,0x00,0x00]
          vfnmsub231ph  (%rip){1to16}, %ymm23, %ymm22

// CHECK: vfnmsub231ph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x45,0x20,0xbe,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfnmsub231ph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfnmsub231ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xa7,0xbe,0x71,0x7f]
          vfnmsub231ph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmsub231ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0xb7,0xbe,0x72,0x80]
          vfnmsub231ph  -256(%rdx){1to16}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfnmsub231ph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x45,0x00,0xbe,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfnmsub231ph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfnmsub231ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x45,0x07,0xbe,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfnmsub231ph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfnmsub231ph  (%rip){1to8}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x10,0xbe,0x35,0x00,0x00,0x00,0x00]
          vfnmsub231ph  (%rip){1to8}, %xmm23, %xmm22

// CHECK: vfnmsub231ph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x45,0x00,0xbe,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfnmsub231ph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfnmsub231ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x87,0xbe,0x71,0x7f]
          vfnmsub231ph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfnmsub231ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x45,0x97,0xbe,0x72,0x80]
          vfnmsub231ph  -256(%rdx){1to8}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfcmaddcph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x47,0x20,0x56,0xf0]
          vfcmaddcph %ymm24, %ymm23, %ymm22

// CHECK: vfcmaddcph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x47,0x27,0x56,0xf0]
          vfcmaddcph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfcmaddcph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x47,0xa7,0x56,0xf0]
          vfcmaddcph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfcmaddcph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x47,0x00,0x56,0xf0]
          vfcmaddcph %xmm24, %xmm23, %xmm22

// CHECK: vfcmaddcph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x47,0x07,0x56,0xf0]
          vfcmaddcph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfcmaddcph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x47,0x87,0x56,0xf0]
          vfcmaddcph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfcmaddcph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x47,0x20,0x56,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfcmaddcph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfcmaddcph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x47,0x27,0x56,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfcmaddcph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfcmaddcph  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x47,0x30,0x56,0x35,0x00,0x00,0x00,0x00]
          vfcmaddcph  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vfcmaddcph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x47,0x20,0x56,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfcmaddcph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfcmaddcph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x47,0xa7,0x56,0x71,0x7f]
          vfcmaddcph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfcmaddcph  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x47,0xb7,0x56,0x72,0x80]
          vfcmaddcph  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfcmaddcph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x47,0x00,0x56,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfcmaddcph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfcmaddcph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x47,0x07,0x56,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfcmaddcph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfcmaddcph  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x47,0x10,0x56,0x35,0x00,0x00,0x00,0x00]
          vfcmaddcph  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vfcmaddcph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x47,0x00,0x56,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfcmaddcph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfcmaddcph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x47,0x87,0x56,0x71,0x7f]
          vfcmaddcph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfcmaddcph  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x47,0x97,0x56,0x72,0x80]
          vfcmaddcph  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfcmulcph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x47,0x20,0xd6,0xf0]
          vfcmulcph %ymm24, %ymm23, %ymm22

// CHECK: vfcmulcph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x47,0x27,0xd6,0xf0]
          vfcmulcph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfcmulcph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x47,0xa7,0xd6,0xf0]
          vfcmulcph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfcmulcph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x47,0x00,0xd6,0xf0]
          vfcmulcph %xmm24, %xmm23, %xmm22

// CHECK: vfcmulcph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x47,0x07,0xd6,0xf0]
          vfcmulcph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfcmulcph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x47,0x87,0xd6,0xf0]
          vfcmulcph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfcmulcph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x47,0x20,0xd6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfcmulcph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfcmulcph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x47,0x27,0xd6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfcmulcph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfcmulcph  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x47,0x30,0xd6,0x35,0x00,0x00,0x00,0x00]
          vfcmulcph  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vfcmulcph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x47,0x20,0xd6,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfcmulcph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfcmulcph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x47,0xa7,0xd6,0x71,0x7f]
          vfcmulcph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfcmulcph  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x47,0xb7,0xd6,0x72,0x80]
          vfcmulcph  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfcmulcph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x47,0x00,0xd6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfcmulcph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfcmulcph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x47,0x07,0xd6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfcmulcph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfcmulcph  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x47,0x10,0xd6,0x35,0x00,0x00,0x00,0x00]
          vfcmulcph  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vfcmulcph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x47,0x00,0xd6,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfcmulcph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfcmulcph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x47,0x87,0xd6,0x71,0x7f]
          vfcmulcph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfcmulcph  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x47,0x97,0xd6,0x72,0x80]
          vfcmulcph  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddcph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x46,0x20,0x56,0xf0]
          vfmaddcph %ymm24, %ymm23, %ymm22

// CHECK: vfmaddcph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x46,0x27,0x56,0xf0]
          vfmaddcph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmaddcph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x46,0xa7,0x56,0xf0]
          vfmaddcph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmaddcph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x46,0x00,0x56,0xf0]
          vfmaddcph %xmm24, %xmm23, %xmm22

// CHECK: vfmaddcph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x46,0x07,0x56,0xf0]
          vfmaddcph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmaddcph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x46,0x87,0x56,0xf0]
          vfmaddcph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddcph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x46,0x20,0x56,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddcph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmaddcph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x46,0x27,0x56,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddcph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmaddcph  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x46,0x30,0x56,0x35,0x00,0x00,0x00,0x00]
          vfmaddcph  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vfmaddcph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x46,0x20,0x56,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmaddcph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmaddcph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x46,0xa7,0x56,0x71,0x7f]
          vfmaddcph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmaddcph  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x46,0xb7,0x56,0x72,0x80]
          vfmaddcph  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmaddcph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x46,0x00,0x56,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmaddcph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmaddcph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x46,0x07,0x56,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmaddcph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmaddcph  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x46,0x10,0x56,0x35,0x00,0x00,0x00,0x00]
          vfmaddcph  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vfmaddcph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x46,0x00,0x56,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmaddcph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmaddcph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x46,0x87,0x56,0x71,0x7f]
          vfmaddcph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmaddcph  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x46,0x97,0x56,0x72,0x80]
          vfmaddcph  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmulcph %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x46,0x20,0xd6,0xf0]
          vfmulcph %ymm24, %ymm23, %ymm22

// CHECK: vfmulcph %ymm24, %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x46,0x27,0xd6,0xf0]
          vfmulcph %ymm24, %ymm23, %ymm22 {%k7}

// CHECK: vfmulcph %ymm24, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x46,0xa7,0xd6,0xf0]
          vfmulcph %ymm24, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmulcph %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x46,0x00,0xd6,0xf0]
          vfmulcph %xmm24, %xmm23, %xmm22

// CHECK: vfmulcph %xmm24, %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0x86,0x46,0x07,0xd6,0xf0]
          vfmulcph %xmm24, %xmm23, %xmm22 {%k7}

// CHECK: vfmulcph %xmm24, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0x86,0x46,0x87,0xd6,0xf0]
          vfmulcph %xmm24, %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmulcph  268435456(%rbp,%r14,8), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xa6,0x46,0x20,0xd6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmulcph  268435456(%rbp,%r14,8), %ymm23, %ymm22

// CHECK: vfmulcph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x46,0x27,0xd6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmulcph  291(%r8,%rax,4), %ymm23, %ymm22 {%k7}

// CHECK: vfmulcph  (%rip){1to8}, %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x46,0x30,0xd6,0x35,0x00,0x00,0x00,0x00]
          vfmulcph  (%rip){1to8}, %ymm23, %ymm22

// CHECK: vfmulcph  -1024(,%rbp,2), %ymm23, %ymm22
// CHECK: encoding: [0x62,0xe6,0x46,0x20,0xd6,0x34,0x6d,0x00,0xfc,0xff,0xff]
          vfmulcph  -1024(,%rbp,2), %ymm23, %ymm22

// CHECK: vfmulcph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x46,0xa7,0xd6,0x71,0x7f]
          vfmulcph  4064(%rcx), %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmulcph  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x46,0xb7,0xd6,0x72,0x80]
          vfmulcph  -512(%rdx){1to8}, %ymm23, %ymm22 {%k7} {z}

// CHECK: vfmulcph  268435456(%rbp,%r14,8), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xa6,0x46,0x00,0xd6,0xb4,0xf5,0x00,0x00,0x00,0x10]
          vfmulcph  268435456(%rbp,%r14,8), %xmm23, %xmm22

// CHECK: vfmulcph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}
// CHECK: encoding: [0x62,0xc6,0x46,0x07,0xd6,0xb4,0x80,0x23,0x01,0x00,0x00]
          vfmulcph  291(%r8,%rax,4), %xmm23, %xmm22 {%k7}

// CHECK: vfmulcph  (%rip){1to4}, %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x46,0x10,0xd6,0x35,0x00,0x00,0x00,0x00]
          vfmulcph  (%rip){1to4}, %xmm23, %xmm22

// CHECK: vfmulcph  -512(,%rbp,2), %xmm23, %xmm22
// CHECK: encoding: [0x62,0xe6,0x46,0x00,0xd6,0x34,0x6d,0x00,0xfe,0xff,0xff]
          vfmulcph  -512(,%rbp,2), %xmm23, %xmm22

// CHECK: vfmulcph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x46,0x87,0xd6,0x71,0x7f]
          vfmulcph  2032(%rcx), %xmm23, %xmm22 {%k7} {z}

// CHECK: vfmulcph  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe6,0x46,0x97,0xd6,0x72,0x80]
          vfmulcph  -512(%rdx){1to4}, %xmm23, %xmm22 {%k7} {z}

