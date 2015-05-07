// CQ#368125

// RUN: %clang_cc1 -fintel-ms-compatibility -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK1 %s
// CHECK1-NOT: !llvm.dbg.ms.obj = !{!{{.*}}}
// CHECK1-NOT: !llvm.dbg.ms.pdb = !{!{{.*}}}

// RUN: %clang_cc1 -fintel-ms-compatibility -fms-debug-info-obj-file=out.obj -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK2 %s
// CHECK2-NOT: !llvm.dbg.ms.pdb = !{!{{.*}}}
// CHECK2: !llvm.dbg.ms.obj = !{!{{.*}}}
// CHECK2: !{{.*}} = !{!"out.obj"}

// RUN: %clang_cc1 -fintel-ms-compatibility -fms-debug-info-pdb-file=out.pdb -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK3 %s
// CHECK3-NOT: !llvm.dbg.ms.obj = !{!{{.*}}}
// CHECK3: !llvm.dbg.ms.pdb = !{!{{.*}}}
// CHECK3: !{{.*}} = !{!"out.pdb"}

// RUN: %clang_cc1 -fintel-ms-compatibility -fms-debug-info-obj-file=out.obj -fms-debug-info-pdb-file=out.pdb -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK4 %s
// CHECK4: !llvm.dbg.ms.obj = !{!{{.*}}}
// CHECK4: !llvm.dbg.ms.pdb = !{!{{.*}}}
// CHECK4: !{{.*}} = !{!"out.obj"}
// CHECK4: !{{.*}} = !{!"out.pdb"}

// RUN: %clang_cc1 -fintel-ms-compatibility -fms-debug-info-file-type=pdb -fms-debug-info-obj-file=out.obj -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK5 %s
// CHECK5-NOT: !llvm.dbg.ms.pdb = !{!{{.*}}}
// CHECK5: !llvm.dbg.ms.obj = !{!{{.*}}}
// CHECK5: !{{.*}} = !{!"out.obj"}

// RUN: %clang_cc1 -fintel-ms-compatibility -fms-debug-info-file-type=obj -fms-debug-info-pdb-file=out.pdb -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK6 %s
// CHECK6-NOT: !llvm.dbg.ms.obj = !{!{{.*}}}
// CHECK6: !llvm.dbg.ms.pdb = !{!{{.*}}}
// CHECK6: !{{.*}} = !{!"out.pdb"}

// RUN: %clang_cc1 -fintel-ms-compatibility -fms-debug-info-file-type=pdb -fms-debug-info-obj-file=out.obj -fms-debug-info-pdb-file=out.pdb -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK7 %s
// CHECK7: !llvm.dbg.ms.obj = !{!{{.*}}}
// CHECK7: !llvm.dbg.ms.pdb = !{!{{.*}}}
// CHECK7: !{{.*}} = !{!"out.obj"}
// CHECK7: !{{.*}} = !{!"out.pdb"}

int main() {
  return 0;
}
