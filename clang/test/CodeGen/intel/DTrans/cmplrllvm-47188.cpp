// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

struct Vector {
  virtual void add(__complex__ float);
};

void foo() {
  Vector *u;
  u->add(-1);
}

// CHECK: define dso_local void @_Z3foov
// CHECK: alloca ptr, align 8, !intel_dtrans_type ![[VECTOR_PTR:[0-9]+]]
// CHECK: call void %{{.*}} !intel_dtrans_type ![[FUNC_MD:[0-9]+]]

// CHECK: ![[VECTOR_PTR]] = !{%struct._ZTS6Vector.Vector zeroinitializer, i32 1}

// CHECK: ![[FUNC_MD]] = !{!"F", i1 false, i32 2, ![[VOID:[0-9]+]], ![[VECTOR_PTR]], ![[FLOAT_VECTOR:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[FLOAT_VECTOR]] = !{!"V", i32 2, ![[FLOAT:[0-9]+]]}
// CHECK: ![[FLOAT]] = !{float {{.*}}, i32 0}

