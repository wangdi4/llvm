// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
struct a {
  union {
    int a;
    char *b;
    char **c;
  };
};
char *d[];
struct a a[] = {{.a = 0}};

struct a b[] = {{.b = d}};
struct a c[] = {{.c = d}, {.b = d}};
struct a e[];

// CHECK: @a = global [1 x { { i32, [4 x i8] } }] [{ { i32, [4 x i8] } } { { i32, [4 x i8] } { i32 0, [4 x i8] undef } }], align 8, !intel_dtrans_type ![[A:[0-9]+]]
// PTR: @b = global [1 x %struct._ZTS1a.a] [%struct._ZTS1a.a { %union._ZTSN1aUt_E.anon { i8* bitcast ([1 x i8*]* @d to i8*) } }], align 8, !intel_dtrans_type ![[B:[0-9]+]]
// OPQ: @b = global [1 x %struct._ZTS1a.a] [%struct._ZTS1a.a { %union._ZTSN1aUt_E.anon { ptr @d } }], align 8, !intel_dtrans_type ![[B:[0-9]+]]
// PTR: @c = global <{ { { i8** } }, %struct._ZTS1a.a }> <{ { { i8** } } { { i8** } { i8** getelementptr inbounds ([1 x i8*], [1 x i8*]* @d, i32 0, i32 0) } }, %struct._ZTS1a.a { %union._ZTSN1aUt_E.anon { i8* bitcast ([1 x i8*]* @d to i8*) } } }>, align 16, !intel_dtrans_type ![[C:[0-9]+]]
// OPQ: @c = global [2 x %struct._ZTS1a.a] [%struct._ZTS1a.a { %union._ZTSN1aUt_E.anon { ptr @d } }, %struct._ZTS1a.a { %union._ZTSN1aUt_E.anon { ptr @d } }], align 16, !intel_dtrans_type ![[C:[0-9]+]]
// PTR: @d = global [1 x i8*] zeroinitializer, align 8, !intel_dtrans_type ![[D:[0-9]+]]
// OPQ: @d = global [1 x ptr] zeroinitializer, align 8, !intel_dtrans_type ![[D:[0-9]+]]
// Note: e has the same type as b in this case.
// CHECK: @e = global [1 x %struct._ZTS1a.a] zeroinitializer, align 8, !intel_dtrans_type ![[B:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[ANON_UNION:[0-9]+]], ![[A_STRUCT:[0-9]+]]}

// CHECK: ![[A]] = !{!"A", i32 1, ![[A_LIT_REF:[0-9]+]]}
// CHECK: ![[A_LIT_REF]] = !{![[A_LIT:[0-9]+]], i32 0}
// CHECK: ![[A_LIT]] = !{!"L", i32 1, ![[A_LIT2_REF:[0-9]+]]}
// CHECK: ![[A_LIT2_REF]] = !{![[A_LIT2:[0-9]+]], i32 0}
// CHECK: ![[A_LIT2]] = !{!"L", i32 2, ![[INT:[0-9]+]], ![[CHAR4_ARRAY:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[CHAR4_ARRAY]] = !{!"A", i32 4, ![[CHAR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[B]] = !{!"A", i32 1, ![[A_REF:[0-9]+]]}
// CHECK: ![[A_REF]] = !{%struct._ZTS1a.a zeroinitializer, i32 0}
// PTR: ![[C]] = !{![[C_LIT:[0-9]+]], i32 0}
// PTR: ![[C_LIT]] = !{!"L", i32 2, ![[C_LIT2:[0-9]+]], ![[A_REF]]}
// PTR: ![[C_LIT2]] = !{![[C_LIT3:[0-9]+]], i32 0}
// PTR: ![[C_LIT3]] = !{!"L", i32 1, ![[LIT_C_PTR_PTR_REF:[0-9]+]]}
// PTR: ![[LIT_C_PTR_PTR_REF]] = !{![[LIT_CHAR_PTR_PTR:[0-9]+]], i32 0}
// PTR: ![[LIT_CHAR_PTR_PTR]] = !{!"L", i32 1, ![[CHAR_PTR_PTR:[0-9]+]]}
// PTR: ![[CHAR_PTR_PTR]] = !{i8 0, i32 2}
// "C" Changes from a strange 'literal' struct to just an array of two things,
// which appears to be more consistent with the type itself.
// OPQ: ![[C]] = !{!"A", i32 2, ![[A_REF]]}
// CHECK: ![[D]] = !{!"A", i32 1, ![[CHAR_PTR:[0-9]+]]}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}
// CHECK: ![[ANON_UNION]] = !{!"S", %union._ZTSN1aUt_E.anon zeroinitializer, i32 1, ![[CHAR_PTR]]}
// CHECK: ![[A_STRUCT]] = !{!"S", %struct._ZTS1a.a zeroinitializer, i32 1, ![[ANON_UNION_REF:[0-9]+]]}
// CHECK: ![[ANON_UNION_REF]] = !{%union._ZTSN1aUt_E.anon zeroinitializer, i32 0}

