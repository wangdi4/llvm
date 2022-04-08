// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,LIN
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,WIN

// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,LIN
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,WIN

struct has_bitfields {
  int i;
  unsigned int bf1 : 16;
  unsigned int bf2 : 16;
};

const struct has_bitfields hbfs1[] = {{1,2,3}};
// CHECK: @hbfs1 = {{.*}}constant [1 x { i32, i8, i8, i8, i8 }] {{.+}}!intel_dtrans_type ![[HBFS1:[0-9]+]]
const struct has_bitfields hbfs2[] = {};
// LIN: @hbfs2 = constant [0 x %struct._ZTS13has_bitfields.has_bitfields] {{.+}}!intel_dtrans_type ![[HBFS2:[0-9]+]]
// WIN: @hbfs2 = dso_local constant [0 x %"struct..?AUhas_bitfields@@.has_bitfields"] {{.+}}!intel_dtrans_type ![[HBFS2:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[HAS_BF_TY:[0-9]+]]}
// CHECK: ![[HBFS1]] = !{!"A", i32 1, ![[LITERAL_REF:[0-9]+]]}
// CHECK: ![[LITERAL_REF]] = !{![[LITERAL:[0-9]+]], i32 0}
// CHECK: ![[LITERAL]] = !{!"L", i32 5, ![[I32:[0-9]+]], ![[CHAR:[0-9]+]], ![[CHAR]], ![[CHAR]], ![[CHAR]]}
// CHECK: ![[I32]] = !{i32 0, i32 0}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[HBFS2]] = !{!"A", i32 0, ![[HAS_BF_ZINIT:[0-9]+]]}
// LIN: ![[HAS_BF_ZINIT]] = !{%struct._ZTS13has_bitfields.has_bitfields zeroinitializer, i32 0}
// WIN: ![[HAS_BF_ZINIT]] = !{%"struct..?AUhas_bitfields@@.has_bitfields" zeroinitializer, i32 0}
// LIN: ![[HAS_BF_TY]] = !{!"S", %struct._ZTS13has_bitfields.has_bitfields zeroinitializer, i32 2, ![[I32]], ![[I32]]}
// WIN: ![[HAS_BF_TY]] = !{!"S", %"struct..?AUhas_bitfields@@.has_bitfields" zeroinitializer, i32 2, ![[I32]], ![[I32]]}
