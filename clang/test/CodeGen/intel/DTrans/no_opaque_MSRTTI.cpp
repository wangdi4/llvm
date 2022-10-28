// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows-pc -fexceptions -fcxx-exceptions -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows-pc -fexceptions -fcxx-exceptions -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK,OPQ

struct Base {
  virtual ~Base(){}
};
template<class Elem> struct Arr : Base {
  virtual ~Arr(){}
};

static Arr<int> Arr1;
static Arr<long> Arr2;

// PTR: %rtti.TypeDescriptor13 = type { i8**, i8*, [14 x i8] }
// OPQ: %rtti.TypeDescriptor13 = type { ptr, ptr, [14 x i8] }
// PTR: %rtti.TypeDescriptor10 = type { i8**, i8*, [11 x i8] }
// OPQ: %rtti.TypeDescriptor10 = type { ptr, ptr, [11 x i8] }

// CHECK: !intel.dtrans.types = !{![[ARRINTTY:[0-9]+]], ![[BASETY:[0-9]+]], ![[ARRLONGTY:[0-9]+]], ![[TD13:[0-9]+]], ![[TD10:[0-9]+]]}
// CHECK: ![[CHARPTR:[0-9]+]] = !{i8 0, i32 1}
// CHECK: ![[INT:[0-9]+]] = !{i32 0, i32 0}
// CHECK: ![[ARRINTTY]] = !{!"S", %"struct..?AU?$Arr@H@@.Arr" zeroinitializer, i32 1, ![[BASE_REF:[0-9]+]]}
// CHECK: ![[BASE_REF]] = !{%"struct..?AUBase@@.Base" zeroinitializer, i32 0}
// CHECK: ![[BASETY]] = !{!"S", %"struct..?AUBase@@.Base" zeroinitializer, i32 1, ![[BASE_VTBL:[0-9]+]]}
// CHECK: ![[BASE_VTBL]] = !{![[BASE_VTBL_FUNC:[0-9]+]], i32 2}
// CHECK: ![[BASE_VTBL_FUNC]] = !{!"F", i1 true, i32 0, ![[INT]]}
// CHECK: ![[ARRLONGTY]] = !{!"S", %"struct..?AU?$Arr@J@@.Arr" zeroinitializer, i32 1, ![[BASE_REF]]}
// CHECK: ![[TD13]] = !{!"S", %rtti.TypeDescriptor13 zeroinitializer, i32 3, ![[CHARPTRPTR:[0-9]+]], ![[CHARPTR]], ![[STR14:[0-9]+]]}
// CHECK: ![[CHARPTRPTR]] = !{i8 0, i32 2}
// CHECK: ![[STR14]] = !{!"A", i32 14, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[TD10]] = !{!"S", %rtti.TypeDescriptor10 zeroinitializer, i32 3, ![[CHARPTRPTR]], ![[CHARPTR]], ![[STR11:[0-9]+]]}
// CHECK: ![[STR11]] = !{!"A", i32 11, ![[CHAR]]}
