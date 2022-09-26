// REQUIRES: intel_feature_isa_avx512_ne_convert
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      {evex} vcvtne2ps2ph %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x45,0x40,0x67,0xf0]
               {evex} vcvtne2ps2ph %zmm24, %zmm23, %zmm22

// CHECK:      {evex} vcvtne2ps2ph  268435456(%rbp,%r14,8), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa2,0x45,0x40,0x67,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtne2ps2ph  268435456(%rbp,%r14,8), %zmm23, %zmm22

// CHECK:      {evex} vcvtne2ps2ph  (%rip){1to16}, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x50,0x67,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtne2ps2ph  (%rip){1to16}, %zmm23, %zmm22

// CHECK:      {evex} vcvtne2ps2ph  -2048(,%rbp,2), %zmm23, %zmm22
// CHECK: encoding: [0x62,0xe2,0x45,0x40,0x67,0x34,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtne2ps2ph  -2048(,%rbp,2), %zmm23, %zmm22

// CHECK:      {evex} vcvtne2ps2ph {rn-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x45,0x10,0x67,0xf0]
               {evex} vcvtne2ps2ph {rn-sae}, %zmm24, %zmm23, %zmm22

// CHECK:      {evex} vcvtne2ps2ph {rz-sae}, %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x82,0x45,0x70,0x67,0xf0]
               {evex} vcvtne2ps2ph {rz-sae}, %zmm24, %zmm23, %zmm22

// CHECK:      {evex} vbcstnebf162ps  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0xb1,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vbcstnebf162ps  268435456(%rbp,%r14,8), %zmm22

// CHECK:      {evex} vbcstnebf162ps  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7e,0x4f,0xb1,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vbcstnebf162ps  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK:      {evex} vbcstnebf162ps  (%rip), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xb1,0x35,0x00,0x00,0x00,0x00]
               {evex} vbcstnebf162ps  (%rip), %zmm22

// CHECK:      {evex} vbcstnebf162ps  -64(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xb1,0x34,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnebf162ps  -64(,%rbp,2), %zmm22

// CHECK:      {evex} vbcstnesh2ps  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0xb1,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vbcstnesh2ps  268435456(%rbp,%r14,8), %zmm22

// CHECK:      {evex} vbcstnesh2ps  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7d,0x4f,0xb1,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vbcstnesh2ps  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK:      {evex} vbcstnesh2ps  (%rip), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0xb1,0x35,0x00,0x00,0x00,0x00]
               {evex} vbcstnesh2ps  (%rip), %zmm22

// CHECK:      {evex} vbcstnesh2ps  -64(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0xb1,0x34,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnesh2ps  -64(,%rbp,2), %zmm22

// CHECK:      {evex} vcvtneebf162ps  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneebf162ps  268435456(%rbp,%r14,8), %zmm22

// CHECK:      {evex} vcvtneebf162ps  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7e,0x4f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneebf162ps  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK:      {evex} vcvtneebf162ps  (%rip), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneebf162ps  (%rip), %zmm22

// CHECK:      {evex} vcvtneebf162ps  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xb0,0x34,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneebf162ps  -2048(,%rbp,2), %zmm22

// CHECK:      {evex} vcvtneebf162ps  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0xcf,0xb0,0x71,0x7f]
               {evex} vcvtneebf162ps  8128(%rcx), %zmm22 {%k7} {z}

// CHECK:      {evex} vcvtneebf162ps  -8192(%rdx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7e,0xcf,0xb0,0x72,0x80]
               {evex} vcvtneebf162ps  -8192(%rdx), %zmm22 {%k7} {z}

// CHECK:      {evex} vcvtneeph2ps  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneeph2ps  268435456(%rbp,%r14,8), %zmm22

// CHECK:      {evex} vcvtneeph2ps  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7d,0x4f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneeph2ps  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK:      {evex} vcvtneeph2ps  (%rip), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneeph2ps  (%rip), %zmm22

// CHECK:      {evex} vcvtneeph2ps  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0xb0,0x34,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneeph2ps  -2048(,%rbp,2), %zmm22

// CHECK:      {evex} vcvtneeph2ps  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7d,0xcf,0xb0,0x71,0x7f]
               {evex} vcvtneeph2ps  8128(%rcx), %zmm22 {%k7} {z}

// CHECK:      {evex} vcvtneeph2ps  -8192(%rdx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7d,0xcf,0xb0,0x72,0x80]
               {evex} vcvtneeph2ps  -8192(%rdx), %zmm22 {%k7} {z}

// CHECK:      {evex} vcvtneobf162ps  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa2,0x7f,0x48,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneobf162ps  268435456(%rbp,%r14,8), %zmm22

// CHECK:      {evex} vcvtneobf162ps  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7f,0x4f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneobf162ps  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK:      {evex} vcvtneobf162ps  (%rip), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x48,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneobf162ps  (%rip), %zmm22

// CHECK:      {evex} vcvtneobf162ps  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7f,0x48,0xb0,0x34,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneobf162ps  -2048(,%rbp,2), %zmm22

// CHECK:      {evex} vcvtneobf162ps  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7f,0xcf,0xb0,0x71,0x7f]
               {evex} vcvtneobf162ps  8128(%rcx), %zmm22 {%k7} {z}

// CHECK:      {evex} vcvtneobf162ps  -8192(%rdx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7f,0xcf,0xb0,0x72,0x80]
               {evex} vcvtneobf162ps  -8192(%rdx), %zmm22 {%k7} {z}

// CHECK:      {evex} vcvtneoph2ps  268435456(%rbp,%r14,8), %zmm22
// CHECK: encoding: [0x62,0xa2,0x7c,0x48,0xb0,0xb4,0xf5,0x00,0x00,0x00,0x10]
               {evex} vcvtneoph2ps  268435456(%rbp,%r14,8), %zmm22

// CHECK:      {evex} vcvtneoph2ps  291(%r8,%rax,4), %zmm22 {%k7}
// CHECK: encoding: [0x62,0xc2,0x7c,0x4f,0xb0,0xb4,0x80,0x23,0x01,0x00,0x00]
               {evex} vcvtneoph2ps  291(%r8,%rax,4), %zmm22 {%k7}

// CHECK:      {evex} vcvtneoph2ps  (%rip), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x48,0xb0,0x35,0x00,0x00,0x00,0x00]
               {evex} vcvtneoph2ps  (%rip), %zmm22

// CHECK:      {evex} vcvtneoph2ps  -2048(,%rbp,2), %zmm22
// CHECK: encoding: [0x62,0xe2,0x7c,0x48,0xb0,0x34,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneoph2ps  -2048(,%rbp,2), %zmm22

// CHECK:      {evex} vcvtneoph2ps  8128(%rcx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7c,0xcf,0xb0,0x71,0x7f]
               {evex} vcvtneoph2ps  8128(%rcx), %zmm22 {%k7} {z}

// CHECK:      {evex} vcvtneoph2ps  -8192(%rdx), %zmm22 {%k7} {z}
// CHECK: encoding: [0x62,0xe2,0x7c,0xcf,0xb0,0x72,0x80]
               {evex} vcvtneoph2ps  -8192(%rdx), %zmm22 {%k7} {z}

