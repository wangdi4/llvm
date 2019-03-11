// RUN: %clang_cc1 %s -triple "spir-unknown-unknown-inteleyeq" -emit-llvm -o - | FileCheck %s -check-prefix=SPIR
// RUN: %clang_cc1 %s -triple "spir64-unknown-unknown-inteleyeq" -emit-llvm -o - | FileCheck %s -check-prefix=SPIR64
// RUN: %clang_cc1 %s -triple "x86_64-unknown-unknown-inteleyeq" -emit-llvm -o - | FileCheck %s -check-prefix=X86_64

// SPIR:   target triple = "spir-unknown-unknown-inteleyeq"
// SPIR64: target triple = "spir64-unknown-unknown-inteleyeq"
// X86_64: target triple = "x86_64-unknown-unknown-inteleyeq"

kernel void foo(global long *arg) {
    arg[0] = 0;
}
