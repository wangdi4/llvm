// REQUIRES: intel_feature_isa_movget64b
// RUN: llvm-mc -triple i686-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      movget64bl  268435456(%esp,%esi,8), %ebx
// CHECK: encoding: [0x0f,0x38,0xfa,0x9c,0xf4,0x00,0x00,0x00,0x10]
               movget64b  268435456(%esp,%esi,8), %ebx

// CHECK:      movget64bl  291(%edi,%eax,4), %ebx
// CHECK: encoding: [0x0f,0x38,0xfa,0x9c,0x87,0x23,0x01,0x00,0x00]
               movget64b  291(%edi,%eax,4), %ebx

// CHECK:      movget64bl  (%eax), %ebx
// CHECK: encoding: [0x0f,0x38,0xfa,0x18]
               movget64b  (%eax), %ebx

// CHECK:      movget64bl  -2048(,%ebp,2), %eax
// CHECK: encoding: [0x0f,0x38,0xfa,0x04,0x6d,0x00,0xf8,0xff,0xff]
               movget64b  -2048(,%ebp,2), %eax

// CHECK:      movget64bl  8128(%ecx), %eax
// CHECK: encoding: [0x0f,0x38,0xfa,0x81,0xc0,0x1f,0x00,0x00]
               movget64b  8128(%ecx), %eax

// CHECK:      movget64bl  -8192(%edx), %eax
// CHECK: encoding: [0x0f,0x38,0xfa,0x82,0x00,0xe0,0xff,0xff]
               movget64b  -8192(%edx), %eax

