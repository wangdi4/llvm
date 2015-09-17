// REQUIRES: llvm-backend
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

// CHECK-LABEL: @main
int main() {
  void const *p;
  unsigned int e;
  unsigned int h;
  unsigned int c;
  // CHECK: call void @ia32_monitorx(i8* %{{.+}}, i32 %{{.+}}, i32 %{{.+}})
  __builtin_ia32_monitorx(p, e, h);
  // CHECK: call void @ia32_mwaitx(i32 %{{.+}}, i32 %{{.+}}, i32 %{{.+}})
  __builtin_ia32_mwaitx(e, h, c);
  return 0;
}

