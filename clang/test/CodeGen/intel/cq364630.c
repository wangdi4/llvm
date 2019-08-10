// RUN: %clang_cc1 -fintel-compatibility-enable=AllowExtraArgument %s -emit-llvm -o - | FileCheck %s

void foo1(int *arg1, const int *arg2);
int foo2(int arg1, int arg2, int *arg3);
float foo3(void);

void check() {
  int a1[42];
  int a2[42];
  int offset1, offset2;
  int *arg3;

  // CHECK: call void @foo1(i32* %{{.+}}, i32* %{{.+}})
  foo1(a1 + offset1, a2 + offset2, &arg3);

  // CHECK: call i32 @foo2(i32 %{{.+}}, i32 %{{.+}}, i32* %{{.+}})
  foo2(offset1, offset2, a1, a2);

  // CHECK: call float @foo3()
  foo3(a1, a2, offset1, offset2, arg3, arg3, arg3);
}
