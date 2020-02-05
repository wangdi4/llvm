// REQUIRES: clang-driver
// REQUIRES: x86-registered-target

/// Check that -fintel-compatibility option is not passed to clang CC1 when compiling for the device when --intel is used.
// RUN: %clang -### -fsycl --intel %s 2>&1 | FileCheck %s
// CHECK-NOT: clang{{.*}} "-fsycl-is-device"{{.*}} "-fintel-compatibility"
