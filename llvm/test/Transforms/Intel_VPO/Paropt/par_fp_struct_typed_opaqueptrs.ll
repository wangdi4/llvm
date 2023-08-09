; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %s | FileCheck %s

; Test src:

; typedef struct {
;   long u, v;
;   int w, x;
; } S;
;
; void f() {
;   S a;
;   #pragma omp parallel firstprivate(a)
;     ;
; }

; Make sure that firstprivate initialization of a struct uses memcpy with
; the correct size.

; CHECK: define internal void @f{{.*}}(ptr %tid, ptr %bid, ptr [[A_ORIG:%a]])
; CHECK:   [[A_FP:%a.fpriv]] = alloca %struct.S, align 8
; CHECK:   call void @llvm.memcpy.p0.p0.i64(ptr align 8 [[A_FP]], ptr align 8 [[A_ORIG]], i64 24, i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { i64, i64, i32, i32 }

define dso_local void @f() {
entry:
  %a = alloca %struct.S, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %a, %struct.S zeroinitializer, i32 1) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
