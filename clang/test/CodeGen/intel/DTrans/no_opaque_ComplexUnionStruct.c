// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

typedef struct {
  unsigned long long a;
} PassedAsi64;

typedef struct {
  PassedAsi64 a;
} AlsoPassedAsi64;

AlsoPassedAsi64 Funci64(PassedAsi64 a, AlsoPassedAsi64 b) { return b; }
// No pointers involved, so no dtrans info.
// CHECK: define dso_local i64 @Funci64(i64 %{{.*}}, i64 %{{.*}})

typedef struct {
  unsigned long long a;
  unsigned long long b;
} PassedAs2i64;

typedef struct {
  PassedAs2i64 a;
} AlsoPassedAs2i64;

AlsoPassedAs2i64 Func2i64(PassedAs2i64 a, AlsoPassedAs2i64 b) { return b; }
// CHECK: define dso_local { i64, i64 }  @Func2i64(i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}})

typedef struct {
  unsigned long long a;
  int b;
} PassedAsi64i32;

typedef struct {
  PassedAsi64i32 a;
} AlsoPassedAsi64i32;

AlsoPassedAsi64i32 Funci64i32(PassedAsi64i32 a, AlsoPassedAsi64i32 b) { return b; }
// CHECK: define dso_local { i64, i32 } @Funci64i32(i64 %{{.*}}, i32 %{{.*}}, i64 %{{.*}}, i32 %{{.*}})

typedef struct {
  AlsoPassedAsi64i32 a;
} AlsoAlsoPassedAsi64i32;

typedef union {
  AlsoAlsoPassedAsi64i32 a;
} AlsoAlsoPassedAsi64i32Union;

AlsoAlsoPassedAsi64i32Union FuncAlsoi64i32(AlsoAlsoPassedAsi64i32 a, AlsoAlsoPassedAsi64i32Union b) { return b; }
// CHECK: define dso_local { i64, i32 } @FuncAlsoi64i32(i64 %{{.*}}, i32 %{{.*}}, i64 %{{.*}}, i32 %{{.*}})

typedef struct {
  unsigned long long *a;
  float b;
} PassedAsPtrfloat;

typedef struct {
  PassedAsPtrfloat a;
} AlsoPassedAsPtrfloat;

AlsoPassedAsPtrfloat FuncPtri64(PassedAsPtrfloat a, AlsoPassedAsPtrfloat b) { return b; }
// PTR: define dso_local "intel_dtrans_func_index"="1" { i64*, float } @FuncPtri64(i64* "intel_dtrans_func_index"="2" %{{.*}}, float %{{.*}}, i64* "intel_dtrans_func_index"="3" %{{.*}}, float %{{.*}}) {{.*}}!intel.dtrans.func.type ![[FuncPtri64_FUNC_MD:[0-9]+]]
// OPQ: define dso_local "intel_dtrans_func_index"="1" { ptr, float } @FuncPtri64(ptr "intel_dtrans_func_index"="2" %{{.*}}, float %{{.*}}, ptr "intel_dtrans_func_index"="3" %{{.*}}, float %{{.*}}) {{.*}}!intel.dtrans.func.type ![[FuncPtri64_FUNC_MD:[0-9]+]]

typedef struct {
  int *a;
} Pointer;

typedef struct {
  Pointer a;
  float *b;
} PointerPointer;

typedef struct {
  PointerPointer p;
} HasPointerPointer;

HasPointerPointer FuncPP(PointerPointer a, HasPointerPointer b) { return b; }
// PTR: define dso_local "intel_dtrans_func_index"="1" { i32*, float* } @FuncPP(i32* "intel_dtrans_func_index"="2" %{{.*}}, float* "intel_dtrans_func_index"="3" %{{.*}}, i32* "intel_dtrans_func_index"="4" %{{.*}}, float* "intel_dtrans_func_index"="5"  %{{.*}}) {{.*}}!intel.dtrans.func.type ![[FUNCPP_FUNC_MD:[0-9]+]]
// OPQ: define dso_local "intel_dtrans_func_index"="1" { ptr, ptr } @FuncPP(ptr "intel_dtrans_func_index"="2" %{{.*}}, ptr "intel_dtrans_func_index"="3" %{{.*}}, ptr "intel_dtrans_func_index"="4" %{{.*}}, ptr "intel_dtrans_func_index"="5"  %{{.*}}) {{.*}}!intel.dtrans.func.type ![[FUNCPP_FUNC_MD:[0-9]+]]


// CHECK: !intel.dtrans.types = !{![[AlsoPassedAsi64:[0-9]+]], ![[PassedAsi64:[0-9]+]], ![[AlsoPassedAs2i64:[0-9]+]], ![[PassedAs2i64:[0-9]+]], ![[AlsoPassedAsi64i32:[0-9]+]], ![[PassedAsi64i32:[0-9]+]], ![[AlsoAlsoPassedAsi64i32Union:[0-9]+]], ![[AlsoAlsoPassedAsi64i32:[0-9]+]], ![[AlsoPassedAsPtrfloat:[0-9]+]], ![[PassedAsPtrfloat:[0-9]+]], ![[HasPointerPointer:[0-9]+]], ![[PointerPointer:[0-9]+]], ![[Pointer:[0-9]+]]}
// CHECK: ![[AlsoPassedAsi64]] = !{!"S", %struct._ZTS15AlsoPassedAsi64.AlsoPassedAsi64 zeroinitializer, i32 1, ![[PassedAsi64Ref:[0-9]+]]}
// CHECK: ![[PassedAsi64Ref]] = !{%struct._ZTS11PassedAsi64.PassedAsi64 zeroinitializer, i32 0}
// CHECK: ![[PassedAsi64]] = !{!"S", %struct._ZTS11PassedAsi64.PassedAsi64 zeroinitializer, i32 1, ![[LONGLONG:[0-9]+]]}
// CHECK: ![[LONGLONG]] = !{i64 0, i32 0}
// CHECK: ![[AlsoPassedAs2i64]] = !{!"S", %struct._ZTS16AlsoPassedAs2i64.AlsoPassedAs2i64 zeroinitializer, i32 1, ![[PassedAs2i64Ref:[0-9]+]]
// CHECK: ![[PassedAs2i64Ref]] = !{%struct._ZTS12PassedAs2i64.PassedAs2i64 zeroinitializer, i32 0}
// CHECK: ![[PassedAs2i64]] = !{!"S", %struct._ZTS12PassedAs2i64.PassedAs2i64 zeroinitializer, i32 2, ![[LONGLONG]], ![[LONGLONG]]}
// CHECK: ![[AlsoPassedAsi64i32]] = !{!"S", %struct._ZTS18AlsoPassedAsi64i32.AlsoPassedAsi64i32 zeroinitializer, i32 1, ![[PassedAsi64i32Ref:[0-9]+]]}
// CHECK: ![[PassedAsi64i32Ref]] = !{%struct._ZTS14PassedAsi64i32.PassedAsi64i32 zeroinitializer, i32 0}
// CHECK: ![[PassedAsi64i32]] = !{!"S", %struct._ZTS14PassedAsi64i32.PassedAsi64i32 zeroinitializer, i32 2, ![[LONGLONG]], ![[INT:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[AlsoAlsoPassedAsi64i32Union]] = !{!"S", %union._ZTS27AlsoAlsoPassedAsi64i32Union.AlsoAlsoPassedAsi64i32Union zeroinitializer, i32 1, ![[AlsoAlsoPassedAsi64i32Ref:[0-9]+]]}
// CHECK: ![[AlsoAlsoPassedAsi64i32Ref]] = !{%struct._ZTS22AlsoAlsoPassedAsi64i32.AlsoAlsoPassedAsi64i32 zeroinitializer, i32 0}
// CHECK: ![[AlsoAlsoPassedAsi64i32]] = !{!"S", %struct._ZTS22AlsoAlsoPassedAsi64i32.AlsoAlsoPassedAsi64i32 zeroinitializer, i32 1, ![[AlsoPassedAsi64i32Ref:[0-9]+]]}
// CHECK: ![[AlsoPassedAsi64i32Ref]] = !{%struct._ZTS18AlsoPassedAsi64i32.AlsoPassedAsi64i32 zeroinitializer, i32 0}
// CHECK: ![[AlsoPassedAsPtrfloat]] = !{!"S", %struct._ZTS20AlsoPassedAsPtrfloat.AlsoPassedAsPtrfloat zeroinitializer, i32 1, ![[PassedAsPtrFloatRef:[0-9]+]]}
// CHECK: ![[PassedAsPtrFloatRef]] = !{%struct._ZTS16PassedAsPtrfloat.PassedAsPtrfloat zeroinitializer, i32 0}
// CHECK: ![[PassedAsPtrfloat]] = !{!"S", %struct._ZTS16PassedAsPtrfloat.PassedAsPtrfloat zeroinitializer, i32 2, ![[LONGLONGPTR:[0-9]+]], ![[FLOAT:[0-9]+]]}
// CHECK: ![[LONGLONGPTR]] = !{i64 0, i32 1}
// CHECK: ![[FLOAT]] = !{float 0.00{{.+}}, i32 0}
// CHECK: ![[HasPointerPointer]] = !{!"S", %struct._ZTS17HasPointerPointer.HasPointerPointer zeroinitializer, i32 1, ![[PointerPointerRef:[0-9]+]]}
// CHECK: ![[PointerPointerRef]] = !{%struct._ZTS14PointerPointer.PointerPointer zeroinitializer, i32 0}
// CHECK: ![[PointerPointer]] = !{!"S", %struct._ZTS14PointerPointer.PointerPointer zeroinitializer, i32 2, ![[PointerRef:[0-9]+]], ![[FLOAT_PTR:[0-9]+]]}
// CHECK: ![[PointerRef]] = !{%struct._ZTS7Pointer.Pointer zeroinitializer, i32 0}
// CHECK: ![[FLOAT_PTR]] = !{float 0.00{{.+}}, i32 1}
// CHECK: ![[Pointer]] = !{!"S", %struct._ZTS7Pointer.Pointer zeroinitializer, i32 1, ![[INT_PTR:[0-9]+]]}
// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}

// Function Infos.
// CHECK: ![[FuncPtri64_FUNC_MD]] = distinct !{![[FuncPtri64Ret_Literal:[0-9]+]], ![[LONGLONGPTR]], ![[LONGLONGPTR]]}
// CHECK: ![[FuncPtri64Ret_Literal]] = !{!"L", i32 2, ![[LONGLONGPTR]], ![[FLOAT]]}

// CHECK: ![[FUNCPP_FUNC_MD]] = distinct !{![[FUNCPP_RET_LITERAL:[0-9]+]], ![[INT_PTR]], ![[FLOAT_PTR]], ![[INT_PTR]], ![[FLOAT_PTR]]}
// CHECK: ![[FUNCPP_RET_LITERAL]] = !{!"L", i32 2, ![[INT_PTR]], ![[FLOAT_PTR]]}
