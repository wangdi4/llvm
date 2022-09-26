// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
char a[] = {30, 0, 0, 0, 0, 0, 0, 0, 0};
// CHECK: @a = global <{ i8, [8 x i8] }>{{.+}}!intel_dtrans_type ![[A:[0-9]+]]
int *a2[] = {(int *)30, 0, 0, 0, 0, 0, 0, 0, 0};
// PTR: @a2 = global <{ i32*, [8 x i32*] }>{{.+}}!intel_dtrans_type ![[A2:[0-9]+]]
// OPQ: @a2 = global <{ ptr, [8 x ptr] }>{{.+}}!intel_dtrans_type ![[A2:[0-9]+]]
char b[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
// CHECK: @b = global [9 x i8]{{.+}}!intel_dtrans_type ![[B:[0-9]+]]
int *b2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
// PTR: @b2 = global [9 x i32*]{{.+}}!intel_dtrans_type ![[B2:[0-9]+]]
// OPQ: @b2 = global [9 x ptr]{{.+}}!intel_dtrans_type ![[B2:[0-9]+]]

int c[] = {1, 100000, 0, 0, 0, 0, 0, 0, 0, 0};
// CHECK: @c = global <{ i32, i32, [8 x i32] }> {{.+}}!intel_dtrans_type ![[C:[0-9]+]]
int *c2[] = {(int *)1, (int *)100000, 0, 0, 0, 0, 0, 0, 0, 0};
// PTR: @c2 = global <{ i32*, i32*, [8 x i32*] }> {{.+}}!intel_dtrans_type ![[C2:[0-9]+]]
// OPQ: @c2 = global <{ ptr, ptr, [8 x ptr] }> {{.+}}!intel_dtrans_type ![[C2:[0-9]+]]

enum { d };
typedef struct {
    char b[24];
} e;
e f[] = {d, 1.0};
// CHECK: @f = global [1 x { <{ i8, i8, [22 x i8] }> }] [{ <{ i8, i8, [22 x i8] }> } { <{ i8, i8, [22 x i8] }> <{ i8 0, i8 1, [22 x i8] zeroinitializer }> }]{{.+}}!intel_dtrans_type ![[F:[0-9]+]]

char g[][48][1] = {{{}, {}, {}, {}, {}, {}, 5, 10}};
// CHECK: @g = global [1 x <{ [8 x [1 x i8]], [40 x [1 x i8]] }>] [<{ [8 x [1 x i8]], [40 x [1 x i8]] }> <{{.+}}>]{{.+}}!intel_dtrans_type ![[G:[0-9]+]]

char h[] = {0, 1, 1, 1, 1, 1, sizeof(int), 100, 0, 0, 0, 0, 0, 0, 0, 0};
// CHECK: @h = global <{ [8 x i8], [8 x i8] }> <{ [8 x i8] c"\00\01\01\01\01\01\04d", [8 x i8] zeroinitializer }>{{.+}}!intel_dtrans_type ![[H:[0-9]+]]

struct i {
} j[2] = {};
// CHECK: @j = global [2 x %struct._ZTS1i.i] zeroinitializer{{.+}}!intel_dtrans_type ![[J:[0-9]+]]

// CHECK: ![[A]] = !{![[CHAR_LITERAL:[0-9]+]], i32 0}
// CHECK: ![[CHAR_LITERAL]] = !{!"L", i32 2, ![[CHAR:[0-9]+]], ![[CHAR_ARR:[0-9]+]]}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[CHAR_ARR]] = !{!"A", i32 8, ![[CHAR]]}

// CHECK: ![[A2]] = !{![[INT_LITERAL:[0-9]+]], i32 0}
// CHECK: ![[INT_LITERAL]] = !{!"L", i32 2, ![[INT_PTR:[0-9]+]], ![[INT_PTR_ARR:[0-9]+]]}
// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}
// CHECK: ![[INT_PTR_ARR]] = !{!"A", i32 8, ![[INT_PTR]]}

// CHECK: ![[B]] = !{!"A", i32 9, ![[CHAR]]}
// CHECK: ![[B2]] = !{!"A", i32 9, ![[INT_PTR]]}

// CHECK: ![[C]] = !{![[INT_INT_LITERAL:[0-9]+]], i32 0}
// CHECK: ![[INT_INT_LITERAL]] = !{!"L", i32 3, ![[INT:[0-9]+]], ![[INT]], ![[INT_ARR:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[INT_ARR]] = !{!"A", i32 8, ![[INT]]}
// CHECK: ![[C2]] = !{![[C2_LITERAL:[0-9]+]], i32 0}
// CHECK: ![[C2_LITERAL]] = !{!"L", i32 3, ![[INT_PTR]], ![[INT_PTR]], ![[INT_PTR_ARR]]}

// CHECK: ![[F]] = !{!"A", i32 1, ![[F_LITERAL_ARRAY_REF:[0-9]+]]}
// CHECK: ![[F_LITERAL_ARRAY_REF]] = !{![[LITERAL_ARRAY:[0-9]+]], i32 0}
// CHECK: ![[LITERAL_ARRAY]] = !{!"L", i32 1, ![[F_ELEM_LITERAL_REF:[0-9]+]]}
// CHECK: ![[F_ELEM_LITERAL_REF]] = !{![[F_ELEM_LITERAL:[0-9]+]], i32 0}
// CHECK: ![[F_ELEM_LITERAL]] = !{!"L", i32 3, ![[CHAR]], ![[CHAR]], ![[CHAR22_ARRAY:[0-9]+]]}
// CHECK: ![[CHAR22_ARRAY]] = !{!"A", i32 22, ![[CHAR]]}

// CHECK: ![[G]] = !{!"A", i32 1, ![[G_LITERAL_REF:[0-9]+]]}
// CHECK: ![[G_LITERAL_REF]] = !{![[G_LITERAL:[0-9]+]], i32 0}
// CHECK: ![[G_LITERAL]] = !{!"L", i32 2, ![[G_ARRAY_OF_ARRAYS8:[0-9]+]], ![[G_ARRAY_OF_ARRAYS40:[0-9]+]]}
// CHECK: ![[G_ARRAY_OF_ARRAYS8]] = !{!"A", i32 8, ![[G_CHAR_ARRAY:[0-9]+]]}
// CHECK: ![[G_CHAR_ARRAY]] = !{!"A", i32 1, ![[CHAR]]}
// CHECK: ![[G_ARRAY_OF_ARRAYS40]] = !{!"A", i32 40, ![[G_CHAR_ARRAY]]}

// CHECK: ![[H]] = !{![[H_LITERAL:[0-9]+]], i32 0}
// CHECK: ![[H_LITERAL]] = !{!"L", i32 2, ![[CHAR_ARR]], ![[CHAR_ARR]]}

// CHECK: ![[J]] = !{!"A", i32 2, ![[I_REF:[0-9]+]]}
// CHECK: ![[I_REF]] = !{%struct._ZTS1i.i zeroinitializer, i32 0}
