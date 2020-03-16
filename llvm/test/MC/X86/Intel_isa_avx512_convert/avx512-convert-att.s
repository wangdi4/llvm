// REQUIRES: intel_feature_isa_avx512_convert
// RUN: llvm-mc -triple i686-unknown-unknown -mattr=+avx512convert --show-encoding %s | FileCheck %s

// CHECK:      vcvt2ps2ph %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x67,0xd4]
               vcvt2ps2ph %zmm4, %zmm3, %zmm2

// CHECK:      vcvt2ps2ph {rn-sae}, %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x18,0x67,0xd4]
               vcvt2ps2ph {rn-sae}, %zmm4, %zmm3, %zmm2

// CHECK:      vcvt2ps2ph %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x4f,0x67,0xd4]
               vcvt2ps2ph %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vcvt2ps2ph {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xff,0x67,0xd4]
               vcvt2ps2ph {rz-sae}, %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vcvt2ps2ph  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
               vcvt2ps2ph  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vcvt2ps2ph  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x4f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
               vcvt2ps2ph  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vcvt2ps2ph  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x58,0x67,0x10]
               vcvt2ps2ph  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vcvt2ps2ph  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0x67,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vcvt2ps2ph  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vcvt2ps2ph  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xcf,0x67,0x51,0x7f]
               vcvt2ps2ph  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vcvt2ps2ph  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xdf,0x67,0x52,0x80]
               vcvt2ps2ph  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vcvtbf162ph %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0x67,0xd3]
               vcvtbf162ph %zmm3, %zmm2

// CHECK:      vcvtbf162ph {rn-sae}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x18,0x67,0xd3]
               vcvtbf162ph {rn-sae}, %zmm3, %zmm2

// CHECK:      vcvtbf162ph %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0x67,0xd3]
               vcvtbf162ph %zmm3, %zmm2 {%k7}

// CHECK:      vcvtbf162ph {rz-sae}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xff,0x67,0xd3]
               vcvtbf162ph {rz-sae}, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vcvtbf162ph  268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
               vcvtbf162ph  268435456(%esp,%esi,8), %zmm2

// CHECK:      vcvtbf162ph  291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7e,0x4f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
               vcvtbf162ph  291(%edi,%eax,4), %zmm2 {%k7}

// CHECK:      vcvtbf162ph  (%eax){1to32}, %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x58,0x67,0x10]
               vcvtbf162ph  (%eax){1to32}, %zmm2

// CHECK:      vcvtbf162ph  -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0x67,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vcvtbf162ph  -2048(,%ebp,2), %zmm2

// CHECK:      vcvtbf162ph  8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xcf,0x67,0x51,0x7f]
               vcvtbf162ph  8128(%ecx), %zmm2 {%k7} {z}

// CHECK:      vcvtbf162ph  -256(%edx){1to32}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7e,0xdf,0x67,0x52,0x80]
               vcvtbf162ph  -256(%edx){1to32}, %zmm2 {%k7} {z}

// CHECK:      vcvtneph2bf16 %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0x67,0xd3]
               vcvtneph2bf16 %zmm3, %zmm2

// CHECK:      vcvtneph2bf16 %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x4f,0x67,0xd3]
               vcvtneph2bf16 %zmm3, %zmm2 {%k7}

// CHECK:      vcvtneph2bf16 %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0xcf,0x67,0xd3]
               vcvtneph2bf16 %zmm3, %zmm2 {%k7} {z}

// CHECK:      vcvtneph2bf16  268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0x67,0x94,0xf4,0x00,0x00,0x00,0x10]
               vcvtneph2bf16  268435456(%esp,%esi,8), %zmm2

// CHECK:      vcvtneph2bf16  291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x7f,0x4f,0x67,0x94,0x87,0x23,0x01,0x00,0x00]
               vcvtneph2bf16  291(%edi,%eax,4), %zmm2 {%k7}

// CHECK:      vcvtneph2bf16  (%eax){1to32}, %zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x58,0x67,0x10]
               vcvtneph2bf16  (%eax){1to32}, %zmm2

// CHECK:      vcvtneph2bf16  -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf2,0x7f,0x48,0x67,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vcvtneph2bf16  -2048(,%ebp,2), %zmm2

// CHECK:      vcvtneph2bf16  8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0xcf,0x67,0x51,0x7f]
               vcvtneph2bf16  8128(%ecx), %zmm2 {%k7} {z}

// CHECK:      vcvtneph2bf16  -256(%edx){1to32}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x7f,0xdf,0x67,0x52,0x80]
               vcvtneph2bf16  -256(%edx){1to32}, %zmm2 {%k7} {z}

