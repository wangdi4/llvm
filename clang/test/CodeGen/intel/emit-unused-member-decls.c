// CQ#368123

// RUN: %clang_cc1 -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK1 %s
// CHECK1-NOT: !llvm.dbg.intel.emit_unused_member_decls = !{!{{.*}}}

// RUN: %clang_cc1 -fintel-compatibility -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK2 %s
// CHECK2: !llvm.dbg.intel.emit_unused_member_decls = !{!{{.*}}}
// CHECK2: !{{.*}} = !{!"true"}

// RUN: %clang_cc1 -fintel-compatibility --emit-unused-member-decls -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK3 %s
// CHECK3: !llvm.dbg.intel.emit_unused_member_decls = !{!{{.*}}}
// CHECK3: !{{.*}} = !{!"true"}

// RUN: %clang_cc1 -fintel-compatibility --no-emit-unused-member-decls -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK4 %s
// CHECK4: !llvm.dbg.intel.emit_unused_member_decls = !{!{{.*}}}
// CHECK4: !{{.*}} = !{!"false"}

int main() {
  return 0;
}
