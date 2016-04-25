// The test checks Intel-enabled declspecs allowed in GNU mode.
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu %s -emit-llvm -fintel-compatibility -o - | FileCheck %s

// selectany turns extern "C" variable declarations into definitions.
extern __declspec(selectany) int x1;
extern "C" __declspec(selectany) int x2;
extern "C++" __declspec(selectany) int x3;
extern "C" { __declspec(selectany) int x4; }
__declspec(selectany) int x5;

// CHECK: @x1 = weak_odr global i32 0, comdat, align 4
// CHECK: @x2 = weak_odr global i32 0, comdat, align 4
// CHECK: @_Z2x3 = weak_odr global i32 0, comdat, align 4
// CHECK: @x4 = weak_odr global i32 0, comdat, align 4
// CHECK: @x5 = weak_odr global i32 0, comdat, align 4

struct __declspec(uuid("12345678-1234-1234-1234-1234567890aB")) S1 {} s1;
struct __declspec(uuid("87654321-4321-4321-4321-ba0987654321")) S2 {};

