// RUN: %clang_cc1 -fintel-compatibility -triple=x86_64-unknown-linux-gnu -O0 %s -emit-llvm -o - | FileCheck %s -check-prefix=CHECK-S1
// RUN: %clang_cc1 -fintel-compatibility -triple=x86_64-unknown-linux-gnu -O0 %s -emit-llvm -o - | FileCheck %s -check-prefix=CHECK-S2

struct S1 {
  static int f() { return 1; }
};

struct S2 {
  static int f() { return 2; }
};

struct A {
  template<class B> int f() {
    return B::f();
  };
};

template<class T, class B>
int g(T *p) {
  // Check that the proper method is called via template instansiation.
  return p->f<B>();
}

int main() {
  A a;
  // CHECK-S1: %{{.+}} = call i32 @_Z1gI1A2S1EiPT_(%struct.A* %{{.+}})
  // CHECK-S2: %{{.+}} = call i32 @_Z1gI1A2S2EiPT_(%struct.A* %{{.+}})
  int x1 = g<A, S1>(&a);
  int x2 = g<A, S2>(&a);
  return 0;
}

// CHECK-S1: define {{.*}}i32 @_Z1gI1A2S1EiPT_(%struct.A* %{{.+}})
// CHECK-S2: define {{.*}}i32 @_Z1gI1A2S2EiPT_(%struct.A* %{{.+}})
// CHECK-S1: %{{.+}} = call i32 @_ZN1A1fI2S1EEiv(%struct.A* %{{.+}})
// CHECK-S2: %{{.+}} = call i32 @_ZN1A1fI2S2EEiv(%struct.A* %{{.+}})

// CHECK-S1: define {{.*}}i32 @_ZN1A1fI2S1EEiv(%struct.A* %{{.+}})
// CHECK-S2: define {{.*}}i32 @_ZN1A1fI2S2EEiv(%struct.A* %{{.+}})
// CHECK-S1: %{{.+}} = call i32 @_ZN2S11fEv()
// CHECK-S2: %{{.+}} = call i32 @_ZN2S21fEv()

// CHECK-S1: define {{.*}}i32 @_ZN2S11fEv()
// CHECK-S2: define {{.*}}i32 @_ZN2S21fEv()
// CHECK-S1: ret i32 1
// CHECK-S2: ret i32 2
