// REQUIRES: intel_feature_isa_avx512_ne_convert
// RUN: llvm-mc -triple i686-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      {evex} vbcstnebf162ps  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xb1,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vbcstnebf162ps  268435456(%esp,%esi,8), %xmm2

// CHECK:      {evex} vbcstnebf162ps  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0xb1,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vbcstnebf162ps  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK:      {evex} vbcstnebf162ps  (%eax), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xb1,0x10]
               {evex} vbcstnebf162ps  (%eax), %xmm2

// CHECK:      {evex} vbcstnebf162ps  -64(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xb1,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnebf162ps  -64(,%ebp,2), %xmm2

// CHECK:      {evex} vbcstnebf162ps  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xb1,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vbcstnebf162ps  268435456(%esp,%esi,8), %ymm2

// CHECK:      {evex} vbcstnebf162ps  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0xb1,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vbcstnebf162ps  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK:      {evex} vbcstnebf162ps  (%eax), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xb1,0x10]
               {evex} vbcstnebf162ps  (%eax), %ymm2

// CHECK:      {evex} vbcstnebf162ps  -64(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xb1,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnebf162ps  -64(,%ebp,2), %ymm2

// CHECK:      {evex} vbcstnesh2ps  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xb1,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vbcstnesh2ps  268435456(%esp,%esi,8), %xmm2

// CHECK:      {evex} vbcstnesh2ps  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0xb1,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vbcstnesh2ps  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK:      {evex} vbcstnesh2ps  (%eax), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xb1,0x10]
               {evex} vbcstnesh2ps  (%eax), %xmm2

// CHECK:      {evex} vbcstnesh2ps  -64(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xb1,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnesh2ps  -64(,%ebp,2), %xmm2

// CHECK:      {evex} vbcstnesh2ps  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xb1,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vbcstnesh2ps  268435456(%esp,%esi,8), %ymm2

// CHECK:      {evex} vbcstnesh2ps  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0xb1,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vbcstnesh2ps  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK:      {evex} vbcstnesh2ps  (%eax), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xb1,0x10]
               {evex} vbcstnesh2ps  (%eax), %ymm2

// CHECK:      {evex} vbcstnesh2ps  -64(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xb1,0x14,0x6d,0xc0,0xff,0xff,0xff]
               {evex} vbcstnesh2ps  -64(,%ebp,2), %ymm2

// CHECK:      {evex} vcvtne2ps2ph %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x67,0xd4]
               {evex} vcvtne2ps2ph %ymm4, %ymm3, %ymm2

// CHECK:      {evex} vcvtne2ps2ph %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x67,0xd4]
               {evex} vcvtne2ps2ph %xmm4, %xmm3, %xmm2

// CHECK:      {evex} vcvtne2ps2ph  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtne2ps2ph  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK:      {evex} vcvtne2ps2ph  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x38,0x67,0x10]
               {evex} vcvtne2ps2ph  (%eax){1to8}, %ymm3, %ymm2

// CHECK:      {evex} vcvtne2ps2ph  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x67,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtne2ps2ph  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK:      {evex} vcvtne2ps2ph  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtne2ps2ph  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:      {evex} vcvtne2ps2ph  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0x67,0x10]
               {evex} vcvtne2ps2ph  (%eax){1to4}, %xmm3, %xmm2

// CHECK:      {evex} vcvtne2ps2ph  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x67,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtne2ps2ph  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:      {evex} vcvtneebf162ps  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneebf162ps  268435456(%esp,%esi,8), %xmm2

// CHECK:      {evex} vcvtneebf162ps  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneebf162ps  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK:      {evex} vcvtneebf162ps  (%eax), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xb0,0x10]
               {evex} vcvtneebf162ps  (%eax), %xmm2

// CHECK:      {evex} vcvtneebf162ps  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xb0,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneebf162ps  -512(,%ebp,2), %xmm2

// CHECK:      {evex} vcvtneebf162ps  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0x8f,0xb0,0x51,0x7f]
               {evex} vcvtneebf162ps  2032(%ecx), %xmm2 {%k7} {z}

// CHECK:      {evex} vcvtneebf162ps  -2048(%edx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0x8f,0xb0,0x52,0x80]
               {evex} vcvtneebf162ps  -2048(%edx), %xmm2 {%k7} {z}

// CHECK:      {evex} vcvtneebf162ps  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneebf162ps  268435456(%esp,%esi,8), %ymm2

// CHECK:      {evex} vcvtneebf162ps  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneebf162ps  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK:      {evex} vcvtneebf162ps  (%eax), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xb0,0x10]
               {evex} vcvtneebf162ps  (%eax), %ymm2

// CHECK:      {evex} vcvtneebf162ps  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xb0,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneebf162ps  -1024(,%ebp,2), %ymm2

// CHECK:      {evex} vcvtneebf162ps  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xaf,0xb0,0x51,0x7f]
               {evex} vcvtneebf162ps  4064(%ecx), %ymm2 {%k7} {z}

// CHECK:      {evex} vcvtneebf162ps  -4096(%edx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xaf,0xb0,0x52,0x80]
               {evex} vcvtneebf162ps  -4096(%edx), %ymm2 {%k7} {z}

// CHECK:      {evex} vcvtneeph2ps  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneeph2ps  268435456(%esp,%esi,8), %xmm2

// CHECK:      {evex} vcvtneeph2ps  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x0f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneeph2ps  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK:      {evex} vcvtneeph2ps  (%eax), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xb0,0x10]
               {evex} vcvtneeph2ps  (%eax), %xmm2

// CHECK:      {evex} vcvtneeph2ps  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x08,0xb0,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneeph2ps  -512(,%ebp,2), %xmm2

// CHECK:      {evex} vcvtneeph2ps  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7d,0x8f,0xb0,0x51,0x7f]
               {evex} vcvtneeph2ps  2032(%ecx), %xmm2 {%k7} {z}

// CHECK:      {evex} vcvtneeph2ps  -2048(%edx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7d,0x8f,0xb0,0x52,0x80]
               {evex} vcvtneeph2ps  -2048(%edx), %xmm2 {%k7} {z}

// CHECK:      {evex} vcvtneeph2ps  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneeph2ps  268435456(%esp,%esi,8), %ymm2

// CHECK:      {evex} vcvtneeph2ps  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7d,0x2f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneeph2ps  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK:      {evex} vcvtneeph2ps  (%eax), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xb0,0x10]
               {evex} vcvtneeph2ps  (%eax), %ymm2

// CHECK:      {evex} vcvtneeph2ps  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7d,0x28,0xb0,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneeph2ps  -1024(,%ebp,2), %ymm2

// CHECK:      {evex} vcvtneeph2ps  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7d,0xaf,0xb0,0x51,0x7f]
               {evex} vcvtneeph2ps  4064(%ecx), %ymm2 {%k7} {z}

// CHECK:      {evex} vcvtneeph2ps  -4096(%edx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7d,0xaf,0xb0,0x52,0x80]
               {evex} vcvtneeph2ps  -4096(%edx), %ymm2 {%k7} {z}

// CHECK:      {evex} vcvtneobf162ps  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneobf162ps  268435456(%esp,%esi,8), %xmm2

// CHECK:      {evex} vcvtneobf162ps  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneobf162ps  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK:      {evex} vcvtneobf162ps  (%eax), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0xb0,0x10]
               {evex} vcvtneobf162ps  (%eax), %xmm2

// CHECK:      {evex} vcvtneobf162ps  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0xb0,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneobf162ps  -512(,%ebp,2), %xmm2

// CHECK:      {evex} vcvtneobf162ps  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0x8f,0xb0,0x51,0x7f]
               {evex} vcvtneobf162ps  2032(%ecx), %xmm2 {%k7} {z}

// CHECK:      {evex} vcvtneobf162ps  -2048(%edx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0x8f,0xb0,0x52,0x80]
               {evex} vcvtneobf162ps  -2048(%edx), %xmm2 {%k7} {z}

// CHECK:      {evex} vcvtneobf162ps  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneobf162ps  268435456(%esp,%esi,8), %ymm2

// CHECK:      {evex} vcvtneobf162ps  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x2f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneobf162ps  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK:      {evex} vcvtneobf162ps  (%eax), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0xb0,0x10]
               {evex} vcvtneobf162ps  (%eax), %ymm2

// CHECK:      {evex} vcvtneobf162ps  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0xb0,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneobf162ps  -1024(,%ebp,2), %ymm2

// CHECK:      {evex} vcvtneobf162ps  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0xaf,0xb0,0x51,0x7f]
               {evex} vcvtneobf162ps  4064(%ecx), %ymm2 {%k7} {z}

// CHECK:      {evex} vcvtneobf162ps  -4096(%edx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0xaf,0xb0,0x52,0x80]
               {evex} vcvtneobf162ps  -4096(%edx), %ymm2 {%k7} {z}

// CHECK:      {evex} vcvtneoph2ps  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneoph2ps  268435456(%esp,%esi,8), %xmm2

// CHECK:      {evex} vcvtneoph2ps  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x0f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneoph2ps  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK:      {evex} vcvtneoph2ps  (%eax), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0xb0,0x10]
               {evex} vcvtneoph2ps  (%eax), %xmm2

// CHECK:      {evex} vcvtneoph2ps  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x08,0xb0,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vcvtneoph2ps  -512(,%ebp,2), %xmm2

// CHECK:      {evex} vcvtneoph2ps  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0x8f,0xb0,0x51,0x7f]
               {evex} vcvtneoph2ps  2032(%ecx), %xmm2 {%k7} {z}

// CHECK:      {evex} vcvtneoph2ps  -2048(%edx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0x8f,0xb0,0x52,0x80]
               {evex} vcvtneoph2ps  -2048(%edx), %xmm2 {%k7} {z}

// CHECK:      {evex} vcvtneoph2ps  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0xb0,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vcvtneoph2ps  268435456(%esp,%esi,8), %ymm2

// CHECK:      {evex} vcvtneoph2ps  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7c,0x2f,0xb0,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vcvtneoph2ps  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK:      {evex} vcvtneoph2ps  (%eax), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0xb0,0x10]
               {evex} vcvtneoph2ps  (%eax), %ymm2

// CHECK:      {evex} vcvtneoph2ps  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7c,0x28,0xb0,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vcvtneoph2ps  -1024(,%ebp,2), %ymm2

// CHECK:      {evex} vcvtneoph2ps  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0xaf,0xb0,0x51,0x7f]
               {evex} vcvtneoph2ps  4064(%ecx), %ymm2 {%k7} {z}

// CHECK:      {evex} vcvtneoph2ps  -4096(%edx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7c,0xaf,0xb0,0x52,0x80]
               {evex} vcvtneoph2ps  -4096(%edx), %ymm2 {%k7} {z}

