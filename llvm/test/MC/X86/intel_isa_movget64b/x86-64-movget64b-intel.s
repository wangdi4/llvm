// REQUIRES: intel_feature_isa_movget64b
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      movget64b rax, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x4a,0x0f,0x38,0xfa,0x84,0xf5,0x00,0x00,0x00,0x10]
               movget64b rax, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      movget64b rbx, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x49,0x0f,0x38,0xfa,0x9c,0x80,0x23,0x01,0x00,0x00]
               movget64b rbx, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      movget64b rax, zmmword ptr [rip]
// CHECK: encoding: [0x48,0x0f,0x38,0xfa,0x05,0x00,0x00,0x00,0x00]
               movget64b rax, zmmword ptr [rip]

// CHECK:      movget64b rax, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x48,0x0f,0x38,0xfa,0x04,0x6d,0x00,0xf8,0xff,0xff]
               movget64b rax, zmmword ptr [2*rbp - 2048]

// CHECK:      movget64b rax, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x48,0x0f,0x38,0xfa,0x81,0xc0,0x1f,0x00,0x00]
               movget64b rax, zmmword ptr [rcx + 8128]

// CHECK:      movget64b rax, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x48,0x0f,0x38,0xfa,0x82,0x00,0xe0,0xff,0xff]
               movget64b rax, zmmword ptr [rdx - 8192]

// CHECK:      movget64b ebx, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x67,0x0f,0x38,0xfa,0x9c,0xf4,0x00,0x00,0x00,0x10]
               movget64b ebx, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      movget64b ebx, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x67,0x0f,0x38,0xfa,0x9c,0x87,0x23,0x01,0x00,0x00]
               movget64b ebx, zmmword ptr [edi + 4*eax + 291]

// CHECK:      movget64b ebx, zmmword ptr [eax]
// CHECK: encoding: [0x67,0x0f,0x38,0xfa,0x18]
               movget64b ebx, zmmword ptr [eax]

// CHECK:      movget64b eax, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x67,0x0f,0x38,0xfa,0x04,0x6d,0x00,0xf8,0xff,0xff]
               movget64b eax, zmmword ptr [2*ebp - 2048]

// CHECK:      movget64b eax, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x67,0x0f,0x38,0xfa,0x81,0xc0,0x1f,0x00,0x00]
               movget64b eax, zmmword ptr [ecx + 8128]

// CHECK:      movget64b eax, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x67,0x0f,0x38,0xfa,0x82,0x00,0xe0,0xff,0xff]
               movget64b eax, zmmword ptr [edx - 8192]

