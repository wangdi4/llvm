// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -emit-llvm --gnu_fabi_version=4 -o - | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -emit-llvm --gnu_fabi_version=5 -o - | FileCheck %s --check-prefix=ABI5

typedef int (*fp)(int *, double *d) __attribute__((noreturn));
int g123(int *i, double *d) __attribute__((noreturn));

int g123(int *i, double *d) {
  fp func = 0;
  return func(i, d);
}

// CHECK-LABEL: _Z4g123PiPd

void g456(fp func) {
  int i;
  double d;
  func(&i, &d);
}

// CHECK-LABEL: @_Z4g456PVFiPiPdE
// ABI5-NOT: @_Z4g456PVFiPiPdE
int foo() {
  fp f = g123;
  int i = 666;
  double d = 777.888;

  g456(f);
  g123(&i, &d);
  g456(g123);
  return 0;
}

// CHECK-LABEL: @{{.*}}foo
// CHECK: call{{.+}}@_Z4g456PVFiPiPdE
// CHECK: call{{.+}}@_Z4g123PiPd
// ABI5: call{{.+}}@_Z4g456PFiPiPdE
// ABI5-NOT: call{{.+}}@_Z4g456PVFiPiPdE

void g789(int i, double d, fp func) {
  func(&i, &d);
}

// CHECK-LABEL: @_Z4g789idPVFiPiPdE
