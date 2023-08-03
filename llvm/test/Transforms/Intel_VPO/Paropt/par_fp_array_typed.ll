; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %s | FileCheck %s

; Test src:

; void f() {
;   int a[2][2];
;   #pragma omp parallel firstprivate(a)
;     a;
; }

; Make sure that firstprivate initialization of a typed 2-D array uses memcpy
; with the size of the array, not just one element/dimension.

; CHECK: define internal void @f{{.*}}(ptr %tid, ptr %bid, ptr [[A_ORIG:%a]])
; CHECK:   [[A_FP:%a.fpriv]] = alloca [4 x i32], align 16
; CHECK:   [[A_FP_GEP:%a.fpriv.gep]] = getelementptr inbounds [4 x i32], ptr [[A_FP]], i32 0, i32 0
; CHECK:   call void @llvm.memcpy.p0.p0.i64(ptr align 4 [[A_FP_GEP]], ptr align 4 [[A_ORIG]], i64 16, i1 false)
; CHECK:   %arraydecay = getelementptr inbounds [2 x [2 x i32]], ptr [[A_FP_GEP]], i64 0, i64 0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @f() {
entry:
  %a = alloca [2 x [2 x i32]], align 16

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %a, i32 0, i64 4) ]

  %arraydecay = getelementptr inbounds [2 x [2 x i32]], ptr %a, i64 0, i64 0
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
