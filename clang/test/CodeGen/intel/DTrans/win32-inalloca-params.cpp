// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple i386-pc-windows-msvc -emit-dtrans-info -emit-llvm -opaque-pointers %s -o - | FileCheck %s 

// Validate that we properly generate DTrans data for an inalloc of a collection
// of parameters.
//
struct ItrBase {
  ItrBase(const ItrBase &theRhs);
};

struct ListTy {
  int &constructNode(ItrBase B, ListTy &, int i, int[], int[i]);
};

void use(ListTy &List, ItrBase &B) {
  int arr[5];
  List.constructNode(B, List, 5, arr, arr);
}

// CHECK: define dso_local void @"?use@@YAXAAUListTy@@AAUItrBase@@@Z"(ptr {{.*}}"intel_dtrans_func_index"="1" %{{.*}}, ptr {{.*}}"intel_dtrans_func_index"="2" %{{.*}}){{.*}}!intel.dtrans.func.type ![[USE:[0-9]+]]
// CHECK: alloca [5 x i32]{{.*}}!intel_dtrans_type ![[INT_ARR_5:[0-9]+]]

// CHECK: declare !intel.dtrans.func.type ![[CONSTR_NODE:[0-9]+]] dso_local x86_thiscallcc {{.*}}"intel_dtrans_func_index"="1" ptr @"?constructNode@ListTy@@QAEAAHUItrBase@@AAU1@HQAH2@Z"(ptr {{.*}}"intel_dtrans_func_index"="2", ptr inalloca(<{ %"struct..?AUItrBase@@.ItrBase", [3 x i8], ptr, i32, ptr, ptr }>) "intel_dtrans_func_index"="3")

// CHECK: declare !intel.dtrans.func.type ![[BASE_CTOR:[0-9]+]] dso_local x86_thiscallcc {{.*}}"intel_dtrans_func_index"="1" ptr @"??0ItrBase@@QAE@ABU0@@Z"(ptr {{.*}}"intel_dtrans_func_index"="2", ptr {{.*}}"intel_dtrans_func_index"="3")

// CHECK: !intel.dtrans.types = !{![[LIST_TY:[0-9]+]], ![[BASE_TY:[0-9]+]]}

// CHECK: ![[LIST_TY]] = !{!"S", %"struct..?AUListTy@@.ListTy" zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[BASE_TY]] = !{!"S", %"struct..?AUItrBase@@.ItrBase" zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK: ![[USE]] = distinct !{![[LIST_PTR:[0-9]+]], ![[BASE_PTR:[0-9]+]]}
// CHECK: ![[LIST_PTR]] = !{%"struct..?AUListTy@@.ListTy" zeroinitializer, i32 1}
// CHECK: ![[BASE_PTR]] = !{%"struct..?AUItrBase@@.ItrBase" zeroinitializer, i32 1} 
// CHECK: ![[INT_ARR_5]] = !{!"A", i32 5, ![[INT:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}

// CHECK: ![[CONSTR_NODE]] = distinct !{![[INT_PTR:[0-9]+]], ![[LIST_PTR]], ![[INALLOCA_LIT_PTR:[0-9]+]]}
// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}

// CHECK: ![[INALLOCA_LIT_PTR]] = !{!"L", i32 6, ![[BASE:[0-9]+]], ![[PADDING_ARR:[0-9]+]], ![[LIST_PTR]], ![[INT]], ![[INT_PTR]], ![[INT_PTR]]}
// CHECK: ![[BASE]] = !{%"struct..?AUItrBase@@.ItrBase" zeroinitializer, i32 0} 
// CHECK: ![[PADDING_ARR]] = !{!"A", i32 3, ![[CHAR]]}

// CHECK: ![[BASE_CTOR]] = distinct !{![[BASE_PTR]], ![[BASE_PTR]], ![[BASE_PTR]]} 
