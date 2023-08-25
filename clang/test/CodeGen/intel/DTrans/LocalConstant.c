// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
struct S {
  char *f;
};

const char *const g[14] = {
          "!!", " P", "*P", " N", "*N", " K", "*K", " R", "*R", " Q",
                  "*Q", " B", "*B", "  "
                     };
// OPQ: @g = constant [14 x ptr] [ptr {{.*}} !intel_dtrans_type ![[CHAR14_ARRAY:[0-9]+]]
// OPQ: constant [14 x ptr] [ptr {{.*}} !intel_dtrans_type ![[CHAR14_ARRAY:[0-9]+]]
// CHECK: @[[FOO_NAME:.+]] = internal global %struct._ZTS1S.S {{{.+}}}, align 8
// OPQ: internal global ptr @[[FOO_NAME]], align 8, !intel_dtrans_type ![[STRUCT_PTR:[0-9]+]]

int main() {
    const char *l[14] = {
              "!!", " P", "*P", " N", "*N", " K", "*K", " R", "*R", " Q",
                      "*Q", " B", "*B", "  "
                         };
    static struct S foo = {&g[0]};
    static struct S *foo_ptr = &foo;
    return 0;
}
// CHECK: !intel.dtrans.types = !{![[STRUCT:[0-9]+]]}

// CHECK: [[CHAR14_ARRAY]] = !{!"A", i32 14, ![[CHAR_PTR:[0-9]+]]}
// CHECK: [[CHAR_PTR]] = !{i8 0, i32 1}
// CHECK: [[STRUCT_PTR]] = !{%struct._ZTS1S.S zeroinitializer, i32 1}
// CHECK: [[STRUCT]] = !{!"S", %struct._ZTS1S.S zeroinitializer, i32 1, ![[CHAR_PTR]]}
