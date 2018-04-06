// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -emit-llvm -o - -O1 %s | FileCheck %s

__attribute__((gnu_inline)) extern inline void getline(int a, double b) {
  b = a;
}

void getline(int a, double b) { b = a; }

// CHECK: Function Attrs: {{.*}}inlinehint
// CHECK: define void @{{.*}}getline{{.*}}(i32 %{{.*}}, double %{{.*}}) {{.*}} [[ATTR:#[0-9]+]]
// CHECK: attributes [[ATTR]] = { {{.*}}inlinehint
