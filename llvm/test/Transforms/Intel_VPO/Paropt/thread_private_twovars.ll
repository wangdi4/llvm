; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-tpv -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt-tpv' -S %s | FileCheck %s
;
; Make sure that vpo-paropt-tpv does not generate incorrect code
; for a test containing two threadprivate variables.
;
; Test src:
;
; #include <stdio.h>
;
; int i, j;
; #pragma omp threadprivate(i,j)
;
; int main() {
; #pragma omp parallel num_threads(1)
;   printf("i = %d, j = %d\n", i, j);
;
;   return 0;
; }

; CHECK: @__tpv_ptr_i = internal global ptr null, align 64
; CHECK: @__tpv_ptr_j = internal global ptr null, align 64
; CHECK: [[TPV_CALL_I:%[^ ]+]] = call ptr @__kmpc_threadprivate_cached({{.*}}, ptr @i, {{.*}}@__tpv_ptr_i)
; CHECK: store ptr [[TPV_CALL_I]], ptr [[TPV_ALLOCA_I:%[^ ]+]]
; CHECK: [[TPV_LOAD_I:%[^ ]+]] = load ptr, ptr [[TPV_ALLOCA_I]]

; CHECK: [[TPV_CALL_J:%[^ ]+]] = call ptr @__kmpc_threadprivate_cached({{.*}}, ptr @j, {{.*}}@__tpv_ptr_j)
; CHECK: store ptr [[TPV_CALL_J]], ptr [[TPV_ALLOCA_J:%[^ ]+]]
; CHECK: [[TPV_LOAD_J:%[^ ]+]] = load ptr, ptr [[TPV_ALLOCA_J]]

; CHECK: "DIR.OMP.PARALLEL"()
; CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[TPV_LOAD_I]], i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[TPV_LOAD_J]], i32 0, i32 1)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = common dso_local thread_private global i32 0, align 4
@j = common dso_local thread_private global i32 0, align 4
@.str = private unnamed_addr constant [16 x i8] c"i = %d, j = %d\0A\00", align 1

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @j, i32 0, i32 1) ]

  %1 = load i32, ptr @i, align 4
  %2 = load i32, ptr @j, align 4
  %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %1, i32 %2)

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr, ...)
