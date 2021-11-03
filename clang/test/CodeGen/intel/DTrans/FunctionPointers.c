// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

struct points {
  int x, y;
};

typedef void (*fptr)(long, struct points *);

struct fptr_test {
  fptr m_func;
};

// CHECK: define dso_local void @foo(i64 %x, %struct._ZTS6points.points* "intel_dtrans_func_index"="1" %array) {{.*}}!intel.dtrans.func.type ![[FOO_MD:[0-9]+]]
void foo(long x, struct points *array) {
  // CHECK: alloca %struct._ZTS6points.points*, align 8, !intel_dtrans_type ![[POINTS_PTR:[0-9]+]]
}

int main() {
  struct fptr_test instance;
  instance.m_func = foo;

  return 0;
}

// CHECK: !intel.dtrans.types = !{![[POINTS:[0-9]+]], ![[FPTR_TEST:[0-9]+]]}

// CHECK: ![[POINTS]] = !{!"S", %struct._ZTS6points.points zeroinitializer, i32 2, ![[INT:[0-9]+]], ![[INT]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[FPTR_TEST]] = !{!"S", %struct._ZTS9fptr_test.fptr_test zeroinitializer, i32 1, ![[FPTR:[0-9]+]]}
// CHECK: ![[FPTR]] = !{![[FUNC:[0-9]+]], i32 1}
// CHECK: ![[FUNC]] = !{!"F", i1 false, i32 2, ![[VOID:[0-9]+]], ![[LONG_PARAM:[0-9]+]], ![[POINTS_PTR]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[LONG_PARAM]] = !{i64 0, i32 0}
// CHECK: ![[POINTS_PTR]] = !{%struct._ZTS6points.points zeroinitializer, i32 1}
// CHECK: ![[FOO_MD]] = distinct !{![[POINTS_PTR]]}
