// REQUIRES: intel_feature_isa_avx512_sat_cvt
// RUN: llvm-mc -triple i386 --show-encoding %s | FileCheck %s

// CHECK: vcvtnebf162ibs $123, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x69,0xd3,0x7b]
          vcvtnebf162ibs $123, %zmm3, %zmm2

// CHECK: vcvtnebf162ibs $123, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x69,0xd3,0x7b]
          vcvtnebf162ibs $123, %zmm3, %zmm2 {%k7}

// CHECK: vcvtnebf162ibs $123, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x69,0xd3,0x7b]
          vcvtnebf162ibs $123, %zmm3, %zmm2 {%k7} {z}

// CHECK: vcvtnebf162ibs  $123, 268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x69,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvtnebf162ibs  $123, 268435456(%esp,%esi,8), %zmm2

// CHECK: vcvtnebf162ibs  $123, 291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x69,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvtnebf162ibs  $123, 291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvtnebf162ibs  $123, (%eax){1to32}, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x58,0x69,0x10,0x7b]
          vcvtnebf162ibs  $123, (%eax){1to32}, %zmm2

// CHECK: vcvtnebf162ibs  $123, -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x69,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtnebf162ibs  $123, -2048(,%ebp,2), %zmm2

// CHECK: vcvtnebf162ibs  $123, 8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x69,0x51,0x7f,0x7b]
          vcvtnebf162ibs  $123, 8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvtnebf162ibs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xdf,0x69,0x52,0x80,0x7b]
          vcvtnebf162ibs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}

// CHECK: vcvtnebf162iubs $123, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x6b,0xd3,0x7b]
          vcvtnebf162iubs $123, %zmm3, %zmm2

// CHECK: vcvtnebf162iubs $123, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x6b,0xd3,0x7b]
          vcvtnebf162iubs $123, %zmm3, %zmm2 {%k7}

// CHECK: vcvtnebf162iubs $123, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x6b,0xd3,0x7b]
          vcvtnebf162iubs $123, %zmm3, %zmm2 {%k7} {z}

// CHECK: vcvtnebf162iubs  $123, 268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x6b,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvtnebf162iubs  $123, 268435456(%esp,%esi,8), %zmm2

// CHECK: vcvtnebf162iubs  $123, 291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x6b,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvtnebf162iubs  $123, 291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvtnebf162iubs  $123, (%eax){1to32}, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x58,0x6b,0x10,0x7b]
          vcvtnebf162iubs  $123, (%eax){1to32}, %zmm2

// CHECK: vcvtnebf162iubs  $123, -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x6b,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtnebf162iubs  $123, -2048(,%ebp,2), %zmm2

// CHECK: vcvtnebf162iubs  $123, 8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x6b,0x51,0x7f,0x7b]
          vcvtnebf162iubs  $123, 8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvtnebf162iubs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xdf,0x6b,0x52,0x80,0x7b]
          vcvtnebf162iubs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}

// CHECK: vcvtph2ibs $123, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x69,0xd3,0x7b]
          vcvtph2ibs $123, %zmm3, %zmm2

// CHECK: vcvtph2ibs $123, {rn-sae}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x18,0x69,0xd3,0x7b]
          vcvtph2ibs $123, {rn-sae}, %zmm3, %zmm2

// CHECK: vcvtph2ibs $123, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x69,0xd3,0x7b]
          vcvtph2ibs $123, %zmm3, %zmm2 {%k7}

// CHECK: vcvtph2ibs $123, {rz-sae}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7c,0xff,0x69,0xd3,0x7b]
          vcvtph2ibs $123, {rz-sae}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vcvtph2ibs  $123, 268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x69,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvtph2ibs  $123, 268435456(%esp,%esi,8), %zmm2

// CHECK: vcvtph2ibs  $123, 291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x69,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvtph2ibs  $123, 291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvtph2ibs  $123, (%eax){1to32}, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x58,0x69,0x10,0x7b]
          vcvtph2ibs  $123, (%eax){1to32}, %zmm2

// CHECK: vcvtph2ibs  $123, -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x69,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtph2ibs  $123, -2048(,%ebp,2), %zmm2

// CHECK: vcvtph2ibs  $123, 8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7c,0xcf,0x69,0x51,0x7f,0x7b]
          vcvtph2ibs  $123, 8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvtph2ibs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7c,0xdf,0x69,0x52,0x80,0x7b]
          vcvtph2ibs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}

// CHECK: vcvtph2iubs $123, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x6b,0xd3,0x7b]
          vcvtph2iubs $123, %zmm3, %zmm2

// CHECK: vcvtph2iubs $123, {rn-sae}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x18,0x6b,0xd3,0x7b]
          vcvtph2iubs $123, {rn-sae}, %zmm3, %zmm2

// CHECK: vcvtph2iubs $123, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x6b,0xd3,0x7b]
          vcvtph2iubs $123, %zmm3, %zmm2 {%k7}

// CHECK: vcvtph2iubs $123, {rz-sae}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7c,0xff,0x6b,0xd3,0x7b]
          vcvtph2iubs $123, {rz-sae}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vcvtph2iubs  $123, 268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x6b,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvtph2iubs  $123, 268435456(%esp,%esi,8), %zmm2

// CHECK: vcvtph2iubs  $123, 291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x6b,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvtph2iubs  $123, 291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvtph2iubs  $123, (%eax){1to32}, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x58,0x6b,0x10,0x7b]
          vcvtph2iubs  $123, (%eax){1to32}, %zmm2

// CHECK: vcvtph2iubs  $123, -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x6b,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtph2iubs  $123, -2048(,%ebp,2), %zmm2

// CHECK: vcvtph2iubs  $123, 8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7c,0xcf,0x6b,0x51,0x7f,0x7b]
          vcvtph2iubs  $123, 8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvtph2iubs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7c,0xdf,0x6b,0x52,0x80,0x7b]
          vcvtph2iubs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}

// CHECK: vcvtps2ibs $123, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x69,0xd3,0x7b]
          vcvtps2ibs $123, %zmm3, %zmm2

// CHECK: vcvtps2ibs $123, {rn-sae}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x18,0x69,0xd3,0x7b]
          vcvtps2ibs $123, {rn-sae}, %zmm3, %zmm2

// CHECK: vcvtps2ibs $123, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x69,0xd3,0x7b]
          vcvtps2ibs $123, %zmm3, %zmm2 {%k7}

// CHECK: vcvtps2ibs $123, {rz-sae}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7d,0xff,0x69,0xd3,0x7b]
          vcvtps2ibs $123, {rz-sae}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vcvtps2ibs  $123, 268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x69,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvtps2ibs  $123, 268435456(%esp,%esi,8), %zmm2

// CHECK: vcvtps2ibs  $123, 291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x69,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvtps2ibs  $123, 291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvtps2ibs  $123, (%eax){1to16}, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x58,0x69,0x10,0x7b]
          vcvtps2ibs  $123, (%eax){1to16}, %zmm2

// CHECK: vcvtps2ibs  $123, -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x69,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtps2ibs  $123, -2048(,%ebp,2), %zmm2

// CHECK: vcvtps2ibs  $123, 8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7d,0xcf,0x69,0x51,0x7f,0x7b]
          vcvtps2ibs  $123, 8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvtps2ibs  $123, -512(%edx){1to16}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7d,0xdf,0x69,0x52,0x80,0x7b]
          vcvtps2ibs  $123, -512(%edx){1to16}, %zmm2 {%k7} {z}

// CHECK: vcvtps2iubs $123, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x6b,0xd3,0x7b]
          vcvtps2iubs $123, %zmm3, %zmm2

// CHECK: vcvtps2iubs $123, {rn-sae}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x18,0x6b,0xd3,0x7b]
          vcvtps2iubs $123, {rn-sae}, %zmm3, %zmm2

// CHECK: vcvtps2iubs $123, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x6b,0xd3,0x7b]
          vcvtps2iubs $123, %zmm3, %zmm2 {%k7}

// CHECK: vcvtps2iubs $123, {rz-sae}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7d,0xff,0x6b,0xd3,0x7b]
          vcvtps2iubs $123, {rz-sae}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vcvtps2iubs  $123, 268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x6b,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvtps2iubs  $123, 268435456(%esp,%esi,8), %zmm2

// CHECK: vcvtps2iubs  $123, 291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x6b,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvtps2iubs  $123, 291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvtps2iubs  $123, (%eax){1to16}, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x58,0x6b,0x10,0x7b]
          vcvtps2iubs  $123, (%eax){1to16}, %zmm2

// CHECK: vcvtps2iubs  $123, -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x6b,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvtps2iubs  $123, -2048(,%ebp,2), %zmm2

// CHECK: vcvtps2iubs  $123, 8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7d,0xcf,0x6b,0x51,0x7f,0x7b]
          vcvtps2iubs  $123, 8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvtps2iubs  $123, -512(%edx){1to16}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7d,0xdf,0x6b,0x52,0x80,0x7b]
          vcvtps2iubs  $123, -512(%edx){1to16}, %zmm2 {%k7} {z}

// CHECK: vcvttnebf162ibs $123, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x68,0xd3,0x7b]
          vcvttnebf162ibs $123, %zmm3, %zmm2

// CHECK: vcvttnebf162ibs $123, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x68,0xd3,0x7b]
          vcvttnebf162ibs $123, %zmm3, %zmm2 {%k7}

// CHECK: vcvttnebf162ibs $123, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x68,0xd3,0x7b]
          vcvttnebf162ibs $123, %zmm3, %zmm2 {%k7} {z}

// CHECK: vcvttnebf162ibs  $123, 268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x68,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvttnebf162ibs  $123, 268435456(%esp,%esi,8), %zmm2

// CHECK: vcvttnebf162ibs  $123, 291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x68,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvttnebf162ibs  $123, 291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvttnebf162ibs  $123, (%eax){1to32}, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x58,0x68,0x10,0x7b]
          vcvttnebf162ibs  $123, (%eax){1to32}, %zmm2

// CHECK: vcvttnebf162ibs  $123, -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x68,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttnebf162ibs  $123, -2048(,%ebp,2), %zmm2

// CHECK: vcvttnebf162ibs  $123, 8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x68,0x51,0x7f,0x7b]
          vcvttnebf162ibs  $123, 8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvttnebf162ibs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xdf,0x68,0x52,0x80,0x7b]
          vcvttnebf162ibs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}

// CHECK: vcvttnebf162iubs $123, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x6a,0xd3,0x7b]
          vcvttnebf162iubs $123, %zmm3, %zmm2

// CHECK: vcvttnebf162iubs $123, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x6a,0xd3,0x7b]
          vcvttnebf162iubs $123, %zmm3, %zmm2 {%k7}

// CHECK: vcvttnebf162iubs $123, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x6a,0xd3,0x7b]
          vcvttnebf162iubs $123, %zmm3, %zmm2 {%k7} {z}

// CHECK: vcvttnebf162iubs  $123, 268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x6a,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvttnebf162iubs  $123, 268435456(%esp,%esi,8), %zmm2

// CHECK: vcvttnebf162iubs  $123, 291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7f,0x4f,0x6a,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvttnebf162iubs  $123, 291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvttnebf162iubs  $123, (%eax){1to32}, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x58,0x6a,0x10,0x7b]
          vcvttnebf162iubs  $123, (%eax){1to32}, %zmm2

// CHECK: vcvttnebf162iubs  $123, -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7f,0x48,0x6a,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttnebf162iubs  $123, -2048(,%ebp,2), %zmm2

// CHECK: vcvttnebf162iubs  $123, 8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xcf,0x6a,0x51,0x7f,0x7b]
          vcvttnebf162iubs  $123, 8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvttnebf162iubs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7f,0xdf,0x6a,0x52,0x80,0x7b]
          vcvttnebf162iubs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}

// CHECK: vcvttph2ibs $123, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x68,0xd3,0x7b]
          vcvttph2ibs $123, %zmm3, %zmm2

// CHECK: vcvttph2ibs $123, {sae}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x18,0x68,0xd3,0x7b]
          vcvttph2ibs $123, {sae}, %zmm3, %zmm2

// CHECK: vcvttph2ibs $123, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x68,0xd3,0x7b]
          vcvttph2ibs $123, %zmm3, %zmm2 {%k7}

// CHECK: vcvttph2ibs $123, {sae}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7c,0x9f,0x68,0xd3,0x7b]
          vcvttph2ibs $123, {sae}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vcvttph2ibs  $123, 268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x68,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvttph2ibs  $123, 268435456(%esp,%esi,8), %zmm2

// CHECK: vcvttph2ibs  $123, 291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x68,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvttph2ibs  $123, 291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvttph2ibs  $123, (%eax){1to32}, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x58,0x68,0x10,0x7b]
          vcvttph2ibs  $123, (%eax){1to32}, %zmm2

// CHECK: vcvttph2ibs  $123, -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x68,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttph2ibs  $123, -2048(,%ebp,2), %zmm2

// CHECK: vcvttph2ibs  $123, 8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7c,0xcf,0x68,0x51,0x7f,0x7b]
          vcvttph2ibs  $123, 8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvttph2ibs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7c,0xdf,0x68,0x52,0x80,0x7b]
          vcvttph2ibs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}

// CHECK: vcvttph2iubs $123, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x6a,0xd3,0x7b]
          vcvttph2iubs $123, %zmm3, %zmm2

// CHECK: vcvttph2iubs $123, {sae}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x18,0x6a,0xd3,0x7b]
          vcvttph2iubs $123, {sae}, %zmm3, %zmm2

// CHECK: vcvttph2iubs $123, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x6a,0xd3,0x7b]
          vcvttph2iubs $123, %zmm3, %zmm2 {%k7}

// CHECK: vcvttph2iubs $123, {sae}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7c,0x9f,0x6a,0xd3,0x7b]
          vcvttph2iubs $123, {sae}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vcvttph2iubs  $123, 268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x6a,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvttph2iubs  $123, 268435456(%esp,%esi,8), %zmm2

// CHECK: vcvttph2iubs  $123, 291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7c,0x4f,0x6a,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvttph2iubs  $123, 291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvttph2iubs  $123, (%eax){1to32}, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x58,0x6a,0x10,0x7b]
          vcvttph2iubs  $123, (%eax){1to32}, %zmm2

// CHECK: vcvttph2iubs  $123, -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7c,0x48,0x6a,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttph2iubs  $123, -2048(,%ebp,2), %zmm2

// CHECK: vcvttph2iubs  $123, 8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7c,0xcf,0x6a,0x51,0x7f,0x7b]
          vcvttph2iubs  $123, 8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvttph2iubs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7c,0xdf,0x6a,0x52,0x80,0x7b]
          vcvttph2iubs  $123, -256(%edx){1to32}, %zmm2 {%k7} {z}

// CHECK: vcvttps2ibs $123, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x68,0xd3,0x7b]
          vcvttps2ibs $123, %zmm3, %zmm2

// CHECK: vcvttps2ibs $123, {sae}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x18,0x68,0xd3,0x7b]
          vcvttps2ibs $123, {sae}, %zmm3, %zmm2

// CHECK: vcvttps2ibs $123, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x68,0xd3,0x7b]
          vcvttps2ibs $123, %zmm3, %zmm2 {%k7}

// CHECK: vcvttps2ibs $123, {sae}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7d,0x9f,0x68,0xd3,0x7b]
          vcvttps2ibs $123, {sae}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vcvttps2ibs  $123, 268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x68,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvttps2ibs  $123, 268435456(%esp,%esi,8), %zmm2

// CHECK: vcvttps2ibs  $123, 291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x68,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvttps2ibs  $123, 291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvttps2ibs  $123, (%eax){1to16}, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x58,0x68,0x10,0x7b]
          vcvttps2ibs  $123, (%eax){1to16}, %zmm2

// CHECK: vcvttps2ibs  $123, -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x68,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttps2ibs  $123, -2048(,%ebp,2), %zmm2

// CHECK: vcvttps2ibs  $123, 8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7d,0xcf,0x68,0x51,0x7f,0x7b]
          vcvttps2ibs  $123, 8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvttps2ibs  $123, -512(%edx){1to16}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7d,0xdf,0x68,0x52,0x80,0x7b]
          vcvttps2ibs  $123, -512(%edx){1to16}, %zmm2 {%k7} {z}

// CHECK: vcvttps2iubs $123, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x6a,0xd3,0x7b]
          vcvttps2iubs $123, %zmm3, %zmm2

// CHECK: vcvttps2iubs $123, {sae}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x18,0x6a,0xd3,0x7b]
          vcvttps2iubs $123, {sae}, %zmm3, %zmm2

// CHECK: vcvttps2iubs $123, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x6a,0xd3,0x7b]
          vcvttps2iubs $123, %zmm3, %zmm2 {%k7}

// CHECK: vcvttps2iubs $123, {sae}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7d,0x9f,0x6a,0xd3,0x7b]
          vcvttps2iubs $123, {sae}, %zmm3, %zmm2 {%k7} {z}

// CHECK: vcvttps2iubs  $123, 268435456(%esp,%esi,8), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x6a,0x94,0xf4,0x00,0x00,0x00,0x10,0x7b]
          vcvttps2iubs  $123, 268435456(%esp,%esi,8), %zmm2

// CHECK: vcvttps2iubs  $123, 291(%edi,%eax,4), %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf3,0x7d,0x4f,0x6a,0x94,0x87,0x23,0x01,0x00,0x00,0x7b]
          vcvttps2iubs  $123, 291(%edi,%eax,4), %zmm2 {%k7}

// CHECK: vcvttps2iubs  $123, (%eax){1to16}, %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x58,0x6a,0x10,0x7b]
          vcvttps2iubs  $123, (%eax){1to16}, %zmm2

// CHECK: vcvttps2iubs  $123, -2048(,%ebp,2), %zmm2
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x6a,0x14,0x6d,0x00,0xf8,0xff,0xff,0x7b]
          vcvttps2iubs  $123, -2048(,%ebp,2), %zmm2

// CHECK: vcvttps2iubs  $123, 8128(%ecx), %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7d,0xcf,0x6a,0x51,0x7f,0x7b]
          vcvttps2iubs  $123, 8128(%ecx), %zmm2 {%k7} {z}

// CHECK: vcvttps2iubs  $123, -512(%edx){1to16}, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf3,0x7d,0xdf,0x6a,0x52,0x80,0x7b]
          vcvttps2iubs  $123, -512(%edx){1to16}, %zmm2 {%k7} {z}

