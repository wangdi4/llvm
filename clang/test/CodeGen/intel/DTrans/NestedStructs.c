// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s
struct inner_state_t {
  unsigned int TTProbes;
  unsigned int TTHits;
  unsigned int TTStores;
};

struct outer_state_t {
  int threadid;
  struct inner_state_t stats;
};

int main() {
  struct outer_state_t instance;

  instance.threadid = 0;
  instance.stats.TTHits = 0;

  return 0;
}

// CHECK: !intel.dtrans.types = !{![[OUTER:[0-9]+]], ![[INNER:[0-9]+]]}
// CHECK: ![[OUTER]] = !{!"S", %struct._ZTS13outer_state_t.outer_state_t zeroinitializer, i32 2, ![[INT:[0-9]+]], ![[INNER_STRUCT:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[INNER_STRUCT]] = !{%struct._ZTS13inner_state_t.inner_state_t zeroinitializer, i32 0}
// CHECK: ![[INNER]] = !{!"S", %struct._ZTS13inner_state_t.inner_state_t zeroinitializer, i32 3, ![[INT]], ![[INT]], ![[INT]]}
