// REQUIRES: intel_feature_isa_uli
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK: uiret
// CHECK: encoding: [0xf3,0x0f,0x01,0xec]
uiret

// CHECK: clui
// CHECK: encoding: [0xf3,0x0f,0x01,0xee]
clui

// CHECK: stui
// CHECK: encoding: [0xf3,0x0f,0x01,0xef]
stui

// CHECK: testui
// CHECK: encoding: [0xf3,0x0f,0x01,0xed]
testui

// CHECK: senduipi %rax
// CHECK: encoding: [0xf3,0x0f,0xc7,0xf0]
senduipi %rax

// CHECK: senduipi %rdx
// CHECK: encoding: [0xf3,0x0f,0xc7,0xf2]
senduipi %rdx

// CHECK: senduipi %r8
// CHECK: encoding: [0xf3,0x41,0x0f,0xc7,0xf0]
senduipi %r8

// CHECK: senduipi %r13
// CHECK: encoding: [0xf3,0x41,0x0f,0xc7,0xf5]
senduipi %r13
