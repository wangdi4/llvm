// CQ#368119

// RUN: %clang_cc1 -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK1 %s
// CHECK1-NOT: !llvm.dbg.ms.filetype = !{!{{.*}}}

// RUN: %clang_cc1 -fintel-ms-compatibility -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK2 %s
// CHECK2-NOT: !llvm.dbg.ms.filetype = !{!{{.*}}}

// RUN: %clang_cc1 -fms-debug-info-file-type=obj -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK3 %s
// CHECK3-NOT: !llvm.dbg.ms.filetype = !{!{{.*}}}

// RUN: %clang_cc1 -fintel-ms-compatibility -fms-debug-info-file-type=obj -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK4 %s
// CHECK4: !llvm.dbg.ms.filetype = !{!{{.*}}}
// CHECK4: !{{.*}} = !{!"obj"}

// RUN: %clang_cc1 -fintel-ms-compatibility -fms-debug-info-file-type=pdb -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK5 %s
// CHECK5: !llvm.dbg.ms.filetype = !{!{{.*}}}
// CHECK5: !{{.*}} = !{!"pdb"}

int main() {
  return 0;
}
