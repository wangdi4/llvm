// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

struct S {S();};
S Global;

// CHECK: @llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr {{.+}}, ptr null }], !intel_dtrans_type ![[GLOBAL_CTORS_MD:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[S_MD:[0-9]+]]}
// CHECK: ![[GLOBAL_CTORS_MD]] =  !{!"A", i32 1, ![[STRUCT_REF:[0-9]+]]
// CHECK: ![[STRUCT_REF]] = !{![[STRUCT:[0-9]+]], i32 0}
// CHECK: ![[STRUCT]] = !{!"L", i32 3, ![[INT:[0-9]+]], ![[FPTR:[0-9]+]], ![[ACC_DATA:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[FPTR]] = !{![[FUNC:[0-9]+]], i32 1}
// CHECK: ![[FUNC]] = !{!"F", i1 false, i32 0, ![[VOID:[0-9]+]]}
// CHECK: ![[VOID]] = !{!"void", i32 0}
// CHECK: ![[ACC_DATA]] = !{i8 0, i32 1}

// CHECK: ![[S_MD]] = !{!"S", %struct._ZTS1S.S zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
