// INTEL_FEATURE_CPU_RYL
// REQUIRES: intel_feature_cpu_ryl

// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm %s -o - | FileCheck %s --check-prefix=LINUX
// RUN: %clang_cc1 -triple x86_64-windows-pc -emit-llvm %s -o - | FileCheck %s --check-prefix=WINDOWS

int __attribute__((target("arch=royal"))) foo(void) {return 14;}
int __attribute__((target("default"))) foo(void) { return 2; }

// LINUX: define{{.*}} i32 @foo.arch_royal()
// LINUX: ret i32 14

// WINDOWS: define dso_local i32 @foo.arch_royal()
// WINDOWS: ret i32 14

// end INTEL_FEATURE_CPU_RYL
