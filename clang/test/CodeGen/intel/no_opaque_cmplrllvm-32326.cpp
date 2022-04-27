// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O3 -disable-llvm-passes -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s

struct HasCrash {
  float *DoThing() const {
    if (!Var1)
      return nullptr;

    // Note: Most of these variables are required just to make the fake-load
    // memory location get overwritten.
    float* __restrict Var3Cpy = Var3;
    const int * __restrict Var2Copy = Var2;

    int Var1Index = Var1[0];
    return &Var3Cpy[0];
  }

  int *Var1;
  int *Var2;
  float *Var3;
};

void CauseEmit() {
  HasCrash Cr;
  Cr.DoThing();
}

// CHECK: define{{.*}} float* @_ZNK8HasCrash7DoThingEv(
// CHECK: ret float*
