// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility -O3 %s -disable-llvm-optzns -emit-llvm -o - | FileCheck %s
//
// Check that we generate fakeload intrinsic for the return pointers
//
struct S {
  int a[4];
  int b[4];

  int& geta(int i) {
    return a[i];
  }
  int& getb(int i) {
    return b[i];
  }
};

int foo(S& s, int i, int j) {
  s.geta(i) = 0;
  s.getb(j) = 1;
  return s.geta(i);
}

// CHECK:  %{{.*}} = call i32* @llvm.intel.fakeload.p0i32(i32* %{{.*}}, metadata !{{.*}})
