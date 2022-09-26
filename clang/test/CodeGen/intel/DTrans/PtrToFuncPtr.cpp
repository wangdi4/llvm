// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-pc -emit-dtrans-info -emit-llvm -opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK


using FPtr = void(*)();

void foo(FPtr *F){}

// CHECK: define{{.*}} void @_Z3fooPPFvvE(ptr noundef "intel_dtrans_func_index"="1" %{{.+}}){{.*}}!intel.dtrans.func.type ![[FOO:[0-9]+]]

// CHECK: ![[FOO]] = distinct !{![[PTRS_TO_FUNC:[0-9]+]]}
// CHECK: ![[PTRS_TO_FUNC]] = !{![[FUNC:[0-9]+]], i32 2}
// CHECK: ![[FUNC]] = !{!"F", i1 false, i32 0, ![[VOID:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
