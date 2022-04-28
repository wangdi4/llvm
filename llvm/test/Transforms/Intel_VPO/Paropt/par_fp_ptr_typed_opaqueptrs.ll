; RUN: opt -enable-new-pm=0 -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="vpo-paropt" -S %s | FileCheck %s

; Test src:

; a() {
;   int *b;
; #pragma omp parallel firstprivate(b)
;   ;
; }

; CHECK: define dso_local i32 @a() {
; CHECK:   [[B:%.*]] = alloca ptr, align 8
; CHECK:   [[BVAL:%.*]] = load ptr, ptr [[B]], align 8
; CHECK:   call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr @{{.*}}, i32 1, ptr [[OUTLINED_FN:@[^ ,]+]], ptr [[BVAL]])
;
; CHECK: define internal void [[OUTLINED_FN]](ptr %tid, ptr %bid, ptr [[BVAL_PAS:%.*]])
; CHECK:   [[B_FPRIV:%.*]] = alloca ptr, align 8
; CHECK:   store ptr [[BVAL_PAS]], ptr [[B_FPRIV]], align 8

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @a() #0 {
bb0:
  %retval = alloca i32, align 4
  %b = alloca ptr, align 8
  br label %bb1

bb1:                                              ; preds = %bb0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %b, ptr null, i32 1) ]
  br label %bb2

bb2:                                              ; preds = %bb1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %bb3

bb3:                                              ; preds = %bb2
  %1 = load i32, ptr %retval, align 4
  ret i32 %1
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
