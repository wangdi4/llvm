// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -opaque-pointers -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

struct Struct;
void NormalStruct(struct Struct*);
void VAList(__builtin_va_list*);

void use(void) {
  (void)NormalStruct;
  (void)VAList;
}


// CHECK: declare !intel.dtrans.func.type ![[NORMAL_FUNC:[0-9]+]] void @NormalStruct(ptr noundef "intel_dtrans_func_index"="1")
// CHECK: declare !intel.dtrans.func.type ![[VALIST_FUNC:[0-9]+]] void @VAList(ptr noundef "intel_dtrans_func_index"="1")

// CHECK: !intel.dtrans.types = !{![[NORMAL_STRUCT:[0-9]+]], ![[VALIST_STRUCT:[0-9]+]]}

// CHECK: ![[NORMAL_STRUCT]] = !{!"S", %struct._ZTS6Struct.Struct zeroinitializer, i32 -1}
// CHECK: ![[VALIST_STRUCT]] = !{!"S", %struct._ZTS13__va_list_tag.__va_list_tag zeroinitializer, i32 4, ![[INT:[0-9]+]], ![[INT]], ![[CHAR_PTR:[0-9]+]], ![[CHAR_PTR]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}

// CHECK: ![[NORMAL_FUNC]] = distinct !{![[NORMAL_FUNC_DISTINCT:[0-9]+]]}
// CHECK: ![[NORMAL_FUNC_DISTINCT]] = !{%struct._ZTS6Struct.Struct zeroinitializer, i32 1}

// __builtin_va_list shows up as this goofy pointer-to-array-of-length-1. 
// CHECK: ![[VALIST_FUNC]] = distinct !{![[VALIST_FUNC_DISTINCT:[0-9]+]]}
// CHECK: ![[VALIST_FUNC_DISTINCT]] = !{![[ARR:[0-9]+]], i32 1}
// CHECK: ![[ARR]] = !{!"A", i32 1, ![[VALIST_REF:[0-9]+]]}
// CHECK: ![[VALIST_REF]] = !{%struct._ZTS13__va_list_tag.__va_list_tag zeroinitializer, i32 0}

