// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

struct ContainsFPType;
using FPType = int(*)(ContainsFPType*);

struct ContainsFPType {
  FPType F;
};

int func1(ContainsFPType*) {
  return 1;
}

struct ContainsFPType2;
using FPType2 = int(*)(ContainsFPType2*);

struct ContainsFPType2 {
  FPType2 F;
};

int func2(ContainsFPType2*) {
  return 1;
}

// PTR: %struct._ZTS14ContainsFPType.ContainsFPType = type { %"__Intel$Empty$Struct"* }
// OPQ: %struct._ZTS14ContainsFPType.ContainsFPType = type { ptr }
// PTR: %struct._ZTS15ContainsFPType2.ContainsFPType2 = type { %"__Intel$Empty$Struct.0"* }
// OPQ: %struct._ZTS15ContainsFPType2.ContainsFPType2 = type { ptr }

// PTR: define dso_local noundef i32 @_Z5func1P14ContainsFPType(%struct._ZTS14ContainsFPType.ContainsFPType* {{.*}} "intel_dtrans_func_index"="1" %{{.*}}) {{.*}}!intel.dtrans.func.type ![[FUNC1:[0-9]+]]
// OPQ: define dso_local noundef i32 @_Z5func1P14ContainsFPType(ptr {{.*}} "intel_dtrans_func_index"="1" %{{.*}}) {{.*}}!intel.dtrans.func.type ![[FUNC1:[0-9]+]]
// PTR: define dso_local noundef i32 @_Z5func2P15ContainsFPType2(%struct._ZTS15ContainsFPType2.ContainsFPType2* {{.*}} "intel_dtrans_func_index"="1" %{{.*}}) {{.*}}!intel.dtrans.func.type ![[FUNC2:[0-9]+]]
// OPQ: define dso_local noundef i32 @_Z5func2P15ContainsFPType2(ptr {{.*}} "intel_dtrans_func_index"="1" %{{.*}}) {{.*}}!intel.dtrans.func.type ![[FUNC2:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[CONT_FP_TYPE:[0-9]+]], ![[CONT_FP_TYPE2:[0-9]+]]}

// CHECK: ![[CONT_FP_TYPE]] = !{!"S", %struct._ZTS14ContainsFPType.ContainsFPType zeroinitializer, i32 1, ![[FPTR:[0-9]+]]}
// CHECK: ![[FPTR]] = !{![[FUNC_INFO:[0-9]+]], i32 1}
// CHECK: ![[FUNC_INFO]] = !{!"F", i1 false, i32 1, ![[INT:[0-9]]], ![[FP_TY_PTR:[0-9]+]]}
// CHECK: ![[FP_TY_PTR]] = !{%struct._ZTS14ContainsFPType.ContainsFPType zeroinitializer, i32 1}
// CHECK: ![[CONT_FP_TYPE2]] = !{!"S", %struct._ZTS15ContainsFPType2.ContainsFPType2 zeroinitializer, i32 1, ![[FPTR2:[0-9]+]]}
// CHECK: ![[FPTR2]] = !{![[FUNC_INFO2:[0-9]+]], i32 1}
// CHECK: ![[FUNC_INFO2]] = !{!"F", i1 false, i32 1, ![[INT:[0-9]]], ![[FP_TY2_PTR:[0-9]+]]}
// CHECK: ![[FP_TY2_PTR]] = !{%struct._ZTS15ContainsFPType2.ContainsFPType2 zeroinitializer, i32 1}

// CHECK: ![[FUNC1]] = distinct !{![[FP_TY_PTR]]}
// CHECK: ![[FUNC2]] = distinct !{![[FP_TY2_PTR]]}
