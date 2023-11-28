// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

struct poly_int {
  poly_int();
  union U *coeffs[1];
};

// CHECK: define dso_local "intel_dtrans_func_index"="1" ptr @{{.*}}(ptr "intel_dtrans_func_index"="2" %{{.*}}){{.*}} !intel.dtrans.func.type ![[FUNC_MD:[0-9]+]]
poly_int to_poly_offset (poly_int J)
{
  poly_int I;
  return I;
}

// CHECK: declare !intel.dtrans.func.type ![[POLY_CTOR:[0-9]+]] void {{.*}}(ptr {{.*}} "intel_dtrans_func_index"="1")

// CHECK: !intel.dtrans.types = !{![[POLY_INT_TY:[0-9]+]], ![[UNION_TY:[0-9]+]]}

// CHECK: ![[POLY_INT_TY]] = !{!"S", %struct._ZTS8poly_int.poly_int zeroinitializer, i32 1, ![[ARR:[0-9]+]]}
// CHECK: ![[ARR]] = !{!"A", i32 1, ![[UNION_PTR:[0-9]+]]}
// CHECK: ![[UNION_PTR]] = !{%union._ZTS1U.U zeroinitializer, i32 1}
// CHECK: ![[UNION_TY]] = !{!"S", %union._ZTS1U.U zeroinitializer, i32 -1}

// CHECK: ![[FUNC_MD]] = distinct !{![[UNION_PTR]], ![[UNION_PTR]]}

// CHECK: ![[POLY_CTOR]] = distinct !{![[POLY_PTR:[0-9]+]]}
// CHECK: ![[POLY_PTR]] = !{%struct._ZTS8poly_int.poly_int zeroinitializer, i32 1}
