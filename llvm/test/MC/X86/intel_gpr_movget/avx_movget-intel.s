// REQUIRES: intel_feature_isa_gpr_movget
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      movget eax, dword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xf3,0x0f,0x38,0xfa,0x84,0xf4,0x00,0x00,0x00,0x10]
               movget eax, dword ptr [esp + 8*esi + 268435456]

// CHECK:      movget ebx, dword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xf3,0x0f,0x38,0xfa,0x9c,0x87,0x23,0x01,0x00,0x00]
               movget ebx, dword ptr [edi + 4*eax + 291]

// CHECK:      movget ecx, dword ptr [eax]
// CHECK: encoding: [0xf3,0x0f,0x38,0xfa,0x08]
               movget ecx, dword ptr [eax]

// CHECK:      movget eax, dword ptr [2*ebp - 512]
// CHECK: encoding: [0xf3,0x0f,0x38,0xfa,0x04,0x6d,0x00,0xfe,0xff,0xff]
               movget eax, dword ptr [2*ebp - 512]

// CHECK:      movget eax, dword ptr [ecx + 2032]
// CHECK: encoding: [0xf3,0x0f,0x38,0xfa,0x81,0xf0,0x07,0x00,0x00]
               movget eax, dword ptr [ecx + 2032]

// CHECK:      movget eax, dword ptr [edx - 2048]
// CHECK: encoding: [0xf3,0x0f,0x38,0xfa,0x82,0x00,0xf8,0xff,0xff]
               movget eax, dword ptr [edx - 2048]

