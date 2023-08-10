// INTEL

// RUN: %clang -target x86_64-unknown-linux-gnu -fsycl -fsycl-targets=spir64-unknown-unknown %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=CHECK-DEFAULT
// RUN: %clang -target x86_64-unknown-linux-gnu -fsycl -fsycl-targets=spir64-unknown-unknown -fsycl-nonsemantic-debuginfo %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=CHECK-NONSEMANTIC
// RUN: %clangxx -target x86_64-unknown-linux-gnu --intel -fiopenmp -fopenmp-targets=spir64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=CHECK-DEFAULT
// RUN: %clangxx -target x86_64-unknown-linux-gnu --intel -fiopenmp -fopenmp-targets=spir64 -fsycl-nonsemantic-debuginfo %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=CHECK-NONSEMANTIC

// CHECK-DEFAULT: llvm-spirv{{.*}}-spirv-debug-info-version=ocl-100
// CHECK-DEFAULT: +SPV_KHR_non_semantic_info
// CHECK-NONSEMANTIC: llvm-spirv{{.*}}-spirv-debug-info-version=nonsemantic-shader-200
