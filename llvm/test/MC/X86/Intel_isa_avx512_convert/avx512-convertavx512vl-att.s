// REQUIRES: intel_feature_isa_avx512_convert
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=i+avx512vl,+avx512convert --show-encoding %s | FileCheck %s

// CHECK:  vcvt2ps2ph %ymm4, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x67,0xd4]
     {evex} vcvt2ps2ph %ymm4, %ymm3, %ymm2

// CHECK:  vcvt2ps2ph %ymm4, %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x2f,0x67,0xd4]
     {evex} vcvt2ps2ph %ymm4, %ymm3, %ymm2 {%k7}

// CHECK:  vcvt2ps2ph %ymm4, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xaf,0x67,0xd4]
     {evex} vcvt2ps2ph %ymm4, %ymm3, %ymm2 {%k7} {z}

// CHECK:  vcvt2ps2ph %xmm4, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x67,0xd4]
     {evex} vcvt2ps2ph %xmm4, %xmm3, %xmm2

// CHECK:  vcvt2ps2ph %xmm4, %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x0f,0x67,0xd4]
     {evex} vcvt2ps2ph %xmm4, %xmm3, %xmm2 {%k7}

// CHECK:  vcvt2ps2ph %xmm4, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0x8f,0x67,0xd4]
     {evex} vcvt2ps2ph %xmm4, %xmm3, %xmm2 {%k7} {z}

// CHECK:  vcvt2ps2ph  268435456(%esp,%esi,8), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vcvt2ps2ph  268435456(%esp,%esi,8), %ymm3, %ymm2

// CHECK:  vcvt2ps2ph  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x2f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vcvt2ps2ph  291(%edi,%eax,4), %ymm3, %ymm2 {%k7}

// CHECK:  vcvt2ps2ph  (%eax){1to8}, %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x38,0x67,0x10]
     {evex} vcvt2ps2ph  (%eax){1to8}, %ymm3, %ymm2

// CHECK:  vcvt2ps2ph  -1024(,%ebp,2), %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x65,0x28,0x67,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vcvt2ps2ph  -1024(,%ebp,2), %ymm3, %ymm2

// CHECK:  vcvt2ps2ph  4064(%ecx), %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xaf,0x67,0x51,0x7f]
     {evex} vcvt2ps2ph  4064(%ecx), %ymm3, %ymm2 {%k7} {z}

// CHECK:  vcvt2ps2ph  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xbf,0x67,0x52,0x80]
     {evex} vcvt2ps2ph  -512(%edx){1to8}, %ymm3, %ymm2 {%k7} {z}

// CHECK:  vcvt2ps2ph  268435456(%esp,%esi,8), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vcvt2ps2ph  268435456(%esp,%esi,8), %xmm3, %xmm2

// CHECK:  vcvt2ps2ph  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x0f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vcvt2ps2ph  291(%edi,%eax,4), %xmm3, %xmm2 {%k7}

// CHECK:  vcvt2ps2ph  (%eax){1to4}, %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0x67,0x10]
     {evex} vcvt2ps2ph  (%eax){1to4}, %xmm3, %xmm2

// CHECK:  vcvt2ps2ph  -512(,%ebp,2), %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x08,0x67,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vcvt2ps2ph  -512(,%ebp,2), %xmm3, %xmm2

// CHECK:  vcvt2ps2ph  2032(%ecx), %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0x8f,0x67,0x51,0x7f]
     {evex} vcvt2ps2ph  2032(%ecx), %xmm3, %xmm2 {%k7} {z}

// CHECK:  vcvt2ps2ph  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0x9f,0x67,0x52,0x80]
     {evex} vcvt2ps2ph  -512(%edx){1to4}, %xmm3, %xmm2 {%k7} {z}

// CHECK:  vcvtbf162ph %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x67,0xd3]
     {evex} vcvtbf162ph %xmm3, %xmm2

// CHECK:  vcvtbf162ph %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x67,0xd3]
     {evex} vcvtbf162ph %xmm3, %xmm2 {%k7}

// CHECK:  vcvtbf162ph %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0x8f,0x67,0xd3]
     {evex} vcvtbf162ph %xmm3, %xmm2 {%k7} {z}

// CHECK:  vcvtbf162ph %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0x67,0xd3]
     {evex} vcvtbf162ph %ymm3, %ymm2

// CHECK:  vcvtbf162ph %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0x67,0xd3]
     {evex} vcvtbf162ph %ymm3, %ymm2 {%k7}

// CHECK:  vcvtbf162ph %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xaf,0x67,0xd3]
     {evex} vcvtbf162ph %ymm3, %ymm2 {%k7} {z}

// CHECK:  vcvtbf162ph  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vcvtbf162ph  268435456(%esp,%esi,8), %xmm2

// CHECK:  vcvtbf162ph  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x0f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vcvtbf162ph  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK:  vcvtbf162ph  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x18,0x67,0x10]
     {evex} vcvtbf162ph  (%eax){1to8}, %xmm2

// CHECK:  vcvtbf162ph  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0x67,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vcvtbf162ph  -512(,%ebp,2), %xmm2

// CHECK:  vcvtbf162ph  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0x8f,0x67,0x51,0x7f]
     {evex} vcvtbf162ph  2032(%ecx), %xmm2 {%k7} {z}

// CHECK:  vcvtbf162ph  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0x9f,0x67,0x52,0x80]
     {evex} vcvtbf162ph  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK:  vcvtbf162ph  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vcvtbf162ph  268435456(%esp,%esi,8), %ymm2

// CHECK:  vcvtbf162ph  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x2f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vcvtbf162ph  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK:  vcvtbf162ph  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x38,0x67,0x10]
     {evex} vcvtbf162ph  (%eax){1to16}, %ymm2

// CHECK:  vcvtbf162ph  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0x67,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vcvtbf162ph  -1024(,%ebp,2), %ymm2

// CHECK:  vcvtbf162ph  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xaf,0x67,0x51,0x7f]
     {evex} vcvtbf162ph  4064(%ecx), %ymm2 {%k7} {z}

// CHECK:  vcvtbf162ph  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xbf,0x67,0x52,0x80]
     {evex} vcvtbf162ph  -256(%edx){1to16}, %ymm2 {%k7} {z}

// CHECK:  vcvtneph2bf16 %xmm3, %xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0x67,0xd3]
     {evex} vcvtneph2bf16 %xmm3, %xmm2

// CHECK:  vcvtneph2bf16 %xmm3, %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0x67,0xd3]
     {evex} vcvtneph2bf16 %xmm3, %xmm2 {%k7}

// CHECK:  vcvtneph2bf16 %xmm3, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0x8f,0x67,0xd3]
     {evex} vcvtneph2bf16 %xmm3, %xmm2 {%k7} {z}

// CHECK:  vcvtneph2bf16 %ymm3, %ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0x67,0xd3]
     {evex} vcvtneph2bf16 %ymm3, %ymm2

// CHECK:  vcvtneph2bf16 %ymm3, %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x2f,0x67,0xd3]
     {evex} vcvtneph2bf16 %ymm3, %ymm2 {%k7}

// CHECK:  vcvtneph2bf16 %ymm3, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0xaf,0x67,0xd3]
     {evex} vcvtneph2bf16 %ymm3, %ymm2 {%k7} {z}

// CHECK:  vcvtneph2bf16  268435456(%esp,%esi,8), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vcvtneph2bf16  268435456(%esp,%esi,8), %xmm2

// CHECK:  vcvtneph2bf16  291(%edi,%eax,4), %xmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x0f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vcvtneph2bf16  291(%edi,%eax,4), %xmm2 {%k7}

// CHECK:  vcvtneph2bf16  (%eax){1to8}, %xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x18,0x67,0x10]
     {evex} vcvtneph2bf16  (%eax){1to8}, %xmm2

// CHECK:  vcvtneph2bf16  -512(,%ebp,2), %xmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x08,0x67,0x14,0x6d,0x00,0xfe,0xff,0xff]
     {evex} vcvtneph2bf16  -512(,%ebp,2), %xmm2

// CHECK:  vcvtneph2bf16  2032(%ecx), %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0x8f,0x67,0x51,0x7f]
     {evex} vcvtneph2bf16  2032(%ecx), %xmm2 {%k7} {z}

// CHECK:  vcvtneph2bf16  -256(%edx){1to8}, %xmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0x9f,0x67,0x52,0x80]
     {evex} vcvtneph2bf16  -256(%edx){1to8}, %xmm2 {%k7} {z}

// CHECK:  vcvtneph2bf16  268435456(%esp,%esi,8), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
     {evex} vcvtneph2bf16  268435456(%esp,%esi,8), %ymm2

// CHECK:  vcvtneph2bf16  291(%edi,%eax,4), %ymm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x2f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
     {evex} vcvtneph2bf16  291(%edi,%eax,4), %ymm2 {%k7}

// CHECK:  vcvtneph2bf16  (%eax){1to16}, %ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x38,0x67,0x10]
     {evex} vcvtneph2bf16  (%eax){1to16}, %ymm2

// CHECK:  vcvtneph2bf16  -1024(,%ebp,2), %ymm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x28,0x67,0x14,0x6d,0x00,0xfc,0xff,0xff]
     {evex} vcvtneph2bf16  -1024(,%ebp,2), %ymm2

// CHECK:  vcvtneph2bf16  4064(%ecx), %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0xaf,0x67,0x51,0x7f]
     {evex} vcvtneph2bf16  4064(%ecx), %ymm2 {%k7} {z}

// CHECK:  vcvtneph2bf16  -256(%edx){1to16}, %ymm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0xbf,0x67,0x52,0x80]
     {evex} vcvtneph2bf16  -256(%edx){1to16}, %ymm2 {%k7} {z}

