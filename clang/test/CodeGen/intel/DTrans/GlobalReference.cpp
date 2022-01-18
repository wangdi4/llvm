// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s -check-prefix=CHECK-LINUX
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows-msvc -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s -check-prefix=CHECK-WINDOWS

class foo {
  char x;
};

foo a;
foo &b = a;
foo *c = &a;

// CHECK-LINUX: @a = global %class._ZTS3foo.foo zeroinitializer, align 1
// CHECK-LINUX: @b = constant %class._ZTS3foo.foo* @a, align 8, !intel_dtrans_type ![[FOO_PTR:[0-9]+]]
// CHECK-LINUX: @c = global %class._ZTS3foo.foo* @a, align 8, !intel_dtrans_type ![[FOO_PTR]]

// CHECK-LINUX: !intel.dtrans.types = !{![[FOO:[0-9]+]]}

// CHECK-LINUX: ![[FOO_PTR]] = !{%class._ZTS3foo.foo zeroinitializer, i32 1}
// CHECK-LINUX: ![[FOO]] = !{!"S", %class._ZTS3foo.foo zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK-LINUX: ![[CHAR]] = !{i8 0, i32 0}

// CHECK-WINDOWS: @"?a@@3Vfoo@@A" = dso_local global %"class..?AVfoo@@.foo" zeroinitializer, align 1
// CHECK-WINDOWS: @"?b@@3AEAVfoo@@EA" = dso_local constant %"class..?AVfoo@@.foo"* @"?a@@3Vfoo@@A", align 8, !intel_dtrans_type ![[FOO_PTR:[0-9]+]]
// CHECK-WINDOWS: @"?c@@3PEAVfoo@@EA" = dso_local global %"class..?AVfoo@@.foo"* @"?a@@3Vfoo@@A", align 8, !intel_dtrans_type ![[FOO_PTR]]

// CHECK-WINDOWS: !intel.dtrans.types = !{![[FOO:[0-9]+]]}

// CHECK-WINDOWS: ![[FOO_PTR]] = !{%"class..?AVfoo@@.foo" zeroinitializer, i32 1}
// CHECK-WINDOWS: ![[FOO]] = !{!"S", %"class..?AVfoo@@.foo" zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK-WINDOWS: ![[CHAR]] = !{i8 0, i32 0}
