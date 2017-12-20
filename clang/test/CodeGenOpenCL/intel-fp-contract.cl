// RUN: %clang_cc1 %s -emit-llvm -o - -triple spir-unknown-unknown -ffp-contract=off  | FileCheck -check-prefix=FP_DISABLED %s
// RUN: %clang_cc1 %s -emit-llvm -o - -triple spir-unknown-unknown -ffp-contract=on   | FileCheck -check-prefix=FP_ENABLED  %s
// RUN: %clang_cc1 %s -emit-llvm -o - -triple spir-unknown-unknown -ffp-contract=fast | FileCheck -check-prefix=FP_ENABLED  %s
// RUN: %clang_cc1 %s -emit-llvm -o - -triple spir-unknown-unknown -ffp-contract=fast -D DISABLE_FP_CONTRACT | FileCheck -check-prefix=FP_DISABLED  %s

#ifdef DISABLE_FP_CONTRACT
#pragma OPENCL FP_CONTRACT OFF
#endif// DISABLE_FP_CONTRACT

kernel void t() {}

#ifdef DISABLE_FP_CONTRACT
#pragma OPENCL FP_CONTRACT ON
#endif// DISABLE_FP_CONTRACT

// FP_DISABLED-NOT: opencl.enable.FP_CONTRACT
// FP_ENABLED: opencl.enable.FP_CONTRACT
