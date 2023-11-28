// Test covers various -mllvm-lto behaviors.
// -mllvm <opt> only applies to the compilation step and not the LTO step.
// -mllvm-lto <opt> applies to both the compilation step and LTO step.

// RUN: %clangxx --target=x86_64-unknown-linux-gnu --intel -flto -mllvm \
// RUN:          -check-llvm %s -### 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=CC1_CHECK,LTO_NO_CHECK
// RUN: %clangxx --target=x86_64-unknown-linux-gnu --intel -flto -mllvm-lto \
// RUN:          -check-llvm %s -### 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=CC1_CHECK,LTO_CHECK
// RUN: %clangxx --target=x86_64-unknown-linux-gnu --intel -flto -mllvm \
// RUN:          -check-llvm -mllvm-lto -check-llvm-lto %s -### 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=CC1_CHECK,CC1_CHECK_LTO,LTO_NO_CHECK,LTO_CHECK_LTO

/// Windows too
// RUN: %clang_cl --intel -Qipo -mllvm -check-llvm %s -### 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=CC1_CHECK,WIN_LTO_NO_CHECK
// RUN: %clang_cl --intel -Qipo -mllvm-lto -check-llvm %s -### 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=CC1_CHECK,WIN_LTO_CHECK
// RUN: %clang_cl --intel -Qipo -mllvm -check-llvm -mllvm-lto -check-llvm-lto \
// RUN:   %s -### 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=CC1_CHECK,CC1_CHECK_LTO,WIN_LTO_NO_CHECK,WIN_LTO_CHECK_LTO

// CC1_CHECK: "-mllvm" "-check-llvm"
// CC1_CHECK_LTO: "-mllvm" "-check-llvm-lto"

// LTO_NO_CHECK-NOT: ld{{.*}} "-plugin-opt=-check-llvm"
// LTO_CHECK: ld{{.*}} "-plugin-opt=-check-llvm"
// LTO_CHECK_LTO: "-plugin-opt=-check-llvm-lto"

// WIN_LTO_NO_CHECK-NOT: lld{{.*}} "-mllvm:-check-llvm"
// WIN_LTO_CHECK: lld{{.*}} "-mllvm:-check-llvm"
// WIN_LTO_CHECK_LTO: "-mllvm:-check-llvm-lto"
