// CQ#368123

// RUN: %clang_cc1 -emit-llvm %s -o - | FileCheck -check-prefix CHECK1 %s
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm %s -o - | FileCheck -check-prefix CHECK1 %s
// RUN: %clang_cc1 -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK1 %s
// CHECK1-NOT: !llvm.dbg.intel.emit_class_debug_always = !{!{{.*}}}

// RUN: %clang_cc1 -fintel-compatibility -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK2 %s
// CHECK2: !llvm.dbg.intel.emit_class_debug_always = !{!{{.*}}}
// CHECK2: !{{.*}} = !{!"true"}

// RUN: %clang_cc1 -fintel-compatibility -femit-class-debug-always -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK3 %s
// CHECK3: !llvm.dbg.intel.emit_class_debug_always = !{!{{.*}}}
// CHECK3: !{{.*}} = !{!"true"}

// RUN: %clang_cc1 -fintel-compatibility -fno-emit-class-debug-always -g -emit-llvm %s -o - | FileCheck -check-prefix CHECK4 %s
// CHECK4: !llvm.dbg.intel.emit_class_debug_always = !{!{{.*}}}
// CHECK4: !{{.*}} = !{!"false"}

int main() {
  return 0;
}
