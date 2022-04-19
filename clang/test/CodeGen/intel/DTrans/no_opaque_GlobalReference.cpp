// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK-LINUX,CHECK-LINUX-PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows-msvc -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK-WINDOWS,CHECK-WINDOWS-PTR 

// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK-LINUX,CHECK-LINUX-OPQ
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows-msvc -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK-WINDOWS,CHECK-WINDOWS-OPQ

class foo {
  char x;
};

foo a;
foo &b = a;
foo *c = &a;

// CHECK-LINUX: @a = global %class._ZTS3foo.foo zeroinitializer, align 1
// CHECK-LINUX-PTR: @b = constant %class._ZTS3foo.foo* @a, align 8, !intel_dtrans_type ![[FOO_PTR:[0-9]+]]
// CHECK-LINUX-OPQ: @b = constant ptr @a, align 8, !intel_dtrans_type ![[FOO_PTR:[0-9]+]]
// CHECK-LINUX-PTR: @c = global %class._ZTS3foo.foo* @a, align 8, !intel_dtrans_type ![[FOO_PTR]]
// CHECK-LINUX-OPQ: @c = global ptr @a, align 8, !intel_dtrans_type ![[FOO_PTR]]

// CHECK-LINUX: !intel.dtrans.types = !{![[FOO:[0-9]+]]}

// CHECK-LINUX: ![[FOO_PTR]] = !{%class._ZTS3foo.foo zeroinitializer, i32 1}
// CHECK-LINUX: ![[FOO]] = !{!"S", %class._ZTS3foo.foo zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK-LINUX: ![[CHAR]] = !{i8 0, i32 0}

// CHECK-WINDOWS: @"?a@@3Vfoo@@A" = dso_local global %"class..?AVfoo@@.foo" zeroinitializer, align 1
// CHECK-WINDOWS-PTR: @"?b@@3AEAVfoo@@EA" = dso_local constant %"class..?AVfoo@@.foo"* @"?a@@3Vfoo@@A", align 8, !intel_dtrans_type ![[FOO_PTR:[0-9]+]]
// CHECK-WINDOWS-OPQ: @"?b@@3AEAVfoo@@EA" = dso_local constant ptr @"?a@@3Vfoo@@A", align 8, !intel_dtrans_type ![[FOO_PTR:[0-9]+]]
// CHECK-WINDOWS-PTR: @"?c@@3PEAVfoo@@EA" = dso_local global %"class..?AVfoo@@.foo"* @"?a@@3Vfoo@@A", align 8, !intel_dtrans_type ![[FOO_PTR]]
// CHECK-WINDOWS-OPQ: @"?c@@3PEAVfoo@@EA" = dso_local global ptr @"?a@@3Vfoo@@A", align 8, !intel_dtrans_type ![[FOO_PTR]]

// CHECK-WINDOWS: !intel.dtrans.types = !{![[FOO:[0-9]+]]}

// CHECK-WINDOWS: ![[FOO_PTR]] = !{%"class..?AVfoo@@.foo" zeroinitializer, i32 1}
// CHECK-WINDOWS: ![[FOO]] = !{!"S", %"class..?AVfoo@@.foo" zeroinitializer, i32 1, ![[CHAR:[0-9]+]]}
// CHECK-WINDOWS: ![[CHAR]] = !{i8 0, i32 0}
