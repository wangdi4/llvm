// REQUIRES: intel_feature_isa_avx512_vnni_int16
// RUN: llvm-mc -triple i686-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      vpdpwsud %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0xd2,0xd4]
               vpdpwsud %zmm4, %zmm3, %zmm2

// CHECK:      vpdpwsud %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0xd2,0xd4]
               vpdpwsud %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vpdpwsud %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0xd2,0xd4]
               vpdpwsud %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwsud  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwsud  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vpdpwsud  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwsud  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vpdpwsud  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x58,0xd2,0x10]
               vpdpwsud  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vpdpwsud  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0xd2,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwsud  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vpdpwsud  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0xd2,0x51,0x7f]
               vpdpwsud  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwsud  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xdf,0xd2,0x52,0x80]
               vpdpwsud  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwsuds %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0xd3,0xd4]
               vpdpwsuds %zmm4, %zmm3, %zmm2

// CHECK:      vpdpwsuds %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0xd3,0xd4]
               vpdpwsuds %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vpdpwsuds %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0xd3,0xd4]
               vpdpwsuds %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwsuds  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwsuds  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vpdpwsuds  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x66,0x4f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwsuds  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vpdpwsuds  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x58,0xd3,0x10]
               vpdpwsuds  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vpdpwsuds  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x66,0x48,0xd3,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwsuds  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vpdpwsuds  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xcf,0xd3,0x51,0x7f]
               vpdpwsuds  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwsuds  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x66,0xdf,0xd3,0x52,0x80]
               vpdpwsuds  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwusd %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0xd2,0xd4]
               vpdpwusd %zmm4, %zmm3, %zmm2

// CHECK:      vpdpwusd %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x4f,0xd2,0xd4]
               vpdpwusd %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vpdpwusd %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xcf,0xd2,0xd4]
               vpdpwusd %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwusd  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwusd  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vpdpwusd  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x4f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwusd  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vpdpwusd  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x58,0xd2,0x10]
               vpdpwusd  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vpdpwusd  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0xd2,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwusd  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vpdpwusd  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xcf,0xd2,0x51,0x7f]
               vpdpwusd  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwusd  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xdf,0xd2,0x52,0x80]
               vpdpwusd  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwusds %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0xd3,0xd4]
               vpdpwusds %zmm4, %zmm3, %zmm2

// CHECK:      vpdpwusds %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x4f,0xd3,0xd4]
               vpdpwusds %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vpdpwusds %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xcf,0xd3,0xd4]
               vpdpwusds %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwusds  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwusds  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vpdpwusds  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x65,0x4f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwusds  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vpdpwusds  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x58,0xd3,0x10]
               vpdpwusds  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vpdpwusds  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x65,0x48,0xd3,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwusds  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vpdpwusds  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xcf,0xd3,0x51,0x7f]
               vpdpwusds  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwusds  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x65,0xdf,0xd3,0x52,0x80]
               vpdpwusds  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwuud %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0xd2,0xd4]
               vpdpwuud %zmm4, %zmm3, %zmm2

// CHECK:      vpdpwuud %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0xd2,0xd4]
               vpdpwuud %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vpdpwuud %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0xd2,0xd4]
               vpdpwuud %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwuud  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0xd2,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwuud  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vpdpwuud  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0xd2,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwuud  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vpdpwuud  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x58,0xd2,0x10]
               vpdpwuud  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vpdpwuud  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0xd2,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwuud  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vpdpwuud  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0xd2,0x51,0x7f]
               vpdpwuud  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwuud  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xdf,0xd2,0x52,0x80]
               vpdpwuud  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwuuds %zmm4, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0xd3,0xd4]
               vpdpwuuds %zmm4, %zmm3, %zmm2

// CHECK:      vpdpwuuds %zmm4, %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0xd3,0xd4]
               vpdpwuuds %zmm4, %zmm3, %zmm2 {%k7}

// CHECK:      vpdpwuuds %zmm4, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0xd3,0xd4]
               vpdpwuuds %zmm4, %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwuuds  268435456(%esp,%esi,8), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0xd3,0x94,0xf4,0x00,0x00,0x00,0x10]
               vpdpwuuds  268435456(%esp,%esi,8), %zmm3, %zmm2

// CHECK:      vpdpwuuds  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}
// CHECK: encoding: [0x62,0xf2,0x64,0x4f,0xd3,0x94,0x87,0x23,0x01,0x00,0x00]
               vpdpwuuds  291(%edi,%eax,4), %zmm3, %zmm2 {%k7}

// CHECK:      vpdpwuuds  (%eax){1to16}, %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x58,0xd3,0x10]
               vpdpwuuds  (%eax){1to16}, %zmm3, %zmm2

// CHECK:      vpdpwuuds  -2048(,%ebp,2), %zmm3, %zmm2
// CHECK: encoding: [0x62,0xf2,0x64,0x48,0xd3,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vpdpwuuds  -2048(,%ebp,2), %zmm3, %zmm2

// CHECK:      vpdpwuuds  8128(%ecx), %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xcf,0xd3,0x51,0x7f]
               vpdpwuuds  8128(%ecx), %zmm3, %zmm2 {%k7} {z}

// CHECK:      vpdpwuuds  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}
// CHECK: encoding: [0x62,0xf2,0x64,0xdf,0xd3,0x52,0x80]
               vpdpwuuds  -512(%edx){1to16}, %zmm3, %zmm2 {%k7} {z}

