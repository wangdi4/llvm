// Check LLVM optimization pipeline is run by default for SPIR-V compiled for
// SYCL device target, and can be disabled with -fno-sycl-early-optimizations.
//
// RUN: %clang_cc1 -O2 -fsycl-is-device -triple spir64-unknown-unknown %s -flegacy-pass-manager -mllvm -debug-pass=Structure -emit-llvm -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-EARLYOPT
// INTEL_CUSTOMIZATION
// RUN: %clangxx -fsycl -target x86_64 -O2 -fsycl-device-only %s -mllvm -debug-pass=Structure -emit-llvm -o - 2>&1 | FileCheck %s --check-prefixes=CHECK-EARLYOPT --implicit-check-not "Transform sin and cos calls"
// end INTEL_CUSTOMIZATION
// CHECK-EARLYOPT: Combine redundant instructions
// CHECK-EARLYOPT: Move SYCL printf literal arguments to constant address space
//
// RUN: %clang_cc1 -O2 -fsycl-is-device -triple spir64-unknown-unknown %s -flegacy-pass-manager -mllvm -debug-pass=Structure -emit-llvm -fno-sycl-early-optimizations -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-NOEARLYOPT
// INTEL_CUSTOMIZATION
// RUN: %clangxx -fsycl -target x86_64 -O2 -fsycl -fsycl-device-only -fno-sycl-early-optimizations %s -mllvm -debug-pass=Structure -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix=CHECK-NOEARLYOPT --implicit-check-not "Transform sin and cos calls"
// end INTEL_CUSTOMIZATION
// CHECK-NOEARLYOPT-NOT: Combine redundant instructions
// CHECK-NOEARLYOPT: Move SYCL printf literal arguments to constant address space
//
//
// New pass manager doesn't print all passes tree, only module level.
//
// RUN: %clang_cc1 -O2 -fsycl-is-device -triple spir64-unknown-unknown %s -fno-legacy-pass-manager -mdebug-pass Structure -emit-llvm -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-NEWPM-EARLYOPT
// CHECK-NEWPM-EARLYOPT: ConstantMergePass
// CHECK-NEWPM-EARLYOPT: SYCLMutatePrintfAddrspacePass
//
// RUN: %clang_cc1 -O2 -fsycl-is-device -triple spir64-unknown-unknown %s -fno-legacy-pass-manager -mdebug-pass Structure -emit-llvm -fno-sycl-early-optimizations -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-NEWPM-NOEARLYOPT
// CHECK-NEWPM-NOEARLYOPT-NOT: ConstantMergePass
// CHECK-NEWPM-NOEARLYOPT: SYCLMutatePrintfAddrspacePass
//
// INTEL_CUSTOMIZATION
// Additionally check regular C++:
// RUN: %clang_cc1 -O2 -triple x86_64 %s -mllvm -debug-pass=Structure -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix=CHECK-CPP
// CHECK-CPP: Transform sin and cos calls
// end INTEL_CUSTOMIZATION
