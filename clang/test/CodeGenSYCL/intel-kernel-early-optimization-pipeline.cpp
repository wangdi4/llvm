// Check LLVM optimization pipeline is run by default for SPIR-V compiled for
// SYCL device target, and can be disabled with -fno-sycl-early-optimizations.
//
// Also checks that "Transform sin and cos calls" optimization is disabled in
// SYCL mode, because it inserts built-ins which are not available in any
// standard library available for SYCL.
//
// RUN: %clangxx -fsycl -O2 -fsycl-device-only %s -mllvm -debug-pass=Structure -emit-llvm -o - 2>&1 | FileCheck %s --check-prefixes=CHECK-EARLYOPT --implicit-check-not "Transform sin and cos calls"
// CHECK-EARLYOPT: Lower Work Group Scope Code
// CHECK-EARLYOPT: Combine redundant instructions
//
// RUN: %clangxx -fsycl -O2 -fsycl -fsycl-device-only -fno-sycl-early-optimizations %s -mllvm -debug-pass=Structure -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix=CHECK-NOEARLYOPT --implicit-check-not "Transform sin and cos calls"
// CHECK-NOEARLYOPT: Lower Work Group Scope Code
// CHECK-NOEARLYOPT-NOT: Combine redundant instructions
//
// Additionally check regular C++:
// RUN: %clang_cc1 -O2 -triple x86_64 %s -mllvm -debug-pass=Structure -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix=CHECK-CPP
// CHECK-CPP: Transform sin and cos calls
