// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple i386-windows-pc -emit-dtrans-info -emit-llvm -opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK

struct S {
  using MFPT = void(S::*)();
  explicit S(MFPT);
  void mem_func();
};

void use() {
  S s(&S::mem_func);
}

// CHECK: declare !intel.dtrans.func.type ![[MEM_FUNC:[0-9]+]]{{.*}}@"?mem_func@S@@QAEXXZ"(ptr{{.*}} "intel_dtrans_func_index"="1")
// CHECK: declare !intel.dtrans.func.type ![[CTOR:[0-9]+]]{{.*}}"intel_dtrans_func_index"="1" ptr @"??0S@@QAE@P80@AEXXZ@Z"(ptr{{.*}} "intel_dtrans_func_index"="2", ptr{{.*}} "intel_dtrans_func_index"="3")
// CHECK: ![[MEM_FUNC]] = distinct !{![[S_PTR:[0-9]+]]}
// CHECK: ![[S_PTR]] = !{%"struct..?AUS@@.S" zeroinitializer, i32 1}
// CHECK: ![[CTOR]] = distinct !{![[S_PTR]], ![[S_PTR]], ![[PMF:[0-9]+]]}
// CHECK: ![[PMF]] = !{i8 0, i32 2}
