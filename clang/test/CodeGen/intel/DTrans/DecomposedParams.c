// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

typedef struct { void * PTR; } A;
typedef struct { void * PTR; void * PTR2; } B;
typedef struct { void * PTR; void * PTR2; void* Ptr3;} C;
typedef union { void * PTR; unsigned long LL; } D;

// CHECK: define dso_local void @foo(i8* "intel_dtrans_func_index"="1" %{{.+}}, i8* "intel_dtrans_func_index"="2" %{{.+}}, i8* "intel_dtrans_func_index"="3" %{{.+}}, %struct.C* byval(%struct.C) align 8 "intel_dtrans_func_index"="4" %{{.+}}, i8* "intel_dtrans_func_index"="5" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[FOO_MD:[0-9]+]]
void foo(A a, B b, C c, D d){}

// CHECK: intel.dtrans.types = !{![[A:[0-9]+]], ![[B:[0-9]+]], ![[D:[0-9]+]], ![[C:[0-9]+]]}

// CHECK: ![[A]] = !{!"S", %struct.A zeroinitializer, i32 1, ![[VOID_PTR:[0-9]+]]}
// CHECK: ![[VOID_PTR]] = !{i8 0, i32 1}
// CHECK: ![[B]] = !{!"S", %struct.B zeroinitializer, i32 2, ![[VOID_PTR]], ![[VOID_PTR]]}
// CHECK: ![[D]] = !{!"S", %union.D zeroinitializer, i32 1, ![[VOID_PTR]]}
// CHECK: ![[C]] = !{!"S", %struct.C zeroinitializer, i32 3, ![[VOID_PTR]], ![[VOID_PTR]], ![[VOID_PTR]]}
// CHECK: ![[FOO_MD]] = distinct !{![[VOID_PTR]], ![[VOID_PTR]], ![[VOID_PTR]], ![[C_PTR:[0-9]+]], ![[VOID_PTR]]}
// CHECK: ![[C_PTR]] = !{%struct.C zeroinitializer, i32 1}
