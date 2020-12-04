// RUN: %clang_cc1 -emit-llvm -o - %s | FileCheck %s

// CHECK: !prefer_dsp ![[MD_F1:[0-9]+]]
[[intel::prefer_dsp]] void f1_cxx11() {}
// CHECK: !prefer_dsp ![[MD_F1]]
__attribute__((prefer_dsp)) void f1_gnu() {}

// CHECK: !prefer_dsp ![[MD_F2:[0-9]+]]
[[intel::prefer_softlogic]] void f2_cxx11() {}
// CHECK: !prefer_dsp ![[MD_F2]]
__attribute__((prefer_softlogic)) void f2_gnu() {}

// CHECK: !propagate_dsp_preference ![[MD_F1]]
[[intel::propagate_dsp_preference]] void f3_cxx11() {}
// CHECK: !propagate_dsp_preference ![[MD_F1]]
__attribute__((propagate_dsp_preference)) void f3_gnu() {}

// CHECK-NOT: !prefer_dsp
void f4() {}

struct S {
  // CHECK: !prefer_dsp ![[MD_F1]]
  [[intel::prefer_dsp]] void sf1_cxx11();
  // CHECK: !prefer_dsp ![[MD_F1]]
  __attribute__((prefer_dsp)) void sf1_gnu();

  // CHECK: !prefer_dsp ![[MD_F2]]
  [[intel::prefer_softlogic]] void sf2_cxx11();
  // CHECK: !prefer_dsp ![[MD_F2]]
  __attribute__((prefer_softlogic)) void sf2_gnu();

  // CHECK: !propagate_dsp_preference ![[MD_F1]]
  [[intel::propagate_dsp_preference]] void sf3_cxx11();
  // CHECK: !propagate_dsp_preference ![[MD_F1]]
  __attribute__((propagate_dsp_preference)) void sf3_gnu();

  // CHECK-NOT: !prefer_dsp
  void sf4();
};
void S::sf1_cxx11() {}
void S::sf1_gnu() {}
void S::sf2_cxx11() {}
void S::sf2_gnu() {}
void S::sf3_cxx11() {}
void S::sf3_gnu() {}
void S::sf4() {}

// CHECK: ![[MD_F1]] = !{i32 1}
// CHECK: ![[MD_F2]] = !{i32 0}
