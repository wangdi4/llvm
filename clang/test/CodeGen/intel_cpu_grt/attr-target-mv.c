// REQUIRES: intel_feature_cpu_grt
// RUN: %clang_cc1 -triple x86_64-linux-gnu -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefix=LINUX
// RUN: %clang_cc1 -triple x86_64-windows-pc -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefix=WINDOWS

int __attribute__((target("arch=gracemont"))) foo(void) {return 13;}
int __attribute__((target("default"))) foo(void) { return 2; }

// LINUX: define{{.*}} i32 @foo.arch_gracemont()
// LINUX: ret i32 13

// WINDOWS: define{{.*}} dso_local i32 @foo.arch_gracemont()
// WINDOWS: ret i32 13