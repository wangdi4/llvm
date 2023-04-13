// INTEL_CUSTOMIZATION
/// Test /fsycl-rdc on Windows
// REQUIRES: system-windows

// RUN: touch %t1.cpp
// RUN: %clang_cl -### /fsycl /fsycl-rdc %t1.cpp 2>&1 | FileCheck %s

// CHECK-NOT: {{warning:|error:}}

// end INTEL_CUSTOMIZATION
