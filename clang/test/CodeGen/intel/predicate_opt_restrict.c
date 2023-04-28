// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -disable-llvm-passes -O2 \
// RUN:  -emit-llvm -fintel-compatibility -o - %s | FileCheck %s

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -disable-llvm-passes -O2 \
// RUN:  -emit-llvm -fintel-compatibility-enable=RestrictMetadata -o - %s \
// RUN:  | FileCheck %s

// Off without optimization
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -disable-llvm-passes \
// RUN:  -emit-llvm -fintel-compatibility-enable=RestrictMetadata -o - %s \
// RUN:  | FileCheck %s --check-prefix OFF

// Off without optimization
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -disable-llvm-passes \
// RUN:  -emit-llvm -fintel-compatibility -o - %s \
// RUN:  | FileCheck %s --check-prefix OFF

// Off with C++
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -disable-llvm-passes \
// RUN:  -emit-llvm -fintel-compatibility -o - %s -O2 -x c++ \
// RUN:  -Drestrict=__restrict | FileCheck %s --check-prefix OFF

// Off if disabled with customization control
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -disable-llvm-passes -O2 \
// RUN:  -emit-llvm -fintel-compatibility -o - %s \
// RUN:  -fintel-compatibility-disable=RestrictMetadata \
// RUN:  | FileCheck %s --check-prefix OFF

typedef struct Foo {
  int i,j;
} Foo;

//OFF-NOT: predicate-opt-restrict

//CHECK: define {{.*}}something
int something(Foo *restrict P1, Foo *P2) {
// CHECK: [[P1:%P1.*]] = alloca ptr, align 8, !predicate-opt-restrict ![[N:[0-9]+]]
// CHECK: [[P2:%P2.*]] = alloca ptr, align 8{{$}}
  return P1->i + P1->j;
};

Foo *getFoo(void);

//CHECK: define {{.*}}foo
int foo()
{
// CHECK: [[CP1:%Cp1.*]] = alloca ptr, align 8, !predicate-opt-restrict ![[N]]
// CHECK: [[CP2:%Cp2.*]] = alloca ptr, align 8{{$}}
// CHECK-NOT: [[CPINNER:%CpInner.*]] = alloca{{.*}}!predicate-opt-restrict
// CHECK: [[CP3:%Cp3.*]] = alloca ptr, align 8, !predicate-opt-restrict ![[N]]

  Foo *restrict Cp1 = getFoo();
  Foo *Cp2 = getFoo();

  {
    Foo *restrict CpInner = getFoo();
  }

  Foo *restrict Cp3 = getFoo();

  return something(Cp1, Cp2);
}

// CHECK: ![[N]] = !{}

