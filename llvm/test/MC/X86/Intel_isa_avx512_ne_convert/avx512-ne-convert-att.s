// REQUIRES: intel_feature_isa_avx512_ne_convert
// RUN: llvm-mc -triple i686-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      {evex} vcvtne2ps2ph %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x67,0xd4]
               {evex} vcvtne2ps2ph %zmm4, %zmm3, %zmm2

// CHECK:      {evex} vcvtne2ps2ph  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtne2ps2ph  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      {evex} vcvtne2ps2ph  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x58,0x67,0x10]
               {evex} vcvtne2ps2ph  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      {evex} vcvtne2ps2ph  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x67,0x14,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtne2ps2ph  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      {evex} vcvtne2ps2ph {rn-sae}, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0x67,0xd4]
               {evex} vcvtne2ps2ph {rn-sae}, %zmm4, %zmm3, %zmm2

// CHECK:      {evex} vcvtne2ps2ph {rz-sae}, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x78,0x67,0xd4]
               {evex} vcvtne2ps2ph {rz-sae}, %zmm4, %zmm3, %zmm2

// CHECK:      {evex} vbcstnebf162ps  268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xb1,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vbcstnebf162ps  268435456(%esp,%esi,8), %zmm2

// CHECK:      {evex} vbcstnebf162ps  291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0xb1,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vbcstnebf162ps  291(%edi,%eax,4), %zmm2 {%k7}

// CHECK:      {evex} vbcstnebf162ps  (%eax), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xb1,0x10]
               {evex} vbcstnebf162ps  (%eax), %zmm2

// CHECK:      {evex} vbcstnebf162ps  -64(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xb1,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnebf162ps  -64(,%ebp,2), %zmm2

// CHECK:      {evex} vbcstnesh2ps  268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xb1,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vbcstnesh2ps  268435456(%esp,%esi,8), %zmm2

// CHECK:      {evex} vbcstnesh2ps  291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x4f,0xb1,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vbcstnesh2ps  291(%edi,%eax,4), %zmm2 {%k7}

// CHECK:      {evex} vbcstnesh2ps  (%eax), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xb1,0x10]
               {evex} vbcstnesh2ps  (%eax), %zmm2

// CHECK:      {evex} vbcstnesh2ps  -64(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xb1,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnesh2ps  -64(,%ebp,2), %zmm2

// CHECK:      {evex} vcvtneebf162ps  268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneebf162ps  268435456(%esp,%esi,8), %zmm2

// CHECK:      {evex} vcvtneebf162ps  291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneebf162ps  291(%edi,%eax,4), %zmm2 {%k7}

// CHECK:      {evex} vcvtneebf162ps  (%eax), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xb0,0x10]
               {evex} vcvtneebf162ps  (%eax), %zmm2

// CHECK:      {evex} vcvtneebf162ps  -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xb0,0x14,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneebf162ps  -2048(,%ebp,2), %zmm2

// CHECK:      {evex} vcvtneebf162ps  8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xcf,0xb0,0x51,0x7f]
               {evex} vcvtneebf162ps  8128(%ecx), %zmm2 {%k7} {z}

// CHECK:      {evex} vcvtneebf162ps  -8192(%edx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xcf,0xb0,0x52,0x80]
               {evex} vcvtneebf162ps  -8192(%edx), %zmm2 {%k7} {z}

// CHECK:      {evex} vcvtneeph2ps  268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneeph2ps  268435456(%esp,%esi,8), %zmm2

// CHECK:      {evex} vcvtneeph2ps  291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x4f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneeph2ps  291(%edi,%eax,4), %zmm2 {%k7}

// CHECK:      {evex} vcvtneeph2ps  (%eax), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xb0,0x10]
               {evex} vcvtneeph2ps  (%eax), %zmm2

// CHECK:      {evex} vcvtneeph2ps  -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0xb0,0x14,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneeph2ps  -2048(,%ebp,2), %zmm2

// CHECK:      {evex} vcvtneeph2ps  8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7d,0xcf,0xb0,0x51,0x7f]
               {evex} vcvtneeph2ps  8128(%ecx), %zmm2 {%k7} {z}

// CHECK:      {evex} vcvtneeph2ps  -8192(%edx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7d,0xcf,0xb0,0x52,0x80]
               {evex} vcvtneeph2ps  -8192(%edx), %zmm2 {%k7} {z}

// CHECK:      {evex} vcvtneobf162ps  268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneobf162ps  268435456(%esp,%esi,8), %zmm2

// CHECK:      {evex} vcvtneobf162ps  291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x4f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneobf162ps  291(%edi,%eax,4), %zmm2 {%k7}

// CHECK:      {evex} vcvtneobf162ps  (%eax), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0xb0,0x10]
               {evex} vcvtneobf162ps  (%eax), %zmm2

// CHECK:      {evex} vcvtneobf162ps  -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0xb0,0x14,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneobf162ps  -2048(,%ebp,2), %zmm2

// CHECK:      {evex} vcvtneobf162ps  8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0xcf,0xb0,0x51,0x7f]
               {evex} vcvtneobf162ps  8128(%ecx), %zmm2 {%k7} {z}

// CHECK:      {evex} vcvtneobf162ps  -8192(%edx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0xcf,0xb0,0x52,0x80]
               {evex} vcvtneobf162ps  -8192(%edx), %zmm2 {%k7} {z}

// CHECK:      {evex} vcvtneoph2ps  268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneoph2ps  268435456(%esp,%esi,8), %zmm2

// CHECK:      {evex} vcvtneoph2ps  291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x4f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneoph2ps  291(%edi,%eax,4), %zmm2 {%k7}

// CHECK:      {evex} vcvtneoph2ps  (%eax), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0xb0,0x10]
               {evex} vcvtneoph2ps  (%eax), %zmm2

// CHECK:      {evex} vcvtneoph2ps  -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x48,0xb0,0x14,0x6d,0x00,0xf8,0xff,0xff]
               {evex} vcvtneoph2ps  -2048(,%ebp,2), %zmm2

// CHECK:      {evex} vcvtneoph2ps  8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0xcf,0xb0,0x51,0x7f]
               {evex} vcvtneoph2ps  8128(%ecx), %zmm2 {%k7} {z}

// CHECK:      {evex} vcvtneoph2ps  -8192(%edx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0xcf,0xb0,0x52,0x80]
               {evex} vcvtneoph2ps  -8192(%edx), %zmm2 {%k7} {z}

